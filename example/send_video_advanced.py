import sys
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

    connection_type = ndi.MetadataFrame()
    connection_type.data = '<ndi_product long_name="NDILib Send Example." short_name="NDILib Send" manufacturer="CoolCo, inc." version="1.000.000" model_name="S1" serial="ABCDEFG"/>'

    ndi.send_add_connection_metadata(ndi_send, connection_type)

    video_frame = ndi.VideoFrameV2()
    video_frame.FourCC = ndi.FOURCC_VIDEO_TYPE_BGRA

    for idx in range(1000):
        if not ndi.send_get_no_connections(ndi_send, 10000):
            print('No current connections, so no rendering needed (%s).' % idx)
        else:
            metadata_desc = ndi.MetadataFrame()
            if ndi.send_capture(ndi_send, metadata_desc, 0):
                if metadata_desc.data[0:11].lower() == '<ndi_format':
                    pass
                print('Received meta-data : %s' % metadata_desc.data)
                ndi.send_free_metadata(metadata_desc)

        tally = ndi.Tally()
        ndi.send_get_tally(ndi_send, tally, 0)

        data = np.zeros((1080, 1920, 4), dtype=np.uint8)
        data[:, :, 0] = 255
        data[:, :, 1] = 128
        data[:, :, 2] = 128
        for r in range(data.shape[0]):
            data[r, :, 3] = 255 if r + idx & 16 else 128
        video_frame.data = data

        ndi.send_send_video_v2(ndi_send, video_frame)

        if idx % 100 == 0:
            print('Frame number %s sent. %s%s' % (
                1+idx, 'PGM ' if tally.on_program else '', 'PVW ' if tally.on_preview else ''))

    ndi.send_destroy(ndi_send)

    ndi.destroy()

    return 0


if __name__ == "__main__":
    sys.exit(main())
