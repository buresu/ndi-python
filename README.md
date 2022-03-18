# ndi-python
NewTek NDI Python wrapper

## Setup
### Windows, Mac
Donwload and install NDI SDK.  
https://ndi.tv/sdk/

### Ubuntu
Donwload and install NDI SDK.  
https://ndi.tv/sdk/
```
sudo apt install avahi-daemon
sudo systemctl enable --now avahi-daemon
```

### Arch Linux
```
yay -S avahi ndi-sdk
sudo systemctl enable --now avahi-daemon
```

## Build
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
Enjoy!  

## License
ndi-python is MIT License  
NDI follows NDI's license
