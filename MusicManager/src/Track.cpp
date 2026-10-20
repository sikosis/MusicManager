#include "Track.h"

#include <Node.h>
#include <Path.h>
#include <TypeConstants.h>

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {

status_t
ReadStringAttribute(BNode& node, const char* name, BString& value)
{
	attr_info info;
	status_t status = node.GetAttrInfo(name, &info);
	if (status != B_OK || info.size <= 0)
		return status != B_OK ? status : B_BAD_DATA;

	std::vector<char> buffer((size_t)info.size + 1, '\0');
	ssize_t bytesRead = node.ReadAttr(name, info.type, 0, buffer.data(),
		(size_t)info.size);
	if (bytesRead <= 0)
		return bytesRead < 0 ? (status_t)bytesRead : B_BAD_DATA;

	buffer[(size_t)bytesRead] = '\0';
	value.SetTo(buffer.data());
	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
ReadFirstStringAttribute(BNode& node, const char* first, const char* second,
	BString& value)
{
	status_t status = ReadStringAttribute(node, first, value);
	if (status == B_OK)
		return status;

	return ReadStringAttribute(node, second, value);
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
ReadIntegerAttribute(BNode& node, const char* name, int64& value)
{
	attr_info info;
	if (node.GetAttrInfo(name, &info) != B_OK)
		return false;

	if (info.type == B_INT32_TYPE) {
		int32 temporary = 0;
		if (node.ReadAttr(name, info.type, 0, &temporary, sizeof(temporary))
			== (ssize_t)sizeof(temporary)) {
			value = temporary;
			return true;
		}
	}

	if (info.type == B_INT64_TYPE) {
		int64 temporary = 0;
		if (node.ReadAttr(name, info.type, 0, &temporary, sizeof(temporary))
			== (ssize_t)sizeof(temporary)) {
			value = temporary;
			return true;
		}
	}
	if (info.type == B_UINT32_TYPE) {
		uint32 temporary = 0;
		if (node.ReadAttr(name, info.type, 0, &temporary, sizeof(temporary))
			== (ssize_t)sizeof(temporary)) {
			value = temporary;
			return true;
		}
	}
	if (info.type == B_UINT64_TYPE) {
		uint64 temporary = 0;
		if (node.ReadAttr(name, info.type, 0, &temporary, sizeof(temporary))
			== (ssize_t)sizeof(temporary)) {
			value = (int64)temporary;
			return true;
		}
	}

	BString text;
	if (ReadStringAttribute(node, name, text) == B_OK) {
		char* end = NULL;
		value = strtoll(text.String(), &end, 10);
		while (end != NULL && *end != '\0' && isspace((unsigned char)*end))
			end++;
		return end != text.String() && end != NULL && *end == '\0';
	}

	return false;
}
//---------------------------------------------------------------------------------------------------------------------------------//


int64
ParseLegacyDuration(const BString& duration)
{
	const char* separator = strchr(duration.String(), ':');
	if (separator == NULL)
		return strtoll(duration.String(), NULL, 10) * 1000000LL;

	int64 minutes = strtoll(duration.String(), NULL, 10);
	int64 seconds = strtoll(separator + 1, NULL, 10);
	return (minutes * 60 + seconds) * 1000000LL;
}
//---------------------------------------------------------------------------------------------------------------------------------//

}


Track::Track(const entry_ref& ref)
	:
	fEntryRef(ref),
	fNumber(0),
	fDuration(0),
	fStatus(B_NO_INIT)
{
	fNodeRef.device = -1;
	fNodeRef.node = -1;
	fStatus = _ReadMetadata();
}
//---------------------------------------------------------------------------------------------------------------------------------//


Track::Track(const Track& other)
{
	*this = other;
}
//---------------------------------------------------------------------------------------------------------------------------------//


Track::~Track()
{
}
//---------------------------------------------------------------------------------------------------------------------------------//


Track&
Track::operator=(const Track& other)
{
	fEntryRef = other.fEntryRef;
	fNodeRef = other.fNodeRef;
	fArtist = other.fArtist;
	fAlbum = other.fAlbum;
	fTitle = other.fTitle;
	fNumber = other.fNumber;
	fDuration = other.fDuration;
	fStatus = other.fStatus;
	return *this;
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
Track::InitCheck() const
{
	return fStatus;
}
//---------------------------------------------------------------------------------------------------------------------------------//


const entry_ref&
Track::EntryRef() const
{
	return fEntryRef;
}
//---------------------------------------------------------------------------------------------------------------------------------//


const node_ref&
Track::NodeRef() const
{
	return fNodeRef;
}
//---------------------------------------------------------------------------------------------------------------------------------//


const char*
Track::Artist() const
{
	return fArtist.String();
}
//---------------------------------------------------------------------------------------------------------------------------------//


const char*
Track::Album() const
{
	return fAlbum.String();
}
//---------------------------------------------------------------------------------------------------------------------------------//


const char*
Track::Title() const
{
	return fTitle.String();
}
//---------------------------------------------------------------------------------------------------------------------------------//


int32
Track::Number() const
{
	return fNumber;
}
//---------------------------------------------------------------------------------------------------------------------------------//


int64
Track::Duration() const
{
	return fDuration;
}
//---------------------------------------------------------------------------------------------------------------------------------//


BString
Track::DisplayName(bool includeCollection) const
{
	BString result;
	if (includeCollection)
		result << fArtist << " — " << fAlbum << " — ";
	if (fNumber > 0)
		result << fNumber << ". ";
	result << fTitle;
	return result;
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
Track::_ReadMetadata()
{
	BNode node(&fEntryRef);
	status_t status = node.InitCheck();
	if (status != B_OK)
		return status;

	status = node.GetNodeRef(&fNodeRef);
	if (status != B_OK)
		return status;

	if (ReadFirstStringAttribute(node, "Media:Artist", "Audio:Artist", fArtist)
		!= B_OK) {
		fArtist = "Unknown artist";
	}
	if (ReadFirstStringAttribute(node, "Media:Album", "Audio:Album", fAlbum)
		!= B_OK) {
		fAlbum = "Unknown album";
	}
	if (ReadFirstStringAttribute(node, "Media:Title", "Audio:Title", fTitle)
		!= B_OK) {
		fTitle = fEntryRef.name;
	}

	int64 number = 0;
	if (ReadIntegerAttribute(node, "Media:Track", number)
		|| ReadIntegerAttribute(node, "Audio:Track", number)) {
		fNumber = (int32)number;
	}

	int64 duration = 0;
	if (ReadIntegerAttribute(node, "Media:Duration", duration)) {
		fDuration = duration;
	} else {
		BString legacyDuration;
		if (ReadIntegerAttribute(node, "Audio:Length", duration))
			fDuration = duration > 1000000 ? duration : duration * 1000000LL;
		else if (ReadStringAttribute(node, "Audio:Length", legacyDuration) == B_OK)
			fDuration = ParseLegacyDuration(legacyDuration);
	}

	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//
