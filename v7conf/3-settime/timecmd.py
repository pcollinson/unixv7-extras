#!/usr/bin/env python3
""" Make a paper tape
should start and end with 100 blank values
"""
import time


def cmd():
    """ make a date cmd """

    tspec = time.strftime("%y%m%d%H%M")
    cmd = "date " + tspec + '\n'
    body = bytearray(cmd, "ascii")
    with open("tc0.dec", "r+b") as f:
    	f.write(body)

if __name__ == '__main__':

    cmd()
