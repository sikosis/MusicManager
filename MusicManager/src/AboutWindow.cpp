#include "AboutWindow.h"

#include <AppFileInfo.h>
#include <Application.h>
#include <Bitmap.h>
#include <Button.h>
#include <File.h>
#include <Font.h>
#include <InterfaceDefs.h>
#include <Message.h>
#include <Screen.h>
#include <String.h>
#include <View.h>

#include <cstring>

namespace {

BString
VersionString()
{
	BString version("Version 0.1.0");
	app_info appInfo;
	if (be_app->GetAppInfo(&appInfo) != B_OK)
		return version;

	BFile file(&appInfo.ref, B_READ_ONLY);
	BAppFileInfo fileInfo(&file);
	version_info info;
	memset(&info, 0, sizeof(info));
	if (fileInfo.GetVersionInfo(&info, B_APP_VERSION_KIND) != B_OK)
		return version;

	version.SetToFormat("Version %" B_PRIu32 ".%" B_PRIu32 ".%" B_PRIu32,
		info.major, info.middle, info.minor);
	const char* variety = NULL;
	switch (info.variety) {
		case B_DEVELOPMENT_VERSION:
			variety = "development";
			break;
		case B_ALPHA_VERSION:
			variety = "alpha";
			break;
		case B_BETA_VERSION:
			variety = "beta";
			break;
		case B_GAMMA_VERSION:
			variety = "gamma";
			break;
		case B_GOLDEN_MASTER_VERSION:
			variety = "release candidate";
			break;
		default:
			break;
	}
	if (variety != NULL) {
		version << " " << variety;
		if (info.internal > 0)
			version << " " << info.internal;
	}
	return version;
}
//---------------------------------------------------------------------------------------------------------------------------------//


class AboutView : public BView {
public:
	AboutView();
	virtual ~AboutView();

	virtual void AttachedToWindow();
	virtual void Draw(BRect updateRect);
	virtual void MouseDown(BPoint where);

private:
	BBitmap* fIcon;
	BButton* fCloseButton;
	BString fVersion;
};


AboutView::AboutView()
	:
	BView(BRect(0, 0, 450, 270), "about view", B_FOLLOW_ALL, B_WILL_DRAW),
	fIcon(new BBitmap(BRect(0, 0, 31, 31), B_CMAP8)),
	fCloseButton(new BButton(BRect(360, 230, 430, 258), "close", "OK",
		new BMessage(B_QUIT_REQUESTED))),
	fVersion(VersionString())
{
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	app_info appInfo;
	if (fIcon->InitCheck() == B_OK
		&& be_app->GetAppInfo(&appInfo) == B_OK) {
		BFile file(&appInfo.ref, B_READ_ONLY);
		BAppFileInfo fileInfo(&file);
		if (fileInfo.GetIcon(fIcon, B_LARGE_ICON) != B_OK) {
			delete fIcon;
			fIcon = NULL;
		}
	} else {
		delete fIcon;
		fIcon = NULL;
	}

	AddChild(fCloseButton);
}
//---------------------------------------------------------------------------------------------------------------------------------//


AboutView::~AboutView()
{
	delete fIcon;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
AboutView::AttachedToWindow()
{
	fCloseButton->SetTarget(Window());
	fCloseButton->MakeDefault(true);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
AboutView::Draw(BRect updateRect)
{
	(void)updateRect;
	BRect bounds = Bounds();
	SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),
		B_LIGHTEN_2_TINT));
	FillRect(bounds);

	SetHighColor(255, 203, 51);
	FillRect(BRect(0, 0, bounds.right, 27));
	SetHighColor(0, 102, 203);
	FillRect(BRect(0, 28, 12, bounds.bottom));
	SetHighColor(230, 134, 0);
	FillRect(BRect(13, 28, 19, bounds.bottom));

	if (fIcon != NULL) {
		SetDrawingMode(B_OP_OVER);
		DrawBitmap(fIcon, fIcon->Bounds(), BRect(40, 52, 135, 147));
		SetDrawingMode(B_OP_COPY);
	}

	BFont titleFont(*be_bold_font);
	titleFont.SetSize(26);
	SetFont(&titleFont);
	SetHighColor(35, 75, 105);
	DrawString("Music Manager", BPoint(160, 73));

	BFont versionFont(*be_plain_font);
	versionFont.SetSize(13);
	SetFont(&versionFont);
	SetHighColor(ui_color(B_PANEL_TEXT_COLOR));
	DrawString(fVersion.String(), BPoint(161, 99));

	SetFont(be_plain_font);
	DrawString("A native music library and playlist manager",
		BPoint(161, 128));
	DrawString("for Haiku.", BPoint(161, 147));

	SetHighColor(tint_color(ui_color(B_PANEL_TEXT_COLOR),
		B_DISABLED_LABEL_TINT));
	DrawString("Inspired by MusicManager2 by Mikael Eiman",
		BPoint(40, 187));
	DrawString("Haiku recreation " B_UTF8_COPYRIGHT " 2026 Sikosis",
		BPoint(40, 207));
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
AboutView::MouseDown(BPoint where)
{
	(void)where;
	Window()->PostMessage(B_QUIT_REQUESTED);
}
//---------------------------------------------------------------------------------------------------------------------------------//

}


AboutWindow::AboutWindow()
	:
	BWindow(BRect(0, 0, 450, 270), "About Music Manager",
		B_NO_BORDER_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL,
		B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS
			| B_CLOSE_ON_ESCAPE)
{
	AddChild(new AboutView());

	BScreen screen(this);
	BRect screenFrame = screen.Frame();
	MoveTo(screenFrame.left + (screenFrame.Width() - Frame().Width()) / 2,
		screenFrame.top + (screenFrame.Height() - Frame().Height()) / 2);
}
//---------------------------------------------------------------------------------------------------------------------------------//
