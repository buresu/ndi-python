import gc

import pytest

import NDIlib as ndi

# No discovery server needs to run at this address; the instances only try to
# connect to it in the background.
DISCOVERY_SERVER = '127.0.0.1:5959'


@pytest.mark.parametrize('kind, settings_type', [
    ('recv_listener', ndi.RecvListenerCreateV2),
    ('send_listener', ndi.SendListenerCreateV2),
    ('recv_advertiser', ndi.RecvAdvertiserCreateV2),
    ('send_advertiser', ndi.SendAdvertiserCreateV2),
])
def test_discovery_instance_destroy(kind, settings_type):
    create = getattr(ndi, kind + '_create')
    destroy = getattr(ndi, kind + '_destroy')

    # Destroyed explicitly, then collected.
    instance = create(settings_type(DISCOVERY_SERVER))
    assert instance is not None
    destroy(instance)
    del instance
    gc.collect()

    # Only collected.
    instance = create(settings_type(DISCOVERY_SERVER))
    assert instance is not None
    del instance
    gc.collect()


def test_create_and_destroy():
    finder = ndi.find_create_v2(ndi.FindCreate())
    assert finder is not None
    ndi.find_destroy(finder)

    sender = ndi.send_create(ndi.SendCreate('ndi-python-lifecycle'))
    assert sender is not None
    receiver = ndi.recv_create_v3(ndi.RecvCreateV3())
    assert receiver is not None
    framesync = ndi.framesync_create(receiver)
    assert framesync is not None
    routing = ndi.routing_create(ndi.RoutingCreate('ndi-python-lifecycle-routing'))
    assert routing is not None

    ndi.routing_destroy(routing)
    ndi.framesync_destroy(framesync)
    ndi.recv_destroy(receiver)
    ndi.send_destroy(sender)
