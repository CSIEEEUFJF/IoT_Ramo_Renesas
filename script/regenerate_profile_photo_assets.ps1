param(
    [string]$Root = "C:\Users\killl\e2_studio\workspace\IoTRamoRenesas",
    [int]$Size = 120,
    [int]$Quality = 82
)

Add-Type -AssemblyName System.Drawing
$ErrorActionPreference = "Stop"

$bundlePath = Join-Path $Root "script\firebase_bundle\firebase_profiles.json"
$manifestPath = Join-Path $Root "script\firebase_bundle\photos\photo_manifest.json"
$originalDir = Join-Path $Root "script\firebase_bundle\photos\original"
$downscaledDir = Join-Path $Root "script\firebase_bundle\photos\downscaled"
$outputPath = Join-Path $Root "src\assets\profile_photo_assets.h"

$bundle = Get-Content -Raw -Path $bundlePath | ConvertFrom-Json
$manifest = Get-Content -Raw -Path $manifestPath | ConvertFrom-Json
$manifestByUid = @{}

foreach ($item in $manifest)
{
    if ($item.status -eq "ok")
    {
        $manifestByUid[[string]$item.firebase_uid] = $item
    }
}

function Get-SafeSymbol([string]$Value)
{
    return ($Value -replace "[^0-9A-Za-z_]", "_")
}

function Convert-Rgb565([System.Drawing.Color]$Color)
{
    return ((($Color.R -band 0xF8) -shl 8) -bor (($Color.G -band 0xFC) -shl 3) -bor ($Color.B -shr 3))
}

$lines = New-Object System.Collections.Generic.List[string]
$lines.Add("#ifndef PROFILE_PHOTO_ASSETS_H") | Out-Null
$lines.Add("#define PROFILE_PHOTO_ASSETS_H") | Out-Null
$lines.Add("") | Out-Null
$lines.Add("#include <stdint.h>") | Out-Null
$lines.Add("#include <stddef.h>") | Out-Null
$lines.Add("") | Out-Null
$lines.Add("typedef struct") | Out-Null
$lines.Add("{") | Out-Null
$lines.Add("    const char *photo_id;") | Out-Null
$lines.Add("    uint16_t width;") | Out-Null
$lines.Add("    uint16_t height;") | Out-Null
$lines.Add("    const uint16_t *pixels;") | Out-Null
$lines.Add("} profile_photo_asset_t;") | Out-Null
$lines.Add("") | Out-Null

$assetEntries = New-Object System.Collections.Generic.List[string]
$assetCount = 0

foreach ($profile in $bundle.profiles)
{
    $firebaseUid = [string]$profile.firebase_uid
    $photoId = [string]$profile.photo_id

    if ([string]::IsNullOrWhiteSpace($firebaseUid) -or [string]::IsNullOrWhiteSpace($photoId))
    {
        continue
    }

    if (-not $manifestByUid.ContainsKey($firebaseUid))
    {
        continue
    }

    $entry = $manifestByUid[$firebaseUid]
    $originalFile = [string]$entry.original_file

    if ([string]::IsNullOrWhiteSpace($originalFile))
    {
        continue
    }

    $imagePath = Join-Path $originalDir $originalFile
    if (-not (Test-Path $imagePath))
    {
        continue
    }

    $source = [System.Drawing.Bitmap]::new($imagePath)
    try
    {
        $cropSize = [Math]::Min($source.Width, $source.Height)
        $srcX = [int](($source.Width - $cropSize) / 2)
        $srcY = [int](($source.Height - $cropSize) / 2)
        $dest = [System.Drawing.Bitmap]::new($Size, $Size)

        try
        {
            $gfx = [System.Drawing.Graphics]::FromImage($dest)
            try
            {
                $gfx.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
                $gfx.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality
                $gfx.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
                $gfx.CompositingQuality = [System.Drawing.Drawing2D.CompositingQuality]::HighQuality
                $gfx.DrawImage(
                    $source,
                    [System.Drawing.Rectangle]::new(0, 0, $Size, $Size),
                    [System.Drawing.Rectangle]::new($srcX, $srcY, $cropSize, $cropSize),
                    [System.Drawing.GraphicsUnit]::Pixel
                )
            }
            finally
            {
                $gfx.Dispose()
            }

            $jpgEncoder = [System.Drawing.Imaging.ImageCodecInfo]::GetImageEncoders() |
                Where-Object { $_.MimeType -eq "image/jpeg" }
            $encParams = [System.Drawing.Imaging.EncoderParameters]::new(1)
            $encParams.Param[0] = [System.Drawing.Imaging.EncoderParameter]::new(
                [System.Drawing.Imaging.Encoder]::Quality,
                [long]$Quality
            )
            $downscaledPath = Join-Path $downscaledDir ($firebaseUid + ".jpg")
            $dest.Save($downscaledPath, $jpgEncoder, $encParams)

            $symbol = "g_profile_photo_" + (Get-SafeSymbol $firebaseUid)
            $lines.Add("static const uint16_t ${symbol}[${Size}U * ${Size}U] = {") | Out-Null

            $chunk = New-Object System.Collections.Generic.List[string]
            for ($y = 0; $y -lt $Size; $y++)
            {
                for ($x = 0; $x -lt $Size; $x++)
                {
                    $rgb565 = Convert-Rgb565 ($dest.GetPixel($x, $y))
                    $chunk.Add(("0x{0:X4}" -f $rgb565)) | Out-Null
                    if ($chunk.Count -eq 12)
                    {
                        $lines.Add("    " + (($chunk.ToArray()) -join ",") + ",") | Out-Null
                        $chunk.Clear()
                    }
                }
            }

            if ($chunk.Count -gt 0)
            {
                $lines.Add("    " + (($chunk.ToArray()) -join ",") + ",") | Out-Null
            }

            $lines.Add("};") | Out-Null
            $lines.Add("") | Out-Null
            $assetEntries.Add(('    {{"{0}", (uint16_t){1}U, (uint16_t){1}U, {2}}},' -f $photoId, $Size, $symbol)) | Out-Null

            $entry.width = $Size
            $entry.height = $Size
            $entry.downscaled_file = $firebaseUid + ".jpg"
            $assetCount++
        }
        finally
        {
            $dest.Dispose()
        }
    }
    finally
    {
        $source.Dispose()
    }
}

$lines.Add("static const profile_photo_asset_t g_profile_photo_assets[] = {") | Out-Null
foreach ($assetEntry in $assetEntries)
{
    $lines.Add($assetEntry) | Out-Null
}
$lines.Add("};") | Out-Null
$lines.Add("") | Out-Null
$lines.Add("#define PROFILE_PHOTO_ASSET_COUNT (sizeof(g_profile_photo_assets) / sizeof(g_profile_photo_assets[0]))") | Out-Null
$lines.Add("") | Out-Null
$lines.Add("#endif") | Out-Null
$lines.Add("") | Out-Null

[System.IO.File]::WriteAllText($outputPath, ($lines -join "`n"), [System.Text.UTF8Encoding]::new($false))
[System.IO.File]::WriteAllText($manifestPath, (($manifest | ConvertTo-Json -Depth 8) + "`n"), [System.Text.UTF8Encoding]::new($false))

Write-Output ("Assets gerados: " + $assetCount)
Write-Output ("Header: " + $outputPath)
