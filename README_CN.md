# NDI Python
简体中文 · [English](./README.md)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Python Version](https://img.shields.io/badge/python-3.7+-blue.svg)](https://www.python.org/downloads/)
[![NDI SDK](https://img.shields.io/badge/NDI%20SDK-6.2.0-green.svg)](https://www.newtek.com/ndi/)

**NewTek NDI（网络设备接口）SDK 的高性能 Python 绑定**

NDI Python 使 Python 应用程序能够使用行业标准的 NDI 协议通过 IP 网络发送和接收高质量、低延迟的视频和音频流。

## 🌟 特性

- **🎥 视频流**: 发送和接收高质量视频帧，支持多种格式（UYVY、BGRA、RGBA 等）
- **🎵 音频流**: 处理未压缩（V2）和压缩（V3）音频格式
- **🌐 网络发现**: 自动发现网络上的 NDI 源
- **📊 实时性能**: 针对低延迟流应用进行优化
- **🔄 元数据支持**: 与音频/视频一起发送和接收元数据
- **📈 性能监控**: 内置性能统计和队列监控
- **🛡️ 内存安全**: 自动内存管理，零拷贝 numpy 集成
- **⚡ 线程安全**: 非阻塞操作时释放 GIL
- **🎛️ Tally 支持**: 节目/预览 tally 状态管理
- **🔧 高级错误处理**: 具有重试机制的全面错误处理

## 📋 要求

### 系统要求
- **操作系统**: Windows 10+、macOS 10.12+ 或 Linux（Ubuntu 18.04+）
- **Python**: 3.7 或更高版本
- **架构**: x86_64 或 ARM64（Apple Silicon）

### 依赖项
- **NDI SDK 6.2.0**: 必须从 [NewTek](https://www.newtek.com/ndi/sdk/) 单独安装
- **NumPy**: 用于高效数组操作
- **Python 开发头文件**: 用于构建扩展

### NDI SDK 安装

#### Windows
1. 从 [NewTek 网站](https://www.newtek.com/ndi/sdk/) 下载 NDI SDK
2. 运行安装程序并按照设置向导操作
3. SDK 将安装到 `C:\Program Files\NewTek\NDI 6 SDK\`

#### macOS
1. 下载适用于 macOS 的 NDI SDK
2. 挂载 DMG 并运行安装程序
3. SDK 将安装到 `/Library/NDI SDK for Apple/`

#### Linux
1. 下载 Linux NDI SDK
2. 解压缩存档并运行安装脚本：
```bash
sudo sh install.sh
```
3. 确保 NDI 库在您的库路径中

## 🚀 安装

### 从 PyPI 安装（推荐）
```bash
pip install ndi-python
```

**支持的环境:**
- Windows x64 Python (3.7-3.13)
- macOS x64 (>=10.12), arm64 (>=11.0) Python (3.7-3.13)
- Linux x64, aarch64 Python (3.7-3.13)

### 从源码安装
```bash
# 克隆仓库
git clone --recursive https://github.com/buresu/ndi-python.git
cd ndi-python

# 开发模式安装
pip install -e .

# 或构建 wheel
python setup.py bdist_wheel
pip install dist/*.whl
```

### 构建要求
从源码构建需要：
- CMake 3.17+
- C++17 兼容编译器
- NDI SDK 安装在标准位置

#### 平台特定构建说明

**macOS:**
```bash
export NDI_SDK_PATH="/Library/NDI SDK for Apple"
export DYLD_LIBRARY_PATH="/Library/NDI SDK for Apple/lib/macOS"
python setup.py build
```

**Linux:**
```bash
# 安装 Avahi 用于源发现
sudo apt install avahi-daemon
sudo systemctl enable --now avahi-daemon

# 使用自定义 NDI SDK 路径构建（如有需要）
env CMAKE_ARGS="-DNDI_SDK_DIR=/path/to/ndisdk" python setup.py build
```

**Arch Linux:**
```bash
# 安装 NDI SDK
yay -S ndi-sdk

# 安装 Avahi
sudo pacman -S avahi
sudo systemctl enable --now avahi-daemon
```

## 🎯 快速开始

### 基础接收器
```python
import NDIlib as ndi
import cv2

# 初始化 NDI
if not ndi.initialize():
    exit("NDI 初始化失败")

try:
    # 查找源
    finder = ndi.Finder()
    sources = finder.get_sources()
    
    if not sources:
        exit("未找到 NDI 源")
    
    print(f"找到 {len(sources)} 个源:")
    for i, source in enumerate(sources):
        print(f"  {i}: {source.ndi_name}")
    
    # 连接到第一个源
    receiver = ndi.Receiver(ndi.RecvCreate(sources[0]))
    
    print(f"正在从以下接收: {sources[0].ndi_name}")
    
    # 捕获循环
    while True:
        frame_type, video, audio, metadata = receiver.capture_v2(timeout_ms=5000)
        
        if frame_type == ndi.FRAME_TYPE_VIDEO:
            print(f"视频帧: {video.xres}x{video.yres}")
            
            # 转换为 OpenCV 格式并显示
            frame_bgr = cv2.cvtColor(video.data, cv2.COLOR_BGRA2BGR)
            cv2.imshow('NDI 流', frame_bgr)
            
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
        
        elif frame_type == ndi.FRAME_TYPE_NONE:
            print("超时 - 未接收到帧")

finally:
    ndi.destroy()
    cv2.destroyAllWindows()
```

### 基础发送器
```python
import NDIlib as ndi
import numpy as np
import time

# 初始化 NDI
ndi.initialize()

try:
    # 创建发送器
    sender = ndi.Sender(ndi.SendCreate("Python Sender"))
    
    # 发送测试图案
    for frame_num in range(300):  # 30fps 下 10 秒
        # 创建 1080p BGRA 帧
        height, width = 1080, 1920
        video_frame = ndi.VideoFrameV2(width, height, ndi.FOURCC_VIDEO_TYPE_BGRA)
        
        # 生成测试图案
        frame_data = np.zeros((height, width, 4), dtype=np.uint8)
        
        # 移动渐变
        x_pos = (frame_num * 5) % width
        frame_data[:, :x_pos, 0] = 255  # 蓝色
        frame_data[:, x_pos:, 2] = 255  # 红色
        frame_data[:, :, 3] = 255       # Alpha
        
        video_frame.data = frame_data
        
        # 发送帧
        sender.send_video_v2(video_frame)
        
        print(f"已发送帧 {frame_num}，连接数: {sender.get_no_connections(0)}")
        time.sleep(1/30)  # 30 FPS

finally:
    ndi.destroy()
```

## 📚 示例

### 运行包含的示例
```bash
git clone --recursive https://github.com/buresu/ndi-python.git
cd ndi-python/example
pip install -r requirements.txt

# 发现 NDI 源
python find.py

# 接收视频流
python recv.py

# 发送视频流
python send_video.py
```

### 完整示例

包括以下综合示例：
- 高级视频处理
- 音频流
- 网络监控
- 性能分析
- 自定义处理管道

请参阅 [API 文档](API_DOCUMENTATION.md) 了解详细示例和使用模式。

## 📚 文档

### 完整 API 文档
请参阅 [API_DOCUMENTATION.md](API_DOCUMENTATION.md) 获取全面的 API 参考，包括：

- **安装和设置**: 详细安装说明
- **核心类**: 带参数的完整类参考
- **帧类型**: 视频、音频和元数据帧处理
- **网络发现**: 源发现和监控
- **发送/接收**: 数据传输和捕获
- **错误处理**: 异常处理和安全方法
- **性能监控**: 统计和优化
- **高级示例**: 实际使用模式

### 关键类概述

#### 核心类
- `Source`: 表示 NDI 源
- `Finder`: 发现网络上的 NDI 源
- `Sender`: 发送视频/音频/元数据
- `Receiver`: 接收视频/音频/元数据

#### 帧类
- `VideoFrameV2`: 用于发送的视频帧
- `CapturedVideoFrame`: 接收的视频帧（只读）
- `AudioFrameV2`: 音频帧（float32 格式）
- `AudioFrameV3`: 音频帧（压缩格式）
- `MetadataFrame`: 元数据容器

#### 实用类
- `Tally`: Tally 状态管理
- `RecvPerformance`: 性能统计
- `RecvQueue`: 队列监控
- `CaptureResult`: 增强的捕获结果

## 🛠️ 开发

### 从源码构建
```bash
# 克隆仓库
git clone --recursive https://github.com/buresu/ndi-python.git
cd ndi-python

# 安装开发依赖
pip install -r requirements-dev.txt

# 构建扩展
python setup.py build_ext --inplace

# 运行测试
python -m pytest tests/

# 构建文档
cd docs
make html
```

### 构建选项

**标准构建:**
```bash
python setup.py build
```

**自定义 NDI SDK 路径（Linux）:**
```bash
env CMAKE_ARGS="-DNDI_SDK_DIR=/path/to/ndisdk" python setup.py build
```

**特定 Python 版本:**
```bash
env CMAKE_ARGS="-DPYTHON_EXECUTABLE=/path/to/python3.8 -DPYBIND11_PYTHON_VERSION=3.8" /path/to/python3.8 setup.py build
```

**构建 wheel 包:**
```bash
python setup.py bdist_wheel
```

**仅 CMake 构建:**
```bash
mkdir build && cd build
cmake /path/to/ndi-python
cmake --build . --config Release
```

### 测试
```bash
# 运行基础测试
python test_upgrade.py

# 使用特定 NDI 源测试
python test_recv_get_source_name.py

# 内存管理测试
python test_memory_management.py

# 性能测试
python -m pytest tests/test_performance.py -v
```

### 项目结构
```
ndi-python/
├── src/
│   └── main.cpp              # 主要的 pybind11 绑定
├── NDIlib/
│   └── __init__.py           # Python 包
├── example/
│   ├── find.py               # 源发现示例
│   ├── recv.py               # 接收器示例
│   └── send_video.py         # 发送器示例
├── tests/
│   └── test_*.py             # 测试套件
├── docs/
│   └── *.md                  # 文档
├── CMakeLists.txt            # CMake 构建配置
├── setup.py                 # Python 构建脚本
└── README.md                 # 此文件
```

## 🤝 贡献

我们欢迎贡献！以下是入门方法：

### 设置开发环境
1. Fork 仓库
2. 克隆您的 fork: `git clone https://github.com/yourusername/ndi-python.git`
3. 创建虚拟环境: `python -m venv venv`
4. 激活它: `source venv/bin/activate` (Linux/macOS) 或 `venv\Scripts\activate` (Windows)
5. 开发模式安装: `pip install -e .`

### 进行更改
1. 创建功能分支: `git checkout -b feature-name`
2. 进行更改
3. 为新功能添加测试
4. 运行测试套件: `python -m pytest`
5. 根据需要更新文档
6. 提交更改: `git commit -m "更改描述"`
7. 推送到您的 fork: `git push origin feature-name`
8. 创建 Pull Request

### 代码风格
- Python 代码遵循 PEP 8
- 使用有意义的变量和函数名
- 为公共函数添加文档字符串
- 保持 C++ 代码与现有风格一致

### 报告问题
- 使用 GitHub 问题跟踪器
- 包括 Python 版本、操作系统和 NDI SDK 版本
- 提供重现问题的最小代码
- 包括错误消息和堆栈跟踪

## 📞 支持

### 社区支持
- **GitHub Issues**: [报告错误或请求功能](https://github.com/buresu/ndi-python/issues)
- **讨论**: [社区讨论和问题](https://github.com/buresu/ndi-python/discussions)

### 文档
- **API 参考**: [API_DOCUMENTATION.md](API_DOCUMENTATION.md)
- **示例**: [example/](example/) 目录
- **NDI SDK 文档**: [NDI 开发者门户](https://www.ndi.tv/sdk/)

### 故障排除

#### 常见问题

**"NDI 初始化失败"**
- 确保 NDI SDK 已正确安装
- 检查 NDI 库是否在系统 PATH 中
- 验证 CPU 支持 NDI（使用 `ndi.is_supported_CPU()`）

**"未找到源"**
- 检查网络连接
- 验证防火墙设置允许 NDI 流量
- 确保源在同一网络段上
- 安装并启动 Avahi 守护进程（Linux）

**大帧内存错误**
- 监控系统内存使用
- 及时处理帧以避免积累
- 考虑降低视频分辨率或帧率

**构建错误**
- 验证 NDI SDK 安装路径
- 检查已安装开发工具（CMake、编译器）
- 确保 Python 开发头文件可用

#### 平台特定说明

**Linux 设置:**
```bash
# Ubuntu/Debian
sudo apt install avahi-daemon
sudo systemctl enable --now avahi-daemon

# Arch Linux
sudo pacman -S avahi
sudo systemctl enable --now avahi-daemon
```

**macOS:**
- 可能需要在系统偏好设置中批准网络访问
- 通用二进制文件支持 Intel 和 Apple Silicon
- 开发时可能需要设置 DYLD_LIBRARY_PATH

**Windows:**
- 可能需要 Visual Studio 构建工具
- Windows Defender 可能需要 NDI 流量例外
- 使用 64 位 Python 以获得最佳性能

## 📝 许可证

本项目根据 MIT 许可证授权 - 详细信息请参阅 [LICENSE](LICENSE) 文件。

**注意**: NDI SDK 遵循 NDI 的单独许可条款。

## 🙏 致谢

- **NewTek/Vizrt** 提供 NDI SDK 和协议
- **pybind11** 开发者提供出色的 Python 绑定框架
- **NumPy** 社区提供高效的数组操作
- **贡献者** 帮助改进此项目

## 🚀 路线图

### 即将推出的功能
- [ ] 增强的音频处理功能
- [ ] 录制和回放功能
- [ ] PTZ（平移-倾斜-缩放）摄像头控制
- [ ] 高级元数据处理
- [ ] 性能分析工具
- [ ] Docker 容器示例
- [ ] 集成示例（OBS、Unity 等）

### 版本历史
- **6.2.0**: 当前版本，支持 NDI SDK 6.2.0
- **6.1.x**: Beta 版本和测试
- **6.0.x**: 开发版本

---

**使用 NDI Python 愉快地进行流媒体传输！🎬✨**