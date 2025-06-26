# NDI Python
English · [简体中文](./README_CN.md)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Python Version](https://img.shields.io/badge/python-3.7+-blue.svg)](https://www.python.org/downloads/)
[![NDI SDK](https://img.shields.io/badge/NDI%20SDK-6.2.0-green.svg)](https://www.newtek.com/ndi/)

**High-performance Python bindings for NewTek NDI (Network Device Interface) SDK**

NDI Python enables Python applications to send and receive high-quality, low-latency video and audio streams over IP networks using the industry-standard NDI protocol.

## 🌟 Features

- **🎥 Video Streaming**: Send and receive high-quality video frames with support for multiple formats (UYVY, BGRA, RGBA, etc.)
- **🎵 Audio Streaming**: Handle both uncompressed (V2) and compressed (V3) audio formats
- **🌐 Network Discovery**: Automatic discovery of NDI sources on the network
- **📊 Real-time Performance**: Optimized for low-latency streaming applications
- **🔄 Metadata Support**: Send and receive metadata alongside audio/video
- **📈 Performance Monitoring**: Built-in performance statistics and queue monitoring
- **🛡️ Memory Safe**: Automatic memory management with zero-copy numpy integration
- **⚡ Thread Safe**: GIL-release for non-blocking operations
- **🎛️ Tally Support**: Program/preview tally state management
- **🔧 Advanced Error Handling**: Comprehensive error handling with retry mechanisms

## 📋 Requirements

### System Requirements
- **Operating System**: Windows 10+, macOS 10.12+, or Linux (Ubuntu 18.04+)
- **Python**: 3.7 or later
- **Architecture**: x86_64 or ARM64 (Apple Silicon)

### Dependencies
- **NDI SDK 6.2.0**: Must be installed separately from [NewTek](https://www.newtek.com/ndi/sdk/)
- **NumPy**: For efficient array operations
- **Python Development Headers**: For building the extension

### NDI SDK Installation

#### Windows
1. Download NDI SDK from [NewTek's website](https://www.newtek.com/ndi/sdk/)
2. Run the installer and follow the setup wizard
3. SDK will be installed to `C:\Program Files\NewTek\NDI 6 SDK\`

#### macOS
1. Download NDI SDK for macOS
2. Mount the DMG and run the installer
3. SDK will be installed to `/Library/NDI SDK for Apple/`

#### Linux
1. Download the Linux NDI SDK
2. Extract the archive and run the install script:
```bash
sudo sh install.sh
```
3. Ensure the NDI libraries are in your library path

## 🚀 Installation

### From PyPI (Recommended)
```bash
pip install ndi-python
```

**Supported environments:**
- Windows x64 Python (3.7-3.13)
- macOS x64 (>=10.12), arm64 (>=11.0) Python (3.7-3.13)
- Linux x64, aarch64 Python (3.7-3.13)

### From Source
```bash
# Clone the repository
git clone --recursive https://github.com/buresu/ndi-python.git
cd ndi-python

# Install in development mode
pip install -e .

# Or build wheel
python setup.py bdist_wheel
pip install dist/*.whl
```

### Build Requirements
For building from source, you'll need:
- CMake 3.17+
- C++17 compatible compiler
- NDI SDK installed in standard location

#### Platform-specific Build Notes

**macOS:**
```bash
export NDI_SDK_PATH="/Library/NDI SDK for Apple"
export DYLD_LIBRARY_PATH="/Library/NDI SDK for Apple/lib/macOS"
python setup.py build
```

**Linux:**
```bash
# Install Avahi for source discovery
sudo apt install avahi-daemon
sudo systemctl enable --now avahi-daemon

# Build with custom NDI SDK path (if needed)
env CMAKE_ARGS="-DNDI_SDK_DIR=/path/to/ndisdk" python setup.py build
```

**Arch Linux:**
```bash
# Install NDI SDK
yay -S ndi-sdk

# Install Avahi
sudo pacman -S avahi
sudo systemctl enable --now avahi-daemon
```

## 🎯 Quick Start

### Basic Receiver
```python
import NDIlib as ndi
import cv2

# Initialize NDI
if not ndi.initialize():
    exit("Failed to initialize NDI")

try:
    # Find sources
    finder = ndi.Finder()
    sources = finder.get_sources()
    
    if not sources:
        exit("No NDI sources found")
    
    print(f"Found {len(sources)} sources:")
    for i, source in enumerate(sources):
        print(f"  {i}: {source.ndi_name}")
    
    # Connect to first source
    receiver = ndi.Receiver(ndi.RecvCreate(sources[0]))
    
    print(f"Receiving from: {sources[0].ndi_name}")
    
    # Capture loop
    while True:
        frame_type, video, audio, metadata = receiver.capture_v2(timeout_ms=5000)
        
        if frame_type == ndi.FRAME_TYPE_VIDEO:
            print(f"Video frame: {video.xres}x{video.yres}")
            
            # Convert to OpenCV format and display
            frame_bgr = cv2.cvtColor(video.data, cv2.COLOR_BGRA2BGR)
            cv2.imshow('NDI Stream', frame_bgr)
            
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
        
        elif frame_type == ndi.FRAME_TYPE_NONE:
            print("Timeout - no frame received")

finally:
    ndi.destroy()
    cv2.destroyAllWindows()
```

### Basic Sender
```python
import NDIlib as ndi
import numpy as np
import time

# Initialize NDI
ndi.initialize()

try:
    # Create sender
    sender = ndi.Sender(ndi.SendCreate("Python Sender"))
    
    # Send test pattern
    for frame_num in range(300):  # 10 seconds at 30fps
        # Create 1080p BGRA frame
        height, width = 1080, 1920
        video_frame = ndi.VideoFrameV2(width, height, ndi.FOURCC_VIDEO_TYPE_BGRA)
        
        # Generate test pattern
        frame_data = np.zeros((height, width, 4), dtype=np.uint8)
        
        # Moving gradient
        x_pos = (frame_num * 5) % width
        frame_data[:, :x_pos, 0] = 255  # Blue
        frame_data[:, x_pos:, 2] = 255  # Red
        frame_data[:, :, 3] = 255       # Alpha
        
        video_frame.data = frame_data
        
        # Send frame
        sender.send_video_v2(video_frame)
        
        print(f"Sent frame {frame_num}, connections: {sender.get_no_connections(0)}")
        time.sleep(1/30)  # 30 FPS

finally:
    ndi.destroy()
```

## 📚 Examples

### Run Included Examples
```bash
git clone --recursive https://github.com/buresu/ndi-python.git
cd ndi-python/example
pip install -r requirements.txt

# Discover NDI sources
python find.py

# Receive video stream
python recv.py

# Send video stream
python send_video.py
```

### Complete Examples

For comprehensive examples including:
- Advanced video processing
- Audio streaming
- Network monitoring
- Performance analysis
- Custom processing pipelines

See the [API Documentation](API_DOCUMENTATION.md) for detailed examples and usage patterns.

## 📚 Documentation

### Complete API Documentation
See [API_DOCUMENTATION.md](API_DOCUMENTATION.md) for comprehensive API reference including:

- **Installation & Setup**: Detailed installation instructions
- **Core Classes**: Complete class reference with parameters
- **Frame Types**: Video, audio, and metadata frame handling
- **Network Discovery**: Source discovery and monitoring
- **Sending/Receiving**: Data transmission and capture
- **Error Handling**: Exception handling and safe methods
- **Performance Monitoring**: Statistics and optimization
- **Advanced Examples**: Real-world usage patterns

### Key Classes Overview

#### Core Classes
- `Source`: Represents an NDI source
- `Finder`: Discovers NDI sources on network
- `Sender`: Sends video/audio/metadata
- `Receiver`: Receives video/audio/metadata

#### Frame Classes
- `VideoFrameV2`: Video frame for sending
- `CapturedVideoFrame`: Received video frame (read-only)
- `AudioFrameV2`: Audio frame (float32 format)
- `AudioFrameV3`: Audio frame (compressed format)
- `MetadataFrame`: Metadata container

#### Utility Classes
- `Tally`: Tally state management
- `RecvPerformance`: Performance statistics
- `RecvQueue`: Queue monitoring
- `CaptureResult`: Enhanced capture results

## 🛠️ Development

### Building from Source
```bash
# Clone repository
git clone --recursive https://github.com/buresu/ndi-python.git
cd ndi-python

# Install development dependencies
pip install -r requirements-dev.txt

# Build extension
python setup.py build_ext --inplace

# Run tests
python -m pytest tests/

# Build documentation
cd docs
make html
```

### Build Options

**Standard build:**
```bash
python setup.py build
```

**Custom NDI SDK path (Linux):**
```bash
env CMAKE_ARGS="-DNDI_SDK_DIR=/path/to/ndisdk" python setup.py build
```

**Specific Python version:**
```bash
env CMAKE_ARGS="-DPYTHON_EXECUTABLE=/path/to/python3.8 -DPYBIND11_PYTHON_VERSION=3.8" /path/to/python3.8 setup.py build
```

**Build wheel package:**
```bash
python setup.py bdist_wheel
```

**CMake only build:**
```bash
mkdir build && cd build
cmake /path/to/ndi-python
cmake --build . --config Release
```

### Testing
```bash
# Run basic tests
python test_upgrade.py

# Test with specific NDI source
python test_recv_get_source_name.py

# Memory management tests
python test_memory_management.py

# Performance tests
python -m pytest tests/test_performance.py -v
```

### Project Structure
```
ndi-python/
├── src/
│   └── main.cpp              # Main pybind11 bindings
├── NDIlib/
│   └── __init__.py           # Python package
├── example/
│   ├── find.py               # Source discovery example
│   ├── recv.py               # Receiver example
│   └── send_video.py         # Sender example
├── tests/
│   └── test_*.py             # Test suite
├── docs/
│   └── *.md                  # Documentation
├── CMakeLists.txt            # CMake build configuration
├── setup.py                 # Python build script
└── README.md                 # This file
```

## 🤝 Contributing

We welcome contributions! Here's how to get started:

### Setting Up Development Environment
1. Fork the repository
2. Clone your fork: `git clone https://github.com/yourusername/ndi-python.git`
3. Create a virtual environment: `python -m venv venv`
4. Activate it: `source venv/bin/activate` (Linux/macOS) or `venv\Scripts\activate` (Windows)
5. Install in development mode: `pip install -e .`

### Making Changes
1. Create a feature branch: `git checkout -b feature-name`
2. Make your changes
3. Add tests for new functionality
4. Run the test suite: `python -m pytest`
5. Update documentation if needed
6. Commit your changes: `git commit -m "Description of changes"`
7. Push to your fork: `git push origin feature-name`
8. Create a Pull Request

### Code Style
- Follow PEP 8 for Python code
- Use meaningful variable and function names
- Add docstrings to public functions
- Keep C++ code consistent with existing style

### Reporting Issues
- Use the GitHub issue tracker
- Include Python version, OS, and NDI SDK version
- Provide minimal code to reproduce the issue
- Include error messages and stack traces

## 📞 Support

### Community Support
- **GitHub Issues**: [Report bugs or request features](https://github.com/buresu/ndi-python/issues)
- **Discussions**: [Community discussions and questions](https://github.com/buresu/ndi-python/discussions)

### Documentation
- **API Reference**: [API_DOCUMENTATION.md](API_DOCUMENTATION.md)
- **Examples**: [example/](example/) directory
- **NDI SDK Documentation**: [NDI Developer Portal](https://www.ndi.tv/sdk/)

### Troubleshooting

#### Common Issues

**"Failed to initialize NDI"**
- Ensure NDI SDK is properly installed
- Check that NDI libraries are in system PATH
- Verify CPU supports NDI (use `ndi.is_supported_CPU()`)

**"No sources found"**
- Check network connectivity
- Verify firewall settings allow NDI traffic
- Ensure sources are on the same network segment
- Install and start Avahi daemon (Linux)

**Memory errors with large frames**
- Monitor system memory usage
- Process frames promptly to avoid accumulation
- Consider reducing video resolution or frame rate

**Build errors**
- Verify NDI SDK installation path
- Check that development tools are installed (CMake, compiler)
- Ensure Python development headers are available

#### Platform-Specific Notes

**Linux Setup:**
```bash
# Ubuntu/Debian
sudo apt install avahi-daemon
sudo systemctl enable --now avahi-daemon

# Arch Linux
sudo pacman -S avahi
sudo systemctl enable --now avahi-daemon
```

**macOS:**
- May need to approve network access in System Preferences
- Universal binaries support both Intel and Apple Silicon
- DYLD_LIBRARY_PATH may need to be set for development

**Windows:**
- Visual Studio Build Tools may be required
- Windows Defender may need NDI traffic exceptions
- Use 64-bit Python for best performance

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

**Note**: NDI SDK follows NDI's separate license terms.

## 🙏 Acknowledgments

- **NewTek/Vizrt** for the NDI SDK and protocol
- **pybind11** developers for the excellent Python binding framework
- **NumPy** community for efficient array operations
- **Contributors** who have helped improve this project

## 🚀 Roadmap

### Upcoming Features
- [ ] Enhanced audio processing capabilities
- [ ] Recording and playback functionality
- [ ] PTZ (Pan-Tilt-Zoom) camera control
- [ ] Advanced metadata handling
- [ ] Performance profiling tools
- [ ] Docker container examples
- [ ] Integration examples (OBS, Unity, etc.)

### Version History
- **6.2.0**: Current release with NDI SDK 6.2.0 support
- **6.1.x**: Beta releases and testing
- **6.0.x**: Development versions

---

**Happy streaming with NDI Python! 🎬✨**