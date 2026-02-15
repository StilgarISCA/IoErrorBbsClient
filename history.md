# Changelog (Legacy Rollup)

This file merges the old docs that used to be in `old/`:

- `README-1.5`
- `README-2.2`
- `ChangeLog-2.0.txt`
- `ChangeLog-2.1.txt`
- `ChangeLog-2.2.txt`
- `INSTALL.unix`
- `INSTALL.term`
- `INSTALL.socks`

## Pre-2.0 Notes

- Original ISCA BBS client line was maintained by volunteers.
- Early docs were clear that support was limited and mostly community-driven.
- Build/install was originally documented for many Unix/VMS setups.
- By the 2.2 era, IO ERROR maintained the main feature line based on 1.5.1.

## 2.0 Series

### 1995-04

- Started 2.0 branch.
- Added automatic away-message autoreply support.
- Added Ctrl-N to paste/recall recently seen names from posts/X messages.
- Added early term compatibility option.
- Released `2.0.1` through `2.0.4`.

### 1995-06

- Fixed infinite autoreply loop behavior.
- Added packaging/dist build updates.
- Released `2.0.5`, `2.0.6`, `2.0.7`.

### 1995-09 to 1995-12

- Pulled in stdio updates and general stability fixes.
- Expanded Ctrl-N history behavior (name scroll with dedupe).
- Added ping-on-blank behavior controls, then later refined/removed as server behavior changed.
- Fixed several ANSI/capture/segfault issues.
- UI cleanup in config prompts and option naming.
- Released `2.0.8` through `2.0.13`.

### 1996-02 to 1996-07

- Added friend-list color support and bold/non-bold ANSI preferences.
- Introduced major I/O routine updates.
- Added compatibility path for telnet daemons ("Heinous" patch).
- Added login-shell mode with PID-scoped temp files.
- Released `2.0.14` through `2.0.18`.

### 1996-09 to 1996-12

- Fixed shell feature regression.
- Updated banner/address details.
- Synced parsing logic with server-side text changes.
- Fixed enemy-list behavior in ANSI mode.
- Added command key remapping support and related send-path changes.
- Released `2.0.19` through `2.0.25`.

### 1997-05

- Updated built-in default ISCA address after host move.
- Released `2.0.26`.

### 1998-01

- Added `xland` behavior for quicker reply setup.

## 2.1 Series

### 1998-10

- Integrated client updates from the Client 9/Blackout work:
- Unlimited friend/enemy list sizing.
- Ctrl-P backward name scroll.
- Alternate BBS site via command line.
- `.bbsfriends` reintegrated into `.bbsrc`.
- New default macro keys and first-time setup updates.
- Known beta issues were documented at the time (name matching and incomplete non-Unix support work).
- Released `2.1.0` (beta), `2.1.1` (beta), `2.1.2` (beta).

## 2.2 Series

### 1999-08 to 1999-10

- Reworked telnet parsing and moved key logic into `filter.c`.
- Added enemy kill-notification squelch option.
- Added initial color system for posts and express messages.
- Added automatic username support.
- Reworked away-from-keyboard behavior (moved away from macro hack).
- Added copyright/license/warranty info menu.
- Released `2.2.0`.

### 2000-03

- Applied non-color-mode enemy-list fix (Sbum patch).
- Corrected extended time decoder behavior.
- Updated copyright and web address references.
- Released `2.2.1`.

### 2001-03

- Added workaround for glibc `setjmp()` issue affecting second edit run.
- Fixed non-ANSI MORE-prompt color leakage in `filter.c`.
- Released `2.2.2`.

## Legacy Build Notes (From Old INSTALL Files)

- `INSTALL.unix`: classic Unix `make` + `make install` flow, plus a long list of supported systems from that era.
- `INSTALL.term`: instructions for building/running through legacy `term` (`tredir`, `termnet.h`, `libtermnet.a`).
- `INSTALL.socks`: instructions for building a SOCKS-aware binary with the old make target.

These notes are kept for historical context only. Current build instructions for this fork are in `README.md`.
