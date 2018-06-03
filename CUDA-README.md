# My GPUs

- Nvidia 9400M : integrated, compute capability 1.1
- Nvidia 9600M GT : compute capability 1.1

# GPU driver

- DONT TOUCH IT !
- On Mac, driver version is bound to OS version.
- It is updated whenever system is updated.
- Version can be read from "System preferences > CUDA" under "GPU Driver Version".

# CUDA driver

- Middleware between the CUDA API and the GPU driver.
- Can be downloaded and updated manually from [](http://www.nvidia.com/object/mac-driver-archive.html).
- Version can be read from "System preferences > CUDA" under "CUDA Driver Version".
- A CUDA driver has a minimum GPU driver version AND a minimum GPU compute capability.
- Last CUDA driver version to support compute capability 1.1 is 6.5 (see [](https://stackoverflow.com/questions/28932864/cuda-compute-capability-requirements)).

# CUDA toolkit

- Package of tools for CUDA programming : NVCC ...
- CUDA toolkit version must match CUDA driver version.
- Can be downloaded and updated manually from [](https://developer.nvidia.com/cuda-toolkit-archive).
- Version can be read from the CLI $ /usr/local/cuda/bin/nvcc --version

# NVCC

- NVCC is the CUDA compiler from the CUDA toolkit.
- It forwards host code to the host compiler which is bound to be Clang on Mac OSX (no way to use custom g++ via -ccbin option).
- Clang is managed via Xcode but must not be updated above 7.3 for NVCC (see [](https://github.com/arrayfire/arrayfire/issues/1384)).

# Device selection

It appears that CUDA does not detect GPU 9600M GT unless it's already active. Therefore there is no point in using cudaSetDevice, just do CUDA and choose GPU before execution with gfxCardStatus.

# CUDA API cheatsheet

- cudaDeviceSynchronize
- cudaGetDeviceCount
- cudaGetDevice
- cudaGetDeviceProperties
- cudaSetDevice

# Other references

- [](https://devtalk.nvidia.com/default/topic/418143/dyld-library-not-loaded-rpath-libcudart-dylib-/)
- [](http://www.atreid.com/drivers-cuda-mac-os-x-10.7.x-jusqu-a-10.11.5.html)
- [](https://devblogs.nvidia.com/separate-compilation-linking-cuda-device-code/) separate compilation
