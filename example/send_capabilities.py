import sys
import time
import numpy as np
import NDIlib as ndi


def main():

    if not ndi.initialize():
        print('Cannot run NDI.')
        return 0

    send_create_desc = ndi.SendCreate()
    send_create_desc.ndi_name = 'My Video'

    ndi_send = ndi.send_create(send_create_desc)

    if ndi_send is None:
        return 0

    ndi_capabilities = ndi.MetadataFrame()
    ndi_capabilities.data = '<ndi_capabilities web_control="http://%IP%//MyControl"/>'

    ndi.send_add_connection_metadata(ndi_send, ndi_capabilities)

    video_frame = ndi.VideoFrameV2()
    video_frame.FourCC = ndi.FOURCC_VIDEO_TYPE_RGBA

    start = time.time()
    while time.time() - start < 60 * 5:
        start_send = time.time()

        for idx in reversed(range(200)):
            video_frame.data = np.full(
                (1080, 1920, 4), 255 if idx % 2 else 0, dtype=np.uint8)
            ndi.send_send_video_v2(ndi_send, video_frame)

        print('200 frames sent, at %1.2ffps' %
              (200.0 / (time.time() - start_send)))

    ndi.send_destroy(ndi_send)

    ndi.destroy()

    return 0


if __name__ == "__main__":
    sys.exit(main())
