#!/usr/bin/env python3

import json
import subprocess
import sys
from pathlib import Path


ENDPOINT = "https://espverse.com/api/v1/firmware/publish"
PROJECT_SLUG = "chronos-navio"
MANIFEST_PATH = Path(__file__).resolve().parent.parent / "firmware" / "firmware.json"

CHIP_FAMILIES = {
    "esp32": "ESP32",
    "esp32c2": "ESP32-C2",
    "esp32c3": "ESP32-C3",
    "esp32c5": "ESP32-C5",
    "esp32c6": "ESP32-C6",
    "esp32h2": "ESP32-H2",
    "esp32p4": "ESP32-P4",
    "esp32s2": "ESP32-S2",
    "esp32s3": "ESP32-S3",
}

REQUIRED_FIELDS = ("id", "file", "address", "chip", "version")


def load_manifest(path):
    try:
        with path.open(encoding="utf-8") as manifest_file:
            manifest = json.load(manifest_file)
    except FileNotFoundError:
        raise ValueError(f"Manifest not found: {path}") from None
    except json.JSONDecodeError as error:
        raise ValueError(f"Invalid JSON in {path}: {error}") from error

    if not isinstance(manifest, dict):
        raise ValueError(f"Manifest must contain an object of board entries: {path}")

    return manifest


def chip_family(chip):
    normalized = str(chip).strip().lower().replace("-", "").replace("_", "")
    try:
        return CHIP_FAMILIES[normalized]
    except KeyError:
        supported = ", ".join(sorted(CHIP_FAMILIES))
        raise ValueError(
            f"Unsupported chip '{chip}'. Supported manifest values: {supported}"
        ) from None


def validate_entry(key, entry, manifest_dir):
    if not isinstance(entry, dict):
        raise ValueError(f"Entry '{key}' must be an object")

    missing = [field for field in REQUIRED_FIELDS if not entry.get(field)]
    if missing:
        raise ValueError(f"Entry '{key}' is missing fields: {', '.join(missing)}")

    firmware_path = manifest_dir / entry["file"]
    if not firmware_path.is_file():
        raise ValueError(f"Firmware file for '{key}' not found: {firmware_path}")

    return firmware_path, chip_family(entry["chip"])


def publish(api_key, entry, firmware_path, family):
    return [
        "curl",
        "--fail-with-body",
        "--show-error",
        "--silent",
        "-X",
        "POST",
        ENDPOINT,
        "-H",
        f"Authorization: Bearer {api_key}",
        "-F",
        f"project_slug={PROJECT_SLUG}",
        "-F",
        f"version={entry['version']}",
        "-F",
        f"board_slug={entry['id']}",
        "-F",
        f"chip_family={family}",
        "-F",
        f"flash_offset={entry['address']}",
        "-F",
        "status=published",
        "-F",
        "allow_unspecified_board=0",
        "-F",
        f"firmware=@{firmware_path};type=application/octet-stream",
    ]


def main():
    if len(sys.argv) != 2:
        print(f"Usage: {Path(sys.argv[0]).name} <api-key>", file=sys.stderr)
        return 2

    api_key = sys.argv[1]

    try:
        manifest = load_manifest(MANIFEST_PATH)
        validated = [
            (entry, *validate_entry(key, entry, MANIFEST_PATH.parent))
            for key, entry in manifest.items()
        ]
    except ValueError as error:
        print(f"Error: {error}", file=sys.stderr)
        return 1

    for entry, firmware_path, family in validated:
        print(
            f"Publishing {entry['id']} {entry['version']} "
            f"({family}, {entry['address']})"
        )

        command = publish(api_key, entry, firmware_path, family)
        result = subprocess.run(command, text=True, capture_output=True)
        if result.stdout:
            print(result.stdout.rstrip())
        if result.returncode:
            if result.stderr:
                print(result.stderr.rstrip(), file=sys.stderr)
            print(f"Error: Failed to publish {entry['id']}", file=sys.stderr)
            return result.returncode

    print(f"Published {len(validated)} firmware file(s).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
