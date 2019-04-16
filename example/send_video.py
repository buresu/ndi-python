import sys
import time
import numpy as np
import NDIlib
import ctypes

def main():

    if not NDIlib.initialize():
        return 0

    ndi_send = NDIlib.send_create()

    if ndi_send is 0:
        return 0

    img = np.zeros((1080, 1920, 4), dtype=np.uint8)
    h, w = img.shape[:2]

    video_frame = NDIlib.VideoFrameV2()

    video_frame.xres = w
    video_frame.yres = h
    video_frame.FourCC = NDIlib.FOURCC_TYPE_BGRX
    #video_frame.data = img.ctypes.data_as(NDIlib.POINTER(NDIlib.c_uint8))
    #video_frame.data = img.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8))
    print(type(video_frame.data))

    start = time.time()
    while time.time() - start < 60 * 5:
        start_send = time.time()

        for idx in reversed(range(200)):
            img.fill(255 if idx % 2 else 0)
            NDIlib.send_send_video_v2(ndi_send, video_frame)

        print('200 frames sent, at %1.2ffps' % (200.0 / (time.time() - start_send)))

    NDIlib.send_destroy(ndi_send)

    NDIlib.destroy()

    return 0

if __name__ == "__main__":
    sys.exit(main())
