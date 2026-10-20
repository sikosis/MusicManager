#include "LibraryQuery.h"

#include "Messages.h"

#include <Message.h>
#include <Volume.h>
#include <VolumeRoster.h>

LibraryQuery::LibraryQuery(const BMessenger& target)
	:
	fTarget(target),
	fThread(-1),
	fStopping(false)
{
}
//---------------------------------------------------------------------------------------------------------------------------------//


LibraryQuery::~LibraryQuery()
{
	fStopping = true;
	if (fThread >= B_OK) {
		status_t result;
		wait_for_thread(fThread, &result);
	}
	for (size_t index = 0; index < fQueries.size(); index++)
		delete fQueries[index];
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
LibraryQuery::Start()
{
	if (fThread >= B_OK)
		return B_BUSY;

	fThread = spawn_thread(_ScanThread, "music library query",
		B_NORMAL_PRIORITY, this);
	if (fThread < B_OK)
		return fThread;

	status_t status = resume_thread(fThread);
	if (status != B_OK) {
		kill_thread(fThread);
		fThread = -1;
	}
	return status;
}
//---------------------------------------------------------------------------------------------------------------------------------//


int32
LibraryQuery::_ScanThread(void* data)
{
	return static_cast<LibraryQuery*>(data)->_Scan();
}
//---------------------------------------------------------------------------------------------------------------------------------//


int32
LibraryQuery::_Scan()
{
	BVolumeRoster roster;
	BVolume volume;
	status_t result = B_ENTRY_NOT_FOUND;

	while (!fStopping && roster.GetNextVolume(&volume) == B_OK) {
		if (!volume.KnowsQuery())
			continue;

		BQuery* query = new BQuery();
		status_t status = query->SetVolume(&volume);
		if (status == B_OK)
			status = query->SetPredicate("BEOS:TYPE==audio/*");
		if (status == B_OK)
			status = query->SetTarget(fTarget);
		if (status == B_OK)
			status = query->Fetch();

		if (status != B_OK) {
			delete query;
			continue;
		}

		fQueries.push_back(query);
		result = B_OK;

		entry_ref ref;
		while (!fStopping && query->GetNextRef(&ref) == B_OK) {
			BMessage message(kLibraryAdd);
			message.AddRef("refs", &ref);
			fTarget.SendMessage(&message);
		}
	}

	if (!fStopping)
		fTarget.SendMessage(kLibraryScanComplete);
	return result;
}
//---------------------------------------------------------------------------------------------------------------------------------//
