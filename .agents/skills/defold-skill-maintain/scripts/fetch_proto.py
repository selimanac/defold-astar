#!/usr/bin/env python3
"""Download defoldsdk.zip and extract share/proto into skill assets."""

import json
import os
import shutil
import sys
import tempfile
import urllib.request
import zipfile
from pathlib import Path


def find_project_root(start_dir: Path) -> Path:
	"""Find the project root by looking for game.project."""
	dir_path = start_dir.resolve()
	for _ in range(8):
		candidate = dir_path / "game.project"
		if candidate.exists():
			return dir_path
		parent = dir_path.parent
		if parent == dir_path:
			break
		dir_path = parent
	raise RuntimeError("Failed to locate project root (game.project not found)")


def download_to_file(url: str, out_path: Path) -> None:
	"""Download URL to file with progress indication."""
	tmp_path = out_path.with_suffix(out_path.suffix + ".tmp")
	out_path.parent.mkdir(parents=True, exist_ok=True)

	print(f"  Downloading...")
	request = urllib.request.Request(url, headers={"User-Agent": "sync-proto-py"})

	with urllib.request.urlopen(request, timeout=600) as response:
		total = response.headers.get("Content-Length")
		total_size = int(total) if total else None

		with open(tmp_path, "wb") as f:
			downloaded = 0
			block_size = 8192

			while True:
				chunk = response.read(block_size)
				if not chunk:
					break
				f.write(chunk)
				downloaded += len(chunk)

				if total_size:
					pct = downloaded * 100 // total_size
					print(f"\r  Downloaded: {downloaded:,} / {total_size:,} bytes ({pct}%)", end="", flush=True)
				else:
					print(f"\r  Downloaded: {downloaded:,} bytes", end="", flush=True)

		print()

	tmp_path.rename(out_path)


def extract_share_proto(zip_path: Path, output_dir: Path) -> None:
	"""Extract share/proto/ entries from defoldsdk.zip into output_dir."""
	prefix = "defoldsdk/share/proto/"

	if output_dir.exists():
		print(f"  Removing existing {output_dir}...")
		shutil.rmtree(output_dir)

	output_dir.mkdir(parents=True, exist_ok=True)

	extracted = 0
	with zipfile.ZipFile(zip_path, "r") as zf:
		for entry in zf.infolist():
			if entry.filename.endswith("/"):
				continue
			if not entry.filename.startswith(prefix):
				continue

			rel_path = entry.filename[len(prefix):]
			out_path = output_dir / rel_path

			resolved = out_path.resolve()
			if not str(resolved).startswith(str(output_dir.resolve()) + os.sep):
				raise RuntimeError(f"Zip-slip detected: {entry.filename}")

			out_path.parent.mkdir(parents=True, exist_ok=True)
			with zf.open(entry) as src, open(out_path, "wb") as dst:
				shutil.copyfileobj(src, dst)
			extracted += 1

	if extracted == 0:
		raise RuntimeError(f"No files found under '{prefix}' in the SDK zip")

	print(f"  Extracted {extracted} proto file(s)")


def main() -> None:
	script_dir = Path(__file__).parent
	project_root = find_project_root(script_dir)
	proto_dir = Path(__file__).resolve().parent.parent / "assets" / "proto"

	print("Fetching Defold stable release info...")
	request = urllib.request.Request(
		"https://d.defold.com/stable/info.json",
		headers={"User-Agent": "sync-proto-py"},
	)
	with urllib.request.urlopen(request, timeout=30) as response:
		info = json.loads(response.read().decode("utf-8"))

	version = info["version"]
	sha1 = info["sha1"]
	print(f"  Defold version: {version} (sha1: {sha1})")

	sdk_url = f"https://github.com/defold/defold/releases/download/{version}/defoldsdk.zip"
	print(f"  SDK URL: {sdk_url}")

	tmp_dir = Path(tempfile.mkdtemp(prefix="sync_proto_"))
	try:
		zip_path = tmp_dir / "defoldsdk.zip"
		download_to_file(sdk_url, zip_path)

		print(f"  Extracting share/proto/ -> {proto_dir.relative_to(project_root)}")
		extract_share_proto(zip_path, proto_dir)
	finally:
		shutil.rmtree(tmp_dir, ignore_errors=True)

	print()
	print("Done.")


if __name__ == "__main__":
	main()
