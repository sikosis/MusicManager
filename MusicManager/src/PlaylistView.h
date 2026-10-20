#ifndef MUSIC_MANAGER_PLAYLIST_VIEW_H
#define MUSIC_MANAGER_PLAYLIST_VIEW_H

#include <Entry.h>
#include <ListView.h>

class Track;
class TrackItem;

class PlaylistView : public BListView {
public:
	explicit PlaylistView(const char* name);
	virtual ~PlaylistView();

	virtual void MessageReceived(BMessage* message);
	virtual void MouseDown(BPoint where);
	virtual bool InitiateDrag(BPoint point, int32 index, bool wasSelected);
	virtual void KeyDown(const char* bytes, int32 count);

	status_t AddRef(const entry_ref& ref, int32 index = -1);
	void AddTrack(const Track& track, int32 index = -1);
	TrackItem* TrackItemAt(int32 index) const;
	void RemoveSelected();
	void CropToSelection();

private:
	void _NotifyChanged();
	void _HandleDrop(BMessage* message);
};

#endif
