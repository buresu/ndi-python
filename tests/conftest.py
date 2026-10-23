import time
import uuid
from dataclasses import dataclass

import pytest

import NDIlib as ndi


@pytest.fixture(scope='session', autouse=True)
def ndi_runtime():
    if not ndi.initialize():
        pytest.exit('NDI runtime could not be initialized', returncode=1)
    yield
    ndi.destroy()


@dataclass
class Loopback:
    name: str
    source: ndi.Source
    sender: object
    receiver: object


def find_source(name, timeout=10):
    finder = ndi.find_create_v2()
    try:
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            ndi.find_wait_for_sources(finder, 500)
            for source in ndi.find_get_current_sources(finder):
                if source.ndi_name.endswith('(%s)' % name):
                    return source
        return None
    finally:
        ndi.find_destroy(finder)


def create_receiver(source):
    recv_create = ndi.RecvCreateV3()
    recv_create.color_format = ndi.RECV_COLOR_FORMAT_BGRX_BGRA
    receiver = ndi.recv_create_v3(recv_create)
    ndi.recv_connect(receiver, source)
    return receiver


def unique_name(prefix='ndi-python-test'):
    return '%s-%s' % (prefix, uuid.uuid4().hex[:8])


@pytest.fixture
def loopback():
    """A sender and a receiver connected to it within this process."""
    name = unique_name()
    send_create = ndi.SendCreate()
    send_create.ndi_name = name
    # Send frames immediately instead of pacing them to the frame rate.
    send_create.clock_video = False
    send_create.clock_audio = False
    sender = ndi.send_create(send_create)

    source = find_source(name)
    if source is None:
        ndi.send_destroy(sender)
        pytest.skip('loopback source was not discovered (is mDNS available?)')
    receiver = create_receiver(source)

    yield Loopback(name, source, sender, receiver)

    ndi.recv_destroy(receiver)
    ndi.send_destroy(sender)
