# Technical Report — Design and Deployment of an Ethernet-Enabled Embedded Access Control Platform on a Renesas Synergy MCU

Issue date: `2026-03-20`

Project: `IoTRamoRenesas`  
Target platform: `Renesas SK-S7G2`

## 1. Project objective

`IoTRamoRenesas` was developed to implement an embedded access-control system focused on technical demonstration, local operation, and remote administration. The system integrates:

- RFID-based authentication;
- local TFT display interface with touch input;
- administrative web interface;
- persistent storage in QSPI memory;
- user photo display;
- door and lighting control;
- access logging with network-synchronized timestamps.

The main goal was to transform an experimental codebase into a functional, demonstrable, and reusable platform for the IEEE UFJF Student Branch and the IEEE Computer Society.

## 2. Executive summary

Throughout development, the system evolved from an early baseline with UI, network, and persistence instability into an integrated platform featuring:

- working RFID reads;
- refined local interface;
- administrative web panel split into lightweight pages;
- user profiles with name, role, chapter, photo, multiple cards, and administrator privileges;
- photo upload and display support;
- persistence of profiles, photos, and logs in QSPI/FileX;
- automatic IP assignment through DHCP;
- time synchronization through NTP;
- administrative authentication through per-profile PINs.

The current result is suitable both for technical demonstrations and for future academic and institutional extensions.

## 3. Main implementation phases

### 3.1 Initial application stabilization

The first phase focused on recovering a reliable execution baseline:

- fixes for local UI crashes;
- touch and screen navigation adjustments;
- RFID read-path recovery;
- consolidation of the main authentication flow;
- restoration of a stable firmware baseline.

Associated milestone:

- `818dcf9` — `Stable app baseline without persistence worker`

### 3.2 Network recovery and stabilization

An extensive investigation was carried out to restore the board networking stack without breaking the rest of the system.

Main deliverables:

- recovery of a working HTTP server;
- reorganization of network bring-up;
- isolation of code paths that caused freezes;
- redesign of the web interface into smaller pages;
- reduction of latency and overly heavy HTTP responses.

Associated milestones:

- `b55418d` — `Restore working web network flow`
- `f633395` — `Network stack adjustments`
- `0e06671` — `Network stack adjustments`

### 3.3 Local UI consolidation

The local interface went through several iterations until it reached a format suitable for demonstration and actual use.

Main deliverables:

- idle screen aligned with project visual identity;
- access-result screen with photo, name, and profile data;
- local gear screen converted into a network/IP information screen;
- automatic return after result presentation;
- removal of unnecessary manual-return interactions;
- better handling of long names and improved photo-frame usage;
- visual dimming to about `30%` after `60s` of inactivity.

Associated milestone:

- `eec1cb5` — `Final UI, ready for profile pictures`

### 3.4 QSPI persistence

A central part of the work was making QSPI usable with FileX without compromising board boot.

Implemented items:

- delayed and safe media loading;
- automatic profile persistence in `users.json`;
- photo persistence in separate binary files;
- access log persistence in `access.log`;
- serialization of profiles with multiple cards;
- background-worker strategy to avoid blocking sensitive threads.

Persisted files currently include:

- `users.json`;
- `access.log`;
- `photo_XXXXXXXX.bin`.

Associated milestones:

- `6fee797` — `Display, card reading, and saving ready`
- `aa58035` — `Profile pictures saved in QSPI flash memory`

### 3.5 User profiles

The user model evolved from a simple `name + UID` entry into an expanded profile structure.

Currently supported fields:

- name;
- role;
- IEEE chapter;
- photo;
- administrator flag;
- administrator PIN;
- up to 4 cards per user.

The following were also implemented:

- paginated profile listing;
- profile creation;
- profile editing;
- profile removal;
- automatic persistence after save.

### 3.6 User photos

Support was added to associate photos with users and display them on the local UI after authentication.

Main deliverables:

- import of preprocessed offline photos;
- web photo upload in small chunks to avoid crashing the embedded server;
- local display of the photo after card read;
- visual fallback when no photo is available;
- persistence of web-uploaded photos in QSPI.

Associated milestone:

- `2d8db02` — `160x160 profile pictures working`

### 3.7 External user and data import

An offline import bridge was created to bring profiles from external systems without runtime dependency.

Delivered capabilities:

- JSON-based profile import through the web interface;
- background import processing;
- support for data exported from Firebase/Firestore through an offline flow;
- preparation of external images in the format used by the board.

This makes it possible to reuse existing user bases for demonstration and migration scenarios.

### 3.8 DHCP, NTP, and real date/time

After network stabilization, the following were integrated:

- DHCP for automatic IP acquisition;
- display of the current IP on the local UI;
- time synchronization through NTP;
- use of synchronized time in access logs.

As a result, the records now use absolute timestamps instead of depending only on system ticks.

Associated milestone:

- `5b233db` — `All systems go, only NTP for RTC missing`

### 3.9 Access logging

The system now maintains an access log suitable for audit and demonstration.

Delivered items:

- in-RAM log for real-time operation;
- dedicated `/access_log` web page;
- real timestamps after NTP synchronization;
- log persistence in QSPI;
- removal of automatic door-close events from the log, keeping only door-open events.

### 3.10 Profile-based administration

In the most recent phase, a profile-based administration layer was added without losing the legacy fallback.

Delivered items:

- profiles marked as administrators;
- dedicated PIN per administrator profile;
- web login through administrator PIN;
- local gear-screen authentication through administrator PIN;
- persistence of that PIN in `users.json`;
- preservation of the default `1234` PIN as a safe fallback.

Associated milestone:

- `38dfc4b` — `Add admin profiles with persisted PIN auth`

## 4. Implemented features

### 4.1 RFID and authentication

- UID reading through RC522;
- support for different UID sizes;
- lookup against persisted user database;
- access grant and denial flows;
- support for multiple cards linked to the same profile.

### 4.2 Local interface

- customized idle screen;
- PIN screen;
- IP/network screen;
- authorized/denied access screen;
- display of photo, name, and `(ROLE-CHAPTER)`;
- inactivity dimming.

### 4.3 Web interface

Main administrative routes:

- `/`
- `/login`
- `/admin_profiles`
- `/profile_form`
- `/upload_photo`
- `/import`
- `/access_log`
- `/door`

Capabilities:

- administrator login;
- profile CRUD;
- photo upload;
- JSON import;
- log visualization;
- door and light control.

### 4.4 Persistence

- profiles persisted in `users.json`;
- photos persisted in binary files;
- logs persisted in `access.log`;
- safe post-boot loading;
- automatic persistence of new profiles.

### 4.5 Network

- working HTTP server;
- lightweight pages to reduce freezes;
- DHCP;
- NTP.

### 4.6 Physical control

- door actuation by timed pulse;
- light actuation;
- local physical buttons;
- onboard LEDs turned off at boot.

## 5. Current architecture

Core modules:

- [`src/main.c`](./src/main.c)
- [`src/rfid.c`](./src/rfid.c)
- [`src/ui.c`](./src/ui.c)
- [`src/net.c`](./src/net.c)
- [`src/storage.c`](./src/storage.c)
- [`src/gpio.c`](./src/gpio.c)

Technologies and components:

- `ThreadX`
- `NetX / NetX Duo`
- `FileX`
- `QSPI`
- `RC522`
- `SX8654`
- `ILI9341`

## 6. Current functional state

In the current project state, the following are operational:

- card-based authentication;
- door opening;
- local UI;
- DHCP-based networking;
- web access log with absolute timestamps;
- profile creation, editing, and removal;
- photo upload;
- persistence of profiles, photos, and logs;
- profile-based administrative authentication.

## 7. Technical impact

From an engineering perspective, the project delivered:

- full integration between embedded hardware and web interface;
- transformation of a proof of concept into a demonstrable system;
- creation of an extensible baseline for future features;
- organization of the firmware into separate modules;
- technical documentation in Portuguese and English.

## 8. Recommended next steps

Although the system is functional, some natural future improvements include:

- optionally removing the `1234` fallback;
- strengthening administrator PIN policies;
- expanding profile export/import flows;
- improving batch photo handling;
- adding filters and export features to the access log;
- consolidating regression tests for web and storage.

## 9. Internal references

Related repository documents:

- [`README.md`](./README.md)
- [`README_EN.md`](./README_EN.md)
- [`CURRENT_STATE.md`](./CURRENT_STATE.md)
- [`CURRENT_STATE_EN.md`](./CURRENT_STATE_EN.md)
- [`NETWORK_NOTES.md`](./NETWORK_NOTES.md)
- [`NETWORK_NOTES_EN.md`](./NETWORK_NOTES_EN.md)

## 10. Milestone summary

- `818dcf9` — initial stable baseline;
- `b55418d` — web/network flow restoration;
- `6fee797` — RFID, display, persistence, and network working together;
- `eec1cb5` — final local UI baseline;
- `2d8db02` — high-resolution photo flow working;
- `aa58035` — photos persisted in QSPI;
- `7ed5264` — main code working except DHCP;
- `5b233db` — system nearly complete, missing RTC/NTP only;
- `38cfd68` — ready-state consolidation;
- `f633395` — network stack adjustments;
- `0e06671` — additional network stack adjustments;
- `38dfc4b` — administrator profiles with persisted PINs.

## 11. Conclusion

The development delivered a complete embedded access-control system, with its own visual identity, web management, flash-memory persistence, photo support, and real-time event recording.

The final result is suitable both for technical demonstrations and for future extensions by the IEEE UFJF Student Branch and the IEEE Computer Society.
