from flask import Flask, request

import random

from PIL import Image, UnidentifiedImageError, ImageOps
from pathlib import Path
import numpy as np


app = Flask(__name__)


def get_random_image(target_height, target_width):
    directory = Path() / "images"

    all_files = [f for f in directory.glob("*") if f.is_file()]
    random.shuffle(all_files)

    for file_path in all_files:
        try:
            with Image.open(file_path) as img:
                img = ImageOps.fit(
                    img,
                    (target_width, target_height),
                    method=Image.Resampling.LANCZOS,
                    centering=(0.5, 0.5),
                )
                return np.asarray(img, dtype=np.float32) / 255.0
        except UnidentifiedImageError:
            continue

    raise RuntimeError(f"No Pillow-compatible images found in '{directory_path}'")


def grayscale(img):
    # Take only the first 3 channels (RGB) in case input is RGBA
    rgb = img[..., :3]

    # Perceptual weights
    weights = np.array([0.299, 0.587, 0.114], dtype=np.float32)

    return np.dot(rgb, weights)


def to_epaper(img):
    return (img < 0.5).astype(np.ubyte)


@app.route("/get")
def get():
    height = request.args.get("height", default=500, type=int)
    width = request.args.get("width", default=500, type=int)

    img = get_random_image(height, width)
    img = grayscale(img)
    img = to_epaper(img)
    print(img)

    return img.tobytes()
