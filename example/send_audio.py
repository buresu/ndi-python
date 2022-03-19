import sys
import numpy as np
import NDIlib as ndi


def main():

    if not ndi.initialize():
        print('Cannot run NDI.')
        return 0

    send_create_desc = ndi.SendCreate()
    send_create_desc.ndi_name = 'My Audio'
    send_create_desc.clock_audio = True

    ndi_send = ndi.send_create(send_create_desc)

    if ndi_send is None:
        return 0

    audio_frame = ndi.AudioFrameV2()
    audio_frame.sample_rate = 48000
    audio_frame.no_channels = 4
    audio_frame.no_samples = 1920

    for i in range(1000):
        audio_frame.data = np.zeros((4, 1920), dtype=np.float)
        ndi.send_send_audio_v2(ndi_send, audio_frame)
        print('Frame number %d sent.' % i)

    ndi.send_destroy(ndi_send)

    ndi.destroy()

    return 0


if __name__ == "__main__":
    sys.exit(main())
