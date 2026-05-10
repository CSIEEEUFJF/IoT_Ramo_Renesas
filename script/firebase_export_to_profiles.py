#!/usr/bin/env python3
"""
Converte um export JSON do Firebase para o formato de perfis usado pelo projeto.

Saida:
[
  {
    "name": "Fulano",
    "uid": "E35C051C",
    "role": "Presidente",
    "chapter": "Computer Society",
    "photo_id": "firebase:abc123",
    "cards": ["E35C051C", "A1B2C3D4"]
  }
]
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Any, Iterable


LIKELY_ROOT_KEYS = (
    "users",
    "usuarios",
    "members",
    "profiles",
    "pessoas",
    "data",
)

NAME_KEYS = ("name", "nome", "fullName", "displayName", "userName")
ROLE_KEYS = ("role", "cargo", "position", "titulo", "title")
CHAPTER_KEYS = ("chapter", "capitulo", "capítulo", "ieeeChapter", "branch", "ramo")
PHOTO_KEYS = ("photo_id", "photoId", "photo", "avatar", "image", "imageUrl", "photoUrl")
CARD_LIST_KEYS = ("cards", "uids", "rfids", "tags", "cartoes", "cartões", "badges")
CARD_SINGLE_KEYS = ("uid", "card", "rfid", "tag", "badge")


def normalize_uid(value: Any) -> str:
    text = str(value or "").strip().upper()
    return re.sub(r"[^0-9A-Z]", "", text)


def pick_string(record: dict[str, Any], keys: Iterable[str]) -> str:
    for key in keys:
        value = record.get(key)
        if isinstance(value, str) and value.strip():
            return value.strip()
    return ""


def extract_cards(record: dict[str, Any]) -> list[str]:
    cards: list[str] = []

    for key in CARD_LIST_KEYS:
        value = record.get(key)
        if isinstance(value, list):
            for item in value:
                uid = normalize_uid(item)
                if uid and uid not in cards:
                    cards.append(uid)
        elif isinstance(value, str):
            for part in value.split(","):
                uid = normalize_uid(part)
                if uid and uid not in cards:
                    cards.append(uid)

    for key in CARD_SINGLE_KEYS:
        uid = normalize_uid(record.get(key, ""))
        if uid and uid not in cards:
            cards.append(uid)

    return cards


def object_looks_like_user(record: Any) -> bool:
    if not isinstance(record, dict):
        return False
    if pick_string(record, NAME_KEYS):
        return True
    if extract_cards(record):
        return True
    return False


def descend_root(data: Any, root: str | None) -> Any:
    if not root:
        return data

    current = data
    for part in root.replace("\\", "/").split("/"):
        if not part:
            continue
        if isinstance(current, dict) and part in current:
            current = current[part]
        else:
            raise KeyError(f"caminho root inexistente: {root}")
    return current


def extract_records(data: Any) -> list[tuple[str, dict[str, Any]]]:
    if isinstance(data, list):
        return [(str(index), item) for index, item in enumerate(data) if isinstance(item, dict)]

    if isinstance(data, dict):
        for key in LIKELY_ROOT_KEYS:
            nested = data.get(key)
            if isinstance(nested, list):
                return [(str(index), item) for index, item in enumerate(nested) if isinstance(item, dict)]
            if isinstance(nested, dict) and all(isinstance(v, dict) for v in nested.values()):
                return [(str(k), v) for k, v in nested.items()]

        if all(isinstance(v, dict) for v in data.values()) and any(object_looks_like_user(v) for v in data.values()):
            return [(str(k), v) for k, v in data.items()]

        if object_looks_like_user(data):
            return [("root", data)]

    raise ValueError("Nao consegui localizar uma colecao de usuarios no JSON exportado.")


def convert_record(source_id: str, record: dict[str, Any], photo_prefix: str) -> dict[str, Any] | None:
    name = pick_string(record, NAME_KEYS)
    role = pick_string(record, ROLE_KEYS)
    chapter = pick_string(record, CHAPTER_KEYS)
    photo_value = pick_string(record, PHOTO_KEYS)
    cards = extract_cards(record)

    if not name or not cards:
        return None

    if not photo_value and source_id:
        photo_value = f"{photo_prefix}{source_id}"

    return {
        "name": name,
        "uid": cards[0],
        "role": role,
        "chapter": chapter,
        "photo_id": photo_value,
        "cards": cards,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description="Converte export do Firebase para perfis do IoTRamoRenesas.")
    parser.add_argument("input_json", type=Path, help="Arquivo JSON exportado do Firebase")
    parser.add_argument("output_json", type=Path, help="Arquivo de saida no formato do firmware")
    parser.add_argument("--root", default=None, help="Caminho opcional ate a colecao de usuarios, ex.: users")
    parser.add_argument("--photo-prefix", default="firebase:", help="Prefixo para photo_id quando nao houver foto")
    args = parser.parse_args()

    with args.input_json.open("r", encoding="utf-8-sig") as fh:
        raw = json.load(fh)

    rooted = descend_root(raw, args.root)
    records = extract_records(rooted)

    profiles: list[dict[str, Any]] = []
    for source_id, record in records:
        profile = convert_record(source_id, record, args.photo_prefix)
        if profile is not None:
            profiles.append(profile)

    args.output_json.parent.mkdir(parents=True, exist_ok=True)
    with args.output_json.open("w", encoding="utf-8", newline="\n") as fh:
        json.dump(profiles, fh, ensure_ascii=False, indent=2)
        fh.write("\n")

    print(f"Perfis convertidos: {len(profiles)}")
    print(f"Saida: {args.output_json}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
