#!/usr/bin/env python3
"""
Monta um users.json compativel com o firmware a partir do bundle offline do
Firebase e de um CSV opcional com os cartoes RFID.
"""

from __future__ import annotations

import argparse
import csv
import json
import re
from pathlib import Path
from typing import Any


def normalize_uid(value: Any) -> str:
    text = str(value or "").strip().upper()
    return re.sub(r"[^0-9A-Z]", "", text)


def parse_cards_csv(value: str) -> list[str]:
    cards: list[str] = []
    for part in str(value or "").split(","):
        uid = normalize_uid(part)
        if uid and uid not in cards:
            cards.append(uid)
    return cards


def load_bindings(path: Path | None) -> dict[str, list[str]]:
    if path is None or not path.exists():
        return {}

    bindings: dict[str, list[str]] = {}
    with path.open("r", encoding="utf-8-sig", newline="") as fh:
        reader = csv.DictReader(fh)
        for row in reader:
            firebase_uid = str(row.get("firebase_uid", "")).strip()
            if not firebase_uid:
                continue
            bindings[firebase_uid] = parse_cards_csv(row.get("cards_csv", ""))
    return bindings


def main() -> int:
    parser = argparse.ArgumentParser(description="Gera users.json do firmware a partir do bundle Firebase.")
    parser.add_argument("--bundle", required=True, type=Path, help="Bundle gerado por export_firebase_profiles.py")
    parser.add_argument("--bindings", type=Path, default=None, help="CSV opcional com os cartoes RFID preenchidos")
    parser.add_argument("--output", required=True, type=Path, help="Arquivo users.json de saida")
    parser.add_argument("--include-cardless", action="store_true", help="Inclui perfis sem cartao no JSON de saida")
    args = parser.parse_args()

    bundle = json.loads(args.bundle.read_text(encoding="utf-8"))
    bindings = load_bindings(args.bindings)

    users = []
    for profile in bundle.get("profiles", []):
        firebase_uid = str(profile.get("firebase_uid", "")).strip()
        cards = bindings.get(firebase_uid, [])
        if not cards:
            cards = [normalize_uid(card) for card in profile.get("cards", []) if normalize_uid(card)]

        cards = [card for index, card in enumerate(cards) if card and card not in cards[:index]]
        if not cards and not args.include_cardless:
            continue

        uid = cards[0] if cards else ""
        user = {
            "name": profile.get("name", ""),
            "uid": uid,
            "role": profile.get("role", ""),
            "chapter": profile.get("chapter", ""),
            "photo_id": profile.get("photo_id", ""),
            "cards": cards,
        }
        users.append(user)

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(users, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")

    print(f"Users gerados: {len(users)}")
    print(f"Arquivo: {args.output}")
    if (len(users) == 0) and (not args.include_cardless):
        print("Aviso: nenhum usuario foi emitido porque o CSV de bindings ainda nao tem cartoes RFID preenchidos.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
