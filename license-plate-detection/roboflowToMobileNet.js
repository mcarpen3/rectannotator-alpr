const {readFileSync, appendFileSync, existsSync, mkdirSync} = require('node:fs')
const {join} = require('node:path')

if (process.argv.length !== 5) {
    console.error(`ERROR not enough args (${process.argv.length}) required 4.
        USAGE "node roboflowToMobileNet.js <path-to-roboflow-style-annotations.json> <path-to-output-text-file.txt> <image-folder-name>"`)
    return 1;
}
const inFile = process.argv[2]
const outPath = process.argv[3]
const images = process.argv[4]
const data = JSON.parse(readFileSync(inFile));
console.log(`INFO found ${data.length} in ${inFile}. Converting to YOLO txt file`)
const maxTextLength = data.reduce((acc, cur) => {
    acc = Math.max(acc, cur.text.length)
    return acc
}, 0)
data.forEach(a => {
    appendFileSync(outPath, `${a.text.padEnd(maxTextLength, '_')} ${images}/${a.filename}\n`)
})
console.log(`INFO done converting ${data.length} from ${inFile} to ${outPath}. ${outPath} saved.`)