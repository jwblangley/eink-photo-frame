import logging
import random
import sys
from pathlib import Path

import numpy as np
from flask import Flask, request
from PIL import Image, ImageOps, UnidentifiedImageError

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    handlers=[logging.StreamHandler(sys.stdout)],
)

BLACK = 0
WHITE = 1
YELLOW = 2
RED = 3

MASK_PATH = ".mask"


app = Flask(__name__)


def get_random_image(target_height, target_width, rotate):
    directory = Path() / "images"

    all_files = [f for f in directory.rglob("*") if f.is_file()]
    random.shuffle(all_files)

    if rotate % 2 == 1:
        tmp = target_height
        target_height = target_width
        target_width = tmp

    for file_path in all_files:
        if file_path.stem.endswith(MASK_PATH):
            continue
        try:
            color_mask_path = (
                file_path.parent / f"{file_path.stem}{MASK_PATH}{file_path.suffix}"
            )
            with Image.open(file_path).convert("RGB") as img:
                img = ImageOps.fit(
                    img,
                    (target_width, target_height),
                    method=Image.Resampling.LANCZOS,
                    centering=(0.5, 0.5),
                )
                img = np.rot90(np.asarray(img, dtype=np.float32), k=rotate) / 255.0

            color_mask = None
            if color_mask_path.is_file():
                with Image.open(color_mask_path).convert("RGB") as color_mask:
                    color_mask = ImageOps.fit(
                        color_mask,
                        (target_width, target_height),
                        method=Image.Resampling.LANCZOS,
                        centering=(0.5, 0.5),
                    )
                    color_mask = (
                        np.rot90(np.asarray(color_mask, dtype=np.float32), k=rotate)
                        / 255.0
                    )

            if color_mask is not None and img.shape != color_mask.shape:
                raise RuntimeError(
                    f"Incorrectly shaped color mask for {file_path=}, {color_mask_path=}"
                )

            logging.info(
                f"Serving {file_path}"
                + ("" if color_mask is None else f" with color mask: {color_mask_path}")
            )
            return img, color_mask

        except UnidentifiedImageError:
            continue

    raise RuntimeError(f"No Pillow-compatible images found in '{directory}'")


def to_epaper(
    img,
    gamma=0.6,
    contrast_factor=1.3,
    highlight_threshold=0.90,
    shadow_threshold=0.10,
    target_mid_gray=0.18,
):

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

    # Adjust exposure
    low_bound = np.percentile(luminance, 10)
    high_bound = np.percentile(luminance, 90)
    non_outlier_mask = (luminance >= low_bound) & (luminance <= high_bound)
    non_outlier_pixels = luminance[non_outlier_mask]
    if not non_outlier_pixels.any():
        non_outlier_pixels = luminance
    log_mean_exposure = np.mean(np.log2(non_outlier_pixels))
    target_exposure = np.log2(target_mid_gray)
    exposure_shift = target_exposure - log_mean_exposure
    logging.info(f"Adjusting exposure by {exposure_shift:.2f} stops")
    luminance = luminance * (2**exposure_shift)
    luminance = np.clip(luminance, 0.0, 1.0)

    # Gamma Correction to lift shadow midtones
    luminance = np.power(luminance, gamma)

    # Apply S-curve / Midtone Contrast Adjustment
    luminance = 0.5 + contrast_factor * (luminance - 0.5)
    luminance = np.clip(luminance, 0.0, 1.0)

    # Highlight and Shadow Clamping (avoid dithering)
    luminance[luminance > highlight_threshold] = 1.0
    luminance[luminance < shadow_threshold] = 0.0

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


def color_mask(img, color_map):
    img = np.array(img)

    if color_map is None:
        return img

    # Green channel for yellow
    yellow_mask = color_map[:, :, 1]
    img[yellow_mask >= 0.5] *= YELLOW

    # Red channel for red
    red_mask = color_map[:, :, 0]
    img[red_mask >= 0.5] *= RED
    return img


@app.route("/get")
def get():
    height = request.args.get("height", default=500, type=int)
    width = request.args.get("width", default=500, type=int)
    rotate = request.args.get("rotate", default=0, type=int)
    logging.info(f"Request for {width=} {height=} {rotate=}")

    img, color_map = get_random_image(height, width, rotate)
    img = to_epaper(img)
    img = color_mask(img, color_map)

    return img.tobytes()
