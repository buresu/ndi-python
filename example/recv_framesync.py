import sys
import time
import numpy as np
import NDIlib as ndi


def main():

    if not ndi.initialize():
        return 0

    ndi_find = ndi.find_create_v2()

    if ndi_find is None:
        return 0

    sources = []
    while not len(sources) > 0:
        print('Looking for sources ...')
        ndi.find_wait_for_sources(ndi_find, 1000)
        sources = ndi.find_get_current_sources(ndi_find)

    ndi_recv = ndi.recv_create_v3()

    if ndi_recv is None:
        return 0

    ndi.recv_connect(ndi_recv, sources[0])

    ndi_framesync = ndi.framesync_create(ndi_recv)

    ndi.find_destroy(ndi_find)

    t = time.time()
    while time.time() - t < 5.0 * 60:
        v = ndi.framesync_capture_video(ndi_framesync)
        ndi.framesync_free_video(ndi_framesync, v)
        a = ndi.framesync_capture_audio(ndi_framesync, 48000, 4, 1600)
        ndi.framesync_free_audio(ndi_framesync, a)
        time.sleep(33/1000)

    ndi.framesync_destoroy(ndi_framesync)

    ndi.recv_destroy(ndi_recv)

    ndi.destroy()

    return 0


if __name__ == "__main__":
    sys.exit(main())
