#!/usr/bin/env python3
"""Generates the OTA manifest for a directory of firmware binaries.

Expects files named esPINDMD-<environment>.bin, the environment is the
PlatformIO environment the firmware was built for. The URLs in the manifest
are relative, so the directory can be served by GitHub Releases as well as
by any local webserver without changes.

Usage: manifest.py <directory> <version> [--release-url URL] [--summary TEXT]
"""

import argparse
import hashlib
import json
import pathlib

PREFIX = "esPINDMD-"


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("directory", type=pathlib.Path)
    parser.add_argument("version")
    parser.add_argument("--release-url", default="")
    parser.add_argument("--summary", default="")
    args = parser.parse_args()

    version = args.version[1:] if args.version.startswith("v") else args.version
    builds = {}
    for binary in sorted(args.directory.glob(PREFIX + "*.bin")):
        data = binary.read_bytes()
        builds[binary.stem[len(PREFIX):]] = {
            "url": binary.name,
            "size": len(data),
            "sha256": hashlib.sha256(data).hexdigest(),
        }
    if not builds:
        raise SystemExit(f"No {PREFIX}*.bin files found in {args.directory}")

    manifest = {
        "version": version,
        "release_url": args.release_url,
        # Home Assistant limits the summary to 255 characters
        "release_summary": args.summary[:255],
        "builds": builds,
    }
    path = args.directory / "manifest.json"
    path.write_text(json.dumps(manifest, indent=2) + "\n")
    print(f"Wrote {path} for version {version}: {', '.join(builds)}")


if __name__ == "__main__":
    main()
