import sys
import time
import NDIlib as ndi


def main():

    if not ndi.initialize():
        return 0

    ndi_find = ndi.find_create_v2()

    if ndi_find is None:
        return 0

    t = time.time()
    while time.time() - t < 1.0 * 60:
        if not ndi.find_wait_for_sources(ndi_find, 5000):
            print('No change to the sources found.')
            continue
        sources = ndi.find_get_current_sources(ndi_find)
        print('Network sources (%s found).' % len(sources))
        for i, s in enumerate(sources):
            print('%s. %s' % (i + 1, s.ndi_name))

    ndi.find_destroy(ndi_find)

    ndi.destroy()

    return 0


if __name__ == "__main__":
    sys.exit(main())
