# Current Project State

Reference date: `2026-03-20`

This file is a quick summary. The main and more complete documentation is in [`README_EN.md`](./README_EN.md).

## Current functional baseline

Today the codebase is running with:

- RFID working
- Local UI working
- Card authentication working
- Administrative web panel working
- Profiles with name, role, chapter, photo, and multiple cards
- DHCP enabled
- NTP enabled
- Profile persistence in QSPI
- Persistence of photos uploaded from web in QSPI
- Persistence of access logs in QSPI

## Important characteristics

- Local and web admin PIN: `1234`
- Automatic return from result screen: `10s`
- Screen dimming after `60s`
- Brightness reduced to about `30%`
- Log records door opening, but not automatic closing
- Web runs in `HTTP only` mode

## Flows considered stable

- Authorized/denied card read
- Create, edit, and remove profile from the web UI
- Pagination at `/admin_profiles/<page>`
- Profile import via JSON
- Photo upload by tiles
- Access log visualization
- Door and light control via web

## Still sensitive points

- Any aggressive increase in HTML page weight
- Very large HTTP uploads
- Changes in FileX/QSPI boot sequence
- Large changes in `users.json` parser/load
- Large changes in network bring-up

## Most important files

- [`src/main.c`](./src/main.c)
- [`src/net.c`](./src/net.c)
- [`src/storage.c`](./src/storage.c)
- [`src/storage.h`](./src/storage.h)
- [`src/ui.c`](./src/ui.c)
- [`src/rfid.c`](./src/rfid.c)
- [`src/gpio.c`](./src/gpio.c)
- [`src/nxd_dhcp_client.c`](./src/nxd_dhcp_client.c)

## Current build

Main output:

- [`Debug/IoTRamoRenesas.elf`](./Debug/IoTRamoRenesas.elf)

## Practical recommendation

Before changing network, FileX, upload, or profile parser logic, re-read:

- [`README_EN.md`](./README_EN.md)
- [`NETWORK_NOTES_EN.md`](./NETWORK_NOTES_EN.md)
