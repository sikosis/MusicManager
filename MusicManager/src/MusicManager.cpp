#include "AboutWindow.h"
#include "AppVersion.h"
#include "MainWindow.h"
#include "Messages.h"

#include <Application.h>
#include <Message.h>
#include <OS.h>
#include <PropertyInfo.h>
#include <String.h>

#include <cstdlib>

namespace {

property_info kMusicManagerProperties[] = {
	{ "Version", { B_GET_PROPERTY }, { B_DIRECT_SPECIFIER },
		"Return the Music Manager version" },
	{ "Playing", { B_GET_PROPERTY }, { B_DIRECT_SPECIFIER },
		"Return whether audio is currently playing" },
	{ "NowPlaying", { B_GET_PROPERTY }, { B_DIRECT_SPECIFIER },
		"Return the current track description" },
	{ "Library", { B_COUNT_PROPERTIES }, { B_DIRECT_SPECIFIER },
		"Count the tracks in the music library" },
	{ "Queue", { B_COUNT_PROPERTIES }, { B_DIRECT_SPECIFIER },
		"Count the tracks in the one-shot queue" },
	{ "Rotation", { B_COUNT_PROPERTIES }, { B_DIRECT_SPECIFIER },
		"Count the tracks in the random rotation" },
	{ "Start", { B_EXECUTE_PROPERTY }, { B_DIRECT_SPECIFIER },
		"Start automatic playback" },
	{ "Stop", { B_EXECUTE_PROPERTY }, { B_DIRECT_SPECIFIER },
		"Stop playback" },
	{ "Next", { B_EXECUTE_PROPERTY }, { B_DIRECT_SPECIFIER },
		"Play the next track" },
	{ 0 }
};


void
SendScriptingReply(BMessage* request, status_t error)
{
	BMessage reply(B_REPLY);
	reply.AddInt32("error", error);
	request->SendReply(&reply);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SendScriptingReply(BMessage* request, status_t error, const char* result)
{
	BMessage reply(B_REPLY);
	reply.AddString("result", result);
	reply.AddInt32("error", error);
	request->SendReply(&reply);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SendScriptingReply(BMessage* request, status_t error, bool result)
{
	BMessage reply(B_REPLY);
	reply.AddBool("result", result);
	reply.AddInt32("error", error);
	request->SendReply(&reply);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SendScriptingReply(BMessage* request, status_t error, int32 result)
{
	BMessage reply(B_REPLY);
	reply.AddInt32("result", result);
	reply.AddInt32("error", error);
	request->SendReply(&reply);
}
//---------------------------------------------------------------------------------------------------------------------------------//

}

class MusicManagerApplication : public BApplication {
public:
	MusicManagerApplication();
	virtual ~MusicManagerApplication();
	virtual void AboutRequested();
	virtual status_t GetSupportedSuites(BMessage* message);
	virtual void MessageReceived(BMessage* message);
	virtual void ReadyToRun();
	virtual BHandler* ResolveSpecifier(BMessage* message, int32 index,
		BMessage* specifier, int32 form, const char* property);

private:
	bool _HandleScriptingMessage(BMessage* message);

	MainWindow* fMainWindow;
};


MusicManagerApplication::MusicManagerApplication()
	:
	BApplication("application/x-vnd.sikosis-MusicManager"),
	fMainWindow(NULL)
{
}
//---------------------------------------------------------------------------------------------------------------------------------//


MusicManagerApplication::~MusicManagerApplication()
{
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
MusicManagerApplication::AboutRequested()
{
	AboutWindow* window = new AboutWindow();
	window->Show();
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
MusicManagerApplication::GetSupportedSuites(BMessage* message)
{
	message->AddString("suites", "suite/vnd.sikosis-MusicManager");
	BPropertyInfo propertyInfo(kMusicManagerProperties);
	message->AddFlat("messages", &propertyInfo);
	return BApplication::GetSupportedSuites(message);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
MusicManagerApplication::MessageReceived(BMessage* message)
{
	if ((message->what == B_GET_PROPERTY
			|| message->what == B_COUNT_PROPERTIES
			|| message->what == B_EXECUTE_PROPERTY)
		&& _HandleScriptingMessage(message)) {
		return;
	}

	BApplication::MessageReceived(message);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
MusicManagerApplication::ReadyToRun()
{
	fMainWindow = new MainWindow();
	fMainWindow->Show();
}
//---------------------------------------------------------------------------------------------------------------------------------//


BHandler*
MusicManagerApplication::ResolveSpecifier(BMessage* message, int32 index,
	BMessage* specifier, int32 form, const char* property)
{
	BPropertyInfo propertyInfo(kMusicManagerProperties);
	if (propertyInfo.FindMatch(message, 0, specifier, form, property) >= 0)
		return this;

	return BApplication::ResolveSpecifier(
		message, index, specifier, form, property);
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
MusicManagerApplication::_HandleScriptingMessage(BMessage* message)
{
	int32 specifierIndex;
	BMessage specifier;
	BString property;
	if (message->GetCurrentSpecifier(&specifierIndex, &specifier) != B_OK
		|| specifier.FindString("property", &property) != B_OK) {
		return false;
	}

	if (message->what == B_GET_PROPERTY && property == "Version") {
		SendScriptingReply(message, B_OK, AppVersion().String());
		return true;
	}

	if (fMainWindow == NULL) {
		SendScriptingReply(message, B_NO_INIT);
		return true;
	}

	if (message->what == B_EXECUTE_PROPERTY) {
		uint32 command = 0;
		if (property == "Start")
			command = kStartPlayback;
		else if (property == "Stop")
			command = kStopPlayback;
		else if (property == "Next")
			command = kNextTrack;
		else
			return false;
		SendScriptingReply(message, fMainWindow->PostMessage(command));
		return true;
	}

	if (!fMainWindow->Lock()) {
		SendScriptingReply(message, B_ERROR);
		return true;
	}

	if (message->what == B_GET_PROPERTY && property == "Playing") {
		bool playing = fMainWindow->IsPlaying();
		fMainWindow->Unlock();
		SendScriptingReply(message, B_OK, playing);
		return true;
	}
	if (message->what == B_GET_PROPERTY && property == "NowPlaying") {
		BString nowPlaying = fMainWindow->NowPlaying();
		fMainWindow->Unlock();
		SendScriptingReply(message, B_OK, nowPlaying.String());
		return true;
	}
	if (message->what == B_COUNT_PROPERTIES && property == "Library") {
		int32 count = fMainWindow->LibraryTrackCount();
		fMainWindow->Unlock();
		SendScriptingReply(message, B_OK, count);
		return true;
	}
	if (message->what == B_COUNT_PROPERTIES && property == "Queue") {
		int32 count = fMainWindow->QueueTrackCount();
		fMainWindow->Unlock();
		SendScriptingReply(message, B_OK, count);
		return true;
	}
	if (message->what == B_COUNT_PROPERTIES && property == "Rotation") {
		int32 count = fMainWindow->RotationTrackCount();
		fMainWindow->Unlock();
		SendScriptingReply(message, B_OK, count);
		return true;
	}

	fMainWindow->Unlock();
	return false;
}
//---------------------------------------------------------------------------------------------------------------------------------//


int
main()
{
	srand((unsigned int)system_time());
	MusicManagerApplication application;
	application.Run();
	return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//
