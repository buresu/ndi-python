import threading
import time

import numpy as np
import pytest

import NDIlib as ndi
from conftest import unique_name

# A blocking call that holds the GIL stalls every other Python thread for as
# long as it blocks. These calls block for about 100 ms at a time.
BLOCK_MS = 100


def clocked_sender():
    return ndi.send_create(ndi.SendCreate(unique_name()))


def send_video(repeat):
    sender = clocked_sender()
    frame = ndi.VideoFrameV2(frame_rate_N=1000 // BLOCK_MS, frame_rate_D=1)
    frame.data = np.zeros((90, 160, 4), dtype=np.uint8)
    frame.FourCC = ndi.FOURCC_VIDEO_TYPE_BGRX
    for _ in range(repeat):
        ndi.send_send_video_v2(sender, frame)
    ndi.send_destroy(sender)


def send_audio(repeat):
    sender = clocked_sender()
    frame = ndi.AudioFrameV2()
    frame.sample_rate = 48000
    frame.data = np.zeros((2, 48 * BLOCK_MS), dtype=np.float32)
    for _ in range(repeat):
        ndi.send_send_audio_v2(sender, frame)
    ndi.send_destroy(sender)


def get_listener_events(kind, settings_type):
    def run(repeat):
        listener = getattr(ndi, kind + '_create')(settings_type('127.0.0.1:5959'))
        for _ in range(repeat):
            getattr(ndi, kind + '_get_events')(listener, BLOCK_MS)
        getattr(ndi, kind + '_destroy')(listener)
    return run


@pytest.mark.parametrize('blocking_call', [
    send_video,
    send_audio,
    get_listener_events('recv_listener', ndi.RecvListenerCreateV2),
    get_listener_events('send_listener', ndi.SendListenerCreateV2),
], ids=['send_video', 'send_audio', 'recv_listener_events', 'send_listener_events'])
def test_blocking_call_releases_gil(blocking_call):
    thread = threading.Thread(target=blocking_call, args=(5,))
    thread.start()
    longest_gap = 0
    last = time.monotonic()
    while thread.is_alive():
        time.sleep(0.001)
        now = time.monotonic()
        longest_gap = max(longest_gap, now - last)
        last = now
    thread.join()
    assert longest_gap < BLOCK_MS / 2 / 1000
