# Music Manager artwork

MusicManager.svg is the editable source for the main application icon.
MusicManager.hvif is the generated native Haiku vector icon embedded by
MusicManager.rdef.

Regenerate the HVIF on Haiku after editing the SVG:

    pkgman install hvif_tools
    icon2icon MusicManager.svg MusicManager.hvif

The former stacked-card bitmap artwork is retained in MusicManager.rdef as
the named MUSIC_MANAGER:PLAYLIST_LARGE and MUSIC_MANAGER:PLAYLIST_MINI
resources for future playlist-related use. It is no longer the application
icon.
