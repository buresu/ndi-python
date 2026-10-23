import time

import pytest

import NDIlib as ndi
from helpers import make_audio_frame, make_video_frame, receive_frames

# std::string keeps short strings inline and long ones on the heap, and freeing
# metadata through NDI used to break differently for each, so cover both.
METADATA = pytest.mark.parametrize('metadata', [
    '<ndi_color_info transfer="bt_709" matrix="bt_709" primaries="bt_709" />',
    '<a/>',
], ids=['long', 'short'])


@pytest.mark.parametrize('frame_type', [
    ndi.VideoFrameV2, ndi.AudioFrameV2, ndi.AudioFrameV3])
@METADATA
def test_metadata_roundtrip(frame_type, metadata):
    frame = frame_type()
    assert frame.metadata == ''
    frame.metadata = metadata
    assert frame.metadata == metadata
    assert frame_type(p_metadata=metadata).metadata == metadata


@pytest.mark.loopback
@pytest.mark.parametrize('capture', [ndi.recv_capture_v2, ndi.recv_capture_v3])
@METADATA
def test_recv_video_metadata(loopback, capture, metadata):
    sender, receiver = loopback.sender, loopback.receiver
    frame = make_video_frame(metadata=metadata)

    def capture_one():
        t, v, _, _ = capture(receiver, 100, want_audio=False, want_metadata=False)
        if t != ndi.FRAME_TYPE_VIDEO:
            return False
        assert v.metadata == metadata
        ndi.recv_free_video_v2(receiver, v)
        return True

    receive_frames(lambda: ndi.send_send_video_v2(sender, frame), capture_one)


@pytest.mark.loopback
@pytest.mark.parametrize('capture, free', [
    (ndi.recv_capture_v2, ndi.recv_free_audio_v2),
    (ndi.recv_capture_v3, ndi.recv_free_audio_v3),
], ids=['v2', 'v3'])
@METADATA
def test_recv_audio_metadata(loopback, capture, free, metadata):
    sender, receiver = loopback.sender, loopback.receiver
    frame = make_audio_frame(metadata=metadata)

    def capture_one():
        t, _, a, _ = capture(receiver, 100, want_video=False, want_metadata=False)
        if t != ndi.FRAME_TYPE_AUDIO:
            return False
        assert a.metadata == metadata
        free(receiver, a)
        return True

    receive_frames(lambda: ndi.send_send_audio_v2(sender, frame), capture_one)


@pytest.mark.loopback
@METADATA
def test_free_after_overwriting_metadata(loopback, metadata):
    sender, receiver = loopback.sender, loopback.receiver
    frame = make_video_frame(metadata=metadata)
    replacement = 'x' * 200

    def capture_one():
        t, v, _, _ = ndi.recv_capture_v2(
            receiver, 100, want_audio=False, want_metadata=False)
        if t != ndi.FRAME_TYPE_VIDEO:
            return False
        v.metadata = replacement
        assert v.metadata == replacement
        ndi.recv_free_video_v2(receiver, v)
        return True

    receive_frames(lambda: ndi.send_send_video_v2(sender, frame), capture_one)


@pytest.mark.loopback
@METADATA
def test_framesync_video_metadata(loopback, metadata):
    sender, receiver = loopback.sender, loopback.receiver
    frame = make_video_frame(metadata=metadata)
    framesync = ndi.framesync_create(receiver)

    def capture_one():
        time.sleep(0.05)
        v = ndi.framesync_capture_video(framesync)
        try:
            # Nothing has arrived yet while the frame is still empty.
            if v.xres == 0:
                return False
            assert v.metadata == metadata
            return True
        finally:
            ndi.framesync_free_video(framesync, v)

    try:
        receive_frames(lambda: ndi.send_send_video_v2(sender, frame), capture_one)
    finally:
        ndi.framesync_destroy(framesync)
