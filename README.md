# ndi-python
NewTek NDI Python wrapper

## Installation
ndi-python is support pypi package now. https://pypi.org/project/ndi-python/  
So you can install it using pip if it is a supported environment.  
```
pip install ndi-python
```
The supported environment is as follows.  
- Windows x64 Python(3.7-3.10)
- macOS x64(>=10.12),arm64(>=11.0) Python(3.7-3.10)
- Linux x64,aarch64 Python(3.7-3.10)

## Setup Avahi
Linux requires Avahi to search for NDI sources.  
Please install according to your environment.  

### Ubuntu
```
sudo apt install avahi-daemon
sudo systemctl enable --now avahi-daemon
```

### Arch Linux
```
sudo pacman -S avahi
sudo systemctl enable --now avahi-daemon
```

## Run examples
I've ported some examples to python.  
You can try it by running the ndi-python examples.  
```
git clone --recursive https://github.com/buresu/ndi-python.git
cd ndi-python/example
pip install -r requirements.txt
python find.py
```

## Development

### Install NDI SDK
NDI SDK is required for development.  
You should install according to your environment.  

### Windows, Mac, Ubuntu
Donwload and install NDI SDK.  
https://ndi.tv/sdk/

### Arch Linux
```
yay -S ndi-sdk
```

## Build
### Build with setup.py
```
git clone --recursive https://github.com/buresu/ndi-python.git
cd ndi-python
python setup.py build
```
For ubuntu you need to set the SDK directory in NDI_SDK_DIR as cmake options.  
```
env CMAKE_ARGS="-DNDI_SDK_DIR=/path/to/ndisdk" python setup.py build
```
You can also specify the python version.  
```
env CMAKE_ARGS="-DNDI_SDK_DIR=/path/to/ndisdk -DPYTHON_EXECUTABLE=/path/to/python3.8 -DPYBIND11_PYTHON_VERSION=3.8" /path/to/python3.8 setup.py build
```

### Build for Python package
```
python setup.py bdist_wheel
```

### Build only CMake
```
git clone --recursive https://github.com/buresu/ndi-python.git
cd /path/to/build
cmake /path/to/project
cmake --build /path/to/build --config Release
```

For ubuntu you need to set the SDK directory in NDI_SDK_DIR.  
And build as follows.  
```
cmake /path/to/project -DNDI_SDK_DIR=/path/to/ndisdk
cmake --build /path/to/build --config Release
```
After build copy ndi-python binary and NDI binary to execute directory.  

## License
ndi-python is MIT License  
NDI follows NDI's license
