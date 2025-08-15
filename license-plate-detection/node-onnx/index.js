const ort = require('onnxruntime-node');
const sharp = require('sharp');
const {basename} = require('node:path')
const fs = require('fs');

const IMG_SIZE = 640;

function iou(boxA, boxB) {
    const x1 = Math.max(boxA.x1, boxB.x1);
    const y1 = Math.max(boxA.y1, boxB.y1);
    const x2 = Math.min(boxA.x2, boxB.x2);
    const y2 = Math.min(boxA.y2, boxB.y2);

    const interArea = Math.max(0, x2 - x1) * Math.max(0, y2 - y1);
    const boxAArea = (boxA.x2 - boxA.x1) * (boxA.y2 - boxA.y1);
    const boxBArea = (boxB.x2 - boxB.x1) * (boxB.y2 - boxB.y1);

    return interArea / (boxAArea + boxBArea - interArea);
}

function nms(boxes, iouThreshold = 0.45) {
    boxes.sort((a, b) => b.score - a.score);
    const keep = [];

    while (boxes.length) {
        const chosen = boxes.shift();
        keep.push(chosen);

        boxes = boxes.filter(box => iou(box, chosen) < iouThreshold);
    }

    return keep;
}

async function preprocessImage(imagePath) {
    // Read and resize image using sharp
    const image = sharp(imagePath)
        .resize({
            width: IMG_SIZE,
            height: IMG_SIZE,
            fit: sharp.fit.contain,
            background: { r: 114, g: 114, b: 114 }
        })
        .removeAlpha()
        .raw();

    const { data, info } = await image.toBuffer({ resolveWithObject: true });

    // Convert HWC to CHW and normalize to [0, 1]
    const chw = new Float32Array(3 * IMG_SIZE * IMG_SIZE);
    for (let i = 0; i < IMG_SIZE * IMG_SIZE; i++) {
        chw[i] = data[i * 3] / 255.0;         // R
        chw[i + IMG_SIZE * IMG_SIZE] = data[i * 3 + 1] / 255.0; // G
        chw[i + 2 * IMG_SIZE * IMG_SIZE] = data[i * 3 + 2] / 255.0; // B
    }

    return chw;
}

async function runYOLO(imagePath) {
    const session = await ort.InferenceSession.create('best.onnx');
    const inputTensor = new ort.Tensor('float32', await preprocessImage(imagePath), [1, 3, IMG_SIZE, IMG_SIZE]);

    const feeds = {};
    feeds[session.inputNames[0]] = inputTensor;

    const results = await session.run(feeds);

    // Assume YOLO output is a single tensor
    const output = results[session.outputNames[0]].data;
    console.log('Output shape:', results[session.outputNames[0]].dims);
    // console.log('First 10 values:', output.slice(0, 10));
    const reshaped = [];
    const detectionCount = results[session.outputNames[0]].dims[2]
    for (let i = 0; i < detectionCount; i++) {
        reshaped.push([
            output[i],
            output[i + detectionCount],
            output[i + 2 * detectionCount],
            output[i + 3 * detectionCount],
            output[i + 4 * detectionCount]
        ])
    }
    const confThreshold = 0.25;
    const filtered = reshaped.filter(d => d[4] > confThreshold);
    const boxes = filtered.map(d => {
        const [x, y, w, h, score] = d;
        return {
            x1: x - w / 2,
            y1: y - h / 2,
            x2: x + w / 2,
            y2: y + h / 2,
            score
        };
    });
    const grouped = boxes.reduce((acc, box) => {
        acc[box.class] = acc[box.class] || [];
        acc[box.class].push(box);
        return acc;
    }, {})
    const keep = Object.keys(grouped).reduce((acc, cls) => {
        acc.push(...nms(grouped[cls]));
        return acc;
    }, [])
    const scaled = keep.map(b => {
        return scaleBoxToOriginal(b, 1920, 1080)
    })
    
    drawBoxesOnImage(imagePath, basename(imagePath).replace('.png', '_detect.png'), scaled, 1920, 1080)
}

function scaleBoxToOriginal(box, origW, origH, modelW = 640, modelH = 640) {
    const r = Math.min(modelW / origW, modelH / origH); // 640 / 1920 = 0.333
    const dw = (modelW - origW * r) / 2; // horizontal padding = 0
    const dh = (modelH - origH * r) / 2; // vertical padding = (640 - 360)/2 = 140

    return {
        x1: (box.x1 - dw) / r,
        y1: (box.y1 - dh) / r,
        x2: (box.x2 - dw) / r,
        y2: (box.y2 - dh) / r,
        score: box.score
    };
}

async function drawBoxesOnImage(inputPath, outputPath, boxes, imageWidth, imageHeight) {
    // Base image
    const base = sharp(inputPath).resize(imageWidth, imageHeight);

    // Create transparent canvas to draw boxes
    const svg = `
        <svg width="${imageWidth}" height="${imageHeight}">
            ${boxes.map(box => {
                const { x1, y1, x2, y2, score } = box;
                const width = x2 - x1;
                const height = y2 - y1;
                const color = 'red';
                return `
                    <rect x="${x1}" y="${y1}" width="${width}" height="${height}"
                          fill="none" stroke="${color}" stroke-width="3"/>
                    <text x="${x1 + 2}" y="${y1 - 4}" font-size="24" fill="${color}">
                        ${score.toFixed(2)}
                    </text>
                `;
            }).join('\n')}
        </svg>
    `;

    const overlay = Buffer.from(svg);

    await base
        .composite([{ input: overlay, blend: 'over' }])
        .toFile(outputPath);

    console.log(`Saved annotated image to ${outputPath}`);
}



runYOLO('e:/frame_0061.png').catch(console.error);
