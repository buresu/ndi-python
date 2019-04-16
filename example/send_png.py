import sys
import time
import numpy as np
import cv2
import NDIlib

def main():

    if not NDIlib.initialize():
        return 0

    img = cv2.imread('image/test2.png', cv2.IMREAD_ANYCOLOR)
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGBA)

    h, w = img.shape[:2]

    send_settings = NDIlib.SendCreate()
    send_settings.p_ndi_name = b'ndi-python'

    ndi_send = NDIlib.send_create(send_settings)

    video_frame = NDIlib.VideoFrameV2()
    video_frame.xres = w
    video_frame.yres = h
    video_frame.FourCC = NDIlib.FOURCC_TYPE_RGBA
    video_frame.p_data = img.ctypes.data_as(NDIlib.POINTER(NDIlib.c_uint8))
    video_frame.line_stride_in_bytes = w * 4

    start = time.time()
    while time.time() - start < 60 * 5:
        start_send = time.time()

        for idx in reversed(range(200)):
            NDIlib.send_send_video_v2(ndi_send, video_frame)

        print('200 frames sent, at %1.2ffps' % (200.0 / (time.time() - start_send)))

    NDIlib.send_destroy(ndi_send)

    NDIlib.destroy()

    return 0

if __name__ == "__main__":
    sys.exit(main())
