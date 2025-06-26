# NDI Python API 中文文档
简体中文 · [English](./API_DOCUMENTATION.md)
## 概述

NDI Python 库为 NewTek NDI（网络设备接口）SDK 提供了全面的 Python 绑定，使您能够通过 IP 网络进行高质量、低延迟的视频和音频流传输。

## 目录

1. [安装](#安装)
2. [快速开始](#快速开始)
3. [核心类](#核心类)
4. [帧类型](#帧类型)
5. [网络发现](#网络发现)
6. [发送数据](#发送数据)
7. [接收数据](#接收数据)
8. [错误处理](#错误处理)
9. [性能监控](#性能监控)
10. [示例](#示例)

## 安装

```bash
pip install ndi-python
```

## 快速开始

### 基础接收器示例

```python
import NDIlib as ndi

# 初始化 NDI
if not ndi.initialize():
    print("NDI 初始化失败")
    exit(1)

# 查找源
finder = ndi.Finder()
sources = finder.get_sources()

if sources:
    # 连接到第一个源
    recv_create = ndi.RecvCreate()
    recv_create.source_to_connect = sources[0]
    
    receiver = ndi.Receiver(recv_create)
    
    # 捕获帧
    frame_type, video, audio, metadata = receiver.capture_v2()
    
    if frame_type == ndi.FRAME_TYPE_VIDEO:
        print(f"接收到视频帧: {video.xres}x{video.yres}")

# 清理
ndi.destroy()
```

### 基础发送器示例

```python
import NDIlib as ndi
import numpy as np

# 初始化 NDI
ndi.initialize()

# 创建发送器
send_create = ndi.SendCreate("Python 发送器")
sender = ndi.Sender(send_create)

# 创建视频帧
video_frame = ndi.VideoFrameV2(1920, 1080)

# 创建视频数据（UYVY 格式）
video_data = np.random.randint(0, 255, (1080, 1920, 2), dtype=np.uint8)
video_frame.data = video_data

# 发送帧
sender.send_video_v2(video_frame)

ndi.destroy()
```

## 核心类

### NDIlib 模块函数

#### `initialize() -> bool`
初始化 NDI 库。在使用任何其他 NDI 函数之前必须调用此函数。

**返回值:**
- `bool`: 如果初始化成功则返回 True

#### `destroy() -> None`
销毁 NDI 库并清理资源。完成时应调用此函数。

#### `version() -> str`
获取 NDI SDK 版本字符串。

**返回值:**
- `str`: NDI SDK 的版本字符串

#### `is_supported_CPU() -> bool`
检查当前 CPU 是否支持 NDI 操作。

**返回值:**
- `bool`: 如果支持 CPU 则返回 True

### Source 类

表示网络上的 NDI 源。

```python
class Source:
    def __init__(self, ndi_name: str = "", url_address: str = "")
```

**属性:**
- `ndi_name` (str): NDI 源名称
- `url_address` (str): 源 URL 地址

**示例:**
```python
source = ndi.Source("我的源 (计算机)", "192.168.1.100")
print(f"源: {source.ndi_name} 位于 {source.url_address}")
```

## 帧类型

### VideoFrameV2 类

用于发送的视频帧。

```python
class VideoFrameV2:
    def __init__(self, xres: int = 0, yres: int = 0, 
                 fourcc: int = ndi.FOURCC_VIDEO_TYPE_UYVY,
                 frame_rate_N: int = 30000, frame_rate_D: int = 1001,
                 aspect_ratio: float = 0.0,
                 format: int = ndi.FRAME_FORMAT_TYPE_PROGRESSIVE,
                 timecode: int = 0, timestamp: int = 0)
```

**属性:**
- `data` (numpy.ndarray): 视频数据作为 numpy 数组
- `metadata` (str): 关联的元数据字符串
- `xres` (int): 水平分辨率
- `yres` (int): 垂直分辨率
- `fourcc` (int): 视频格式 fourCC 代码
- `frame_rate_N` (int): 帧率分子
- `frame_rate_D` (int): 帧率分母
- `picture_aspect_ratio` (float): 图像宽高比
- `frame_format_type` (int): 逐行/隔行格式
- `timecode` (int): 帧时间码
- `line_stride_in_bytes` (int): 每行字节数
- `timestamp` (int): 帧时间戳

**支持的视频格式:**
- `FOURCC_VIDEO_TYPE_UYVY`: UYVY 4:2:2 格式
- `FOURCC_VIDEO_TYPE_BGRA`: BGRA 32 位格式
- `FOURCC_VIDEO_TYPE_RGBA`: RGBA 32 位格式
- `FOURCC_VIDEO_TYPE_BGRX`: BGRX 32 位格式

**示例:**
```python
# 创建 1080p 视频帧
video_frame = ndi.VideoFrameV2(1920, 1080, ndi.FOURCC_VIDEO_TYPE_BGRA)

# 创建 BGRA 数据
video_data = np.random.randint(0, 255, (1080, 1920, 4), dtype=np.uint8)
video_frame.data = video_data
video_frame.metadata = "帧元数据"
```

### CapturedVideoFrame 类

表示捕获的视频帧（只读）。

**属性（只读）:**
- `data` (numpy.ndarray): 视频数据作为 numpy 数组
- `metadata` (str): 关联的元数据
- `xres` (int): 水平分辨率
- `yres` (int): 垂直分辨率
- `fourcc` (int): 视频格式
- `frame_rate_N` (int): 帧率分子
- `frame_rate_D` (int): 帧率分母
- 所有其他 VideoFrameV2 属性（只读）

### AudioFrameV2 类

用于发送的音频帧（浮点格式）。

```python
class AudioFrameV2:
    def __init__(self, sample_rate: int = 48000, no_channels: int = 2,
                 no_samples: int = 0, timecode: int = ndi.SEND_TIMECODE_SYNTHESIZE,
                 timestamp: int = 0)
```

**属性:**
- `data` (numpy.ndarray): 音频数据作为 float32 numpy 数组
- `metadata` (str): 关联的元数据
- `sample_rate` (int): 音频采样率（Hz）
- `no_channels` (int): 音频通道数
- `no_samples` (int): 每通道样本数
- `timecode` (int): 音频时间码
- `timestamp` (int): 音频时间戳
- `channel_stride_in_bytes` (int): 通道间字节数

**示例:**
```python
# 创建 48kHz 立体声音频帧
audio_frame = ndi.AudioFrameV2(48000, 2, 1600)  # 48kHz 下约 33ms

# 创建音频数据（正弦波）
audio_data = np.sin(np.linspace(0, 2*np.pi, 1600*2)).astype(np.float32)
audio_data = audio_data.reshape(2, 1600)  # 2 通道，每通道 1600 样本
audio_frame.data = audio_data
```

### AudioFrameV3 类

用于发送的音频帧（压缩格式）。

**属性:**
- 类似于 AudioFrameV2，但有：
- `data` (numpy.ndarray): 音频数据作为 uint8 numpy 数组
- `fourcc` (int): 音频压缩格式

### MetadataFrame 类

用于传输的元数据。

```python
class MetadataFrame:
    def __init__(self, timecode: int = 0, data: str = "")
```

**属性:**
- `data` (str): 元数据内容（通常是 XML）
- `timecode` (int): 元数据时间码
- `length` (int, 只读): 数据长度（字节）

**示例:**
```python
metadata = ndi.MetadataFrame()
metadata.data = '<?xml version="1.0"?><camera><zoom>1.5</zoom></camera>'
metadata.timecode = current_timecode
```

## 网络发现

### FindCreate 类

NDI 源发现的配置。

```python
class FindCreate:
    def __init__(self, show_local: bool = True, groups: str = "", extra_ips: str = "")
```

**属性:**
- `show_local_sources` (bool): 在发现结果中包括本地源
- `groups` (str): 要发现的组的逗号分隔列表
- `extra_ips` (str): 要搜索的其他 IP 地址

### Finder 类

发现网络上的 NDI 源。

```python
class Finder:
    def __init__(self, create_settings: FindCreate)
```

**方法:**

#### `get_sources() -> list[Source]`
获取当前可用的 NDI 源。

**返回值:**
- `list[Source]`: 可用源列表

#### `wait_for_sources(timeout_ms: int) -> None`
等待源列表更改。

**参数:**
- `timeout_ms` (int): 超时时间（毫秒）

**抛出:**
- `NDIException`: 如果操作失败

#### `try_wait_for_sources(timeout_ms: int) -> bool`
wait_for_sources 的非抛出版本。

**返回值:**
- `bool`: 如果在超时内源发生变化则返回 True

#### `wait_for_sources_safe(timeout_ms: int) -> tuple[bool, str, str]`
带错误报告的安全版本。

**返回值:**
- `tuple`: (成功, 状态, 消息)

**示例:**
```python
# 发现源
find_create = ndi.FindCreate(show_local=True)
finder = ndi.Finder(find_create)

# 等待源
if finder.try_wait_for_sources(5000):  # 5 秒超时
    sources = finder.get_sources()
    for source in sources:
        print(f"找到: {source.ndi_name}")
```

## 发送数据

### SendCreate 类

NDI 发送器的配置。

```python
class SendCreate:
    def __init__(self, ndi_name: str = "", groups: str = "",
                 clock_video: bool = True, clock_audio: bool = True)
```

**属性:**
- `ndi_name` (str): 发送器名称
- `groups` (str): 要加入的组
- `clock_video` (bool): 启用视频时钟
- `clock_audio` (bool): 启用音频时钟

### Sender 类

通过 NDI 发送视频、音频和元数据。

```python
class Sender:
    def __init__(self, create_settings: SendCreate)
```

**方法:**

#### `send_video_v2(frame: VideoFrameV2) -> None`
发送视频帧。

**参数:**
- `frame` (VideoFrameV2): 要发送的视频帧

#### `send_audio_v2(frame: AudioFrameV2) -> None`
发送音频帧（V2 格式）。

**参数:**
- `frame` (AudioFrameV2): 要发送的音频帧

#### `send_audio_v3(frame: AudioFrameV3) -> None`
发送音频帧（V3 格式）。

**参数:**
- `frame` (AudioFrameV3): 要发送的音频帧

#### `send_metadata(frame: MetadataFrame) -> None`
发送元数据。

**参数:**
- `frame` (MetadataFrame): 要发送的元数据

#### `capture(timeout_ms: int) -> tuple`
捕获连接元数据。

**返回值:**
- `tuple`: (frame_type, video, audio, metadata)

#### `get_tally(tally: Tally, timeout_ms: int) -> bool`
获取 tally 信息。

**参数:**
- `tally` (Tally): 要填充的 Tally 对象
- `timeout_ms` (int): 超时时间（毫秒）

**返回值:**
- `bool`: 如果收到 tally 信息则返回 True

#### `get_no_connections(timeout_ms: int) -> int`
获取连接数。

**返回值:**
- `int`: 连接数

**示例:**
```python
# 创建发送器
send_create = ndi.SendCreate("Python 摄像头")
sender = ndi.Sender(send_create)

# 发送视频循环
for frame_num in range(100):
    # 创建帧
    video_frame = ndi.VideoFrameV2(1920, 1080)
    video_data = create_test_pattern(frame_num)  # 您的函数
    video_frame.data = video_data
    
    # 发送帧
    sender.send_video_v2(video_frame)
    
    time.sleep(1/30)  # 30 FPS
```

## 接收数据

### RecvCreate 类

NDI 接收器的配置。

```python
class RecvCreate:
    def __init__(self, source_to_connect: Source = Source(),
                 color_format: int = ndi.RECV_COLOR_FORMAT_BGRX_BGRA,
                 bandwidth: int = ndi.RECV_BANDWIDTH_HIGHEST,
                 allow_video_fields: bool = True,
                 ndi_name: str = "")
```

**属性:**
- `source_to_connect` (Source): 要连接的源
- `color_format` (int): 首选颜色格式
- `bandwidth` (int): 带宽模式
- `allow_video_fields` (bool): 允许隔行字段
- `ndi_recv_name` (str): 接收器名称

**颜色格式:**
- `RECV_COLOR_FORMAT_BGRX_BGRA`: BGRX/BGRA 格式
- `RECV_COLOR_FORMAT_UYVY_BGRA`: UYVY/BGRA 格式
- `RECV_COLOR_FORMAT_FASTEST`: 最快可用
- `RECV_COLOR_FORMAT_BEST`: 最佳质量

**带宽模式:**
- `RECV_BANDWIDTH_METADATA_ONLY`: 仅元数据
- `RECV_BANDWIDTH_AUDIO_ONLY`: 仅音频
- `RECV_BANDWIDTH_LOWEST`: 最低带宽
- `RECV_BANDWIDTH_HIGHEST`: 最高带宽

### Receiver 类

通过 NDI 接收视频、音频和元数据。

```python
class Receiver:
    def __init__(self, create_settings: RecvCreate)
```

**方法:**

#### `connect(source: Source) -> None`
连接到 NDI 源。

**参数:**
- `source` (Source): 要连接的源

**抛出:**
- `NDIException`: 如果连接失败

#### `capture_v2(timeout_ms: int = 5000) -> tuple`
从连接的源捕获帧。

**参数:**
- `timeout_ms` (int): 超时时间（毫秒）

**返回值:**
- `tuple`: (frame_type, video, audio, metadata)
  - `frame_type` (int): 接收到的帧类型
  - `video` (CapturedVideoFrame or None): 视频帧（如果可用）
  - `audio` (CapturedAudioFrameV2 or None): 音频帧（如果可用）
  - `metadata` (dict or None): 元数据（如果可用）

#### `capture_v2_safe(timeout_ms: int = 5000, max_retries: int = 2) -> tuple`
带重试逻辑的安全捕获。

**返回值:**
- `tuple`: (success, status, frame_type, video, audio, metadata)

#### `capture_v2_enhanced(timeout_ms: int = 5000) -> CaptureResult`
返回结构化结果的增强捕获。

**返回值:**
- `CaptureResult`: 结构化捕获结果

#### `send_metadata(metadata: MetadataFrame) -> None`
向上游发送元数据。

#### `set_tally(tally: Tally) -> None`
设置 tally 状态。

#### `get_performance() -> RecvPerformance`
获取性能统计信息。

#### `get_queue() -> RecvQueue`
获取队列信息。

**示例:**
```python
# 创建接收器
recv_create = ndi.RecvCreate()
recv_create.color_format = ndi.RECV_COLOR_FORMAT_BGRA_BGRX
receiver = ndi.Receiver(recv_create)

# 连接到源
receiver.connect(sources[0])

# 捕获循环
while True:
    frame_type, video, audio, metadata = receiver.capture_v2(5000)
    
    if frame_type == ndi.FRAME_TYPE_VIDEO and video:
        print(f"视频: {video.xres}x{video.yres}, 格式: {video.fourcc}")
        # 处理视频帧
        frame_data = video.data  # numpy 数组
        
    elif frame_type == ndi.FRAME_TYPE_AUDIO and audio:
        print(f"音频: {audio.no_channels}ch @ {audio.sample_rate}Hz")
        # 处理音频帧
        audio_data = audio.data  # numpy 数组
        
    elif frame_type == ndi.FRAME_TYPE_METADATA and metadata:
        print(f"元数据: {metadata}")
        
    elif frame_type == ndi.FRAME_TYPE_ERROR:
        print("捕获错误发生")
        break
```

## 错误处理

### NDIException

NDI 特定错误的自定义异常。

```python
try:
    ndi.initialize()
    # NDI 操作
except ndi.NDIException as e:
    print(f"NDI 错误: {e}")
```

### 安全方法

许多操作都有"安全"变体，返回状态元组而不是抛出异常：

```python
# 常规方法（错误时抛出异常）
receiver.connect(source)

# 安全方法（返回状态）
success, status, message = receiver.connect_safe(source, retry_count=3)
if not success:
    print(f"连接失败: {message}")
```

### CaptureResult 类

捕获操作的增强结果结构。

**属性:**
- `frame_type` (int): 捕获的帧类型
- `video` (object): 视频帧对象或 None
- `audio` (object): 音频帧对象或 None
- `metadata` (object): 元数据对象或 None
- `success` (bool): 捕获是否成功
- `status_message` (str): 状态/错误消息

**方法:**
- `has_video()` -> bool: 检查是否存在视频帧
- `has_audio()` -> bool: 检查是否存在音频帧
- `has_metadata()` -> bool: 检查是否存在元数据
- `is_timeout()` -> bool: 检查结果是否为超时
- `is_error()` -> bool: 检查结果是否为错误
- `to_tuple()` -> tuple: 转换为传统元组格式

## 性能监控

### RecvPerformance 类

接收器的性能统计信息。

**属性（只读）:**
- `video_frames` (int): 接收的总视频帧数
- `audio_frames` (int): 接收的总音频帧数
- `metadata_frames` (int): 接收的总元数据帧数

### RecvQueue 类

接收器的队列信息。

**属性（只读）:**
- `video_frames` (int): 队列中的视频帧数
- `audio_frames` (int): 队列中的音频帧数
- `metadata_frames` (int): 队列中的元数据帧数

### Tally 类

Tally 状态信息。

```python
class Tally:
    def __init__(self, on_program: bool = False, on_preview: bool = False)
```

**属性:**
- `on_program` (bool): 源是否在节目中
- `on_preview` (bool): 源是否在预览中

**示例:**
```python
# 监控性能
performance = receiver.get_performance()
queue = receiver.get_queue()

print(f"接收了 {performance.video_frames} 个视频帧")
print(f"队列: {queue.video_frames} 视频, {queue.audio_frames} 音频")

# 设置 tally
tally = ndi.Tally(on_program=True, on_preview=False)
receiver.set_tally(tally)
```

## 帧类型常量

```python
# capture 操作返回的帧类型
ndi.FRAME_TYPE_NONE           # 无帧（超时）
ndi.FRAME_TYPE_VIDEO          # 视频帧
ndi.FRAME_TYPE_AUDIO          # 音频帧
ndi.FRAME_TYPE_METADATA       # 元数据帧
ndi.FRAME_TYPE_ERROR          # 发生错误
ndi.FRAME_TYPE_STATUS_CHANGE  # 状态更改
```

## 示例

### 完整发送器示例

```python
import NDIlib as ndi
import numpy as np
import time

def create_test_pattern(frame_num, width=1920, height=1080):
    """创建带移动条的测试图案"""
    frame = np.zeros((height, width, 4), dtype=np.uint8)
    
    # 创建移动的垂直条
    bar_pos = (frame_num * 10) % width
    frame[:, bar_pos:bar_pos+50, :3] = 255  # 白色条
    frame[:, :, 3] = 255  # Alpha 通道
    
    return frame

def main():
    # 初始化 NDI
    if not ndi.initialize():
        print("NDI 初始化失败")
        return
    
    try:
        # 创建发送器
        send_create = ndi.SendCreate("Python 测试发送器")
        sender = ndi.Sender(send_create)
        
        print("正在发送测试图案...")
        
        # 发送帧
        for frame_num in range(300):  # 30fps 下 10 秒
            # 创建视频帧
            video_frame = ndi.VideoFrameV2(1920, 1080, ndi.FOURCC_VIDEO_TYPE_BGRA)
            video_frame.data = create_test_pattern(frame_num)
            
            # 发送帧
            sender.send_video_v2(video_frame)
            
            # 检查连接
            connections = sender.get_no_connections(0)
            if frame_num % 30 == 0:  # 每秒打印一次
                print(f"帧 {frame_num}, 连接数: {connections}")
            
            time.sleep(1/30)  # 30 FPS
            
    finally:
        ndi.destroy()

if __name__ == "__main__":
    main()
```

### 完整接收器示例

```python
import NDIlib as ndi
import cv2
import numpy as np

def main():
    # 初始化 NDI
    if not ndi.initialize():
        print("NDI 初始化失败")
        return
    
    try:
        # 查找源
        find_create = ndi.FindCreate(show_local=True)
        finder = ndi.Finder(find_create)
        
        print("正在查找源...")
        finder.wait_for_sources(5000)
        sources = finder.get_sources()
        
        if not sources:
            print("未找到源")
            return
        
        print(f"找到 {len(sources)} 个源:")
        for i, source in enumerate(sources):
            print(f"  {i}: {source.ndi_name}")
        
        # 连接到第一个源
        recv_create = ndi.RecvCreate()
        recv_create.source_to_connect = sources[0]
        recv_create.color_format = ndi.RECV_COLOR_FORMAT_BGRX_BGRA
        
        receiver = ndi.Receiver(recv_create)
        
        print(f"已连接到: {sources[0].ndi_name}")
        
        # 捕获循环
        frame_count = 0
        while frame_count < 1000:  # 捕获 1000 帧
            frame_type, video, audio, metadata = receiver.capture_v2(5000)
            
            if frame_type == ndi.FRAME_TYPE_VIDEO and video:
                frame_count += 1
                
                # 获取视频数据作为 numpy 数组
                video_data = video.data
                
                # 为 OpenCV 转换 BGRA 到 BGR
                if video_data.shape[2] == 4:
                    bgr_frame = video_data[:, :, :3]
                else:
                    bgr_frame = video_data
                
                # 显示帧信息
                if frame_count % 30 == 0:
                    print(f"帧 {frame_count}: {video.xres}x{video.yres}")
                
                # 可选：保存帧或使用 OpenCV 处理
                # cv2.imshow('NDI 帧', bgr_frame)
                # cv2.waitKey(1)
                
            elif frame_type == ndi.FRAME_TYPE_AUDIO and audio:
                print(f"音频: {audio.no_channels}ch @ {audio.sample_rate}Hz")
                
            elif frame_type == ndi.FRAME_TYPE_ERROR:
                print("捕获错误")
                break
                
            elif frame_type == ndi.FRAME_TYPE_NONE:
                print("超时 - 未接收到帧")
        
        # 获取性能统计
        performance = receiver.get_performance()
        print(f"总共接收帧数: {performance.video_frames} 视频, {performance.audio_frames} 音频")
        
    finally:
        ndi.destroy()

if __name__ == "__main__":
    main()
```

### 音频捕获示例

```python
import NDIlib as ndi
import numpy as np
import soundfile as sf
import time

def main():
    ndi.initialize()
    
    try:
        # 设置仅音频接收器
        recv_create = ndi.RecvCreate()
        recv_create.bandwidth = ndi.RECV_BANDWIDTH_AUDIO_ONLY  # 仅音频
        receiver = ndi.Receiver(recv_create)
        
        # 查找并连接到源
        finder = ndi.Finder(ndi.FindCreate())
        finder.wait_for_sources(5000)
        sources = finder.get_sources()
        
        if sources:
            receiver.connect(sources[0])
            
            audio_buffer = []
            sample_rate = None
            
            # 捕获 5 秒音频
            start_time = time.time()
            while time.time() - start_time < 5.0:
                frame_type, video, audio, metadata = receiver.capture_v2()
                
                if frame_type == ndi.FRAME_TYPE_AUDIO and audio:
                    if sample_rate is None:
                        sample_rate = audio.sample_rate
                        print(f"音频: {audio.no_channels}ch @ {sample_rate}Hz")
                    
                    # 获取音频数据
                    audio_data = audio.data  # 形状: (通道, 样本)
                    audio_buffer.append(audio_data)
            
            if audio_buffer and sample_rate:
                # 连接所有音频数据
                full_audio = np.concatenate(audio_buffer, axis=1)
                
                # 为 soundfile 转置（样本, 通道）
                full_audio = full_audio.T
                
                # 保存到文件
                sf.write('captured_audio.wav', full_audio, sample_rate)
                print("音频已保存到 captured_audio.wav")
        
    finally:
        ndi.destroy()

if __name__ == "__main__":
    main()
```

## 最佳实践

### 内存管理
- NDI 帧会自动管理 - 不要手动删除帧数据
- 及时使用捕获的帧，因为它们可能会失效
- 大型视频帧可能消耗大量内存

### 性能
- 为实时应用使用适当的超时值
- 根据网络容量考虑带宽设置
- 监控队列大小以检测处理瓶颈

### 错误处理
- 始终将 NDI 操作包装在 try-catch 块中
- 对非关键操作使用安全方法
- 在处理帧数据之前检查捕获结果

### 线程
- NDI 操作在适当的地方释放 Python GIL
- 多个接收器/发送器可以在单独的线程中运行
- 注意线程间共享帧数据的安全性

### 网络考虑
- NDI 使用多播进行发现（需要适当的网络配置）
- 防火墙可能需要为 NDI 流量配置
- 高分辨率流需要大量带宽

## 故障排除

### 常见问题

1. **"NDI 初始化失败"**
   - 确保 NDI SDK 已正确安装
   - 检查 NDI 库在系统 PATH 中
   - 验证 CPU 支持 NDI（使用 `ndi.is_supported_CPU()`）

2. **"未找到源"**
   - 检查网络连接
   - 验证防火墙设置允许 NDI 流量
   - 确保源在同一网络段上
   - 检查多播路由（特别是在 Linux 上）

3. **大帧内存错误**
   - 监控系统内存使用
   - 及时处理帧以避免积累
   - 考虑降低视频分辨率或帧率

4. **构建错误**
   - 验证 NDI SDK 安装路径
   - 检查已安装开发工具（CMake、编译器）
   - 确保 Python 开发头文件可用

### 调试信息

```python
# 获取 NDI 版本信息
print(f"NDI 版本: {ndi.version()}")
print(f"运行时版本: {ndi.get_runtime_version()}")
print(f"CPU 支持: {ndi.is_supported_CPU()}")

# 监控接收器性能
performance = receiver.get_performance()
queue = receiver.get_queue()
print(f"性能: {performance.video_frames} 帧")
print(f"队列深度: {queue.video_frames} 视频帧")
```