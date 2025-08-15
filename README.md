# rectannotator-alpr

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