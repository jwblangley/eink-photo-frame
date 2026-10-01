"""
Visually tune this mock display to look the same as the real ePaper display
for quicker feedback loops when tuning the image processing
"""

import argparse
import urllib.request

import numpy as np
import matplotlib.pyplot as plt

WIDTH = 960
HEIGHT = 640

EPAPER_LUT = (
    np.array(
        [
            [28, 30, 32],
            [210, 214, 202],
        ],
        dtype=np.float32,
    )
    * 0.7
    / 255.0
)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("port", type=int)
    parser.add_argument("rotate", type=int)
    args = parser.parse_args()

    url = f"http://127.0.0.1:{args.port}/get?width={WIDTH}&height={HEIGHT}&rotate={args.rotate}"
    with urllib.request.urlopen(url) as response:
        img = np.frombuffer(response.read(), dtype=np.ubyte)
        img.resize(HEIGHT, WIDTH)

    img = EPAPER_LUT[img]

    plt.imshow(img, interpolation="nearest")
    plt.axis("off")
    plt.show()
