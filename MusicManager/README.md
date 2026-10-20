# Music Manager

Music Manager is a native Haiku application inspired by Mikael Eiman's
MusicManager2. It finds audio files through BFS queries, groups them by artist
and album, and provides a one-shot queue plus a reusable random rotation.

Unlike the original application, this version plays audio directly through the
Haiku Media Kit and does not require SoundPlay.

## Features

- Live, query-backed music library grouped by artist and album.
- Support for both `Audio:*` and `Media:*` metadata attributes.
- Drag artists, albums, or tracks into the queue or random rotation.
- Buttons provide an accessible alternative to drag and drop.
- Automatic playback: queued tracks are consumed first, then tracks are chosen
  randomly from the rotation without immediately repeating the last track.
- Remove and crop commands, keyboard deletion, and drag reordering.
- Load and save M3U playlists.

## Build

On Haiku with the development tools installed:

```sh
make
```

The executable is written to the architecture-specific object directory used
by Haiku's `makefile-engine`.

## Metadata and queries

The library query uses the indexed `BEOS:TYPE` attribute to discover audio
files. Music Manager reads artist, album, title, track number, and duration from
both the current `Media:*` names and the legacy `Audio:*` names used by the
original application. Volumes that do not support queries are skipped.

## Controls

- Double-click a library track or playlist entry to play it immediately.
- Select library entries and use **Queue** or **Add to rotation**.
- Drag library entries into either playlist. Drag a playlist entry to reorder it.
- Right-click a playlist for **Remove selected** and **Crop to selection**.
- **Start** begins automatic playback; **Stop** stops it; **Next** skips ahead.
- **Load M3U** and **Save M3U** operate on the random rotation.
