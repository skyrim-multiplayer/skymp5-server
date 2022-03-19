# SkyMP

[![Discord Chat](https://img.shields.io/discord/699653182946803722?label=Discord&logo=Discord)](https://discord.gg/k39uQ9Yudt) 
[![PR's Welcome](https://img.shields.io/badge/PRs%20-welcome-brightgreen.svg)](CONTRIBUTING.md)

SkyMP is an open-source multiplayer mod for Skyrim ⚡

SkyMP is built on top of the [SkyrimPlatform](docs/docs_skyrim_platform.md) - a tool to create Skyrim mods with TypeScript and Chromium. 🚀

This repo hosts all sources to ease local setup and contributing. See [CONTRIBUTING](CONTRIBUTING.md) for build instructions.

![image](skymp.jpg)

### Getting Started

If you just want to play multiplayer, [download the client]().

You can join our test server called [SweetPie]() or [host your own]().

A list of the servers online can be retrieved via API: https://skymp.io/api/servers

### What's Synced

- [x] Player movement and animation sync
- [x] Appearance: race, headparts, tints, etc
- [x] PvP sync: melee damage, attributes, death
- [x] Inventory sync: equipment, containers, craft, etc
- [x] Limited scripts sync (WIP) - we have our own Papyrus engine
- [ ] Who knows what comes next? We work on synchronizing all Skyrim game mechanics, see [ROADMAP](ROADMAP.md).

### Multiplayer Features

- Mostly server-controlled game state - you can't cheat everything
- Store your world in plain files or MongoDB
- Customize your server with TypeScript or Papyrus scripting
- Use esp/esm mods, just ensure both client and server load order are the same
