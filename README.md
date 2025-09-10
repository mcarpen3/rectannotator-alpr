# rectannotator-alpr
* Starts as a Pure-C application
  * Get vcpkg
    * Clone the vcpkg repo and bootstrap it
       - `git clone https://github.com/microsoft/vcpkg.git`
       - `cd vcpkg; .\bootstrap-vcpkg.bat`
       - `$env:VCPKG_ROOT=<PATH_TO_VCPKG>`
       - `$env:PATH=$PATH;$VCPKG_ROOT`
  * Change the CMakeUserPresets to match your vcpkg path.
  * Build with cmake in the root directory
    - Get the Visual C++ toolset with Visual Studio Community 2022
    - Download the Visual Studio installer from Microsoft, then run it and select the Visual C++ development toolset
    - After installation open the Visual Studio Developer Command prompt
      - This command prompt is included as part of the Visual C++ development toolset
    - Go into this directory and run `cmake --preset=default` then `cmake --build build`
      - This will build the raylib-based graphical app to annotate images
      - Call the built app `build\RectAnnotatorAlpr.exe <path-to-images-to-annotate>`
## After generating the annotations using the raylib app...
  - Use the YOLO model in the license-plate-detection folder to train for license plate detection
    - See some of the example scripts for training using Python3
    - See the below pip installation of deps
## install pytorch for training with GPU
* determine the nvidia driver version
`nvidia-smi`
```
Output: CUDA Version: 12.1
or 
Output: CUDA Version: 11.8
```
* install the correct version `cu121` or `cu118` depending on the nvidia driver above
`pip install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cu121`

## build the rectannotator
`cmake --build build`

## generate keyframe jpgs
  - Generate an image for every 4th keyframe in the input-mp4
`ffmpeg -i <input-mp4> -vf "select='eq(pict_type\,I)*not(mod(n\,4))',showinfo" -vsync vfr <path-to-pngs>/frame_%04d.png`

## train the yolo model on a dataset
`yolo detect train data=<path-to-dataset.yaml> model=yolov8n.pt epochs=50 imgsz=640`
  - example yaml file
```
path: dataset
train: images
val: images
labels: labels

names:
    0: license_plate
```
  - This will prompt YOLO to look in the folder dataset for an images folder and a labels folder.
  * The yolov8n.pt is the base YOLOv8n model provided by ultralytics package.
    * To train the model for-instance on a second set of data without losing the original training:
      * Instead of pointing the `yolo detect train...` command to the base model `yolov8n.pt` point it to the best.pt model inside the latest `predict*/train*/weights*` folder
      * The predict*/train*/weights* folders contain the model snapshot from the previous run.

