#!/usr/bin/env python3
"""
Baixa fotos de perfil a partir de um bundle exportado do Firebase e gera uma
versao reduzida para uso futuro no firmware.
"""

from __future__ import annotations

import argparse
import io
import json
from pathlib import Path
from typing import Any


def make_square_thumbnail(image: Any, size: int) -> Any:
    from PIL import Image, ImageOps, ImageFile

    ImageFile.LOAD_TRUNCATED_IMAGES = True

    rgb = image.convert("RGB")
    fitted = ImageOps.fit(rgb, (size, size), method=Image.Resampling.LANCZOS, centering=(0.5, 0.5))
    return fitted


def main() -> int:
    parser = argparse.ArgumentParser(description="Baixa e reduz fotos de perfil do bundle Firebase.")
    parser.add_argument("--service-account", required=True, type=Path, help="JSON da service account")
    parser.add_argument("--bundle", required=True, type=Path, help="Bundle gerado por export_firebase_profiles.py")
    parser.add_argument("--output-dir", required=True, type=Path, help="Pasta de saida das fotos")
    parser.add_argument("--size", type=int, default=120, help="Tamanho final quadrado em pixels")
    parser.add_argument("--quality", type=int, default=82, help="Qualidade JPEG final")
    args = parser.parse_args()

    import firebase_admin
    from firebase_admin import credentials, storage
    from PIL import Image

    bundle = json.loads(args.bundle.read_text(encoding="utf-8"))
    bucket_name = str(bundle.get("source", {}).get("storage_bucket", "")).strip()
    if not bucket_name:
        raise SystemExit("Bundle sem storage_bucket.")

    cred = credentials.Certificate(str(args.service_account))
    app = firebase_admin.initialize_app(cred, {"storageBucket": bucket_name})
    bucket = storage.bucket(app=app)

    originals_dir = args.output_dir / "original"
    downscaled_dir = args.output_dir / "downscaled"
    originals_dir.mkdir(parents=True, exist_ok=True)
    downscaled_dir.mkdir(parents=True, exist_ok=True)

    manifest: list[dict[str, Any]] = []

    for profile in bundle.get("profiles", []):
        photo_id = str(profile.get("photo_id", "")).strip()
        firebase_uid = str(profile.get("firebase_uid", "")).strip()
        if not photo_id or not firebase_uid:
            continue

        blob = bucket.blob(photo_id)
        if not blob.exists():
            manifest.append(
                {
                    "firebase_uid": firebase_uid,
                    "name": profile.get("name", ""),
                    "photo_id": photo_id,
                    "status": "missing",
                }
            )
            continue

        raw_bytes = blob.download_as_bytes()
        suffix = Path(photo_id).suffix.lower() or ".jpg"
        original_path = originals_dir / f"{firebase_uid}{suffix}"
        original_path.write_bytes(raw_bytes)

        image = Image.open(io.BytesIO(raw_bytes))
        thumb = make_square_thumbnail(image, args.size)
        downscaled_path = downscaled_dir / f"{firebase_uid}.jpg"
        thumb.save(downscaled_path, format="JPEG", quality=args.quality, optimize=True)

        manifest.append(
            {
                "firebase_uid": firebase_uid,
                "name": profile.get("name", ""),
                "photo_id": photo_id,
                "original_file": str(original_path.name),
                "downscaled_file": str(downscaled_path.name),
                "width": args.size,
                "height": args.size,
                "status": "ok",
            }
        )

    manifest_path = args.output_dir / "photo_manifest.json"
    manifest_path.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")

    print(f"Fotos processadas: {sum(1 for item in manifest if item['status'] == 'ok')}")
    print(f"Manifesto: {manifest_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
