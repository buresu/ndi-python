import numpy as np
import pytest

import NDIlib as ndi
from conftest import create_receiver, find_source, unique_name
from helpers import make_audio_frame, make_video_frame, receive_frames, wait_until

pytestmark = pytest.mark.loopback


def split_color_image():
    # Distinct left and right halves, so a wrong stride or flipped axis shows.
    data = np.empty((90, 160, 4), dtype=np.uint8)
    data[:, :80] = (255, 0, 0, 255)
    data[:, 80:] = (0, 0, 255, 255)
    return data


def capture_video(receiver, check, capture=ndi.recv_capture_v2):
    def capture_one():
        t, v, _, _ = capture(receiver, 100, want_audio=False, want_metadata=False)
        if t != ndi.FRAME_TYPE_VIDEO:
            return False
        try:
            check(v)
        finally:
            ndi.recv_free_video_v2(receiver, v)
        return True
    return capture_one


def capture_audio(receiver, check, capture=ndi.recv_capture_v2,
                  free=ndi.recv_free_audio_v2):
    def capture_one():
        t, _, a, _ = capture(receiver, 100, want_video=False, want_metadata=False)
        if t != ndi.FRAME_TYPE_AUDIO:
            return False
        try:
            check(a)
        finally:
            free(receiver, a)
        return True
    return capture_one


def test_find_sender(loopback):
    assert loopback.source.ndi_name.endswith('(%s)' % loopback.name)
    assert loopback.source.url_address


@pytest.mark.parametrize('capture', [ndi.recv_capture_v2, ndi.recv_capture_v3])
def test_send_recv_video(loopback, capture):
    data = split_color_image()
    frame = make_video_frame(data)

    def check(v):
        assert (v.xres, v.yres) == (160, 90)
        assert v.FourCC in (ndi.FOURCC_VIDEO_TYPE_BGRX, ndi.FOURCC_VIDEO_TYPE_BGRA)
        # Video is compressed on the wire, so compare loosely.
        np.testing.assert_allclose(v.data[:, :, :3], data[:, :, :3], atol=16)

    receive_frames(lambda: ndi.send_send_video_v2(loopback.sender, frame),
                   capture_video(loopback.receiver, check, capture))


def test_send_recv_video_async(loopback):
    frames = [make_video_frame(split_color_image()) for _ in range(2)]
    sent = 0

    def send():
        nonlocal sent
        # The frame passed last must stay alive until the next call.
        ndi.send_send_video_async_v2(loopback.sender, frames[sent % 2])
        sent += 1

    def check(v):
        assert (v.xres, v.yres) == (160, 90)

    receive_frames(send, capture_video(loopback.receiver, check))
    ndi.send_send_video_async_v2(loopback.sender, None)


@pytest.mark.parametrize('capture, free', [
    (ndi.recv_capture_v2, ndi.recv_free_audio_v2),
    (ndi.recv_capture_v3, ndi.recv_free_audio_v3),
], ids=['v2', 'v3'])
@pytest.mark.parametrize('frame_type, send', [
    (ndi.AudioFrameV2, ndi.send_send_audio_v2),
    (ndi.AudioFrameV3, ndi.send_send_audio_v3),
], ids=['send_v2', 'send_v3'])
def test_send_recv_audio(loopback, frame_type, send, capture, free):
    data = np.empty((2, 1600), dtype=np.float32)
    data[0] = 0.25
    data[1] = -0.5
    frame = frame_type()
    frame.sample_rate = 48000
    frame.data = data

    def check(a):
        assert (a.sample_rate, a.no_channels, a.no_samples) == (48000, 2, 1600)
        np.testing.assert_allclose(a.data, data, atol=1e-3)

    receive_frames(lambda: send(loopback.sender, frame),
                   capture_audio(loopback.receiver, check, capture, free))


@pytest.mark.parametrize('frame_type, send, dtype', [
    (ndi.AudioFrameInterleaved16s, ndi.util_send_send_audio_interleaved_16s, np.int16),
    (ndi.AudioFrameInterleaved32s, ndi.util_send_send_audio_interleaved_32s, np.int32),
    (ndi.AudioFrameInterleaved32f, ndi.util_send_send_audio_interleaved_32f, np.float32),
], ids=['16s', '32s', '32f'])
def test_send_interleaved_audio(loopback, frame_type, send, dtype):
    full_scale = 1.0 if dtype == np.float32 else np.iinfo(dtype).max
    data = np.empty((1600, 2), dtype=dtype)
    data[:, 0] = full_scale / 4
    data[:, 1] = -full_scale / 2
    frame = frame_type()
    frame.data = data

    def check(a):
        assert (a.no_channels, a.no_samples) == (2, 1600)
        left, right = a.data[0], a.data[1]
        # The level of integer samples depends on reference_level, so only
        # check that the channels kept their order and ratio.
        assert left.min() > 0
        np.testing.assert_allclose(right, -2 * left, rtol=1e-3)
        if dtype == np.float32:
            np.testing.assert_allclose(left, 0.25, atol=1e-3)

    receive_frames(lambda: send(loopback.sender, frame),
                   capture_audio(loopback.receiver, check))


def test_send_metadata_to_receiver(loopback):
    sent = ndi.MetadataFrame()
    sent.data = '<hello from="sender"/>'

    def capture_one():
        t, _, _, m = ndi.recv_capture_v2(
            loopback.receiver, 100, want_video=False, want_audio=False)
        if t != ndi.FRAME_TYPE_METADATA:
            return False
        try:
            assert m.data == sent.data
        finally:
            ndi.recv_free_metadata(loopback.receiver, m)
        return True

    receive_frames(lambda: ndi.send_send_metadata(loopback.sender, sent), capture_one)


def test_send_metadata_to_sender(loopback):
    sent = ndi.MetadataFrame()
    sent.data = '<hello from="receiver"/>'

    def capture_one():
        m = ndi.MetadataFrame()
        if ndi.send_capture(loopback.sender, m, 100) != ndi.FRAME_TYPE_METADATA:
            assert m.data == ''
            return False
        try:
            assert m.data == sent.data
        finally:
            ndi.send_free_metadata(loopback.sender, m)
        return True

    receive_frames(lambda: ndi.recv_send_metadata(loopback.receiver, sent), capture_one)


def test_send_metadata_non_ascii(loopback):
    sent = ndi.MetadataFrame()
    sent.data = '<hello name="日本語"/>'

    def capture_one():
        t, _, _, m = ndi.recv_capture_v2(
            loopback.receiver, 100, want_video=False, want_audio=False)
        if t != ndi.FRAME_TYPE_METADATA:
            return False
        try:
            assert m.data == sent.data
        finally:
            ndi.recv_free_metadata(loopback.receiver, m)
        return True

    receive_frames(lambda: ndi.send_send_metadata(loopback.sender, sent), capture_one)


def test_free_after_overwriting_received_metadata(loopback):
    sent = ndi.MetadataFrame()
    sent.data = '<hello/>'

    def capture_one():
        t, _, _, m = ndi.recv_capture_v2(
            loopback.receiver, 100, want_video=False, want_audio=False)
        if t != ndi.FRAME_TYPE_METADATA:
            return False
        m.data = '<overwritten/>'
        assert m.data == '<overwritten/>'
        ndi.recv_free_metadata(loopback.receiver, m)
        return True

    receive_frames(lambda: ndi.send_send_metadata(loopback.sender, sent), capture_one)


def test_free_after_overwriting_captured_metadata(loopback):
    sent = ndi.MetadataFrame()
    sent.data = '<hello/>'
    # A single frame reused across captures, as a polling loop would do.
    m = ndi.MetadataFrame()

    def capture_one():
        if ndi.send_capture(loopback.sender, m, 100) != ndi.FRAME_TYPE_METADATA:
            return False
        m.data = '<overwritten/>'
        ndi.send_free_metadata(loopback.sender, m)
        return True

    receive_frames(lambda: ndi.recv_send_metadata(loopback.receiver, sent), capture_one)


def test_connection_metadata(loopback):
    metadata = ndi.MetadataFrame()
    metadata.data = '<ndi_product short_name="test"/>'
    ndi.send_add_connection_metadata(loopback.sender, metadata)

    # Connection metadata is delivered when a receiver connects.
    receiver = create_receiver(loopback.source)
    try:
        def capture_one():
            t, _, _, m = ndi.recv_capture_v2(
                receiver, 100, want_video=False, want_audio=False)
            if t != ndi.FRAME_TYPE_METADATA:
                return False
            try:
                return m.data == metadata.data
            finally:
                ndi.recv_free_metadata(receiver, m)

        receive_frames(None, capture_one, count=1)
    finally:
        ndi.recv_destroy(receiver)
        ndi.send_clear_connection_metadata(loopback.sender)


def test_connections_and_tally(loopback):
    receive_frames(lambda: ndi.send_send_video_v2(loopback.sender, make_video_frame()),
                   capture_video(loopback.receiver, lambda v: None), count=1)
    assert ndi.send_get_no_connections(loopback.sender, 1000) >= 1
    assert ndi.recv_get_no_connections(loopback.receiver) >= 1

    ndi.recv_set_tally(loopback.receiver, ndi.Tally(on_program=True))
    tally = ndi.Tally()

    def on_program():
        ndi.send_get_tally(loopback.sender, tally, 100)
        return tally.on_program

    assert wait_until(on_program)
    assert not tally.on_preview


def test_source_names(loopback):
    receive_frames(lambda: ndi.send_send_video_v2(loopback.sender, make_video_frame()),
                   capture_video(loopback.receiver, lambda v: None), count=1)
    name = ndi.recv_get_source_name(loopback.receiver, 1000)
    assert name == loopback.source.ndi_name
    assert ndi.send_get_source_name(loopback.sender).ndi_name == loopback.source.ndi_name


def test_performance_and_queue(loopback):
    receive_frames(lambda: ndi.send_send_video_v2(loopback.sender, make_video_frame()),
                   capture_video(loopback.receiver, lambda v: None), count=3)
    total, dropped = ndi.recv_get_performance(loopback.receiver)
    assert total.video_frames >= 3
    assert dropped.video_frames >= 0
    queue = ndi.recv_get_queue(loopback.receiver)
    assert queue.video_frames >= 0


def test_framesync_audio(loopback):
    frame = make_audio_frame(np.full((2, 1600), 0.25, dtype=np.float32))
    framesync = ndi.framesync_create(loopback.receiver)

    def capture_one():
        a = ndi.framesync_capture_audio(framesync, 48000, 2, 1600)
        try:
            assert (a.no_channels, a.no_samples) == (2, 1600)
            # Silence is returned until audio has arrived.
            return np.abs(a.data).max() > 0.2
        finally:
            ndi.framesync_free_audio(framesync, a)

    try:
        receive_frames(lambda: ndi.send_send_audio_v2(loopback.sender, frame),
                       capture_one, count=1)
    finally:
        ndi.framesync_destroy(framesync)


def test_routing(loopback):
    name = unique_name('ndi-python-routing')
    routing = ndi.routing_create(ndi.RoutingCreate(name))
    try:
        assert ndi.routing_change(routing, loopback.source)

        source = find_source(name)
        assert source is not None
        receiver = create_receiver(source)
        try:
            receive_frames(
                lambda: ndi.send_send_video_v2(loopback.sender, make_video_frame()),
                capture_video(receiver, lambda v: None), count=1)
        finally:
            ndi.recv_destroy(receiver)
        assert ndi.routing_get_source_name(routing).ndi_name == source.ndi_name
        ndi.routing_clear(routing)
    finally:
        ndi.routing_destroy(routing)
