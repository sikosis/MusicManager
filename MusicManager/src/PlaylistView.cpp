#include "PlaylistView.h"

#include "Messages.h"
#include "Track.h"
#include "TrackItem.h"

#include <MenuItem.h>
#include <Message.h>
#include <PopUpMenu.h>
#include <Window.h>

enum {
	kRemoveSelected = 'rmse',
	kCropSelected = 'crse',
	kPlaylistDrag = 'pldg'
};

PlaylistView::PlaylistView(const char* name)
	:
	BListView(name, B_MULTIPLE_SELECTION_LIST)
{
	SetInvocationMessage(new BMessage(kPlaylistInvoked));
}
//---------------------------------------------------------------------------------------------------------------------------------//


PlaylistView::~PlaylistView()
{
	while (CountItems() > 0)
		delete RemoveItem((int32)0);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
PlaylistView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kRemoveSelected:
			RemoveSelected();
			break;
		case kCropSelected:
			CropToSelection();
			break;
		case kLibraryDrag:
		case kPlaylistDrag:
		case B_SIMPLE_DATA:
		case B_REFS_RECEIVED:
			_HandleDrop(message);
			break;
		default:
			BListView::MessageReceived(message);
			break;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
PlaylistView::MouseDown(BPoint where)
{
	int32 buttons = B_PRIMARY_MOUSE_BUTTON;
	if (Window() != NULL && Window()->CurrentMessage() != NULL)
		Window()->CurrentMessage()->FindInt32("buttons", &buttons);

	if ((buttons & B_SECONDARY_MOUSE_BUTTON) == 0) {
		BListView::MouseDown(where);
		return;
	}

	BPopUpMenu menu("playlist commands", false, false);
	menu.AddItem(new BMenuItem("Remove selected", new BMessage(kRemoveSelected)));
	menu.AddItem(new BMenuItem("Crop to selection", new BMessage(kCropSelected)));
	menu.SetTargetForItems(this);
	menu.Go(ConvertToScreen(where), true, true, false);
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
PlaylistView::InitiateDrag(BPoint point, int32 index, bool wasSelected)
{
	if (index < 0)
		return false;

	if (!wasSelected)
		Select(index);

	TrackItem* item = TrackItemAt(index);
	if (item == NULL || item->TrackData() == NULL)
		return false;

	BMessage message(kPlaylistDrag);
	message.AddPointer("source", this);
	message.AddInt32("source_index", index);
	message.AddRef("refs", &item->TrackData()->EntryRef());
	DragMessage(&message, ItemFrame(index));
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
PlaylistView::KeyDown(const char* bytes, int32 count)
{
	if (count == 1 && bytes[0] == B_DELETE) {
		RemoveSelected();
		return;
	}

	BListView::KeyDown(bytes, count);
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
PlaylistView::AddRef(const entry_ref& ref, int32 index)
{
	Track* track = new Track(ref);
	status_t status = track->InitCheck();
	if (status != B_OK) {
		delete track;
		return status;
	}

	TrackItem* item = new TrackItem(track, true);
	bool added = index < 0 ? AddItem(item) : AddItem(item, index);
	if (!added) {
		delete item;
		return B_ERROR;
	}

	_NotifyChanged();
	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
PlaylistView::AddTrack(const Track& track, int32 index)
{
	TrackItem* item = new TrackItem(new Track(track), true);
	bool added = index < 0 ? AddItem(item) : AddItem(item, index);
	if (!added)
		delete item;
	else
		_NotifyChanged();
}
//---------------------------------------------------------------------------------------------------------------------------------//


TrackItem*
PlaylistView::TrackItemAt(int32 index) const
{
	return dynamic_cast<TrackItem*>(ItemAt(index));
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
PlaylistView::RemoveSelected()
{
	for (int32 index = CountItems() - 1; index >= 0; index--) {
		if (!IsItemSelected(index))
			continue;
		delete RemoveItem(index);
	}
	_NotifyChanged();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
PlaylistView::CropToSelection()
{
	for (int32 index = CountItems() - 1; index >= 0; index--) {
		if (IsItemSelected(index))
			continue;
		delete RemoveItem(index);
	}
	_NotifyChanged();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
PlaylistView::_NotifyChanged()
{
	if (Window() != NULL)
		Window()->PostMessage(kPlaylistChanged);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
PlaylistView::_HandleDrop(BMessage* message)
{
	void* sourcePointer = NULL;
	int32 sourceIndex = -1;
	message->FindPointer("source", &sourcePointer);
	PlaylistView* source = static_cast<PlaylistView*>(sourcePointer);
	message->FindInt32("source_index", &sourceIndex);

	BPoint point = ConvertFromScreen(message->DropPoint());
	int32 targetIndex = IndexOf(point);
	if (targetIndex < 0)
		targetIndex = CountItems();
	else if (point.y > ItemFrame(targetIndex).top + ItemFrame(targetIndex).Height() / 2)
		targetIndex++;

	if (source == this && sourceIndex >= 0) {
		if (targetIndex > sourceIndex)
			targetIndex--;
		if (targetIndex != sourceIndex)
			MoveItem(sourceIndex, targetIndex);
		_NotifyChanged();
		return;
	}

	entry_ref ref;
	for (int32 index = 0; message->FindRef("refs", index, &ref) == B_OK; index++)
		AddRef(ref, targetIndex++);
}
//---------------------------------------------------------------------------------------------------------------------------------//
