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
- Native scalable HVIF application icon and a resource-driven About window.
- Native Haiku scripting support for use with `hey`.

## Build

On Haiku with the development tools installed:

```sh
make
```

The executable is written to the architecture-specific object directory used
by Haiku's `makefile-engine`.

The application version is defined once in `VERSION`. The build generates the
embedded Haiku version resource from that file, and both the About window and
the scripting interface read the embedded value.

## Scripting

Music Manager exposes a native scripting suite. With the application running,
the following commands can be used from Terminal:

```sh
hey MusicManager get Version
hey MusicManager get Playing
hey MusicManager get NowPlaying
hey MusicManager count Library
hey MusicManager count Queue
hey MusicManager count Rotation
hey MusicManager execute Start
hey MusicManager execute Stop
hey MusicManager execute Next
```

Run `hey MusicManager getsuites` to inspect the complete suite.

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
- Choose **Music Manager → About Music Manager…** for version and credits.
