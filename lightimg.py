#!/usr/bin/env python

import os
import sys
import struct
from PIL import Image

FMT = "HH768s%ss"
HELP = """

lightimg.py <SOURCE>

Utility to convert palette images to "light" format, which is
used in Arduino LightBar project.

<SOURCE> must be some palette image (like GIF or PNG).
"""

if __name__ == '__main__':
    # check input arguments
    if len(sys.argv) < 2:
        sys.exit('ERROR: Missing filepath to source image.' + HELP)

    path = os.path.abspath(sys.argv[1])
    if not os.path.exists(path) or not os.path.isfile(path):
        sys.exit('ERROR: Source image does not exist.' + HELP)

    im = Image.open(path)
    if im.palette is None:
        sys.exit('ERROR: Source image is not palette image.' + HELP)

    # size of source image
    (w, h) = im.size

    # prepare buffer of image data (column oriented)
    data = []
    for x in range(w):
        for y in range(h):
            data.append(chr(im.getpixel((x, y))))
    fmt = FMT % len(data)

    # prepare palette info
    palette = im.palette.getdata()

    # make final output image file
    output = os.path.splitext(path)[0]
    output += '.lig'
    with open(output, 'wb') as f:
        f.write(struct.pack(fmt, w, h, palette[1], "".join(data)))
