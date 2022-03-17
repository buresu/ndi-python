import sys
import time
import random
import NDIlib as ndi

def main():

    if not ndi.initialize():
        print('Cannot run NDI.')
        return 0

    send_create_desc = ndi.RoutingCreate()
    send_create_desc.ndi_name = 'Routing'

    ndi_routing = ndi.routing_create(send_create_desc)

    if ndi_routing is None:
        return 0

    find_create_desc = ndi.FindCreate()
    ndi_find = ndi.find_create_v2(find_create_desc)

    if ndi_find is None:
        return 0

    for i in range(1000):
        sources = ndi.find_get_current_sources(ndi_find)
        if len(sources) > 0:
            new_source = random.choice(sources)
            print('routing to %s' % new_source.ndi_name)
            ndi.routing_change(ndi_routing, new_source)
        else:
            ndi.routing_clear(ndi_routing)
        time.sleep(15)

    ndi.find_destroy(ndi_find)
    ndi.routing_destroy(ndi_routing)

    ndi.destroy()

    return 0

if __name__ == "__main__":
    sys.exit(main())
