/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <Alert.h>
#include <Button.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>
#include <TextControl.h>
#include <String.h>

#include <libinterface/Divider.h>

#include "AccountDialog.h"
#include "ProtocolSettings.h"
#include "TemplateView.h"

const uint32 kCancel	= 'canc';
const uint32 kOK		= 'save';


AccountDialog::AccountDialog(const char* title, ProtocolSettings* settings,
	const char* account)
	:
	BWindow(BRect(0, 0, 1, 1), title, B_MODAL_WINDOW, B_NOT_RESIZABLE
		| B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE),
	fSettings(settings),
	fAccount(account),
	fTarget(NULL)
{
	fAccountName = new BTextControl("accountName", "Account name:", NULL, NULL);
	fAccountName->SetExplicitMinSize(BSize(300, B_SIZE_UNSET));
	fAccountName->SetFont(be_bold_font);

	fAccountName->SetText(fAccount.String());

	Divider* divider = new Divider("divider", B_WILL_DRAW);

	fTop = new TemplateView("top");
	if (fAccount.Length() > 0)
		fSettings->Load(fAccount.String(), fTop);
	else
		fSettings->Load(NULL, fTop);

	BButton* cancel = new BButton("Cancel", new BMessage(kCancel));
	BButton* ok = new BButton("OK", new BMessage(kOK));

	const float spacing = be_control_look->DefaultItemSpacing();
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.Add(fAccountName)
		.Add(divider)
		.Add(fTop)
		.AddGroup(B_HORIZONTAL, spacing)
			.AddGlue()
			.Add(cancel)
			.Add(ok)
		.End()
		.AddGlue()
		.SetInsets(spacing, spacing, spacing, 0);

	fAccountName->MakeFocus(true);

	CenterOnScreen();
}


void
AccountDialog::SetTarget(BHandler* target)
{
	fTarget = target;
}


void
AccountDialog::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case kOK: {
			// Are we renaming or adding?
			bool renaming = ((fAccount.Length() > 0)
				&& (fAccount != fAccountName->Text()));
			bool adding = fAccount.Length() == 0;

			// Rename account settings
			if (renaming) {
				if (fSettings->Rename(fAccount.String(), fAccountName->Text()) != B_OK) {
					BString text("An error is occurred renaming the account ");
					text << fAccount << " to " << fAccountName->Text() << "!";
					BAlert* alert = new BAlert("", text.String(), "OK", NULL, NULL,
						B_WIDTH_AS_USUAL, B_STOP_ALERT);
					alert->Go();
					return;
				}
			}

			// Save account settings
			BString error = "An error has occured saving the settings.\n"
				"Check if your disk has enough space.";

			if (fSettings->Save(fAccountName->Text(), fTop, &error) == B_OK) {
				if (fTarget && (adding || renaming)) {
					BMessage* saveMsg = new BMessage(renaming
							? kAccountRenamed : kAccountAdded);
					saveMsg->AddPointer("settings", fSettings);
					if (renaming) {
						saveMsg->AddString("from", fAccount.String());
						saveMsg->AddString("to", fAccountName->Text());
					} else
						saveMsg->AddString("account", fAccountName->Text());
					BMessenger(fTarget).SendMessage(saveMsg);
				}

				Close();
			} else {
				BAlert* alert = new BAlert("", error.String(), "OK", NULL, NULL,
					B_WIDTH_AS_USUAL, B_STOP_ALERT);
				alert->Go();
			}
			break;
		}
		case kCancel:
			Close();
			break;
		case kChanged:
			break;
		default:
			BWindow::MessageReceived(msg);
	}
}
