---
name: defold-skill-maintain
description: "Maintains Defold agent skills. Use when asked to update link lists in api-fetch/docs-fetch/examples-fetch skills, create or update proto file references, or fetch proto schemas."
---

# Defold Skill Maintenance

Handles maintenance tasks for other Defold skills: updating link indexes and managing proto file references.

Read guide first about the best practices for Agent Skills: `references/The-Complete-Guide-to-Building-Skill-for-Claude.md`.

## Capability 1: Updating link lists in fetch skills

The `defold-api-fetch`, `defold-docs-fetch`, and `defold-examples-fetch` skills contain hardcoded link tables. These tables should be kept in sync with the official Defold index files.

### Index sources

- **API index**: `https://defold.com/llms/apis.md` — links to per-namespace API Markdown files
- **Manuals index**: `https://defold.com/llms/manuals.md` — links to per-manual Markdown files
- **Examples index**: `https://defold.com/llms/examples.md` — links to per-example Markdown files

### Procedure: update link lists

1. Fetch the relevant index page(s) by downloading the URL content.
2. Parse the Markdown content. Each index page contains links in the form `[Title](url)` grouped by sections.
3. Compare parsed links against the current SKILL.md of the target skill.
4. Update the SKILL.md link tables to match the index, preserving the table structure and the `## Usage` footer section.

### Target skill files

- `.agents/skills/defold-api-fetch/SKILL.md` — updated from `apis.md`
- `.agents/skills/defold-docs-fetch/SKILL.md` — updated from `manuals.md`
- `.agents/skills/defold-examples-fetch/SKILL.md` — updated from `examples.md`

### Rules

- Keep the YAML frontmatter (`---` block) unchanged.
- Keep the intro line `Fetch documentation from the links below (the URLs point to plain Markdown files).` unchanged.
- Keep the `## Usage` section at the bottom unchanged.
- Replace the link tables between frontmatter and Usage section with the parsed content from the index.
- Preserve section grouping and table formatting style that already exists in each skill.

## Capability 2: Managing proto file references

For detailed instructions on creating, updating, and maintaining proto file references for the `defold-proto-file-editing` skill, see `references/proto-reference-guide.md`.

## Scripts

- `scripts/fetch_proto.py` — downloads proto schemas from the stable Defold SDK into `.agents/skills/defold-skill-maintain/assets/proto/`. Run when proto schemas are missing or need updating: `python .agents/skills/defold-skill-maintain/scripts/fetch_proto.py`
