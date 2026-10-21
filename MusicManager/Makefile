NAME = MusicManager
TYPE = APP

SRCS = \
	src/AboutWindow.cpp \
	src/AppVersion.cpp \
	src/AudioPlayer.cpp \
	src/LibraryQuery.cpp \
	src/MainWindow.cpp \
	src/MusicManager.cpp \
	src/PlaylistView.cpp \
	src/Track.cpp \
	src/TrackItem.cpp

VERSION_RDEF = generated/MusicManagerVersion.rdef

RDEFS = MusicManager.rdef $(VERSION_RDEF)
RSRCS =

LIBS = be media tracker stdc++

SYSTEM_INCLUDE_PATHS =
LOCAL_INCLUDE_PATHS = src

OPTIMIZE = FULL
SYMBOLS = TRUE
DEBUGGER =
COMPILER_FLAGS = -std=c++11
LINKER_FLAGS =

WARNINGS = ALL

include /system/develop/etc/makefile-engine

$(VERSION_RDEF): VERSION tools/generate-version.sh \
	tools/MusicManagerVersion.rdef.in
	sh tools/generate-version.sh VERSION $(VERSION_RDEF)
