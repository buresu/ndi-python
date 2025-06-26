# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [6.2.0.3] - 2024-12-26

### Major Updates
- **Upgraded to NDI SDK 6.2.0** - Full compatibility with the latest NDI SDK version
- **Extended Python Support** - Added support for Python 3.11, 3.12, and 3.13
- **Comprehensive Documentation Overhaul** - Complete rewrite of README and API documentation

### Added
- **Enhanced API Documentation** - Complete API reference with detailed examples
- **Comprehensive README** - Extensive documentation with features, installation guides, and examples
- **Quick Start Examples** - Ready-to-use code snippets for common use cases
- **Multi-language Support** - Added Chinese documentation (README_CN.md, API_DOCUMENTATION_CN.md)
- **Advanced Error Handling** - Comprehensive error handling and recovery mechanisms
- **Performance Monitoring** - Built-in performance statistics and queue monitoring
- **Memory Safety Improvements** - Enhanced automatic memory management with zero-copy numpy integration
- **Thread Safety Enhancements** - Improved GIL-release for non-blocking operations
- **Tally Support** - Program/preview tally state management
- **Metadata Handling** - Enhanced metadata transmission and processing
- **Build Configuration** - Added CLAUDE.md for development guidance

### Changed
- **Version Bump** - Updated from 5.1.1.5 to 6.2.0.3 to align with NDI SDK version
- **CMake Configuration** - Updated project version to 6.2.0
- **pybind11 Submodule** - Updated to latest version for improved compatibility
- **Source Code** - Extensive refactoring and enhancement of main.cpp with comprehensive documentation
- **Build System** - Improved cross-platform build support and configuration
- **Installation Instructions** - Updated for better clarity and platform-specific guidance

### Improved
- **Cross-platform Compatibility** - Enhanced support for Windows, macOS (Intel/Apple Silicon), and Linux
- **Architecture Support** - Better support for x64 and ARM64 architectures
- **Build Process** - Streamlined build process with better error reporting
- **Documentation Structure** - Reorganized and expanded documentation for better usability
- **Example Quality** - Enhanced examples with better error handling and documentation

### Technical Enhancements
- **Advanced C++ Bindings** - Comprehensive pybind11 bindings with modern C++ features
- **Memory Management** - Improved frame management with automatic cleanup
- **Network Discovery** - Enhanced source discovery and monitoring capabilities
- **Real-time Performance** - Optimized for low-latency streaming applications
- **Error Recovery** - Robust error handling with retry mechanisms

### Dependencies
- **NDI SDK** - Now requires NDI SDK 6.2.0
- **Python Support** - Extended compatibility to Python 3.7-3.13
- **Build Requirements** - CMake 3.17+, C++17 compatible compiler

### Platform-Specific Improvements
- **Windows** - Enhanced Visual Studio Build Tools compatibility
- **macOS** - Universal binaries for Intel and Apple Silicon
- **Linux** - Improved Avahi integration for source discovery

### Documentation
- **Installation Guides** - Platform-specific installation instructions
- **API Reference** - Complete class and method documentation
- **Examples Collection** - Comprehensive example suite
- **Troubleshooting** - Common issues and solutions guide
- **Development Guide** - Contributing guidelines and development setup

## Previous Versions

### [5.1.1.5] - Previous Release
- Basic NDI Python bindings
- Support for Python 3.7-3.10
- Core video and audio streaming functionality
- Basic documentation

---

**Note**: This changelog reflects the major version update from 5.1.1.5 to 6.2.0.3, representing a significant enhancement of the NDI Python bindings with comprehensive documentation, extended platform support, and alignment with NDI SDK 6.2.0.