#ifndef MUSIC_MANAGER_MESSAGES_H
#define MUSIC_MANAGER_MESSAGES_H

#include <SupportDefs.h>

enum {
	kLibraryAdd = 'liad',
	kLibraryScanComplete = 'lisc',
	kLibraryDrag = 'lbdg',
	kQueueSelection = 'ques',
	kRotationSelection = 'rots',
	kLibraryInvoked = 'liiv',
	kPlaylistInvoked = 'pliv',
	kPlaylistChanged = 'plch',
	kStartStop = 'stsp',
	kStartPlayback = 'play',
	kStopPlayback = 'stop',
	kNextTrack = 'next',
	kPlaybackFinished = 'pfin',
	kLoadPlaylist = 'load',
	kLoadPlaylistConfirmed = 'ldcf',
	kSavePlaylist = 'save',
	kSavePlaylistConfirmed = 'svcf'
};

#endif
