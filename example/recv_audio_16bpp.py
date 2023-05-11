import sys
import numpy as np
import NDIlib as ndi


def main():

    if not ndi.initialize():
        print('Cannot run NDI.')
        return 0

    find_create_desc = ndi.FindCreate()
    ndi_find = ndi.find_create_v2(find_create_desc)

    if ndi_find is None:
        return 0

    sources = []
    while not len(sources) > 0:
        print('Looking for sources ...')
        ndi.find_wait_for_sources(ndi_find, 1000)
        sources = ndi.find_get_current_sources(ndi_find)

    if len(sources) < 1:
        return 0

    recv_create_desc = ndi.RecvCreateV3()
    recv_create_desc.source_to_connect_to = sources[0]
    recv_create_desc.ndi_recv_name = 'Example Audio Converter Receiver'

    ndi_recv = ndi.recv_create_v3(recv_create_desc)

    if ndi_recv is None:
        ndi.find_destroy(ndi_find)
        return 0

    ndi.find_destroy(ndi_find)

    while True:
        t, v, a, m = ndi.recv_capture_v2(ndi_recv, 1000)

        if t == ndi.FRAME_TYPE_NONE:
            print('No data received.')
            continue

        if t == ndi.FRAME_TYPE_VIDEO:
            print('Video data received (%dx%d).' % (v.xres, v.yres))
            ndi.recv_free_video_v2(ndi_recv, v)
            continue

        if t == ndi.FRAME_TYPE_AUDIO:
            print('Audio data received (%d samples).' % a.no_samples)
            data = np.zeros((a.no_channels, a.no_samples), np.int16)
            audio_frame_16bpp_interleaved = ndi.AudioFrameInterleaved16s()
            audio_frame_16bpp_interleaved.reference_level = 20
            audio_frame_16bpp_interleaved.data = data
            ndi.util_audio_to_interleaved_16s_v2(
                a, audio_frame_16bpp_interleaved)
            ndi.recv_free_audio_v2(ndi_recv, a)
            continue

        if t == ndi.FRAME_TYPE_METADATA:
            print('Meta data received.')
            ndi.recv_free_metadata(ndi_recv, m)
            continue

        if t == ndi.FRAME_TYPE_STATUS_CHANGE:
            print('Receiver connection status changed.')
            continue

    ndi.recv_destroy(ndi_recv)

    ndi.destroy()

    return 0


if __name__ == "__main__":
    sys.exit(main())
