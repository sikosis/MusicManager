#ifndef MUSIC_MANAGER_TRACK_ITEM_H
#define MUSIC_MANAGER_TRACK_ITEM_H

#include <StringItem.h>

#include "Track.h"

class TrackItem : public BStringItem {
public:
	enum Kind {
		kArtist,
		kAlbum,
		kTrack
	};

	TrackItem(const char* label, Kind kind);
	explicit TrackItem(Track* track, bool includeCollection = false);
	virtual ~TrackItem();

	Kind ItemKind() const;
	const Track* TrackData() const;
	Track* DetachTrack();

private:
	Kind fKind;
	Track* fTrack;
};

#endif
