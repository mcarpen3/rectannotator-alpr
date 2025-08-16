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