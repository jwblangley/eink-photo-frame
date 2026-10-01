from flask import Flask, request

import sys
import logging
import random

from PIL import Image, UnidentifiedImageError, ImageOps
from pathlib import Path
import numpy as np


logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    handlers=[logging.StreamHandler(sys.stdout)],
)


app = Flask(__name__)


def get_random_image(target_height, target_width):
    directory = Path() / "images"

    all_files = [f for f in directory.glob("*") if f.is_file()]
    random.shuffle(all_files)

    for file_path in all_files:
        try:
            with Image.open(file_path).convert("RGB") as img:
                img = ImageOps.fit(
                    img,
                    (target_width, target_height),
                    method=Image.Resampling.LANCZOS,
                    centering=(0.5, 0.5),
                )
                logging.info(f"Serving {file_path}")
                return np.asarray(img, dtype=np.float32) / 255.0
        except UnidentifiedImageError:
            continue

    raise RuntimeError(f"No Pillow-compatible images found in '{directory_path}'")


def to_epaper(img, shadow_gamma=0.65, contrast_factor=1.2, highlight_threshold=0.90):

    # Linearize sRGB (Gamma Decode)
    linear_rgb = np.where(
        img <= 0.04045, img / 12.92, np.power((img + 0.055) / 1.055, 2.4)
    )

    # Compute Perceptual Linear Luminance
    luminance = (
        0.2126 * linear_rgb[:, :, 0]
        + 0.7152 * linear_rgb[:, :, 1]
        + 0.0722 * linear_rgb[:, :, 2]
    )

    # Gamma Correction to lift shadow midtones
    luminance = np.power(luminance, shadow_gamma)

    # Apply S-curve / Midtone Contrast Adjustment
    luminance = 0.5 + contrast_factor * (luminance - 0.5)
    luminance = np.clip(luminance, 0.0, 1.0)

    # Highlight Clamping (highlights avoid dithering)
    luminance[luminance > highlight_threshold] = 1.0

    # Atkinson Dithering
    height, width = luminance.shape
    padded = np.pad(luminance, ((0, 2), (1, 2)), mode="edge")

    for y in range(height):
        for x in range(1, width + 1):
            old = padded[y, x]
            new = 1.0 if old > 0.5 else 0.0
            padded[y, x] = new

            err = (old - new) / 8.0

            padded[y, x + 1] += err
            padded[y, x + 2] += err
            padded[y + 1, x - 1] += err
            padded[y + 1, x] += err
            padded[y + 1, x + 1] += err
            padded[y + 2, x] += err

    # Crop back padding
    return (padded[:height, 1 : width + 1]).astype(np.ubyte)


@app.route("/get")
def get():
    height = request.args.get("height", default=500, type=int)
    width = request.args.get("width", default=500, type=int)
    logging.info(f"Request for {width=} {height=}")

    img = get_random_image(height, width)
    img = to_epaper(img)

    return img.tobytes()
