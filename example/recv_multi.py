import sys
import threading
import NDIlib as ndi


def receive_loop(ndi_recv, source_name, stop_event):
    while not stop_event.is_set():
        t, v, a, _ = ndi.recv_capture_v2(ndi_recv, 1000)

        if t == ndi.FRAME_TYPE_NONE:
            continue

        if t == ndi.FRAME_TYPE_VIDEO:
            print('[%s] Video data received (%dx%d).' % (source_name, v.xres, v.yres))
            ndi.recv_free_video_v2(ndi_recv, v)

        if t == ndi.FRAME_TYPE_AUDIO:
            print('[%s] Audio data received (%d samples).' % (source_name, a.no_samples))
            ndi.recv_free_audio_v2(ndi_recv, a)

    ndi.recv_destroy(ndi_recv)


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

    receivers = []
    for source in sources:
        ndi_recv_create = ndi.RecvCreateV3()
        ndi_recv_create.color_format = ndi.RECV_COLOR_FORMAT_BGRX_BGRA
        ndi_recv = ndi.recv_create_v3(ndi_recv_create)
        if ndi_recv is None:
            continue
        ndi.recv_connect(ndi_recv, source)
        receivers.append((ndi_recv, source.ndi_name))

    ndi.find_destroy(ndi_find)

    stop_event = threading.Event()
    threads = []

    for ndi_recv, source_name in receivers:
        t = threading.Thread(target=receive_loop, args=(ndi_recv, source_name, stop_event))
        t.start()
        threads.append(t)

    try:
        for t in threads:
            t.join()
    except KeyboardInterrupt:
        stop_event.set()
        for t in threads:
            t.join()

    ndi.destroy()

    return 0


if __name__ == "__main__":
    sys.exit(main())
