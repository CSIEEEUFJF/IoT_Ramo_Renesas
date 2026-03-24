# Institutional Report
## Design and Deployment of an Ethernet-Enabled Embedded Access Control Platform on a Renesas Synergy MCU

**Date:** `2026-03-20`  
**Branch:** `Universidade Federal de Juiz de Fora IEEE Student Branch` 
**Capítulo** `IEEE Computer Society Universidade Federal de Juiz de Fora Student Branch Chapter and IEEE Robotics and Automation Society Universidade Federal de Juiz de Fora Student Branch`
**Project:** `Design and Deployment of an Ethernet-Enabled Embedded Access Control Platform on a Renesas Synergy MCU`  
**Platform:** `Renesas Synergy S7G2 Starter Kit`

## Context

`Design and Deployment of an Ethernet-Enabled Embedded Access Control Platform on a Renesas Synergy MCU` started as an embedded access-control idea. The initial goal was straightforward: use the `Renesas SK-S7G2` board to read RFID cards, show the result on a local display, and allow some level of administration over the network. As development progressed, the scope grew and the project moved well beyond a simple proof of concept.

At this point, the system combines RFID authentication, a local touch-based interface, an administrative web panel, QSPI-based persistence, user photo display, door control, and event logging. In practice, what began as an experimental baseline now behaves like a complete embedded system ready for demonstration.

## What was built

The work was concentrated around four main fronts.

The first was firmware stabilization. It was necessary to recover the RFID read path, reorganize the local UI, fix freezes, and build a reliable execution baseline. This stage mattered because the project had several sensitive areas, especially around interface handling, networking, and persistent storage access.

The second front was networking. The web interface was redesigned into smaller and lighter pages that fit the limits of the embedded HTTP server. That made it possible to support profile creation, editing, and administration without destabilizing the rest of the firmware. DHCP was also integrated for automatic IP assignment, and NTP was added for time synchronization.

The third front was persistence. The system now stores profiles, photos, and logs in QSPI through FileX without breaking board boot. This required careful handling, since the timing of media access directly affected system stability. The result was a working persistence flow for `users.json`, `access.log`, and binary photo files.

Finally, the user model itself was expanded. The project no longer treats a user as only a UID plus a name. Profiles now support name, role, IEEE chapter, photo, multiple cards, and administrator privileges. PIN-based authentication was also added for administrator profiles in both the web interface and the local interface.

## Current state

As of this report, the project is functional for demonstration and controlled use. The main operational features are:

- RFID card reading and access validation;
- door opening on authorized authentication;
- local interface with idle, PIN, and result screens;
- administrative web interface for profile management;
- profile creation, editing, and removal;
- profile photo upload and display;
- administrator profiles with their own PIN;
- access logging with network-synchronized timestamps;
- persistence of profiles, photos, and logs in QSPI memory.

It is also worth noting that the system already supports profile import through JSON and can reuse data originating from an external database, which opens the door for integration with existing branch workflows.

## Why this matters

The value of `Design and Deployment of an Ethernet-Enabled Embedded Access Control Platform on a Renesas Synergy MCU` comes from the fact that it brings together, in a single system, several engineering layers that are often treated separately in academic projects. Here, they had to work together at the same time: firmware, graphical interface, authentication, networking, persistent storage, and physical board operation.

For the IEEE UFJF Student Branch, this means having a concrete baseline for technical demonstrations, training of new members, and continued development. For the IEEE Computer Society, the project is directly connected to core areas such as embedded systems, embedded networking, low-level software, and hardware-software integration.

## Closing remarks

`Design and Deployment of an Ethernet-Enabled Embedded Access Control Platform on a Renesas Synergy MCU` has reached a point where it can already be presented as a mature technical delivery. There is still room for future improvement, as in any living project, but the core system is built, integrated, and working.

More than an isolated experiment, the project has become a real embedded platform that demonstrates technical capability, development discipline, and clear potential for continuity within IEEE UFJF activities.
