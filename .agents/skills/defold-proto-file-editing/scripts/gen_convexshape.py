# SPDX-License-Identifier: CC0-1.0

"""Generate a Defold .convexshape file from a 2D image's non-transparent silhouette.

Uses PIL/Pillow (bundled with most Python installations) to read images.
Computes a convex hull via Graham scan, simplifies to ≤16 points
using Visvalingam-Whyatt area-based simplification, centers points
at image origin, and outputs Defold-compatible .convexshape format.

Usage:
    python gen_convexshape.py <image_path> [--output <output_path>] [--max-points N] [--alpha-threshold T]

Arguments:
    image_path          Path to a PNG or JPEG image file
    --output, -o        Output .convexshape file path (default: prints to stdout)
    --max-points, -m    Maximum number of hull points (default: 16, Box2D limit in Defold)
    --alpha-threshold   Alpha value threshold for "non-transparent" (0-255, default: 1)
    --force-png-py      Force using bundled png.py instead of PIL (PNG only)

Environment:
    FORCE_PNG_PY=1      Same as --force-png-py

Output:
    Protobuf Text Format .convexshape with TYPE_HULL shape, points centered at image origin.

Exit code 0 on success, 1 on error.
"""

import argparse
import math
import os
import sys
from typing import TextIO

# image_loader is in the same directory; adjust sys.path so it's importable
# both when invoked directly and from the editor script.
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from image_loader import load_alpha_mask


def extract_boundary_pixels(coords: list[tuple[int, int]], width: int, height: int) -> list[tuple[int, int]]:
    """Extract only boundary pixels from the silhouette to reduce point count before hull."""
    pixel_set = set(coords)
    boundary = []
    for x, y in coords:
        is_boundary = False
        for dx, dy in [(-1, 0), (1, 0), (0, -1), (0, 1)]:
            nx, ny = x + dx, y + dy
            if nx < 0 or nx >= width or ny < 0 or ny >= height or (nx, ny) not in pixel_set:
                is_boundary = True
                break
        if is_boundary:
            boundary.append((x, y))
    return boundary


def cross(o: tuple[float, float], a: tuple[float, float], b: tuple[float, float]) -> float:
    """2D cross product of vectors OA and OB."""
    return (a[0] - o[0]) * (b[1] - o[1]) - (a[1] - o[1]) * (b[0] - o[0])


def graham_scan(points: list[tuple[float, float]]) -> list[tuple[float, float]]:
    """Compute convex hull using Andrew's monotone chain (a Graham scan variant).

    Returns points in counter-clockwise order.
    """
    pts = sorted(set(points))
    if len(pts) <= 2:
        return pts

    # Build lower hull
    lower: list[tuple[float, float]] = []
    for p in pts:
        while len(lower) >= 2 and cross(lower[-2], lower[-1], p) <= 0:
            lower.pop()
        lower.append(p)

    # Build upper hull
    upper: list[tuple[float, float]] = []
    for p in reversed(pts):
        while len(upper) >= 2 and cross(upper[-2], upper[-1], p) <= 0:
            upper.pop()
        upper.append(p)

    # Remove last point of each half because it's repeated
    return lower[:-1] + upper[:-1]


def triangle_area(a: tuple[float, float], b: tuple[float, float], c: tuple[float, float]) -> float:
    """Area of triangle formed by three points."""
    return abs(cross(a, b, c)) / 2.0


def simplify_hull(hull: list[tuple[float, float]], max_points: int) -> list[tuple[float, float]]:
    """Reduce hull to max_points using Visvalingam-Whyatt area-based simplification.

    Iteratively removes the vertex that contributes the least area
    to the polygon until we have at most max_points vertices.
    Preserves counter-clockwise winding order.
    """
    if len(hull) <= max_points:
        return hull

    pts = list(hull)

    while len(pts) > max_points:
        n = len(pts)
        min_area = float("inf")
        min_idx = -1

        for i in range(n):
            prev_pt = pts[(i - 1) % n]
            curr_pt = pts[i]
            next_pt = pts[(i + 1) % n]
            area = triangle_area(prev_pt, curr_pt, next_pt)
            if area < min_area:
                min_area = area
                min_idx = i

        pts.pop(min_idx)

    return pts


def ensure_ccw(hull: list[tuple[float, float]]) -> list[tuple[float, float]]:
    """Ensure points are in counter-clockwise order (positive signed area)."""
    signed_area = 0.0
    n = len(hull)
    for i in range(n):
        x1, y1 = hull[i]
        x2, y2 = hull[(i + 1) % n]
        signed_area += (x2 - x1) * (y2 + y1)
    if signed_area > 0:
        hull.reverse()
    return hull


def write_convexshape(hull: list[tuple[float, float]], out: TextIO) -> None:
    """Write hull points as Defold .convexshape format (TYPE_HULL with z=0)."""
    out.write("shape_type: TYPE_HULL\n")
    for x, y in hull:
        out.write(f"data: {x:.1f}\n")
        out.write(f"data: {y:.1f}\n")
        out.write("data: 0.0\n")


def format_float(v: float) -> str:
    """Format float for Defold: always has decimal point, no unnecessary trailing zeros."""
    if v == int(v):
        return f"{int(v)}.0"
    # Use enough precision but strip trailing zeros
    s = f"{v:.6f}".rstrip("0")
    if s.endswith("."):
        s += "0"
    return s


def write_convexshape_formatted(hull: list[tuple[float, float]], out: TextIO) -> None:
    """Write hull points as Defold .convexshape format with proper float formatting."""
    out.write("shape_type: TYPE_HULL\n")
    for x, y in hull:
        out.write(f"data: {format_float(x)}\n")
        out.write(f"data: {format_float(y)}\n")
        out.write("data: 0.0\n")


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Generate a Defold .convexshape file from an image's non-transparent silhouette."
    )
    parser.add_argument("image_path", help="Path to a PNG or JPEG image file")
    parser.add_argument("--output", "-o", help="Output .convexshape file path (default: stdout)")
    parser.add_argument("--max-points", "-m", type=int, default=16,
                        help="Maximum number of hull points (default: 16)")
    parser.add_argument("--alpha-threshold", "-a", type=int, default=1,
                        help="Alpha threshold for non-transparent pixels (0-255, default: 1)")
    parser.add_argument("--inset", "-i", type=float, default=0.0,
                        help="Inset percentage to shrink the shape (0-100, default: 0)")
    parser.add_argument("--force-png-py", action="store_true",
                        help="Force using bundled png.py instead of PIL (PNG only)")
    args = parser.parse_args()

    # Load image and get non-transparent pixels
    try:
        coords, width, height = load_alpha_mask(
            args.image_path, args.alpha_threshold,
            force_png_py=args.force_png_py,
        )
    except Exception as e:
        print(f"ERROR: Failed to read image: {e}", file=sys.stderr)
        return 1

    if len(coords) < 3:
        print("ERROR: Not enough non-transparent pixels to form a convex hull (need at least 3)", file=sys.stderr)
        return 1

    print(f"Image: {args.image_path} ({width}x{height})", file=sys.stderr)
    print(f"Non-transparent pixels: {len(coords)}", file=sys.stderr)

    # Extract boundary pixels to speed up hull computation
    boundary = extract_boundary_pixels(coords, width, height)
    print(f"Boundary pixels: {len(boundary)}", file=sys.stderr)

    # Compute convex hull (in pixel coordinates, Y grows down)
    hull = graham_scan([(float(x), float(y)) for x, y in boundary])
    print(f"Convex hull vertices: {len(hull)}", file=sys.stderr)

    # Simplify to max points
    if len(hull) > args.max_points:
        hull = simplify_hull(hull, args.max_points)
        print(f"Simplified to: {len(hull)} vertices", file=sys.stderr)

    # Center at image origin and flip Y axis (pixel Y grows down, Defold Y grows up)
    cx = width / 2.0
    cy = height / 2.0
    centered_hull = [(x - cx, cy - y) for x, y in hull]

    # Apply inset: shrink each point toward centroid by percentage
    if args.inset > 0.0:
        centroid_x = sum(x for x, y in centered_hull) / len(centered_hull)
        centroid_y = sum(y for x, y in centered_hull) / len(centered_hull)
        scale = 1.0 - args.inset / 100.0
        centered_hull = [
            (centroid_x + (x - centroid_x) * scale, centroid_y + (y - centroid_y) * scale)
            for x, y in centered_hull
        ]

    # Ensure counter-clockwise winding (required by Defold 2D physics)
    centered_hull = ensure_ccw(centered_hull)

    # Write output
    if args.output:
        with open(args.output, "w", newline="\n") as f:
            write_convexshape_formatted(centered_hull, f)
        print(f"Written: {args.output}", file=sys.stderr)
    else:
        write_convexshape_formatted(centered_hull, sys.stdout)

    return 0


if __name__ == "__main__":
    sys.exit(main())
