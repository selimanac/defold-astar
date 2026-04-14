# SPDX-License-Identifier: CC0-1.0

"""Unified image loading with PIL/Pillow and pure-Python png.py fallback.

Provides two functions used by gen_convexshape.py and gen_silhouette_chain.py:

- load_alpha_mask()  — returns list of non-transparent pixel coords + dimensions
- load_binary_mask() — returns 2D boolean mask + dimensions

Backend selection order:
1. PIL/Pillow (supports PNG, JPEG, and other formats).
2. Bundled png.py (PNG only — raises an error for non-PNG files).

Set the environment variable FORCE_PNG_PY=1 or pass force_png_py=True
to bypass PIL and use only the bundled png.py reader.
"""

import os
import sys
from pathlib import Path


def _is_png(path: str) -> bool:
    """Check if file has a .png extension (case-insensitive)."""
    return Path(path).suffix.lower() == ".png"


def _try_import_pil(force_png_py: bool) -> type | None:
    """Try to import PIL.Image, return the class or None."""
    if force_png_py:
        return None
    try:
        from PIL import Image
        return Image
    except ImportError:
        return None


# ---------------------------------------------------------------------------
# PIL backend
# ---------------------------------------------------------------------------

def _load_alpha_mask_pil(
    path: str, threshold: int
) -> tuple[list[tuple[int, int]], int, int]:
    """Load image via PIL and return non-transparent pixel coordinates."""
    from PIL import Image

    img = Image.open(path)
    width, height = img.size

    if img.mode == "RGBA":
        pixels = img.load()
        coords = []
        for y in range(height):
            for x in range(width):
                if pixels[x, y][3] >= threshold:
                    coords.append((x, y))
    elif img.mode == "LA":
        pixels = img.load()
        coords = []
        for y in range(height):
            for x in range(width):
                if pixels[x, y][1] >= threshold:
                    coords.append((x, y))
    elif img.mode == "P":
        img_rgba = img.convert("RGBA")
        pixels = img_rgba.load()
        coords = []
        for y in range(height):
            for x in range(width):
                if pixels[x, y][3] >= threshold:
                    coords.append((x, y))
    else:
        # No alpha channel (RGB, L, etc.) — all pixels are opaque
        coords = [(x, y) for y in range(height) for x in range(width)]

    return coords, width, height


def _load_binary_mask_pil(
    path: str, threshold: int
) -> tuple[list[list[bool]], int, int]:
    """Load image via PIL and return a 2D boolean mask."""
    from PIL import Image

    img = Image.open(path)
    width, height = img.size

    if img.mode != "RGBA":
        img = img.convert("RGBA")
    pixels = img.load()

    mask: list[list[bool]] = []
    for y in range(height):
        row: list[bool] = []
        for x in range(width):
            row.append(pixels[x, y][3] >= threshold)
        mask.append(row)

    return mask, width, height


# ---------------------------------------------------------------------------
# png.py fallback backend
# ---------------------------------------------------------------------------

def _load_png_reader(path: str):
    """Create a png.Reader and read the image as direct RGBA/LA/RGB/L data."""
    # Import the bundled png.py from the same directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    png_module_path = os.path.join(script_dir, "png.py")
    if not os.path.isfile(png_module_path):
        raise ImportError(f"Bundled png.py not found at {png_module_path}")

    import importlib.util
    spec = importlib.util.spec_from_file_location("_bundled_png", png_module_path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)

    reader = mod.Reader(filename=path)
    width, height, rows, info = reader.asDirect()
    # Materialize rows (they may be a generator)
    rows = [list(row) for row in rows]
    return width, height, rows, info


def _load_alpha_mask_png(
    path: str, threshold: int
) -> tuple[list[tuple[int, int]], int, int]:
    """Load a PNG image via bundled png.py and return non-transparent pixel coordinates."""
    width, height, rows, info = _load_png_reader(path)
    planes = info["planes"]
    has_alpha = info["alpha"]

    coords: list[tuple[int, int]] = []
    for y, row in enumerate(rows):
        for x in range(width):
            if has_alpha:
                # Alpha is the last plane value for this pixel
                alpha = row[x * planes + (planes - 1)]
                if alpha >= threshold:
                    coords.append((x, y))
            else:
                # No alpha channel — all pixels are opaque
                coords.append((x, y))

    return coords, width, height


def _load_binary_mask_png(
    path: str, threshold: int
) -> tuple[list[list[bool]], int, int]:
    """Load a PNG image via bundled png.py and return a 2D boolean mask."""
    width, height, rows, info = _load_png_reader(path)
    planes = info["planes"]
    has_alpha = info["alpha"]

    mask: list[list[bool]] = []
    for row in rows:
        mask_row: list[bool] = []
        for x in range(width):
            if has_alpha:
                alpha = row[x * planes + (planes - 1)]
                mask_row.append(alpha >= threshold)
            else:
                mask_row.append(True)
        mask.append(mask_row)

    return mask, width, height


# ---------------------------------------------------------------------------
# Public API
# ---------------------------------------------------------------------------

def _resolve_force_flag(force_png_py: bool | None) -> bool:
    """Resolve the force flag from argument and environment variable."""
    if force_png_py is not None:
        return force_png_py
    return os.environ.get("FORCE_PNG_PY", "").strip() in ("1", "true", "yes")


def load_alpha_mask(
    path: str, threshold: int, *, force_png_py: bool | None = None
) -> tuple[list[tuple[int, int]], int, int]:
    """Load image and return list of non-transparent pixel coordinates, width, height.

    Tries PIL first, falls back to bundled png.py for PNG files.
    Set force_png_py=True or env FORCE_PNG_PY=1 to skip PIL.
    """
    force = _resolve_force_flag(force_png_py)
    pil = _try_import_pil(force)

    if pil is not None:
        return _load_alpha_mask_pil(path, threshold)

    # Fallback to png.py
    if not _is_png(path):
        raise ImportError(
            "Pillow (PIL) is required for non-PNG images (JPEG, etc.).\n"
            "Install it from your terminal:\n"
            "    pip install pillow"
        )
    print("INFO: PIL not available, using bundled png.py reader.", file=sys.stderr)
    return _load_alpha_mask_png(path, threshold)


def load_binary_mask(
    path: str, threshold: int, *, force_png_py: bool | None = None
) -> tuple[list[list[bool]], int, int]:
    """Load image and return a 2D boolean mask (True = opaque), width, height.

    Tries PIL first, falls back to bundled png.py for PNG files.
    Set force_png_py=True or env FORCE_PNG_PY=1 to skip PIL.
    """
    force = _resolve_force_flag(force_png_py)
    pil = _try_import_pil(force)

    if pil is not None:
        return _load_binary_mask_pil(path, threshold)

    # Fallback to png.py
    if not _is_png(path):
        raise ImportError(
            "Pillow (PIL) is required for non-PNG images (JPEG, etc.).\n"
            "Install it from your terminal:\n"
            "    pip install pillow"
        )
    print("INFO: PIL not available, using bundled png.py reader.", file=sys.stderr)
    return _load_binary_mask_png(path, threshold)
