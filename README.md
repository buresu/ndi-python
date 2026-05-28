# ndi-python
NewTek NDI Python wrapper

## Installation
ndi-python is support pypi package now. https://pypi.org/project/ndi-python/  
So you can install it using pip if it is a supported environment.  
```
pip install ndi-python
```
The supported environment is as follows.  
- Windows x64 Python(3.10-3.14)
- macOS arm64(>=13.0) Python(3.10-3.14)
- Linux x64,aarch64,armv7l Python(3.10-3.14)

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
git clone https://github.com/buresu/ndi-python.git
cd ndi-python/example
pip install -r requirements.txt
python find.py
```

## Capture NDI|HX Source
Capturing NDI|HX sources requires an environment capable of decoding H.264 video.  
On Linux, the NDI SDK dynamically loads FFmpeg libraries internally for H.264 decoding.  
Please install a compatible FFmpeg version that matches the version required by your NDI SDK.  

You can check which FFmpeg version is required with a command similar to the following:  
```
strings /usr/lib/libndi.so | grep libavcodec
out:  libavcodec.so.61
```
In this example, libavcodec.so.61 corresponds to FFmpeg 7.x.

## Development

### Install NDI SDK
NDI SDK is required for development.  
You should install according to your environment.  

### Windows, Mac, Ubuntu
Donwload and install NDI SDK.  
https://ndi.video/for-developers/ndi-sdk/download/

### Arch Linux
```
yay -S ndi-sdk
```

## Build

```
git clone https://github.com/buresu/ndi-python.git
cd ndi-python
pip install .
```

If you need to set the NDI SDK directory explicitly:
```
CMAKE_ARGS="-DNDI_SDK_DIR=/path/to/ndisdk" pip install .
```

To build for a specific Python version:
```
CMAKE_ARGS="-DNDI_SDK_DIR=/path/to/ndisdk" /path/to/python3.12 -m pip install .
```

## License
ndi-python is MIT License  
NDI follows NDI's license
