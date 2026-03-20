#!/usr/bin/env python3
"""
Gera um header C com fotos de perfil em RGB565 para uso direto na UI.
"""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path


def symbol_from_uid(firebase_uid: str) -> str:
    return re.sub(r"[^0-9A-Za-z_]", "_", firebase_uid)


def rgb888_to_rgb565(red: int, green: int, blue: int) -> int:
    return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3)


def main() -> int:
    parser = argparse.ArgumentParser(description="Gera header C de fotos de perfil em RGB565.")
    parser.add_argument("--bundle", required=True, type=Path, help="Bundle firebase_profiles.json")
    parser.add_argument("--manifest", required=True, type=Path, help="photo_manifest.json")
    parser.add_argument("--photos-dir", required=True, type=Path, help="Pasta com fotos reduzidas")
    parser.add_argument("--output", required=True, type=Path, help="Header C de saida")
    args = parser.parse_args()

    from PIL import Image

    bundle = json.loads(args.bundle.read_text(encoding="utf-8"))
    manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
    manifest_by_uid = {str(item.get("firebase_uid", "")): item for item in manifest if item.get("status") == "ok"}

    output_lines: list[str] = []
    output_lines.append("#ifndef PROFILE_PHOTO_ASSETS_H")
    output_lines.append("#define PROFILE_PHOTO_ASSETS_H")
    output_lines.append("")
    output_lines.append("#include <stdint.h>")
    output_lines.append("#include <stddef.h>")
    output_lines.append("")
    output_lines.append("typedef struct")
    output_lines.append("{")
    output_lines.append("    const char *photo_id;")
    output_lines.append("    uint16_t width;")
    output_lines.append("    uint16_t height;")
    output_lines.append("    const uint16_t *pixels;")
    output_lines.append("} profile_photo_asset_t;")
    output_lines.append("")

    asset_symbols: list[tuple[str, str]] = []

    for profile in bundle.get("profiles", []):
        firebase_uid = str(profile.get("firebase_uid", "")).strip()
        photo_id = str(profile.get("photo_id", "")).strip()
        if not firebase_uid or not photo_id:
            continue

        entry = manifest_by_uid.get(firebase_uid)
        if entry is None:
            continue

        downscaled_file = str(entry.get("downscaled_file", "")).strip()
        if not downscaled_file:
            continue

        image_path = args.photos_dir / downscaled_file
        if not image_path.exists():
            continue

        image = Image.open(image_path).convert("RGB")
        width, height = image.size
        symbol = f"g_profile_photo_{symbol_from_uid(firebase_uid)}"
        asset_symbols.append((photo_id, symbol))

        pixels = list(image.getdata())
        rgb565_values = [rgb888_to_rgb565(r, g, b) for (r, g, b) in pixels]

        output_lines.append(f"static const uint16_t {symbol}[{width}U * {height}U] = {{")
        row_size = 12
        for offset in range(0, len(rgb565_values), row_size):
            chunk = rgb565_values[offset:offset + row_size]
            output_lines.append("    " + ",".join(f"0x{value:04X}" for value in chunk) + ",")
        output_lines.append("};")
        output_lines.append("")

    output_lines.append("static const profile_photo_asset_t g_profile_photo_assets[] = {")
    for photo_id, symbol in asset_symbols:
        output_lines.append(f'    {{"{photo_id}", (uint16_t){width}U, (uint16_t){height}U, {symbol}}},')
    output_lines.append("};")
    output_lines.append("")
    output_lines.append("#define PROFILE_PHOTO_ASSET_COUNT (sizeof(g_profile_photo_assets) / sizeof(g_profile_photo_assets[0]))")
    output_lines.append("")
    output_lines.append("#endif")
    output_lines.append("")

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text("\n".join(output_lines), encoding="utf-8", newline="\n")

    print(f"Assets gerados: {len(asset_symbols)}")
    print(f"Header: {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
