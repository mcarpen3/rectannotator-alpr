// preprocess.js

const sharp = require("sharp");
const fs = require("fs");
const path = require("path");

// Desired YOLOv8 input size
const targetSize = 640;

async function letterboxAndSave(inputPath, outputPath) {
  const image = sharp(inputPath);
  const metadata = await image.metadata();

  const scale = Math.min(targetSize / metadata.width, targetSize / metadata.height);
  const resizedWidth = Math.round(metadata.width * scale);
  const resizedHeight = Math.round(metadata.height * scale);

  const padX = targetSize - resizedWidth;
  const padY = targetSize - resizedHeight;

  const buffer = await image
    .resize(resizedWidth, resizedHeight)
    .extend({
      top: Math.floor(padY / 2),
      bottom: Math.ceil(padY / 2),
      left: Math.floor(padX / 2),
      right: Math.ceil(padX / 2),
      background: { r: 114, g: 114, b: 114 }
    })
    .toFormat("png")
    .toBuffer();

  fs.writeFileSync(outputPath, buffer);
  console.log(`Saved letterboxed image to ${outputPath}`);
}

(async () => {
  const inputImage = path.resolve("e:/frame_0044.png");
  const outputImage = path.resolve("node_preprocessed.png");
  await letterboxAndSave(inputImage, outputImage);
})();