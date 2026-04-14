"""Download Defold Asset Store JSON and generate a compact TSV index.

Usage:
    python .agents/skills/defold-assets-search/scripts/generate_index.py

Output:
    .agents/skills/defold-assets-search/assets/dependencies_index.tsv
"""

import json
import os
import urllib.request

SOURCE_URL = "https://insality.github.io/asset-store/dependencies_store.json"
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
OUTPUT_DIR = os.path.join(SCRIPT_DIR, os.pardir, "assets")
OUTPUT_FILE = os.path.join(OUTPUT_DIR, "dependencies_index.tsv")


def main() -> None:
    print(f"Downloading {SOURCE_URL} ...")
    with urllib.request.urlopen(SOURCE_URL) as resp:
        raw = json.loads(resp.read().decode("utf-8"))

    # The JSON has an "items" key containing the list
    data: list[dict] = raw.get("items", raw) if isinstance(raw, dict) else raw

    os.makedirs(OUTPUT_DIR, exist_ok=True)

    # Exclude unlisted entries
    entries = [e for e in data if not e.get("unlisted", False)]

    # Sort by stars descending (None → 0)
    entries.sort(key=lambda e: e.get("stars") or 0, reverse=True)

    header = "id\ttitle\tauthor\tdescription\ttags\tstars\tapi\texample_code\tmanifest_url\tlatest_zip"
    lines: list[str] = [header]

    for e in entries:
        latest_zip = ""
        content = e.get("content") or []
        if content:
            latest_zip = content[-1]

        tags_str = ", ".join(e.get("tags") or [])
        desc = (e.get("description") or "").replace("\t", " ").replace("\n", " ")

        line = "\t".join([
            e.get("id") or "",
            e.get("title") or "",
            e.get("author") or "",
            desc,
            tags_str,
            str(e.get("stars") or 0),
            e.get("api") or "",
            e.get("example_code") or "",
            e.get("manifest_url") or "",
            latest_zip,
        ])
        lines.append(line)

    with open(OUTPUT_FILE, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")

    print(f"Generated {OUTPUT_FILE} with {len(entries)} entries.")


if __name__ == "__main__":
    main()
