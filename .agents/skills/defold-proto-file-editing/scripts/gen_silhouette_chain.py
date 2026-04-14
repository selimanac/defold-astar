# SPDX-License-Identifier: CC0-1.0

"""Generate a Defold .collisionobject that approximates a concave silhouette
using a chain of thin, rotated TYPE_BOX shapes along the contour edges.

This lets you simulate concave collision in Box2D (which only supports convex
primitives) by lining up narrow boxes along every edge of the simplified
boundary polygon — ideal for race tracks, terrain outlines, and other static
level geometry.

Algorithm:
    1. Load the image and build a binary alpha mask.
    2. Extract directed boundary edges between opaque and transparent regions.
    3. Chain edges into closed contour loops (handles multiple disconnected
       components and holes).
    4. Simplify each contour with Ramer-Douglas-Peucker.
    5. For every edge of the simplified polygon, emit a thin TYPE_BOX whose
       position is the edge midpoint (in image-centred, Y-up Defold coords)
       and whose rotation quaternion aligns the box along the edge.

Usage:
    python gen_silhouette_chain.py <image_path> [options]

Arguments:
    image_path                Path to a PNG or JPEG image file
    --output, -o              Output .collisionobject file path (default: stdout)
    --epsilon, -e             RDP simplification tolerance in pixels (default: 2.0)
    --thickness, -t           Half-thickness of each wall box in pixels (default: 2.0)
    --alpha-threshold, -a     Alpha threshold for "non-transparent" (0-255, default: 1)
    --force-png-py            Force using bundled png.py instead of PIL (PNG only)
    --group, -g               Collision group (default: "default")
    --mask                    Collision mask group (repeatable, default: "default")
    --friction                Friction coefficient (default: 0.1)
    --restitution             Restitution / bounciness (default: 0.5)

Output:
    Protobuf Text Format .collisionobject with COLLISION_OBJECT_TYPE_STATIC
    and embedded TYPE_BOX shapes.

Environment:
    FORCE_PNG_PY=1            Same as --force-png-py

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
from image_loader import load_binary_mask


# ---------------------------------------------------------------------------
# Contour extraction (directed boundary edges → closed loops)
# ---------------------------------------------------------------------------

def extract_contours(
    mask: list[list[bool]], width: int, height: int
) -> list[list[tuple[float, float]]]:
    """Extract ordered contour loops from the binary mask.

    Uses directed boundary edges on the pixel grid.  Each edge runs between
    two grid vertices (integer coordinates 0..W, 0..H).  Edges are chained
    into closed loops by following start→end links.

    Returns a list of contours, each a list of (x, y) grid-vertex coords
    forming a closed polygon (first point == last point).
    """

    def is_opaque(px: int, py: int) -> bool:
        if 0 <= px < width and 0 <= py < height:
            return mask[py][px]
        return False

    # Pad the mask with a 1-pixel transparent border so shapes touching
    # image edges always produce closed contour loops.
    pad_w = width + 2
    pad_h = height + 2
    padded: list[list[bool]] = [[False] * pad_w for _ in range(pad_h)]
    for py in range(height):
        for px in range(width):
            padded[py + 1][px + 1] = mask[py][px]

    def is_opaque_padded(px: int, py: int) -> bool:
        if 0 <= px < pad_w and 0 <= py < pad_h:
            return padded[py][px]
        return False

    # Collect directed edges as a list, then index by start vertex.
    edge_list: list[tuple[tuple[int, int], tuple[int, int]]] = []

    # Direction convention: walk CW around opaque region in pixel space
    # (Y-down), which becomes CCW in Defold's Y-up space.
    #
    # Vertical edge at column c between rows r and r+1:
    #   opaque on left  (c-1, r) → edge goes DOWN: (c,r)→(c,r+1)
    #   opaque on right (c,   r) → edge goes UP:   (c,r+1)→(c,r)
    for c in range(pad_w + 1):
        for r in range(pad_h):
            left_opaque = is_opaque_padded(c - 1, r)
            right_opaque = is_opaque_padded(c, r)
            if left_opaque == right_opaque:
                continue
            if left_opaque:
                edge_list.append(((c, r), (c, r + 1)))
            else:
                edge_list.append(((c, r + 1), (c, r)))

    # Horizontal edge at row r between columns c and c+1:
    #   opaque below (c, r)   → edge goes RIGHT: (c,r)→(c+1,r)
    #   opaque above (c, r-1) → edge goes LEFT:  (c+1,r)→(c,r)
    for r in range(pad_h + 1):
        for c in range(pad_w):
            top_opaque = is_opaque_padded(c, r - 1)
            bot_opaque = is_opaque_padded(c, r)
            if top_opaque == bot_opaque:
                continue
            if bot_opaque:
                edge_list.append(((c, r), (c + 1, r)))
            else:
                edge_list.append(((c + 1, r), (c, r)))

    # Build adjacency: start_vertex → list of edge indices
    adj: dict[tuple[int, int], list[int]] = {}
    for idx, (s, _e) in enumerate(edge_list):
        adj.setdefault(s, []).append(idx)

    used = [False] * len(edge_list)

    # Chain edges into closed loops
    contours: list[list[tuple[float, float]]] = []
    for start_idx in range(len(edge_list)):
        if used[start_idx]:
            continue
        used[start_idx] = True
        start_vertex = edge_list[start_idx][0]
        loop: list[tuple[float, float]] = [start_vertex, edge_list[start_idx][1]]
        current = edge_list[start_idx][1]

        while current != start_vertex:
            found = False
            if current in adj:
                for ei in adj[current]:
                    if not used[ei]:
                        used[ei] = True
                        nxt = edge_list[ei][1]
                        loop.append(nxt)
                        current = nxt
                        found = True
                        break
            if not found:
                break

        if current == start_vertex and len(loop) >= 4:
            # Shift coordinates back by the padding offset (1, 1)
            contours.append([(x - 1.0, y - 1.0) for x, y in loop])

    return contours


# ---------------------------------------------------------------------------
# Ramer-Douglas-Peucker simplification (for closed polygons)
# ---------------------------------------------------------------------------

def _perpendicular_distance(
    px: float, py: float, ax: float, ay: float, bx: float, by: float
) -> float:
    """Perpendicular distance from point P to line segment A-B."""
    dx = bx - ax
    dy = by - ay
    length_sq = dx * dx + dy * dy
    if length_sq == 0.0:
        return math.hypot(px - ax, py - ay)
    t = max(0.0, min(1.0, ((px - ax) * dx + (py - ay) * dy) / length_sq))
    proj_x = ax + t * dx
    proj_y = ay + t * dy
    return math.hypot(px - proj_x, py - proj_y)


def _rdp_reduce(
    points: list[tuple[float, float]], epsilon: float
) -> list[tuple[float, float]]:
    """Ramer-Douglas-Peucker on an open polyline."""
    if len(points) <= 2:
        return points

    ax, ay = points[0]
    bx, by = points[-1]

    max_dist = 0.0
    max_idx = 0
    for i in range(1, len(points) - 1):
        d = _perpendicular_distance(points[i][0], points[i][1], ax, ay, bx, by)
        if d > max_dist:
            max_dist = d
            max_idx = i

    if max_dist > epsilon:
        left = _rdp_reduce(points[: max_idx + 1], epsilon)
        right = _rdp_reduce(points[max_idx:], epsilon)
        return left[:-1] + right
    else:
        return [points[0], points[-1]]


def simplify_contour(
    contour: list[tuple[float, float]], epsilon: float
) -> list[tuple[float, float]]:
    """Simplify a closed contour using RDP.

    The input contour has first == last point.  Returns a simplified closed
    polygon (first == last) with at least 3 unique vertices.
    """
    # Remove closing duplicate for processing
    pts = contour[:-1]
    if len(pts) < 3:
        return contour

    # For a closed polygon, we split at the point farthest from the
    # line between its neighbours to avoid arbitrary split artefacts.
    # Simple approach: split the ring into two halves and RDP each.
    n = len(pts)
    half = n // 2
    first_half = pts[: half + 1]
    second_half = pts[half:] + [pts[0]]

    simplified_first = _rdp_reduce(first_half, epsilon)
    simplified_second = _rdp_reduce(second_half, epsilon)

    # Merge (remove duplicate junction points)
    merged = simplified_first[:-1] + simplified_second[:-1]

    # Close the polygon
    if len(merged) < 3:
        merged = pts[:3]
    merged.append(merged[0])

    return merged


# ---------------------------------------------------------------------------
# Box generation from polygon edges
# ---------------------------------------------------------------------------

def format_float(v: float) -> str:
    """Format float for Defold: always has decimal point, no unnecessary trailing zeros."""
    if v == int(v):
        return f"{int(v)}.0"
    s = f"{v:.6f}".rstrip("0")
    if s.endswith("."):
        s += "0"
    return s


def angle_to_quat_z(angle: float) -> tuple[float, float]:
    """Convert a Z-axis rotation angle (radians) to quaternion (z, w) components.

    Full quaternion is (0, 0, z, w).
    """
    half = angle / 2.0
    return (math.sin(half), math.cos(half))


def write_collisionobject(
    contours: list[list[tuple[float, float]]],
    thickness: float,
    img_width: int,
    img_height: int,
    group: str,
    masks: list[str],
    friction: float,
    restitution: float,
    out: TextIO,
) -> int:
    """Write a .collisionobject with rotated TYPE_BOX shapes along contour edges.

    Returns the number of boxes written.
    """
    cx = img_width / 2.0
    cy = img_height / 2.0
    ext_z = 10.0

    # Collect all boxes: (pos_x, pos_y, qz, qw, ext_x, ext_y)
    boxes: list[tuple[float, float, float, float, float, float]] = []

    for contour in contours:
        for i in range(len(contour) - 1):
            x1, y1 = contour[i]
            x2, y2 = contour[i + 1]

            # Edge vector and length
            dx = x2 - x1
            dy = y2 - y1
            length = math.hypot(dx, dy)
            if length < 1e-6:
                continue

            # Inward normal: left perpendicular of the CW edge direction
            nx = -dy / length
            ny = dx / length

            # Edge midpoint offset inward by thickness (inner stroke)
            mid_px = (x1 + x2) / 2.0 + nx * thickness
            mid_py = (y1 + y2) / 2.0 + ny * thickness

            # Convert to Defold coordinates (origin at image centre, Y up)
            pos_x = mid_px - cx
            pos_y = cy - mid_py

            # Angle of edge in pixel space (Y down), then negate for Defold (Y up)
            angle_pixel = math.atan2(dy, dx)
            angle_defold = -angle_pixel

            qz, qw = angle_to_quat_z(angle_defold)

            ext_x = length / 2.0
            ext_y = thickness

            boxes.append((pos_x, pos_y, qz, qw, ext_x, ext_y))

    # Write header
    out.write("type: COLLISION_OBJECT_TYPE_STATIC\n")
    out.write("mass: 0.0\n")
    out.write(f"friction: {format_float(friction)}\n")
    out.write(f"restitution: {format_float(restitution)}\n")
    out.write(f'group: "{group}"\n')
    for m in masks:
        out.write(f'mask: "{m}"\n')

    out.write("embedded_collision_shape {\n")

    # Write shapes
    data_index = 0
    for pos_x, pos_y, qz, qw, ext_x, ext_y in boxes:
        out.write("  shapes {\n")
        out.write("    shape_type: TYPE_BOX\n")
        out.write("    position {\n")
        if pos_x != 0.0:
            out.write(f"      x: {format_float(pos_x)}\n")
        if pos_y != 0.0:
            out.write(f"      y: {format_float(pos_y)}\n")
        out.write("    }\n")
        out.write("    rotation {\n")
        if abs(qz) > 1e-7:
            out.write(f"      z: {format_float(qz)}\n")
        if abs(qw - 1.0) > 1e-7:
            out.write(f"      w: {format_float(qw)}\n")
        out.write("    }\n")
        out.write(f"    index: {data_index}\n")
        out.write("    count: 3\n")
        out.write("  }\n")
        data_index += 3

    # Write data array
    for _pos_x, _pos_y, _qz, _qw, ext_x, ext_y in boxes:
        out.write(f"  data: {format_float(ext_x)}\n")
        out.write(f"  data: {format_float(ext_y)}\n")
        out.write(f"  data: {format_float(ext_z)}\n")

    out.write("}\n")

    return len(boxes)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Generate a Defold .collisionobject that approximates a concave "
            "silhouette using a chain of rotated TYPE_BOX shapes along the "
            "contour polygon."
        )
    )
    parser.add_argument("image_path", help="Path to a PNG or JPEG image file")
    parser.add_argument("--output", "-o",
                        help="Output .collisionobject file path (default: stdout)")
    parser.add_argument("--epsilon", "-e", type=float, default=2.0,
                        help="RDP simplification tolerance in pixels (default: 2.0)")
    parser.add_argument("--thickness", "-t", type=float, default=2.0,
                        help="Half-thickness of wall boxes in pixels (default: 2.0)")
    parser.add_argument("--alpha-threshold", "-a", type=int, default=1,
                        help="Alpha threshold for non-transparent pixels (0-255, default: 1)")
    parser.add_argument("--force-png-py", action="store_true",
                        help="Force using bundled png.py instead of PIL (PNG only)")
    parser.add_argument("--group", "-g", default="default",
                        help='Collision group (default: "default")')
    parser.add_argument("--mask", action="append", default=None,
                        help='Collision mask group (repeatable, default: "default")')
    parser.add_argument("--friction", type=float, default=0.1,
                        help="Friction coefficient (default: 0.1)")
    parser.add_argument("--restitution", type=float, default=0.5,
                        help="Restitution / bounciness (default: 0.5)")
    args = parser.parse_args()

    masks = args.mask if args.mask else ["default"]

    # Load image
    try:
        mask, width, height = load_binary_mask(
            args.image_path, args.alpha_threshold,
            force_png_py=args.force_png_py,
        )
    except Exception as e:
        print(f"ERROR: Failed to read image: {e}", file=sys.stderr)
        return 1

    print(f"Image: {args.image_path} ({width}x{height})", file=sys.stderr)

    # Extract contours
    contours = extract_contours(mask, width, height)
    if not contours:
        print("ERROR: No contours found in image", file=sys.stderr)
        return 1

    total_verts = sum(len(c) - 1 for c in contours)
    print(f"Contours: {len(contours)} loops, {total_verts} vertices total", file=sys.stderr)

    # Simplify contours
    simplified: list[list[tuple[float, float]]] = []
    for contour in contours:
        s = simplify_contour(contour, args.epsilon)
        simplified.append(s)

    total_simplified = sum(len(c) - 1 for c in simplified)
    print(
        f"Simplified: {total_simplified} vertices "
        f"(epsilon={args.epsilon})",
        file=sys.stderr,
    )

    # Write output
    if args.output:
        with open(args.output, "w", newline="\n") as f:
            n = write_collisionobject(
                simplified, args.thickness, width, height,
                args.group, masks, args.friction, args.restitution, f,
            )
        print(f"Box shapes: {n}", file=sys.stderr)
        print(f"Written: {args.output}", file=sys.stderr)
    else:
        n = write_collisionobject(
            simplified, args.thickness, width, height,
            args.group, masks, args.friction, args.restitution, sys.stdout,
        )
        print(f"Box shapes: {n}", file=sys.stderr)

    return 0


if __name__ == "__main__":
    sys.exit(main())
