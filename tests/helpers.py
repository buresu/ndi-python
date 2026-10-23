import time

import numpy as np
import pytest

import NDIlib as ndi


def make_video_frame(data=None, metadata=None):
    frame = ndi.VideoFrameV2()
    frame.data = np.zeros((90, 160, 4), dtype=np.uint8) if data is None else data
    frame.FourCC = ndi.FOURCC_VIDEO_TYPE_BGRX
    if metadata is not None:
        frame.metadata = metadata
    return frame


def make_audio_frame(data=None, metadata=None):
    frame = ndi.AudioFrameV2()
    frame.sample_rate = 48000
    frame.data = np.zeros((2, 1600), dtype=np.float32) if data is None else data
    if metadata is not None:
        frame.metadata = metadata
    return frame


# Several frames by default, since a bad free may only crash on a later frame.
def receive_frames(send, capture, count=5, timeout=10):
    """Calls send() until capture() has returned something truthy count times.

    Returns the last value returned by capture().
    """
    received = 0
    result = None
    deadline = time.monotonic() + timeout
    while received < count:
        if time.monotonic() > deadline:
            pytest.fail('received only %d of %d frames' % (received, count))
        if send is not None:
            send()
        captured = capture()
        if captured:
            received += 1
            result = captured
    return result


def wait_until(condition, timeout=10, interval=0.05):
    deadline = time.monotonic() + timeout
    while not condition():
        if time.monotonic() > deadline:
            return False
        time.sleep(interval)
    return True
