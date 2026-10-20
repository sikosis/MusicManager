#include "MainWindow.h"

#include "AudioPlayer.h"
#include "LibraryQuery.h"
#include "Messages.h"
#include "PlaylistView.h"
#include "Track.h"
#include "TrackItem.h"

#include <Alert.h>
#include <Application.h>
#include <Button.h>
#include <Directory.h>
#include <File.h>
#include <FilePanel.h>
#include <LayoutBuilder.h>
#include <MenuItem.h>
#include <Message.h>
#include <NodeInfo.h>
#include <NodeMonitor.h>
#include <OutlineListView.h>
#include <Path.h>
#include <Query.h>
#include <ScrollView.h>
#include <SeparatorView.h>
#include <StringItem.h>
#include <StringView.h>

#include <cctype>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <set>
#include <strings.h>

enum {
	kStartLibraryQuery = 'stlq'
};

namespace {

int
CompareListItems(const BListItem* first, const BListItem* second)
{
	const TrackItem* firstTrack = dynamic_cast<const TrackItem*>(first);
	const TrackItem* secondTrack = dynamic_cast<const TrackItem*>(second);
	if (firstTrack != NULL && secondTrack != NULL
		&& firstTrack->ItemKind() == TrackItem::kTrack
		&& secondTrack->ItemKind() == TrackItem::kTrack
		&& firstTrack->TrackData() != NULL && secondTrack->TrackData() != NULL) {
		int32 firstNumber = firstTrack->TrackData()->Number();
		int32 secondNumber = secondTrack->TrackData()->Number();
		if (firstNumber != secondNumber) {
			if (firstNumber == 0)
				return 1;
			if (secondNumber == 0)
				return -1;
			return firstNumber < secondNumber ? -1 : 1;
		}
	}

	const BStringItem* firstString = dynamic_cast<const BStringItem*>(first);
	const BStringItem* secondString = dynamic_cast<const BStringItem*>(second);
	if (firstString == NULL || secondString == NULL)
		return first < second ? -1 : first != second;
	return strcasecmp(firstString->Text(), secondString->Text());
}
//---------------------------------------------------------------------------------------------------------------------------------//


class LibraryView : public BOutlineListView {
public:
	LibraryView();
	virtual ~LibraryView();
	virtual bool InitiateDrag(BPoint point, int32 index, bool wasSelected);

private:
	void _AddItemRefs(BMessage& message, TrackItem* item,
		std::set<std::string>& paths);
};


LibraryView::LibraryView()
	:
	BOutlineListView("music library", B_MULTIPLE_SELECTION_LIST)
{
	SetInvocationMessage(new BMessage(kLibraryInvoked));
}
//---------------------------------------------------------------------------------------------------------------------------------//


LibraryView::~LibraryView()
{
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
LibraryView::InitiateDrag(BPoint point, int32 index, bool wasSelected)
{
	if (index < 0)
		return false;
	if (!wasSelected)
		Select(index);

	BMessage message(kLibraryDrag);
	std::set<std::string> paths;
	for (int32 selection = 0; CurrentSelection(selection) >= 0; selection++) {
		TrackItem* item = dynamic_cast<TrackItem*>(
			ItemAt(CurrentSelection(selection)));
		_AddItemRefs(message, item, paths);
	}

	entry_ref firstRef;
	if (message.FindRef("refs", &firstRef) != B_OK)
		return false;

	DragMessage(&message, ItemFrame(index));
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
LibraryView::_AddItemRefs(BMessage& message, TrackItem* item,
	std::set<std::string>& paths)
{
	if (item == NULL)
		return;

	if (item->ItemKind() == TrackItem::kTrack && item->TrackData() != NULL) {
		BPath path(&item->TrackData()->EntryRef());
		if (path.InitCheck() == B_OK && paths.insert(path.Path()).second)
			message.AddRef("refs", &item->TrackData()->EntryRef());
		return;
	}

	int32 start = FullListIndexOf(item) + 1;
	uint32 parentLevel = item->OutlineLevel();
	for (int32 index = start; index < FullListCountItems(); index++) {
		TrackItem* child = dynamic_cast<TrackItem*>(FullListItemAt(index));
		if (child == NULL || child->OutlineLevel() <= parentLevel)
			break;
		if (child->ItemKind() == TrackItem::kTrack)
			_AddItemRefs(message, child, paths);
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//

}


MainWindow::MainWindow()
	:
	BWindow(BRect(80, 80, 1080, 720), "Music Manager", B_TITLED_WINDOW,
		B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS
			| B_QUIT_ON_WINDOW_CLOSE),
	fLibraryView(new LibraryView()),
	fQueueView(new PlaylistView("one-shot queue")),
	fRotationView(new PlaylistView("random rotation")),
	fLibrarySummary(new BStringView("library summary", "Scanning music library…")),
	fNowPlaying(new BStringView("now playing", "Nothing playing")),
	fRotationSummary(new BStringView("rotation summary", "Rotation is empty")),
	fStartStopButton(new BButton("start stop", "Start", new BMessage(kStartStop))),
	fOpenPanel(NULL),
	fSavePanel(NULL),
	fLibraryQuery(NULL),
	fAudioPlayer(NULL),
	fAutomaticPlayback(false),
	fLastRandomIndex(-1),
	fTrackCount(0)
{
	fLibraryView->SetExplicitMinSize(BSize(320, 300));
	fQueueView->SetExplicitMinSize(BSize(380, 100));
	fRotationView->SetExplicitMinSize(BSize(380, 180));

	BScrollView* libraryScroller = new BScrollView("library scroller",
		fLibraryView, 0, false, true);
	BScrollView* queueScroller = new BScrollView("queue scroller", fQueueView,
		0, false, true);
	BScrollView* rotationScroller = new BScrollView("rotation scroller",
		fRotationView, 0, false, true);

	BView* libraryPanel = new BView("library panel", B_WILL_DRAW);
	libraryPanel->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	BLayoutBuilder::Group<>(libraryPanel, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.Add(new BStringView("library heading", "Music library"))
		.Add(fLibrarySummary)
		.Add(libraryScroller, 1.0f)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(new BButton("queue selection", "Queue",
				new BMessage(kQueueSelection)))
			.Add(new BButton("rotation selection", "Add to rotation",
				new BMessage(kRotationSelection)))
		.End();

	BView* playlistPanel = new BView("playlist panel", B_WILL_DRAW);
	playlistPanel->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	BLayoutBuilder::Group<>(playlistPanel, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.Add(new BStringView("now playing heading", "Now playing"))
		.Add(fNowPlaying)
		.Add(new BSeparatorView(B_HORIZONTAL))
		.Add(new BStringView("queue heading", "Up next — played once"))
		.Add(queueScroller, 0.7f)
		.Add(new BStringView("rotation heading",
			"Random rotation — used when the queue is empty"))
		.Add(fRotationSummary)
		.Add(rotationScroller, 1.0f)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(fStartStopButton)
			.Add(new BButton("next", "Next", new BMessage(kNextTrack)))
			.AddGlue()
			.Add(new BButton("load", "Load M3U", new BMessage(kLoadPlaylist)))
			.Add(new BButton("save", "Save M3U", new BMessage(kSavePlaylist)))
		.End();

	BLayoutBuilder::Group<>(this, B_HORIZONTAL, 0)
		.Add(libraryPanel, 0.8f)
		.Add(new BSeparatorView(B_VERTICAL))
		.Add(playlistPanel, 1.2f);

	fOpenPanel = new BFilePanel(B_OPEN_PANEL);
	fOpenPanel->SetTarget(BMessenger(this));
	fOpenPanel->SetMessage(new BMessage(kLoadPlaylistConfirmed));
	fSavePanel = new BFilePanel(B_SAVE_PANEL);
	fSavePanel->SetTarget(BMessenger(this));
	fSavePanel->SetMessage(new BMessage(kSavePlaylistConfirmed));

	fAudioPlayer = new AudioPlayer(BMessenger(this));
	PostMessage(kStartLibraryQuery);
}
//---------------------------------------------------------------------------------------------------------------------------------//


MainWindow::~MainWindow()
{
	delete fLibraryQuery;
	delete fAudioPlayer;
	delete fOpenPanel;
	delete fSavePanel;
	for (int32 index = fLibraryView->FullListCountItems() - 1; index >= 0;
		index--) {
		delete fLibraryView->RemoveItem(index);
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
MainWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kStartLibraryQuery:
			_StartLibraryQuery();
			break;
		case kLibraryAdd:
		{
			entry_ref ref;
			if (message->FindRef("refs", &ref) == B_OK)
				_AddLibraryRef(ref);
			break;
		}
		case kLibraryScanComplete:
			_UpdateSummary();
			break;
		case B_QUERY_UPDATE:
		{
			int32 opcode = 0;
			int32 device = -1;
			int64 node = -1;
			message->FindInt32("opcode", &opcode);
			message->FindInt32("device", &device);
			message->FindInt64("node", &node);
			if (opcode == B_ENTRY_REMOVED) {
				_RemoveLibraryNode((dev_t)device, (ino_t)node);
			} else if (opcode == B_ENTRY_CREATED || opcode == B_ENTRY_MOVED) {
				const char* name = NULL;
				int64 directory = -1;
				if (opcode == B_ENTRY_MOVED) {
					_RemoveLibraryNode((dev_t)device, (ino_t)node);
					message->FindInt64("to directory", &directory);
				}
				if (message->FindString("name", &name) == B_OK
					&& (directory >= 0
						|| message->FindInt64("directory", &directory) == B_OK)) {
					entry_ref ref((dev_t)device, (ino_t)directory, name);
					_AddLibraryRef(ref);
				}
			}
			break;
		}
		case kQueueSelection:
			_AddSelectedLibraryItems(fQueueView);
			break;
		case kRotationSelection:
			_AddSelectedLibraryItems(fRotationView);
			break;
		case kLibraryInvoked:
			_PlaySelectedLibraryTrack();
			break;
		case kPlaylistInvoked:
			_PlaySelectedPlaylistTrack();
			break;
		case kPlaylistChanged:
			_UpdateSummary();
			break;
		case kStartStop:
			_SetAutomaticPlayback(!fAutomaticPlayback);
			break;
		case kNextTrack:
			if (!fAutomaticPlayback) {
				fAutomaticPlayback = true;
				fStartStopButton->SetLabel("Stop");
			}
			_PlayNext();
			break;
		case kPlaybackFinished:
			if (fAutomaticPlayback)
				_PlayNext();
			break;
		case kLoadPlaylist:
			fOpenPanel->Show();
			break;
		case kSavePlaylist:
			fSavePanel->Show();
			break;
		case kLoadPlaylistConfirmed:
		{
			entry_ref ref;
			if (message->FindRef("refs", &ref) == B_OK)
				_LoadPlaylist(ref);
			break;
		}
		case kSavePlaylistConfirmed:
		{
			entry_ref directory;
			const char* name = NULL;
			if (message->FindRef("directory", &directory) == B_OK
				&& message->FindString("name", &name) == B_OK) {
				_SavePlaylist(directory, name);
			}
			break;
		}
		default:
			BWindow::MessageReceived(message);
			break;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
MainWindow::QuitRequested()
{
	fAudioPlayer->Stop();
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
MainWindow::_StartLibraryQuery()
{
	if (fLibraryQuery != NULL)
		return;

	fLibraryQuery = new LibraryQuery(BMessenger(this));
	status_t status = fLibraryQuery->Start();
	if (status != B_OK)
		fLibrarySummary->SetText("No query-capable volumes were found");
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
MainWindow::_AddLibraryRef(const entry_ref& ref)
{
	Track* track = new Track(ref);
	if (track->InitCheck() != B_OK) {
		delete track;
		return;
	}

	std::string nodeKey = _NodeKey(track->NodeRef().device, track->NodeRef().node);
	if (fTracks.find(nodeKey) != fTracks.end()) {
		delete track;
		return;
	}

	std::string artistKey = _TextKey(track->Artist());
	TrackItem* artistItem = NULL;
	std::map<std::string, TrackItem*>::iterator artist = fArtists.find(artistKey);
	if (artist == fArtists.end()) {
		artistItem = new TrackItem(track->Artist(), TrackItem::kArtist);
		fLibraryView->AddItem(artistItem);
		fArtists[artistKey] = artistItem;
	} else {
		artistItem = artist->second;
	}

	std::string albumKey = _AlbumKey(track->Artist(), track->Album());
	TrackItem* albumItem = NULL;
	std::map<std::string, TrackItem*>::iterator album = fAlbums.find(albumKey);
	if (album == fAlbums.end()) {
		albumItem = new TrackItem(track->Album(), TrackItem::kAlbum);
		fLibraryView->AddUnder(albumItem, artistItem);
		fAlbums[albumKey] = albumItem;
	} else {
		albumItem = album->second;
	}

	TrackItem* trackItem = new TrackItem(track);
	fLibraryView->AddUnder(trackItem, albumItem);
	fTracks[nodeKey] = trackItem;
	fTrackCount++;

	fLibraryView->SortItemsUnder(NULL, true, CompareListItems);
	fLibraryView->SortItemsUnder(artistItem, true, CompareListItems);
	fLibraryView->SortItemsUnder(albumItem, true, CompareListItems);
	_UpdateSummary();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
MainWindow::_RemoveLibraryNode(dev_t device, ino_t node)
{
	std::string nodeKey = _NodeKey(device, node);
	std::map<std::string, TrackItem*>::iterator found = fTracks.find(nodeKey);
	if (found == fTracks.end())
		return;

	TrackItem* trackItem = found->second;
	TrackItem* albumItem = dynamic_cast<TrackItem*>(
		fLibraryView->Superitem(trackItem));
	TrackItem* artistItem = albumItem != NULL
		? dynamic_cast<TrackItem*>(fLibraryView->Superitem(albumItem)) : NULL;
	BString artistName = artistItem != NULL ? artistItem->Text() : "";
	BString albumName = albumItem != NULL ? albumItem->Text() : "";

	fLibraryView->RemoveItem(trackItem);
	delete trackItem;
	fTracks.erase(found);
	fTrackCount--;

	if (albumItem != NULL && fLibraryView->CountItemsUnder(albumItem, true) == 0) {
		fLibraryView->RemoveItem(albumItem);
		fAlbums.erase(_AlbumKey(artistName.String(), albumName.String()));
		delete albumItem;
	}
	if (artistItem != NULL && fLibraryView->CountItemsUnder(artistItem, true) == 0) {
		fLibraryView->RemoveItem(artistItem);
		fArtists.erase(_TextKey(artistName.String()));
		delete artistItem;
	}
	_UpdateSummary();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
MainWindow::_AddSelectedLibraryItems(PlaylistView* target)
{
	std::set<std::string> addedNodes;
	for (int32 selection = 0; fLibraryView->CurrentSelection(selection) >= 0;
		selection++) {
		TrackItem* item = dynamic_cast<TrackItem*>(
			fLibraryView->ItemAt(fLibraryView->CurrentSelection(selection)));
		_AddItemAndDescendants(item, target, addedNodes);
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
MainWindow::_AddItemAndDescendants(TrackItem* item, PlaylistView* target,
	std::set<std::string>& addedNodes)
{
	if (item == NULL)
		return;
	if (item->ItemKind() == TrackItem::kTrack && item->TrackData() != NULL) {
		const node_ref& node = item->TrackData()->NodeRef();
		if (addedNodes.insert(_NodeKey(node.device, node.node)).second)
			target->AddTrack(*item->TrackData());
		return;
	}

	int32 start = fLibraryView->FullListIndexOf(item) + 1;
	uint32 parentLevel = item->OutlineLevel();
	for (int32 index = start; index < fLibraryView->FullListCountItems(); index++) {
		TrackItem* child = dynamic_cast<TrackItem*>(
			fLibraryView->FullListItemAt(index));
		if (child == NULL || child->OutlineLevel() <= parentLevel)
			break;
		if (child->ItemKind() == TrackItem::kTrack && child->TrackData() != NULL) {
			const node_ref& node = child->TrackData()->NodeRef();
			if (addedNodes.insert(_NodeKey(node.device, node.node)).second)
				target->AddTrack(*child->TrackData());
		}
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
MainWindow::_PlaySelectedLibraryTrack()
{
	TrackItem* item = dynamic_cast<TrackItem*>(
		fLibraryView->ItemAt(fLibraryView->CurrentSelection()));
	if (item == NULL)
		return;

	if (item->ItemKind() != TrackItem::kTrack) {
		if (item->IsExpanded())
			fLibraryView->Collapse(item);
		else
			fLibraryView->Expand(item);
		return;
	}

	if (item->TrackData() != NULL)
		_PlayTrack(*item->TrackData());
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
MainWindow::_PlaySelectedPlaylistTrack()
{
	PlaylistView* source = fQueueView->IsFocus() ? fQueueView : fRotationView;
	TrackItem* item = source->TrackItemAt(source->CurrentSelection());
	if (item != NULL && item->TrackData() != NULL)
		_PlayTrack(*item->TrackData());
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
MainWindow::_PlayTrack(const Track& track)
{
	status_t status = fAudioPlayer->Play(track.EntryRef());
	if (status != B_OK) {
		_ShowError("Unable to play track", status);
		_SetAutomaticPlayback(false);
		return;
	}
	fNowPlaying->SetText(track.DisplayName(true).String());
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
MainWindow::_PlayNext()
{
	fAudioPlayer->Stop();

	if (fQueueView->CountItems() > 0) {
		TrackItem* item = fQueueView->TrackItemAt(0);
		Track* track = item != NULL ? item->DetachTrack() : NULL;
		delete fQueueView->RemoveItem((int32)0);
		if (track != NULL) {
			_PlayTrack(*track);
			delete track;
			_UpdateSummary();
			return;
		}
	}

	int32 count = fRotationView->CountItems();
	if (count > 0) {
		int32 index = fLastRandomIndex;
		do {
			index = rand() % count;
		} while (count > 1 && index == fLastRandomIndex);
		fLastRandomIndex = index;
		TrackItem* item = fRotationView->TrackItemAt(index);
		if (item != NULL && item->TrackData() != NULL) {
			_PlayTrack(*item->TrackData());
			return;
		}
	}

	_SetAutomaticPlayback(false);
	fNowPlaying->SetText("Nothing playing — the queue and rotation are empty");
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
MainWindow::_SetAutomaticPlayback(bool playing)
{
	fAutomaticPlayback = playing;
	fStartStopButton->SetLabel(playing ? "Stop" : "Start");
	if (playing) {
		if (!fAudioPlayer->IsPlaying())
			_PlayNext();
	} else {
		fAudioPlayer->Stop();
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
MainWindow::_UpdateSummary()
{
	BString libraryText;
	libraryText << fTrackCount << (fTrackCount == 1 ? " track" : " tracks");
	fLibrarySummary->SetText(libraryText.String());

	int64 duration = 0;
	for (int32 index = 0; index < fRotationView->CountItems(); index++) {
		TrackItem* item = fRotationView->TrackItemAt(index);
		if (item != NULL && item->TrackData() != NULL)
			duration += item->TrackData()->Duration();
	}

	int64 seconds = duration / 1000000LL;
	BString rotationText;
	rotationText << fRotationView->CountItems();
	rotationText << (fRotationView->CountItems() == 1 ? " track, " : " tracks, ");
	if (seconds >= 3600) {
		rotationText << seconds / 3600 << ":";
		if ((seconds % 3600) / 60 < 10)
			rotationText << "0";
	}
	rotationText << (seconds % 3600) / 60 << ":";
	if (seconds % 60 < 10)
		rotationText << "0";
	rotationText << seconds % 60;
	fRotationSummary->SetText(rotationText.String());
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
MainWindow::_LoadPlaylist(const entry_ref& ref)
{
	BPath playlistPath(&ref);
	if (playlistPath.InitCheck() != B_OK)
		return;

	std::ifstream input(playlistPath.Path());
	if (!input.is_open()) {
		_ShowError("Unable to load playlist", B_ERROR);
		return;
	}

	BPath parentPath;
	playlistPath.GetParent(&parentPath);
	std::string line;
	while (std::getline(input, line)) {
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (line.empty() || line[0] == '#')
			continue;

		BPath trackPath;
		if (line[0] == '/') {
			trackPath.SetTo(line.c_str());
		} else {
			trackPath.SetTo(parentPath.Path());
			trackPath.Append(line.c_str());
		}
		BEntry entry(trackPath.Path(), true);
		entry_ref trackRef;
		if (entry.GetRef(&trackRef) == B_OK)
			fRotationView->AddRef(trackRef);
	}
	_UpdateSummary();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
MainWindow::_SavePlaylist(const entry_ref& directory, const char* name)
{
	BPath path(&directory);
	if (path.InitCheck() != B_OK || path.Append(name) != B_OK)
		return;

	std::ofstream output(path.Path(), std::ios::trunc);
	if (!output.is_open()) {
		_ShowError("Unable to save playlist", B_ERROR);
		return;
	}

	output << "#EXTM3U\n";
	for (int32 index = 0; index < fRotationView->CountItems(); index++) {
		TrackItem* item = fRotationView->TrackItemAt(index);
		if (item == NULL || item->TrackData() == NULL)
			continue;
		BPath trackPath(&item->TrackData()->EntryRef());
		if (trackPath.InitCheck() == B_OK)
			output << trackPath.Path() << "\n";
	}
	output.close();

	BFile file(path.Path(), B_READ_WRITE);
	if (file.InitCheck() == B_OK) {
		BNodeInfo info(&file);
		info.SetType("audio/x-mpegurl");
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
MainWindow::_ShowError(const char* title, status_t status)
{
	BString text(title);
	text << ": " << strerror(status);
	BAlert* alert = new BAlert(title, text.String(), "OK", NULL, NULL,
		B_WIDTH_AS_USUAL, B_STOP_ALERT);
	alert->Go();
}
//---------------------------------------------------------------------------------------------------------------------------------//


std::string
MainWindow::_NodeKey(dev_t device, ino_t node) const
{
	BString key;
	key << (int64)device << ":" << (int64)node;
	return key.String();
}
//---------------------------------------------------------------------------------------------------------------------------------//


std::string
MainWindow::_TextKey(const char* text) const
{
	std::string key = text != NULL ? text : "";
	for (size_t index = 0; index < key.size(); index++)
		key[index] = (char)std::tolower((unsigned char)key[index]);
	return key;
}
//---------------------------------------------------------------------------------------------------------------------------------//


std::string
MainWindow::_AlbumKey(const char* artist, const char* album) const
{
	return _TextKey(artist) + "\n" + _TextKey(album);
}
//---------------------------------------------------------------------------------------------------------------------------------//
