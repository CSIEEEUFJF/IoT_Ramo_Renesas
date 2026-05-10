#!/usr/bin/env python3
"""
Exporta perfis do Firebase (Firestore + opcionalmente Storage) para um bundle
offline de importacao usado pelo IoTRamoRenesas.

O bundle gerado nao cria dependencia em runtime com o Firebase. Ele traz os
campos normalizados do usuario e preserva metadados uteis para futuras etapas
de vinculacao de cartao e foto.
"""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import Any
from urllib.parse import unquote, urlparse


def normalize_uid(value: Any) -> str:
    text = str(value or "").strip().upper()
    return re.sub(r"[^0-9A-Z]", "", text)


def pick_string(record: dict[str, Any], *keys: str) -> str:
    for key in keys:
        value = record.get(key)
        if isinstance(value, str) and value.strip():
            return value.strip()
    return ""


def collect_cards(record: dict[str, Any]) -> list[str]:
    cards: list[str] = []

    for key in ("cards", "uids", "rfids", "cartoes", "cartões", "tags"):
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

    for key in ("uid", "rfid", "card", "cartao", "cartão", "tag"):
        uid = normalize_uid(record.get(key, ""))
        if uid and uid not in cards:
            cards.append(uid)

    return cards


def parse_chapter_roles(record: dict[str, Any]) -> tuple[str, str, dict[str, str]]:
    raw = record.get("chapterRoles")
    chapter_roles: dict[str, str] = {}

    if isinstance(raw, dict):
        for chapter, role in raw.items():
            chapter_text = str(chapter or "").strip()
            role_text = str(role or "").strip()
            if chapter_text and role_text:
                chapter_roles[chapter_text] = role_text

    if chapter_roles:
        primary_chapter = sorted(chapter_roles.keys())[0]
        return primary_chapter, chapter_roles[primary_chapter], chapter_roles

    chapter = pick_string(record, "chapter", "capitulo", "capítulo", "branch", "ramo")
    role = pick_string(record, "role", "cargo", "position", "title")
    return chapter, role, chapter_roles


def extract_storage_path(url_text: str) -> str:
    if not url_text:
        return ""

    parsed = urlparse(url_text)
    marker = "/o/"
    if marker not in parsed.path:
        return ""

    encoded_path = parsed.path.split(marker, 1)[1]
    return unquote(encoded_path)


def choose_photo_id(doc_id: str, record: dict[str, Any], bucket: Any) -> str:
    profile_picture_url = pick_string(record, "profilePictureUrl", "profile_picture_url")
    if profile_picture_url:
        storage_path = extract_storage_path(profile_picture_url)
        if storage_path:
            return storage_path

    explicit = pick_string(
        record,
        "photo_id",
        "photoId",
        "photo",
        "photoPath",
        "photo_path",
        "photoUrl",
        "photoURL",
        "avatar",
        "image",
        "imagePath",
        "image_path",
    )
    if explicit:
        return explicit

    if bucket is not None:
        candidate_prefixes = [
            f"profile_images/{doc_id}.jpg",
            f"profile_images/{doc_id}.jpeg",
            f"profile_images/{doc_id}.png",
            f"profile_pictures/{doc_id}",
            f"profile_pictures/{doc_id}.jpg",
            f"profile_pictures/{doc_id}.jpeg",
            f"profile_pictures/{doc_id}.png",
        ]
        for prefix in candidate_prefixes:
            blobs = list(bucket.list_blobs(prefix=prefix, max_results=1))
            if blobs:
                return blobs[0].name

    return ""


def build_profile(doc_id: str, record: dict[str, Any], bucket: Any) -> dict[str, Any] | None:
    name = pick_string(record, "name", "nome", "displayName", "fullName")
    chapter, role, chapter_roles = parse_chapter_roles(record)
    cards = collect_cards(record)
    profile_picture_url = pick_string(record, "profilePictureUrl", "profile_picture_url")

    if not name:
        return None

    return {
        "firebase_uid": doc_id,
        "name": name,
        "uid": cards[0] if cards else "",
        "role": role,
        "chapter": chapter,
        "photo_id": choose_photo_id(doc_id, record, bucket),
        "cards": cards,
        "chapter_roles": chapter_roles,
        "profile_picture_url": profile_picture_url,
        "phone_number": pick_string(record, "phoneNumber", "phone_number"),
        "email": pick_string(record, "email"),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description="Exporta perfis do Firebase para um bundle offline do firmware.")
    parser.add_argument("--service-account", required=True, type=Path, help="JSON da service account do Firebase")
    parser.add_argument("--collection", required=True, help="Colecao do Firestore com os usuarios")
    parser.add_argument("--output", required=True, type=Path, help="Arquivo JSON de saida")
    parser.add_argument("--storage-bucket", default="", help="Bucket do Firebase Storage, se quiser resolver fotos")
    args = parser.parse_args()

    import firebase_admin
    from firebase_admin import credentials, firestore, storage

    cred = credentials.Certificate(str(args.service_account))
    app_options = {"storageBucket": args.storage_bucket} if args.storage_bucket else None
    app = firebase_admin.initialize_app(cred, app_options)

    db = firestore.client(app=app)
    bucket = storage.bucket(app=app) if args.storage_bucket else None

    profiles: list[dict[str, Any]] = []
    for doc in db.collection(args.collection).stream():
        record = doc.to_dict() or {}
        profile = build_profile(doc.id, record, bucket)
        if profile is not None:
            profiles.append(profile)

    bundle = {
        "source": {
            "project_id": cred.project_id,
            "collection": args.collection,
            "storage_bucket": args.storage_bucket,
        },
        "count": len(profiles),
        "profiles": profiles,
    }

    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", encoding="utf-8", newline="\n") as fh:
        json.dump(bundle, fh, ensure_ascii=False, indent=2)
        fh.write("\n")

    print(f"Perfis exportados: {len(profiles)}")
    print(f"Arquivo gerado: {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
