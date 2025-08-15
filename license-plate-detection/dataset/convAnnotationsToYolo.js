const {readFileSync, writeFileSync} = require('node:fs')

const data = JSON.parse(readFileSync('./annotations.json'));
const annotations = data['annotations']
const images = data['images']
const categories = data['categories']

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

Object.keys(yoloLabelData).forEach(d => {
    writeFileSync(`labels/${d.replace('.png', '.txt')}`, yoloLabelData[d].join('\n'))
})