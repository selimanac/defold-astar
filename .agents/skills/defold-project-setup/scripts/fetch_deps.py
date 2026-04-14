#!/usr/bin/env python3
"""Fetch Defold project dependencies from game.project."""

import io
import json
import os
import re
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


def parse_ini(text: str) -> dict[str, dict[str, str]]:
	"""Minimal Defold-style INI parser."""
	out: dict[str, dict[str, str]] = {}
	section = ""

	for raw_line in text.splitlines():
		line = raw_line.strip()
		if not line or line.startswith(";") or line.startswith("#"):
			continue

		match = re.match(r"^\[([^\]]+)\]$", line)
		if match:
			section = match.group(1).strip()
			if section not in out:
				out[section] = {}
			continue

		eq_idx = line.find("=")
		if eq_idx == -1:
			continue

		key = line[:eq_idx].strip()
		value = line[eq_idx + 1:].strip()
		if section not in out:
			out[section] = {}
		out[section][key] = value

	return out


def parse_project_dependencies(project_text: str) -> list[str]:
	"""Parse project.dependencies#N entries from game.project."""
	ini = parse_ini(project_text)
	project_section = ini.get("project", {})

	indexed: list[tuple[int, str]] = []

	for key, value in project_section.items():
		match = re.match(r"^dependencies#(\d+)$", key)
		if match:
			indexed.append((int(match.group(1)), value))

	indexed.sort(key=lambda x: x[0])
	return [url for _, url in indexed]


def download_to_file(url: str, out_path: Path) -> None:
	"""Download URL to file with progress indication."""
	tmp_path = out_path.with_suffix(out_path.suffix + ".tmp")
	out_path.parent.mkdir(parents=True, exist_ok=True)

	print(f"  Downloading...")
	request = urllib.request.Request(url, headers={"User-Agent": "sync-deps-py"})

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


def find_game_project_in_zip(zip_path: Path) -> tuple[str, str]:
	"""Find game.project in zip, return (zip_root_prefix, project_text)."""
	with zipfile.ZipFile(zip_path, "r") as zf:
		for name in zf.namelist():
			if name.endswith("/"):
				continue
			if name.endswith("game.project"):
				with zf.open(name) as f:
					text = f.read().decode("utf-8")
				idx = name.rfind("game.project")
				prefix = name[:idx] if idx > 0 else ""
				return prefix, text

	raise RuntimeError("No game.project found inside dependency zip")


def parse_library_include_dirs(project_text: str) -> list[str]:
	"""Parse [library] include_dirs from game.project."""
	ini = parse_ini(project_text)
	library = ini.get("library", ini.get("Library", {}))
	raw = library.get("include_dirs", "")

	if not raw:
		raise RuntimeError("Missing [library] include_dirs in dependency game.project")

	return [s.strip() for s in raw.split(",") if s.strip()]


def assert_safe_include_dir(dir_name: str) -> None:
	"""Validate include_dir name for safety."""
	if not dir_name:
		raise RuntimeError("Invalid include_dir (empty)")
	if "/" in dir_name or "\\" in dir_name:
		raise RuntimeError(f"Unsafe include_dir contains slash: {dir_name}")
	if ".." in dir_name:
		raise RuntimeError(f'Unsafe include_dir contains "..": {dir_name}')
	if not re.match(r"^[A-Za-z0-9_-]+$", dir_name):
		raise RuntimeError(f"Unsafe include_dir contains disallowed characters: {dir_name}")


def delete_local_include_dirs(deps_dir: Path, include_dirs: list[str]) -> None:
	"""Delete local include_dirs folders in .deps/."""
	for d in include_dirs:
		target = deps_dir / d
		if target.exists():
			print(f"  Deleting: {target}")
			shutil.rmtree(target)


def extract_selected_dirs(deps_dir: Path, zip_path: Path, zip_root_prefix: str, include_dirs: list[str]) -> None:
	"""Extract only include_dirs from zip to .deps/."""
	prefixes = [f"{zip_root_prefix}{d}/" for d in include_dirs]
	deps_dir.mkdir(parents=True, exist_ok=True)

	with zipfile.ZipFile(zip_path, "r") as zf:
		for entry in zf.infolist():
			if entry.filename.endswith("/"):
				continue

			matched_prefix = None
			for p in prefixes:
				if entry.filename.startswith(p):
					matched_prefix = p
					break

			if not matched_prefix:
				continue

			rel_zip_path = entry.filename[len(zip_root_prefix):]
			out_path = deps_dir / rel_zip_path

			resolved = out_path.resolve()
			if not str(resolved).startswith(str(deps_dir.resolve()) + os.sep):
				raise RuntimeError(f"Zip-slip detected: {entry.filename}")

			out_path.parent.mkdir(parents=True, exist_ok=True)

			with zf.open(entry) as src, open(out_path, "wb") as dst:
				shutil.copyfileobj(src, dst)

	print(f"  Extracted {len(include_dirs)} dir(s)")


def sync_builtins(deps_dir: Path) -> None:
	"""Download and extract builtins from the stable Defold release."""
	builtins_dir = deps_dir / "builtins"
	if builtins_dir.exists():
		print("Builtins already present, skipping.")
		return

	print("Fetching Defold stable release info...")
	request = urllib.request.Request(
		"https://d.defold.com/stable/info.json",
		headers={"User-Agent": "sync-deps-py"},
	)
	with urllib.request.urlopen(request, timeout=30) as response:
		info = json.loads(response.read().decode("utf-8"))

	version = info["version"]
	sha1 = info["sha1"]
	print(f"  Defold version: {version} (sha1: {sha1})")

	release_url = f"https://github.com/defold/defold/releases/download/{version}/Defold-x86_64-win32.zip"
	jar_entry = f"Defold/packages/defold-{sha1}.jar"

	tmp_dir = Path(tempfile.mkdtemp(prefix="sync_builtins_"))
	try:
		release_zip_path = tmp_dir / "defold_release.zip"
		download_to_file(release_url, release_zip_path)

		print(f"  Extracting {jar_entry} from release zip...")
		jar_path = tmp_dir / "defold.jar"
		with zipfile.ZipFile(release_zip_path, "r") as zf:
			with zf.open(jar_entry) as src, open(jar_path, "wb") as dst:
				shutil.copyfileobj(src, dst)

		print("  Extracting builtins/ from jar...")
		deps_dir.mkdir(parents=True, exist_ok=True)
		with zipfile.ZipFile(jar_path, "r") as jf:
			for entry in jf.infolist():
				if not entry.filename.startswith("builtins/"):
					continue
				if entry.filename.endswith("/"):
					continue

				out_path = deps_dir / entry.filename
				resolved = out_path.resolve()
				if not str(resolved).startswith(str(deps_dir.resolve()) + os.sep):
					raise RuntimeError(f"Zip-slip detected: {entry.filename}")

				out_path.parent.mkdir(parents=True, exist_ok=True)
				with jf.open(entry) as src, open(out_path, "wb") as dst:
					shutil.copyfileobj(src, dst)

		print("  Builtins extracted.")
	finally:
		shutil.rmtree(tmp_dir, ignore_errors=True)


def fix_gitignore_builtins(project_root: Path) -> None:
	"""Replace bare 'builtins' with '/builtins' in .gitignore.

	A bare 'builtins' pattern matches at any depth, which causes tools
	to ignore .deps/builtins/.  The anchored '/builtins' only matches
	the top-level builtins/ directory.
	"""
	gitignore_path = project_root / ".gitignore"
	if not gitignore_path.exists():
		return

	text = gitignore_path.read_text(encoding="utf-8")
	lines = text.splitlines(keepends=True)
	changed = False

	for i, line in enumerate(lines):
		stripped = line.rstrip("\n\r")
		if stripped == "builtins" or stripped == "builtins/":
			lines[i] = "/" + stripped + line[len(stripped):]
			changed = True

	if changed:
		gitignore_path.write_text("".join(lines), encoding="utf-8")
		print("  Fixed .gitignore: 'builtins' -> '/builtins'")


def main() -> None:
	dry_run = "--dry-run" in sys.argv

	script_dir = Path(__file__).parent
	project_root = find_project_root(script_dir)
	game_project_path = project_root / "game.project"
	deps_dir = project_root / ".deps"

	game_project_text = game_project_path.read_text(encoding="utf-8")
	deps = parse_project_dependencies(game_project_text)

	print(f"Project root: {project_root}")
	print(f"Dependencies: {len(deps)}")
	print(f"Output dir: {deps_dir.relative_to(project_root)}")

	if dry_run:
		print("DRY-RUN: Will not download/delete/extract.")

	deps_dir.mkdir(parents=True, exist_ok=True)

	print()
	print("== .gitignore ==")
	if not dry_run:
		fix_gitignore_builtins(project_root)
	else:
		print("  Would fix 'builtins' -> '/builtins' in .gitignore")

	if deps:
		tmp_dir = Path(tempfile.mkdtemp(prefix="sync_deps_"))
		try:
			for i, url in enumerate(deps):
				print()
				print(f"== Dependency {i + 1}/{len(deps)} ==")
				print(url)

				if not dry_run:
					zip_path = tmp_dir / f"dep_{i:02d}.zip"
					download_to_file(url, zip_path)

					zip_root_prefix, project_text = find_game_project_in_zip(zip_path)
					include_dirs = parse_library_include_dirs(project_text)

					for d in include_dirs:
						assert_safe_include_dir(d)

					print(f"  include_dirs: {', '.join(include_dirs)}")

					delete_local_include_dirs(deps_dir, include_dirs)
					extract_selected_dirs(deps_dir, zip_path, zip_root_prefix, include_dirs)
				else:
					print("  Would download, inspect zip, read include_dirs, delete local folders, and extract.")
		finally:
			shutil.rmtree(tmp_dir, ignore_errors=True)
	else:
		print("\nNo [project] dependencies found in game.project, skipping library fetch.")

	print()
	print("== Builtins ==")
	if not dry_run:
		sync_builtins(deps_dir)
	else:
		print("  Would download and extract builtins to .deps/builtins")

	print()
	print("Done.")


if __name__ == "__main__":
	main()
