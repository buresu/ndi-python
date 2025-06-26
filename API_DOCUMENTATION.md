# NDI Python API Documentation
English · [简体中文](./API_DOCUMENTATION_CN.md)
## Overview

The NDI Python library provides comprehensive Python bindings for the NewTek NDI (Network Device Interface) SDK, enabling high-quality, low-latency video and audio streaming over IP networks.

## Table of Contents

1. [Installation](#installation)
2. [Quick Start](#quick-start)
3. [Core Classes](#core-classes)
4. [Frame Types](#frame-types)
5. [Network Discovery](#network-discovery)
6. [Sending Data](#sending-data)
7. [Receiving Data](#receiving-data)
8. [Error Handling](#error-handling)
9. [Performance Monitoring](#performance-monitoring)
10. [Examples](#examples)

## Installation

```bash
pip install ndi-python
```

## Quick Start

### Basic Receiver Example

```python
import NDIlib as ndi

# Initialize NDI
if not ndi.initialize():
    print("Failed to initialize NDI")
    exit(1)

# Find sources
finder = ndi.Finder()
sources = finder.get_sources()

if sources:
    # Connect to first source
    recv_create = ndi.RecvCreate()
    recv_create.source_to_connect = sources[0]
    
    receiver = ndi.Receiver(recv_create)
    
    # Capture frames
    frame_type, video, audio, metadata = receiver.capture_v2()
    
    if frame_type == ndi.FRAME_TYPE_VIDEO:
        print(f"Received video frame: {video.xres}x{video.yres}")

# Cleanup
ndi.destroy()
```

### Basic Sender Example

```python
import NDIlib as ndi
import numpy as np

# Initialize NDI
ndi.initialize()

# Create sender
send_create = ndi.SendCreate("Python Sender")
sender = ndi.Sender(send_create)

# Create video frame
video_frame = ndi.VideoFrameV2(1920, 1080)

# Create video data (UYVY format)
video_data = np.random.randint(0, 255, (1080, 1920, 2), dtype=np.uint8)
video_frame.data = video_data

# Send frame
sender.send_video_v2(video_frame)

ndi.destroy()
```

## Core Classes

### NDIlib Module Functions

#### `initialize() -> bool`
Initialize the NDI library. Must be called before using any other NDI functions.

**Returns:**
- `bool`: True if initialization succeeded

#### `destroy() -> None`
Destroy the NDI library and cleanup resources. Should be called when finished.

#### `version() -> str`
Get the NDI SDK version string.

**Returns:**
- `str`: Version string of the NDI SDK

#### `is_supported_CPU() -> bool`
Check if the current CPU supports NDI operations.

**Returns:**
- `bool`: True if CPU is supported

### Source Class

Represents an NDI source on the network.

```python
class Source:
    def __init__(self, ndi_name: str = "", url_address: str = "")
```

**Properties:**
- `ndi_name` (str): The NDI source name
- `url_address` (str): The source URL address

**Example:**
```python
source = ndi.Source("My Source (Computer)", "192.168.1.100")
print(f"Source: {source.ndi_name} at {source.url_address}")
```

## Frame Types

### VideoFrameV2 Class

Represents a video frame for sending.

```python
class VideoFrameV2:
    def __init__(self, xres: int = 0, yres: int = 0, 
                 fourcc: int = ndi.FOURCC_VIDEO_TYPE_UYVY,
                 frame_rate_N: int = 30000, frame_rate_D: int = 1001,
                 aspect_ratio: float = 0.0,
                 format: int = ndi.FRAME_FORMAT_TYPE_PROGRESSIVE,
                 timecode: int = 0, timestamp: int = 0)
```

**Properties:**
- `data` (numpy.ndarray): Video data as numpy array
- `metadata` (str): Associated metadata string
- `xres` (int): Horizontal resolution
- `yres` (int): Vertical resolution
- `fourcc` (int): Video format fourCC code
- `frame_rate_N` (int): Frame rate numerator
- `frame_rate_D` (int): Frame rate denominator
- `picture_aspect_ratio` (float): Picture aspect ratio
- `frame_format_type` (int): Progressive/interlaced format
- `timecode` (int): Frame timecode
- `line_stride_in_bytes` (int): Bytes per line
- `timestamp` (int): Frame timestamp

**Supported Video Formats:**
- `FOURCC_VIDEO_TYPE_UYVY`: UYVY 4:2:2 format
- `FOURCC_VIDEO_TYPE_BGRA`: BGRA 32-bit format
- `FOURCC_VIDEO_TYPE_RGBA`: RGBA 32-bit format
- `FOURCC_VIDEO_TYPE_BGRX`: BGRX 32-bit format

**Example:**
```python
# Create 1080p video frame
video_frame = ndi.VideoFrameV2(1920, 1080, ndi.FOURCC_VIDEO_TYPE_BGRA)

# Create BGRA data
video_data = np.random.randint(0, 255, (1080, 1920, 4), dtype=np.uint8)
video_frame.data = video_data
video_frame.metadata = "Frame metadata"
```

### CapturedVideoFrame Class

Represents a captured video frame (read-only).

**Properties (Read-only):**
- `data` (numpy.ndarray): Video data as numpy array
- `metadata` (str): Associated metadata
- `xres` (int): Horizontal resolution
- `yres` (int): Vertical resolution
- `fourcc` (int): Video format
- `frame_rate_N` (int): Frame rate numerator
- `frame_rate_D` (int): Frame rate denominator
- All other VideoFrameV2 properties (read-only)

### AudioFrameV2 Class

Represents an audio frame for sending (floating-point format).

```python
class AudioFrameV2:
    def __init__(self, sample_rate: int = 48000, no_channels: int = 2,
                 no_samples: int = 0, timecode: int = ndi.SEND_TIMECODE_SYNTHESIZE,
                 timestamp: int = 0)
```

**Properties:**
- `data` (numpy.ndarray): Audio data as float32 numpy array
- `metadata` (str): Associated metadata
- `sample_rate` (int): Audio sample rate (Hz)
- `no_channels` (int): Number of audio channels
- `no_samples` (int): Number of samples per channel
- `timecode` (int): Audio timecode
- `timestamp` (int): Audio timestamp
- `channel_stride_in_bytes` (int): Bytes between channels

**Example:**
```python
# Create stereo audio frame at 48kHz
audio_frame = ndi.AudioFrameV2(48000, 2, 1600)  # ~33ms at 48kHz

# Create audio data (sine wave)
audio_data = np.sin(np.linspace(0, 2*np.pi, 1600*2)).astype(np.float32)
audio_data = audio_data.reshape(2, 1600)  # 2 channels, 1600 samples each
audio_frame.data = audio_data
```

### AudioFrameV3 Class

Represents an audio frame for sending (compressed format).

**Properties:**
- Similar to AudioFrameV2 but with:
- `data` (numpy.ndarray): Audio data as uint8 numpy array
- `fourcc` (int): Audio compression format

### MetadataFrame Class

Represents metadata for transmission.

```python
class MetadataFrame:
    def __init__(self, timecode: int = 0, data: str = "")
```

**Properties:**
- `data` (str): Metadata content (typically XML)
- `timecode` (int): Metadata timecode
- `length` (int, read-only): Data length in bytes

**Example:**
```python
metadata = ndi.MetadataFrame()
metadata.data = '<?xml version="1.0"?><camera><zoom>1.5</zoom></camera>'
metadata.timecode = current_timecode
```

## Network Discovery

### FindCreate Class

Configuration for NDI source discovery.

```python
class FindCreate:
    def __init__(self, show_local: bool = True, groups: str = "", extra_ips: str = "")
```

**Properties:**
- `show_local_sources` (bool): Include local sources in discovery
- `groups` (str): Comma-separated list of groups to discover
- `extra_ips` (str): Additional IP addresses to search

### Finder Class

Discovers NDI sources on the network.

```python
class Finder:
    def __init__(self, create_settings: FindCreate)
```

**Methods:**

#### `get_sources() -> list[Source]`
Get currently available NDI sources.

**Returns:**
- `list[Source]`: List of available sources

#### `wait_for_sources(timeout_ms: int) -> None`
Wait for source list changes.

**Parameters:**
- `timeout_ms` (int): Timeout in milliseconds

**Throws:**
- `NDIException`: If operation fails

#### `try_wait_for_sources(timeout_ms: int) -> bool`
Non-throwing version of wait_for_sources.

**Returns:**
- `bool`: True if sources changed within timeout

#### `wait_for_sources_safe(timeout_ms: int) -> tuple[bool, str, str]`
Safe version with error reporting.

**Returns:**
- `tuple`: (success, status, message)

**Example:**
```python
# Discover sources
find_create = ndi.FindCreate(show_local=True)
finder = ndi.Finder(find_create)

# Wait for sources
if finder.try_wait_for_sources(5000):  # 5 second timeout
    sources = finder.get_sources()
    for source in sources:
        print(f"Found: {source.ndi_name}")
```

## Sending Data

### SendCreate Class

Configuration for NDI sender.

```python
class SendCreate:
    def __init__(self, ndi_name: str = "", groups: str = "",
                 clock_video: bool = True, clock_audio: bool = True)
```

**Properties:**
- `ndi_name` (str): Name of the sender
- `groups` (str): Groups to join
- `clock_video` (bool): Enable video clocking
- `clock_audio` (bool): Enable audio clocking

### Sender Class

Sends video, audio, and metadata over NDI.

```python
class Sender:
    def __init__(self, create_settings: SendCreate)
```

**Methods:**

#### `send_video_v2(frame: VideoFrameV2) -> None`
Send a video frame.

**Parameters:**
- `frame` (VideoFrameV2): Video frame to send

#### `send_audio_v2(frame: AudioFrameV2) -> None`
Send an audio frame (V2 format).

**Parameters:**
- `frame` (AudioFrameV2): Audio frame to send

#### `send_audio_v3(frame: AudioFrameV3) -> None`
Send an audio frame (V3 format).

**Parameters:**
- `frame` (AudioFrameV3): Audio frame to send

#### `send_metadata(frame: MetadataFrame) -> None`
Send metadata.

**Parameters:**
- `frame` (MetadataFrame): Metadata to send

#### `capture(timeout_ms: int) -> tuple`
Capture connection metadata.

**Returns:**
- `tuple`: (frame_type, video, audio, metadata)

#### `get_tally(tally: Tally, timeout_ms: int) -> bool`
Get tally information.

**Parameters:**
- `tally` (Tally): Tally object to populate
- `timeout_ms` (int): Timeout in milliseconds

**Returns:**
- `bool`: True if tally information received

#### `get_no_connections(timeout_ms: int) -> int`
Get number of connections.

**Returns:**
- `int`: Number of connections

**Example:**
```python
# Create sender
send_create = ndi.SendCreate("Python Camera")
sender = ndi.Sender(send_create)

# Send video loop
for frame_num in range(100):
    # Create frame
    video_frame = ndi.VideoFrameV2(1920, 1080)
    video_data = create_test_pattern(frame_num)  # Your function
    video_frame.data = video_data
    
    # Send frame
    sender.send_video_v2(video_frame)
    
    time.sleep(1/30)  # 30 FPS
```

## Receiving Data

### RecvCreate Class

Configuration for NDI receiver.

```python
class RecvCreate:
    def __init__(self, source_to_connect: Source = Source(),
                 color_format: int = ndi.RECV_COLOR_FORMAT_BGRX_BGRA,
                 bandwidth: int = ndi.RECV_BANDWIDTH_HIGHEST,
                 allow_video_fields: bool = True,
                 ndi_name: str = "")
```

**Properties:**
- `source_to_connect` (Source): Source to connect to
- `color_format` (int): Preferred color format
- `bandwidth` (int): Bandwidth mode
- `allow_video_fields` (bool): Allow interlaced fields
- `ndi_recv_name` (str): Receiver name

**Color Formats:**
- `RECV_COLOR_FORMAT_BGRX_BGRA`: BGRX/BGRA format
- `RECV_COLOR_FORMAT_UYVY_BGRA`: UYVY/BGRA format
- `RECV_COLOR_FORMAT_FASTEST`: Fastest available
- `RECV_COLOR_FORMAT_BEST`: Best quality available

**Bandwidth Modes:**
- `RECV_BANDWIDTH_METADATA_ONLY`: Metadata only
- `RECV_BANDWIDTH_AUDIO_ONLY`: Audio only
- `RECV_BANDWIDTH_LOWEST`: Lowest bandwidth
- `RECV_BANDWIDTH_HIGHEST`: Highest bandwidth

### Receiver Class

Receives video, audio, and metadata over NDI.

```python
class Receiver:
    def __init__(self, create_settings: RecvCreate)
```

**Methods:**

#### `connect(source: Source) -> None`
Connect to an NDI source.

**Parameters:**
- `source` (Source): Source to connect to

**Throws:**
- `NDIException`: If connection fails

#### `capture_v2(timeout_ms: int = 5000) -> tuple`
Capture frames from connected source.

**Parameters:**
- `timeout_ms` (int): Timeout in milliseconds

**Returns:**
- `tuple`: (frame_type, video, audio, metadata)
  - `frame_type` (int): Type of frame received
  - `video` (CapturedVideoFrame or None): Video frame if available
  - `audio` (CapturedAudioFrameV2 or None): Audio frame if available  
  - `metadata` (dict or None): Metadata if available

#### `capture_v2_safe(timeout_ms: int = 5000, max_retries: int = 2) -> tuple`
Safe capture with retry logic.

**Returns:**
- `tuple`: (success, status, frame_type, video, audio, metadata)

#### `capture_v2_enhanced(timeout_ms: int = 5000) -> CaptureResult`
Enhanced capture returning structured result.

**Returns:**
- `CaptureResult`: Structured capture result

#### `send_metadata(metadata: MetadataFrame) -> None`
Send metadata upstream.

#### `set_tally(tally: Tally) -> None`
Set tally state.

#### `get_performance() -> RecvPerformance`
Get performance statistics.

#### `get_queue() -> RecvQueue`
Get queue information.

**Example:**
```python
# Create receiver
recv_create = ndi.RecvCreate()
recv_create.color_format = ndi.RECV_COLOR_FORMAT_BGRA_BGRX
receiver = ndi.Receiver(recv_create)

# Connect to source
receiver.connect(sources[0])

# Capture loop
while True:
    frame_type, video, audio, metadata = receiver.capture_v2(5000)
    
    if frame_type == ndi.FRAME_TYPE_VIDEO and video:
        print(f"Video: {video.xres}x{video.yres}, Format: {video.fourcc}")
        # Process video frame
        frame_data = video.data  # numpy array
        
    elif frame_type == ndi.FRAME_TYPE_AUDIO and audio:
        print(f"Audio: {audio.no_channels}ch @ {audio.sample_rate}Hz")
        # Process audio frame
        audio_data = audio.data  # numpy array
        
    elif frame_type == ndi.FRAME_TYPE_METADATA and metadata:
        print(f"Metadata: {metadata}")
        
    elif frame_type == ndi.FRAME_TYPE_ERROR:
        print("Capture error occurred")
        break
```

## Error Handling

### NDIException

Custom exception for NDI-specific errors.

```python
try:
    ndi.initialize()
    # NDI operations
except ndi.NDIException as e:
    print(f"NDI Error: {e}")
```

### Safe Methods

Many operations have "safe" variants that return status tuples instead of throwing exceptions:

```python
# Regular method (throws on error)
receiver.connect(source)

# Safe method (returns status)
success, status, message = receiver.connect_safe(source, retry_count=3)
if not success:
    print(f"Connection failed: {message}")
```

### CaptureResult Class

Enhanced result structure for capture operations.

**Properties:**
- `frame_type` (int): Type of frame captured
- `video` (object): Video frame object or None
- `audio` (object): Audio frame object or None
- `metadata` (object): Metadata object or None
- `success` (bool): Whether capture succeeded
- `status_message` (str): Status/error message

**Methods:**
- `has_video()` -> bool: Check if video frame is present
- `has_audio()` -> bool: Check if audio frame is present
- `has_metadata()` -> bool: Check if metadata is present
- `is_timeout()` -> bool: Check if result is timeout
- `is_error()` -> bool: Check if result is error
- `to_tuple()` -> tuple: Convert to legacy tuple format

## Performance Monitoring

### RecvPerformance Class

Performance statistics for receivers.

**Properties (Read-only):**
- `video_frames` (int): Total video frames received
- `audio_frames` (int): Total audio frames received  
- `metadata_frames` (int): Total metadata frames received

### RecvQueue Class

Queue information for receivers.

**Properties (Read-only):**
- `video_frames` (int): Video frames in queue
- `audio_frames` (int): Audio frames in queue
- `metadata_frames` (int): Metadata frames in queue

### Tally Class

Tally state information.

```python
class Tally:
    def __init__(self, on_program: bool = False, on_preview: bool = False)
```

**Properties:**
- `on_program` (bool): Whether source is on program
- `on_preview` (bool): Whether source is on preview

**Example:**
```python
# Monitor performance
performance = receiver.get_performance()
queue = receiver.get_queue()

print(f"Received {performance.video_frames} video frames")
print(f"Queue: {queue.video_frames} video, {queue.audio_frames} audio")

# Set tally
tally = ndi.Tally(on_program=True, on_preview=False)
receiver.set_tally(tally)
```

## Frame Type Constants

```python
# Frame types returned by capture operations
ndi.FRAME_TYPE_NONE           # No frame (timeout)
ndi.FRAME_TYPE_VIDEO          # Video frame
ndi.FRAME_TYPE_AUDIO          # Audio frame  
ndi.FRAME_TYPE_METADATA       # Metadata frame
ndi.FRAME_TYPE_ERROR          # Error occurred
ndi.FRAME_TYPE_STATUS_CHANGE  # Status change
```

## Examples

### Complete Sender Example

```python
import NDIlib as ndi
import numpy as np
import time

def create_test_pattern(frame_num, width=1920, height=1080):
    """Create a test pattern with moving bar"""
    frame = np.zeros((height, width, 4), dtype=np.uint8)
    
    # Create moving vertical bar
    bar_pos = (frame_num * 10) % width
    frame[:, bar_pos:bar_pos+50, :3] = 255  # White bar
    frame[:, :, 3] = 255  # Alpha channel
    
    return frame

def main():
    # Initialize NDI
    if not ndi.initialize():
        print("Failed to initialize NDI")
        return
    
    try:
        # Create sender
        send_create = ndi.SendCreate("Python Test Sender")
        sender = ndi.Sender(send_create)
        
        print("Sending test pattern...")
        
        # Send frames
        for frame_num in range(300):  # 10 seconds at 30fps
            # Create video frame
            video_frame = ndi.VideoFrameV2(1920, 1080, ndi.FOURCC_VIDEO_TYPE_BGRA)
            video_frame.data = create_test_pattern(frame_num)
            
            # Send frame
            sender.send_video_v2(video_frame)
            
            # Check connections
            connections = sender.get_no_connections(0)
            if frame_num % 30 == 0:  # Print every second
                print(f"Frame {frame_num}, Connections: {connections}")
            
            time.sleep(1/30)  # 30 FPS
            
    finally:
        ndi.destroy()

if __name__ == "__main__":
    main()
```

### Complete Receiver Example

```python
import NDIlib as ndi
import cv2
import numpy as np

def main():
    # Initialize NDI
    if not ndi.initialize():
        print("Failed to initialize NDI")
        return
    
    try:
        # Find sources
        find_create = ndi.FindCreate(show_local=True)
        finder = ndi.Finder(find_create)
        
        print("Looking for sources...")
        finder.wait_for_sources(5000)
        sources = finder.get_sources()
        
        if not sources:
            print("No sources found")
            return
        
        print(f"Found {len(sources)} sources:")
        for i, source in enumerate(sources):
            print(f"  {i}: {source.ndi_name}")
        
        # Connect to first source
        recv_create = ndi.RecvCreate()
        recv_create.source_to_connect = sources[0]
        recv_create.color_format = ndi.RECV_COLOR_FORMAT_BGRX_BGRA
        
        receiver = ndi.Receiver(recv_create)
        
        print(f"Connected to: {sources[0].ndi_name}")
        
        # Capture loop
        frame_count = 0
        while frame_count < 1000:  # Capture 1000 frames
            frame_type, video, audio, metadata = receiver.capture_v2(5000)
            
            if frame_type == ndi.FRAME_TYPE_VIDEO and video:
                frame_count += 1
                
                # Get video data as numpy array
                video_data = video.data
                
                # Convert BGRA to BGR for OpenCV
                if video_data.shape[2] == 4:
                    bgr_frame = video_data[:, :, :3]
                else:
                    bgr_frame = video_data
                
                # Display frame info
                if frame_count % 30 == 0:
                    print(f"Frame {frame_count}: {video.xres}x{video.yres}")
                
                # Optional: Save frame or process with OpenCV
                # cv2.imshow('NDI Frame', bgr_frame)
                # cv2.waitKey(1)
                
            elif frame_type == ndi.FRAME_TYPE_AUDIO and audio:
                print(f"Audio: {audio.no_channels}ch @ {audio.sample_rate}Hz")
                
            elif frame_type == ndi.FRAME_TYPE_ERROR:
                print("Capture error")
                break
                
            elif frame_type == ndi.FRAME_TYPE_NONE:
                print("Timeout - no frame received")
        
        # Get performance stats
        performance = receiver.get_performance()
        print(f"Total frames received: {performance.video_frames} video, {performance.audio_frames} audio")
        
    finally:
        ndi.destroy()

if __name__ == "__main__":
    main()
```

### Audio Capture Example

```python
import NDIlib as ndi
import numpy as np
import soundfile as sf

def main():
    ndi.initialize()
    
    try:
        # Setup receiver for audio
        recv_create = ndi.RecvCreate()
        recv_create.bandwidth = ndi.RECV_BANDWIDTH_AUDIO_ONLY  # Audio only
        receiver = ndi.Receiver(recv_create)
        
        # Find and connect to source
        finder = ndi.Finder(ndi.FindCreate())
        finder.wait_for_sources(5000)
        sources = finder.get_sources()
        
        if sources:
            receiver.connect(sources[0])
            
            audio_buffer = []
            sample_rate = None
            
            # Capture 5 seconds of audio
            start_time = time.time()
            while time.time() - start_time < 5.0:
                frame_type, video, audio, metadata = receiver.capture_v2()
                
                if frame_type == ndi.FRAME_TYPE_AUDIO and audio:
                    if sample_rate is None:
                        sample_rate = audio.sample_rate
                        print(f"Audio: {audio.no_channels}ch @ {sample_rate}Hz")
                    
                    # Get audio data
                    audio_data = audio.data  # Shape: (channels, samples)
                    audio_buffer.append(audio_data)
            
            if audio_buffer and sample_rate:
                # Concatenate all audio data
                full_audio = np.concatenate(audio_buffer, axis=1)
                
                # Transpose for soundfile (samples, channels)
                full_audio = full_audio.T
                
                # Save to file
                sf.write('captured_audio.wav', full_audio, sample_rate)
                print("Audio saved to captured_audio.wav")
        
    finally:
        ndi.destroy()

if __name__ == "__main__":
    main()
```

## Best Practices

### Memory Management
- NDI frames are automatically managed - don't manually delete frame data
- Use captured frames promptly as they may be invalidated
- Large video frames can consume significant memory

### Performance
- Use appropriate timeout values for real-time applications
- Consider bandwidth settings based on network capacity
- Monitor queue sizes to detect processing bottlenecks

### Error Handling
- Always wrap NDI operations in try-catch blocks
- Use safe methods for non-critical operations
- Check capture results before processing frame data

### Threading
- NDI operations release the Python GIL where appropriate
- Multiple receivers/senders can run in separate threads
- Be careful with shared frame data between threads

### Network Considerations
- NDI uses multicast for discovery (requires proper network configuration)
- Firewalls may need configuration for NDI traffic
- High-resolution streams require significant bandwidth

## Troubleshooting

### Common Issues

1. **No sources found**: Check network configuration and firewall settings
2. **Capture timeouts**: Verify source is active and network connectivity
3. **Memory errors**: Ensure proper frame lifecycle management
4. **Performance issues**: Check bandwidth settings and system resources

### Debug Information

```python
# Get NDI version info
print(f"NDI Version: {ndi.version()}")
print(f"Runtime Version: {ndi.get_runtime_version()}")
print(f"CPU Supported: {ndi.is_supported_CPU()}")

# Monitor receiver performance
performance = receiver.get_performance()
queue = receiver.get_queue()
print(f"Performance: {performance.video_frames} frames")
print(f"Queue depth: {queue.video_frames} video frames")
```