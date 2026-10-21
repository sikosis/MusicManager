#include "AppVersion.h"

#include <AppFileInfo.h>
#include <Application.h>
#include <File.h>
#include <Roster.h>

BString
AppVersion()
{
	app_info appInfo;
	if (be_app == NULL || be_app->GetAppInfo(&appInfo) != B_OK)
		return "unknown";

	BFile file(&appInfo.ref, B_READ_ONLY);
	BAppFileInfo fileInfo(&file);
	version_info info = {};
	if (fileInfo.GetVersionInfo(&info, B_APP_VERSION_KIND) != B_OK)
		return "unknown";

	BString version;
	version.SetToFormat("%" B_PRIu32 ".%" B_PRIu32 ".%" B_PRIu32,
		info.major, info.middle, info.minor);
	return version;
}
//---------------------------------------------------------------------------------------------------------------------------------//
