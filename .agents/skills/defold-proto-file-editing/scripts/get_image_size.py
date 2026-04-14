"""Get image dimensions (width, height) from PNG or JPEG files.

No external dependencies — uses only Python stdlib (struct).

Usage:
    python get_image_size.py <image_path> [<image_path> ...]

Output (per image):
    <path> <width> <height>

Exit code 0 on success, 1 if any file fails.
"""

import struct
import sys
from pathlib import Path


def get_png_size(path: str) -> tuple[int, int] | None:
    with open(path, "rb") as f:
        sig = f.read(8)
        if sig[:4] != b"\x89PNG":
            return None
        f.read(4)  # IHDR chunk length
        f.read(4)  # IHDR chunk type
        w, h = struct.unpack(">II", f.read(8))
        return w, h


def get_jpeg_size(path: str) -> tuple[int, int] | None:
    with open(path, "rb") as f:
        soi = f.read(2)
        if soi != b"\xff\xd8":
            return None
        while True:
            marker = f.read(2)
            if len(marker) < 2 or marker[0] != 0xFF:
                return None
            m = marker[1]
            # SOF0 (baseline) or SOF2 (progressive)
            if m in (0xC0, 0xC2):
                f.read(3)  # length (2) + precision (1)
                h, w = struct.unpack(">HH", f.read(4))
                return w, h
            else:
                length = struct.unpack(">H", f.read(2))[0]
                f.read(length - 2)


def get_image_size(path: str) -> tuple[int, int] | None:
    ext = Path(path).suffix.lower()
    if ext == ".png":
        return get_png_size(path)
    elif ext in (".jpg", ".jpeg"):
        return get_jpeg_size(path)
    else:
        return None


def main() -> int:
    if len(sys.argv) < 2:
        print(f"Usage: python {sys.argv[0]} <image_path> [<image_path> ...]", file=sys.stderr)
        return 1

    ok = True
    for path in sys.argv[1:]:
        size = get_image_size(path)
        if size is None:
            print(f"ERROR: Could not read dimensions from: {path}", file=sys.stderr)
            ok = False
        else:
            print(f"{path} {size[0]} {size[1]}")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
