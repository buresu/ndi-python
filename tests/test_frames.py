import numpy as np
import pytest

import NDIlib as ndi


def test_video_frame_data():
    data = np.random.default_rng(0).integers(0, 256, (90, 160, 4), dtype=np.uint8)
    frame = ndi.VideoFrameV2()
    frame.data = data
    assert (frame.xres, frame.yres) == (160, 90)
    assert frame.line_stride_in_bytes == 160 * 4
    assert frame.picture_aspect_ratio == pytest.approx(160 / 90)
    np.testing.assert_array_equal(frame.data, data)


def test_video_frame_non_contiguous_data():
    data = np.random.default_rng(0).integers(0, 256, (90, 320, 4), dtype=np.uint8)
    view = data[:, ::2]
    frame = ndi.VideoFrameV2()
    frame.data = view
    assert (frame.xres, frame.yres) == (160, 90)
    np.testing.assert_array_equal(frame.data, view)


@pytest.mark.parametrize('frame_type', [ndi.AudioFrameV2, ndi.AudioFrameV3])
def test_audio_frame_data(frame_type):
    data = np.random.default_rng(0).uniform(-1, 1, (2, 1600)).astype(np.float32)
    frame = frame_type()
    frame.data = data
    assert (frame.no_channels, frame.no_samples) == (2, 1600)
    assert frame.channel_stride_in_bytes == 1600 * 4
    np.testing.assert_array_equal(frame.data, data)


@pytest.mark.parametrize('frame_type', [ndi.AudioFrameV2, ndi.AudioFrameV3])
def test_audio_frame_transposed_data(frame_type):
    # Audio libraries commonly hand out (samples, channels) arrays.
    samples = np.random.default_rng(0).uniform(-1, 1, (1600, 2)).astype(np.float32)
    frame = frame_type()
    frame.data = samples.T
    assert (frame.no_channels, frame.no_samples) == (2, 1600)
    np.testing.assert_array_equal(frame.data, samples.T)


@pytest.mark.parametrize('frame_type', [ndi.AudioFrameV2, ndi.AudioFrameV3])
def test_audio_frame_rejects_wrong_dimensions(frame_type):
    frame = frame_type()
    with pytest.raises(ValueError):
        frame.data = np.zeros(1600, dtype=np.float32)


@pytest.mark.parametrize('frame_type, dtype', [
    (ndi.AudioFrameInterleaved16s, np.int16),
    (ndi.AudioFrameInterleaved32s, np.int32),
    (ndi.AudioFrameInterleaved32f, np.float32),
])
def test_interleaved_audio_frame_data(frame_type, dtype):
    data = np.arange(1600 * 2, dtype=dtype).reshape(1600, 2)
    frame = frame_type()
    frame.data = data
    assert (frame.no_samples, frame.no_channels) == (1600, 2)
    np.testing.assert_array_equal(frame.data, data)


@pytest.mark.parametrize('interleaved_type, suffix, dtype, tolerance', [
    (ndi.AudioFrameInterleaved16s, '16s', np.int16, 1e-4),
    (ndi.AudioFrameInterleaved32s, '32s', np.int32, 1e-6),
    (ndi.AudioFrameInterleaved32f, '32f', np.float32, 0),
])
@pytest.mark.parametrize('planar_type, version', [
    (ndi.AudioFrameV2, 'v2'),
    (ndi.AudioFrameV3, 'v3'),
])
def test_audio_interleaved_conversion_roundtrip(
        interleaved_type, suffix, dtype, tolerance, planar_type, version):
    planar = np.random.default_rng(0).uniform(-0.5, 0.5, (2, 1600)).astype(np.float32)
    src = planar_type()
    src.data = planar

    interleaved = interleaved_type()
    interleaved.data = np.zeros((1600, 2), dtype=dtype)
    getattr(ndi, 'util_audio_to_interleaved_%s_%s' % (suffix, version))(src, interleaved)
    assert (interleaved.no_samples, interleaved.no_channels) == (1600, 2)
    if dtype == np.float32:
        np.testing.assert_array_equal(interleaved.data, planar.T)

    dst = planar_type()
    dst.data = np.zeros((2, 1600), dtype=np.float32)
    getattr(ndi, 'util_audio_from_interleaved_%s_%s' % (suffix, version))(interleaved, dst)
    assert (dst.no_channels, dst.no_samples) == (2, 1600)
    np.testing.assert_allclose(dst.data, planar, atol=tolerance)


def test_metadata_frame_data():
    frame = ndi.MetadataFrame()
    assert (frame.data, frame.length) == ('', 0)
    frame.data = '<hello/>'
    assert frame.data == '<hello/>'
    frame.data = ''
    assert (frame.data, frame.length) == ('', 0)
    assert ndi.MetadataFrame(p_data='<hello/>').data == '<hello/>'


@pytest.mark.parametrize('data', ['<hello/>', '<a name="日本語"/>'])
def test_metadata_frame_length(data):
    # NDI counts UTF-8 bytes including the terminating NUL.
    expected = len(data.encode()) + 1
    frame = ndi.MetadataFrame()
    frame.data = data
    assert frame.length == expected
    # The length argument is ignored in favor of the data.
    assert ndi.MetadataFrame(100, p_data=data).length == expected
    with pytest.raises(AttributeError):
        frame.length = 100


def test_source():
    source = ndi.Source('HOST (name)', '127.0.0.1:5960')
    assert source.ndi_name == 'HOST (name)'
    assert source.url_address == '127.0.0.1:5960'
    source.ndi_name = 'OTHER (name)'
    assert source.ndi_name == 'OTHER (name)'


def test_create_settings():
    send_create = ndi.SendCreate('sender', 'public', clock_video=False)
    assert (send_create.ndi_name, send_create.groups) == ('sender', 'public')
    assert (send_create.clock_video, send_create.clock_audio) == (False, True)

    find_create = ndi.FindCreate(False, 'public', '127.0.0.1')
    assert find_create.show_local_sources is False
    assert (find_create.groups, find_create.extra_ips) == ('public', '127.0.0.1')

    routing_create = ndi.RoutingCreate('routing', 'public')
    assert (routing_create.ndi_name, routing_create.groups) == ('routing', 'public')

    source = ndi.Source('HOST (name)', '127.0.0.1:5960')
    recv_create = ndi.RecvCreateV3(source, ndi.RECV_COLOR_FORMAT_BGRX_BGRA,
                                   ndi.RECV_BANDWIDTH_LOWEST, False, 'receiver')
    assert recv_create.source_to_connect_to.ndi_name == 'HOST (name)'
    assert recv_create.source_to_connect_to.url_address == '127.0.0.1:5960'
    assert recv_create.color_format == ndi.RECV_COLOR_FORMAT_BGRX_BGRA
    assert recv_create.bandwidth == ndi.RECV_BANDWIDTH_LOWEST
    assert recv_create.allow_video_fields is False
    assert recv_create.ndi_recv_name == 'receiver'
