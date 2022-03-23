import sys
import time
import numpy as np
import av
from fractions import Fraction
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

    recv_create_desc = ndi.RecvCreateV3()
    recv_create_desc.color_format = ndi.RECV_COLOR_FORMAT_BGRX_BGRA

    ndi_recv = ndi.recv_create_v3(recv_create_desc)

    if ndi_recv is None:
        return 0

    ndi.recv_connect(ndi_recv, sources[0])

    ndi.find_destroy(ndi_find)

    fps = 30
    output = av.open('output.mov', mode='w')
    stream = output.add_stream('mpeg4', rate=fps)
    stream.width = 1920
    stream.height = 1080
    stream.pix_fmt = 'yuv420p'
    stream.bit_rate = 8e+6
    stream.bit_rate_tolerance = 12e+6
    stream.codec_context.time_base = Fraction(1, fps)

    start = time.time()
    while time.time() - start < 1.0 * 30:
        t, v, _, _ = ndi.recv_capture_v2(ndi_recv, 5000)

        if t == ndi.FRAME_TYPE_VIDEO:
            print('Video data received (%dx%d).' % (v.xres, v.yres))
            frame_time = time.time() - start
            try:
                frame = av.VideoFrame.from_ndarray(v.data, format='bgra')
                frame.pts = int(
                    round(frame_time / stream.codec_context.time_base))
                for packet in stream.encode(frame):
                    output.mux(packet)
            except Exception as e:
                print(e)
            ndi.recv_free_video_v2(ndi_recv, v)

    for packet in stream.encode():
        output.mux(packet)

    output.close()

    ndi.recv_destroy(ndi_recv)
    ndi.destroy()

    return 0


if __name__ == "__main__":
    sys.exit(main())
