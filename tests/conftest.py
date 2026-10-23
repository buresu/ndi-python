import time
import uuid

import pytest

import NDIlib as ndi


@pytest.fixture(scope='session', autouse=True)
def ndi_runtime():
    if not ndi.initialize():
        pytest.exit('NDI runtime could not be initialized', returncode=1)
    yield
    ndi.destroy()


@pytest.fixture
def loopback():
    """A sender and a receiver connected to it within this process."""
    send_create = ndi.SendCreate()
    send_create.ndi_name = 'ndi-python-test-%s' % uuid.uuid4().hex[:8]
    # Send frames immediately instead of pacing them to the frame rate.
    send_create.clock_video = False
    send_create.clock_audio = False
    sender = ndi.send_create(send_create)

    finder = ndi.find_create_v2()
    source = None
    deadline = time.monotonic() + 10
    while source is None and time.monotonic() < deadline:
        ndi.find_wait_for_sources(finder, 500)
        source = next((s for s in ndi.find_get_current_sources(finder)
                       if send_create.ndi_name in s.ndi_name), None)
    ndi.find_destroy(finder)
    if source is None:
        ndi.send_destroy(sender)
        pytest.skip('loopback source was not discovered (is mDNS available?)')

    recv_create = ndi.RecvCreateV3()
    recv_create.color_format = ndi.RECV_COLOR_FORMAT_BGRX_BGRA
    receiver = ndi.recv_create_v3(recv_create)
    ndi.recv_connect(receiver, source)

    yield sender, receiver

    ndi.recv_destroy(receiver)
    ndi.send_destroy(sender)
