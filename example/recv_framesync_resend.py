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

    create_params = ndi.SendCreate()
    create_params.clock_video = True
    create_params.clock_audio = False

    ndi_send = ndi.send_create(create_params)

    current_time = 0
    t = time.time()
    while time.time() - t < 5.0 * 60:
        v = ndi.framesync_capture_video(ndi_framesync)
        if v.data.size != 0:
            ndi.send_send_video_v2(ndi_send, v)
            frame_start = current_time
            current_time += (v.frame_rate_D * 480000) / v.frame_rate_N
            ndi.framesync_free_video(ndi_framesync, v)
            no_audio_samples = (current_time + 5) / 10 - (frame_start + 5) / 10
            a = ndi.framesync_capture_audio(
                ndi_framesync, 48000, 4, int(no_audio_samples))
            ndi.send_send_audio_v2(ndi_send, a)
            ndi.framesync_free_audio(ndi_framesync, a)
        else:
            time.sleep(33/1000)

    ndi.framesync_destoroy(ndi_framesync)

    ndi.recv_destroy(ndi_recv)

    ndi.destroy()

    return 0


if __name__ == "__main__":
    sys.exit(main())
