# ndi-python
NewTek NDI Python wrapper

## Setup
### Windows and mac
Donwload and install NDI SDK.  
https://ndi.tv/sdk/

### Arch Linux
```
yay ndi-sdk
```

## Clone and build
```
git clone https://github.com/buresu/ndi-python.git --recursive
cd /path/to/build
cmake /path/to/project
cmake --build /path/to/build --config Release
```
After build copy ndi-python binary and NDI binary to execute directory.  
Enjoy!  

## License
ndi-python is MIT License  
NDI follows NDI's license