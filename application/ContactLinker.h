/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CONTACT_LINKER_H_
#define _CONTACT_LINKER_H_

#include <String.h>
#include <Message.h>
#include <Messenger.h>

#include "Notifier.h"
#include "CayaConstants.h"

class BBitmap;

class ChatWindow;
class ContactPopUp;
class ProtocolLooper;
class RosterItem;

class ContactLinker : public Notifier {
public:
					ContactLinker(BString id, BMessenger msgn);

	ChatWindow*		GetChatWindow();
	void 			DeleteWindow();

	void			ShowWindow();		
	void			HideWindow();

	void			ShowPopUp(BPoint where);
	void			DeletePopUp();
	void			HidePopUp();

	RosterItem*		GetRosterItem() { return fRosterItem; }

	BString			GetId() { return fID; }

	BMessenger		Messenger() const;
	void			SetMessenger(BMessenger messenger);

	ProtocolLooper*	GetProtocolLooper() const;
	void			SetProtocolLooper(ProtocolLooper* looper);

	BString			GetName() { return fName; }
	BBitmap*		AvatarBitmap() { return fAvatarBitmap; }

	void			SetNotifyName(BString name);
	void			SetNotifyAvatarBitmap(BBitmap* bitmap);
	void			SetNotifyStatus(CayaStatus status);
	void			SetNotifyPersonalStatus(BString personalStatus);

private:
	void			CreateChatWindow();

	RosterItem*		fRosterItem;
	ChatWindow*		fChatWindow;
	BMessenger		fMessenger;
	ProtocolLooper*	fLooper;

	BString			fID;
	bigtime_t		fInstance;
	BString			fName;
	BString			fPersonalStatus;
	BBitmap*		fAvatarBitmap;
	CayaStatus		fStatus;
	ContactPopUp*	fPopUp;
};

#endif	// _CONTACT_LINKER_H_
