# ndi-python
NewTek NDI Python wrapper

## Setup
### Windows, mac, ubuntu
Donwload and install NDI SDK.  
https://ndi.tv/sdk/

### Arch Linux
```
yay -S ndi-sdk
```

## Clone
```
git clone https://github.com/buresu/ndi-python.git --recursive
```

## Build
```
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
Enjoy!  

## License
ndi-python is MIT License  
NDI follows NDI's license