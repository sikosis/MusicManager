#include "TrackItem.h"

TrackItem::TrackItem(const char* label, Kind kind)
	:
	BStringItem(label),
	fKind(kind),
	fTrack(NULL)
{
}
//---------------------------------------------------------------------------------------------------------------------------------//


TrackItem::TrackItem(Track* track, bool includeCollection)
	:
	BStringItem(track != NULL ? track->DisplayName(includeCollection).String() : ""),
	fKind(kTrack),
	fTrack(track)
{
}
//---------------------------------------------------------------------------------------------------------------------------------//


TrackItem::~TrackItem()
{
	delete fTrack;
}
//---------------------------------------------------------------------------------------------------------------------------------//


TrackItem::Kind
TrackItem::ItemKind() const
{
	return fKind;
}
//---------------------------------------------------------------------------------------------------------------------------------//


const Track*
TrackItem::TrackData() const
{
	return fTrack;
}
//---------------------------------------------------------------------------------------------------------------------------------//


Track*
TrackItem::DetachTrack()
{
	Track* track = fTrack;
	fTrack = NULL;
	return track;
}
//---------------------------------------------------------------------------------------------------------------------------------//
