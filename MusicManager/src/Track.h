#ifndef MUSIC_MANAGER_TRACK_H
#define MUSIC_MANAGER_TRACK_H

#include <Entry.h>
#include <String.h>

class Track {
public:
	explicit Track(const entry_ref& ref);
	Track(const Track& other);
	~Track();

	Track& operator=(const Track& other);

	status_t InitCheck() const;
	const entry_ref& EntryRef() const;
	const node_ref& NodeRef() const;
	const char* Artist() const;
	const char* Album() const;
	const char* Title() const;
	int32 Number() const;
	int64 Duration() const;
	BString DisplayName(bool includeCollection = true) const;

private:
	status_t _ReadMetadata();

	entry_ref fEntryRef;
	node_ref fNodeRef;
	BString fArtist;
	BString fAlbum;
	BString fTitle;
	int32 fNumber;
	int64 fDuration;
	status_t fStatus;
};

#endif
