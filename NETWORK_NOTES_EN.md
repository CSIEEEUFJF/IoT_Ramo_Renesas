# Network Notes

Reference date: `2026-03-20`

This file records the current network state and the most important lessons learned from HTTP/Ethernet bring-up.

## Current state

- Ethernet working
- DHCP working
- HTTP working
- NTP working
- Administrative web interface working
- Current app mode: `HTTP only`

## What is currently part of the stable baseline

- Manual DHCP client in application code
- Local implementation in [`src/nxd_dhcp_client.c`](./src/nxd_dhcp_client.c)
- `packet_pool`, `ip`, and `http_server` initialized before web usage
- Small web pages instead of one heavy single page
- `static` buffers in heavier routes
- HTTP callback serialization to avoid concurrent buffer reuse
- JSON import in background
- Photo upload in small tiles

## Important historical findings

The network problems found during the project were more related to:

- Excessive stack use in the HTTP thread
- HTML pages that were too large
- Large or poorly terminated HTTP responses
- Heavy FileX operations inside web requests
- Inadequate buffer reuse in callback

Than to:

- Network cable
- Switch
- PHY fine tuning

## Current bring-up

Flow summary in [`src/net.c`](./src/net.c):

1. Initialize `packet_pool`, `ip`, and `http_server`
2. Create DHCP client
3. Request lease and wait for valid IP
4. Mark network as ready
5. Start HTTP
6. Try to sync time with NTP
7. Perform periodic NTP re-sync

## NTP

Current priority:

1. NTP server advertised by DHCP (option 42)
2. Fallback:
   - `129.6.15.28`
   - `129.6.15.29`

## Most sensitive routes

Historically, these were the most sensitive:

- Profile listing
- JSON import
- Photo upload

Current mitigations:

- `/admin_profiles` paginated
- `/import` processed in background
- `/upload_photo` uses browser preprocessing and tile upload

## Caution when changing network logic

Avoid changing without need:

- HTML buffer sizes
- HTTP response strategy
- HTTP body parsing inside callback
- Network initialization order
- Manual DHCP bring-up
- Network thread stack

## Main files

- [`src/net.c`](./src/net.c)
- [`src/net.h`](./src/net.h)
- [`src/nxd_dhcp_client.c`](./src/nxd_dhcp_client.c)
- [`src/main.c`](./src/main.c)
- [`src/storage.c`](./src/storage.c)
- [`src/synergy_gen/common_data.c`](./src/synergy_gen/common_data.c)

## Reference

For complete project documentation, use:

- [`README_EN.md`](./README_EN.md)
