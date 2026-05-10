#!/usr/bin/env python3
"""
Gera um CSV para preencher manualmente os cartoes RFID dos perfis importados do
Firebase, sem alterar o firmware.
"""

from __future__ import annotations

import argparse
import csv
import json
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser(description="Gera template CSV para vincular cartoes RFID aos perfis importados.")
    parser.add_argument("--bundle", required=True, type=Path, help="Bundle gerado por export_firebase_profiles.py")
    parser.add_argument("--output", required=True, type=Path, help="CSV de saida")
    args = parser.parse_args()

    bundle = json.loads(args.bundle.read_text(encoding="utf-8"))
    profiles = bundle.get("profiles", [])

    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", encoding="utf-8", newline="") as fh:
        writer = csv.writer(fh)
        writer.writerow(
            [
                "firebase_uid",
                "name",
                "chapter",
                "role",
                "photo_id",
                "cards_csv",
            ]
        )
        for profile in profiles:
            writer.writerow(
                [
                    profile.get("firebase_uid", ""),
                    profile.get("name", ""),
                    profile.get("chapter", ""),
                    profile.get("role", ""),
                    profile.get("photo_id", ""),
                    ",".join(profile.get("cards", [])),
                ]
            )

    print(f"Template gerado: {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
