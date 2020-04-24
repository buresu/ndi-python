import sys
import time
import numpy as np
import cv2 as cv
import NDIlib as ndi

def main():

    if not ndi.initialize():
        return 0

    img = cv.imread('image/test2.png', cv.IMREAD_ANYCOLOR)
    img = cv.cvtColor(img, cv.COLOR_BGR2RGBA)

    send_settings = ndi.SendCreate()
    send_settings.ndi_name = 'ndi-python'

    ndi_send = ndi.send_create(send_settings)

    video_frame = ndi.VideoFrameV2()

    video_frame.data = img
    video_frame.FourCC = ndi.FOURCC_VIDEO_TYPE_RGBX

    start = time.time()
    while time.time() - start < 60 * 5:
        start_send = time.time()

        for _ in reversed(range(200)):
            ndi.send_send_video_v2(ndi_send, video_frame)

        print('200 frames sent, at %1.2ffps' % (200.0 / (time.time() - start_send)))

    ndi.send_destroy(ndi_send)

    ndi.destroy()

    return 0

if __name__ == "__main__":
    sys.exit(main())
