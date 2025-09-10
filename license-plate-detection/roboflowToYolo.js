const {readFileSync, writeFileSync, existsSync, mkdirSync} = require('node:fs')
const {join} = require('node:path')

if (process.argv.length !== 4) {
    console.error(`ERROR not enough args (${process.argv.length}) required 4.\nUSAGE node roboflowToYolo.js <path-to-roboflow-style-annotations.json> <path-to-output-text-files>`)
    return 1;
}
const inFile = process.argv[2]
const outPath = process.argv[3]
const data = JSON.parse(readFileSync(inFile));
const annotations = data['annotations']
const images = data['images']
const categories = data['categories']
console.log(`INFO found ${annotations.length} in ${inFile}. Converting to YOLO txt files...`)
const yoloLabelData = annotations.reduce((acc, a) => {
    let image = images.find(i => i['id'] === a['image_id']);
    if (image) {
        const yoloObj = {
            class: a['category_id'],
            xCenter: (a['bbox'][0] + a['bbox'][2] / 2) / image['width'],
            yCenter: (a['bbox'][1] + a['bbox'][3] / 2) / image['height'],
            width: a['bbox'][2] / image['width'],
            height: a['bbox'][3] / image['height'],
        }
        acc[image['file_name']] = acc[image['file_name']] || [] 
        acc[image['file_name']].push(`${yoloObj.class} ${yoloObj.xCenter} ${yoloObj.yCenter} ${yoloObj.width} ${yoloObj.height}`)
    }
    return acc;
}, {})
console.log(`INFO done converting ${annotations.length} from ${inFile} to YOLO txt files. Creating out files`)
if (!existsSync(join(outPath, 'labels'))) {
    try {
        mkdirSync(join(outPath, 'labels'))
    } catch (e) {
        console.error(`ERROR creating output files: ${e.message}`)
        process.exit(1)
    }
}
Object.keys(yoloLabelData).forEach(d => {
    writeFileSync(`${join(outPath, 'labels', d.replace('.png', '.txt'))}`, yoloLabelData[d].join('\n'))
})