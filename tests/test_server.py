#!/usr/bin/env python
#-*- coding:utf-8 -*-

import sys
import time
import pytest
import subprocess

sys.path.append("client")

import client


@pytest.fixture(scope="module")
def iface(request):
    """
        The server interface.
    """
    return client.AudioServerInterface()


@pytest.fixture()
def srv(request):
    """
        A server for single tests, so we can check its output.
    """
    proc = subprocess.Popen(
        "./sys-server", stdout=subprocess.PIPE, stderr=subprocess.STDOUT
    )
    time.sleep(.1)
    return proc


def chk_output(substr, srv):
    """
        Wait a bit for the server to process the last command, then
        terminate it, and assert its output contains a substring.
    """
    time.sleep(.1)
    srv.terminate()
    assert substr in srv.communicate()[0]


@pytest.fixture(scope="module")
def srv4all(request):
    """
        A server that is supposed to be kept running during all the tests.
    """
    proc = subprocess.Popen(
        "./sys-server", stdout=subprocess.PIPE, stderr=subprocess.STDOUT
    )

    def finalizer():
        proc.terminate()

    request.addfinalizer(finalizer)
    time.sleep(.1)
    return proc


def test_start_uninitialized_source(srv, iface):
    with iface.packet() as pkt:
        pkt.start_sound(0)
    chk_output("necessary to add a source before", srv)
