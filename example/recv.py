import sys
import numpy as np
import NDIlib as ndi


def main():

    if not ndi.initialize():
        return 0

    NDI_find_create_desc = ndi.FindCreate()
    #ndi_source_ip_list = "192.168.100.48,192.168.100.88,192.168.100.95"
    ndi_source_ip_list = "192.168.100.88"
    NDI_find_create_desc.extra_ips = ndi_source_ip_list
    ndi_find = ndi.find_create_v2(NDI_find_create_desc)

    if ndi_find is None:
        return 0

    sources = []
    while not len(sources) > 0:
        print('Looking for sources ...')
        ndi.find_wait_for_sources(ndi_find, 1000)
        sources = ndi.find_get_current_sources(ndi_find)

    ndi_recv_create = ndi.RecvCreateV3()
    ndi_recv_create.color_format = ndi.RECV_COLOR_FORMAT_BGRX_BGRA

    ndi_recv = ndi.recv_create_v3(ndi_recv_create)

    if ndi_recv is None:
        return 0

    ndi.recv_connect(ndi_recv, sources[0])

    ndi.find_destroy(ndi_find)

    while True:
        t, v, a, _ = ndi.recv_capture_v2(ndi_recv, 5000)

        if t == ndi.FRAME_TYPE_NONE:
            print('No data received.')
            continue

        if t == ndi.FRAME_TYPE_VIDEO:
            print('Video data received (%dx%d).' % (v.xres, v.yres))
            ndi.recv_free_video_v2(ndi_recv, v)
            continue

        if t == ndi.FRAME_TYPE_AUDIO:
            print('Audio data received (%d samples).' % a.no_samples)
            ndi.recv_free_audio_v2(ndi_recv, a)
            continue

    ndi.recv_destroy(ndi_recv)

    ndi.destroy()

    return 0


if __name__ == "__main__":
    sys.exit(main())
