#ifndef MUSIC_MANAGER_LIBRARY_QUERY_H
#define MUSIC_MANAGER_LIBRARY_QUERY_H

#include <Messenger.h>
#include <OS.h>
#include <Query.h>

#include <atomic>
#include <vector>

class LibraryQuery {
public:
	explicit LibraryQuery(const BMessenger& target);
	~LibraryQuery();

	status_t Start();

private:
	static int32 _ScanThread(void* data);
	int32 _Scan();

	BMessenger fTarget;
	std::vector<BQuery*> fQueries;
	thread_id fThread;
	std::atomic<bool> fStopping;
};

#endif
