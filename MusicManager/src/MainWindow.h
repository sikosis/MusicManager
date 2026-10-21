#ifndef MUSIC_MANAGER_MAIN_WINDOW_H
#define MUSIC_MANAGER_MAIN_WINDOW_H

#include <Entry.h>
#include <String.h>
#include <Window.h>

#include <map>
#include <set>
#include <string>

class AudioPlayer;
class BButton;
class BFilePanel;
class BOutlineListView;
class BStringView;
class LibraryQuery;
class PlaylistView;
class Track;
class TrackItem;

class MainWindow : public BWindow {
public:
	MainWindow();
	virtual ~MainWindow();

	virtual void MessageReceived(BMessage* message);
	virtual bool QuitRequested();

	bool IsPlaying() const;
	BString NowPlaying() const;
	int32 LibraryTrackCount() const;
	int32 QueueTrackCount() const;
	int32 RotationTrackCount() const;

private:
	void _StartLibraryQuery();
	void _AddLibraryRef(const entry_ref& ref);
	void _RemoveLibraryNode(dev_t device, ino_t node);
	void _AddSelectedLibraryItems(PlaylistView* target);
	void _AddItemAndDescendants(TrackItem* item, PlaylistView* target,
		std::set<std::string>& addedNodes);
	void _PlaySelectedLibraryTrack();
	void _PlaySelectedPlaylistTrack();
	void _PlayTrack(const Track& track);
	void _PlayNext();
	void _SetAutomaticPlayback(bool playing);
	void _UpdateSummary();
	void _LoadPlaylist(const entry_ref& ref);
	void _SavePlaylist(const entry_ref& directory, const char* name);
	void _ShowError(const char* title, status_t status);
	std::string _NodeKey(dev_t device, ino_t node) const;
	std::string _TextKey(const char* text) const;
	std::string _AlbumKey(const char* artist, const char* album) const;

	BOutlineListView* fLibraryView;
	PlaylistView* fQueueView;
	PlaylistView* fRotationView;
	BStringView* fLibrarySummary;
	BStringView* fNowPlaying;
	BStringView* fRotationSummary;
	BButton* fStartStopButton;
	BFilePanel* fOpenPanel;
	BFilePanel* fSavePanel;
	LibraryQuery* fLibraryQuery;
	AudioPlayer* fAudioPlayer;
	bool fAutomaticPlayback;
	int32 fLastRandomIndex;
	int32 fTrackCount;

	std::map<std::string, TrackItem*> fArtists;
	std::map<std::string, TrackItem*> fAlbums;
	std::map<std::string, TrackItem*> fTracks;
};

#endif
