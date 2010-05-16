/*
 * Copyright 2009-2010, Andrea Anzani. All rights reserved.
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <Application.h>
#include <Debug.h>
#include <Entry.h>
#include <Path.h>
#include <TranslationUtils.h>

#include "Account.h"
#include "AccountManager.h"
#include "ProtocolLooper.h"
#include "CayaMessages.h"
#include "CayaProtocol.h"
#include "ChatWindow.h"
#include "ImageCache.h"
#include "ProtocolManager.h"
#include "RosterItem.h"
#include "Server.h"


Server::Server()
	: BMessageFilter(B_ANY_DELIVERY, B_ANY_SOURCE)
{
}


void 
Server::Quit()
{
	ContactLinker* linker = NULL;	

	while ((linker = fRosterMap.ValueAt(0))) {
		linker->DeleteWindow();
		linker->DeletePopUp();
		fRosterMap.RemoveItemAt(0);
	}
}


void
Server::AddProtocolLooper(bigtime_t instanceId, CayaProtocol* cayap)
{
	ProtocolLooper* looper = new ProtocolLooper(cayap);
	fLoopers.AddItem(instanceId, looper);
}


void
Server::RemoveProtocolLooper(bigtime_t instanceId)
{
}


void
Server::LoginAll()
{
	for (uint32 i = 0; i < fLoopers.CountItems(); i++) {
		ProtocolLooper* looper = fLoopers.ValueAt(i);

		BMessage* msg = new BMessage(IM_MESSAGE);
		msg->AddInt32("im_what", IM_SET_STATUS);
		msg->AddInt32("status", CAYA_ONLINE);
		looper->PostMessage(msg);
	}
}


#if 0
void
Server::UpdateSettings(BMessage settings)
{
	fProtocol->Protocol()->UpdateSettings(settings);
}
#endif


void
Server::SendProtocolMessage(BMessage* msg)
{
	// Skip null messages
	if (!msg)
		return;

	// Check if message contains the instance field
	bigtime_t id;
	if (msg->FindInt64("instance", &id) == B_OK) {
		bool found = false;
		ProtocolLooper* looper
			= fLoopers.ValueFor(id, &found);

		if (found)
			looper->PostMessage(msg);
	}
}


void
Server::SendAllProtocolMessage(BMessage* msg)
{
	// Skip null messages
	if (!msg)
		return;

	// Send message to all protocols
	for (uint32 i = 0; i < fLoopers.CountItems(); i++) {
		ProtocolLooper* looper = fLoopers.ValueAt(i);
		looper->PostMessage(msg);
	}
}


filter_result
Server::Filter(BMessage* message, BHandler **target)
{
	filter_result result = B_DISPATCH_MESSAGE;

	switch(message->what) {
		case IM_SERVER_BASED_CONTACT_LIST:
		{
			int i = 0;
			BString id;
			while (message->FindString("id", i++, &id) == B_OK) {
				bool found = false;
				ContactLinker* item = fRosterMap.ValueFor(id, &found);

				if (found)
					continue;

				item = new ContactLinker(id.String(), Looper());
				item->SetProtocolLooper(_LooperFromMessage(message));
				fRosterMap.AddItem(id, item);
			}
			result = B_SKIP_MESSAGE;
			break;
		}
		case IM_MESSAGE_RECEIVED:
		{
			BString id = message->FindString("id");
			if (id.Length() > 0) {
				bool found = false;
				ContactLinker* item = fRosterMap.ValueFor(id, &found);
				if (found) {
					ChatWindow* win = item->GetChatWindow();
					item->ShowWindow();
					win->PostMessage(message);
				}
			}
			result = B_SKIP_MESSAGE;
			break;
		}
		case CAYA_CLOSE_WINDOW:
		{
			BString id = message->FindString("id");
			if (id.Length() > 0) {
				bool found = false;
				ContactLinker *item = fRosterMap.ValueFor(id, &found);

				if (found)
					item->HideWindow();	
			}
			result = B_SKIP_MESSAGE;
			break;
		}
		case IM_MESSAGE:
			result = ImMessage(message);
			break;
		default:
			// Dispatch not handled messages to main window
			break;
	}

	return result;	
}


RosterMap
Server::RosterItems() const
{
	return fRosterMap;
}


RosterItem*		
Server::RosterItemForId(BString id)
{
	bool found = false;
	ContactLinker* item = fRosterMap.ValueFor(id, &found);
	return item ? item->GetRosterItem() : NULL;			
}


filter_result
Server::ImMessage(BMessage* msg)
{	
	filter_result result = B_DISPATCH_MESSAGE;
	int32 im_what = msg->FindInt32("im_what");

	switch (im_what) {
		case IM_STATUS_SET:
		{
			int32 status;
			const char* protocol;

			if (msg->FindInt32("status", &status) != B_OK)
				return B_SKIP_MESSAGE;
			if (msg->FindString("protocol", &protocol) != B_OK)
				return B_SKIP_MESSAGE;

			AccountManager* accountManager = AccountManager::Get();
			accountManager->SetStatus((CayaStatus)status);
			break;
		}
		case IM_STATUS_CHANGED:
		{
			int32 status;

			if (msg->FindInt32("status", &status) != B_OK)
				return B_SKIP_MESSAGE;

			ContactLinker* linker = _EnsureContactLinker(msg);
			linker->SetNotifyStatus((CayaStatus)status);
			linker->SetNotifyPersonalStatus(msg->FindString("message"));
			break;
		}
		case IM_CONTACT_INFO:		
		{
			ContactLinker* linker = _EnsureContactLinker(msg);

			BString fullName = msg->FindString("nick");
			if (fullName != "")
				linker->SetNotifyName(fullName);
			break;
		}
		case IM_AVATAR_CHANGED:
		{
			ContactLinker* linker = _EnsureContactLinker(msg);
			entry_ref ref;
			if (linker) {
				if (msg->FindRef("ref", &ref) == B_OK) {
					//BPath fullPath(&ref);
					//BBitmap *bitmap = ImageCache::GetImage(BString(fullPath.Path()), BString(fullPath.Path()));										
					BBitmap *bitmap = BTranslationUtils::GetBitmap(&ref);
					linker->SetNotifyAvatarBitmap(bitmap);
				} else
					linker->SetNotifyAvatarBitmap(NULL);
			}
			break;
		}
		case IM_SEND_MESSAGE: {
			// Route this message through the appropriate ProtocolLooper
			ContactLinker* linker = _EnsureContactLinker(msg);
			if (linker->GetProtocolLooper())
				linker->GetProtocolLooper()->PostMessage(msg);
			break;
		}
		case IM_MESSAGE_RECEIVED:
		{
			BString id = msg->FindString("id");
			if (id.Length() > 0) {
				bool found = false;
				ContactLinker* item = fRosterMap.ValueFor(id, &found);
				if (found) {
					ChatWindow* win = item->GetChatWindow();
					item->ShowWindow();
					win->PostMessage(msg);
				}
			}
			result = B_SKIP_MESSAGE;
			break;
		}
		default:
			break;
	}

	return result;
}


ContactLinker*	
Server::GetOwnContact()
{
	return fMySelf;
}


ProtocolLooper*
Server::_LooperFromMessage(BMessage* message)
{
	if (!message)
		return NULL;

	bigtime_t identifier;

	if (message->FindInt64("instance", &identifier) == B_OK) {
		bool found = false;

		ProtocolLooper* looper = fLoopers.ValueFor(identifier, &found);
		if (found)
			return looper;
	}

	return NULL;
}


ContactLinker*	
Server::_EnsureContactLinker(BMessage* message)
{
	if (!message)
		return NULL;

	BString id = message->FindString("id");
	ContactLinker* item = NULL;

	if (id.Length() > 0) {
		bool found = false;
		item = fRosterMap.ValueFor(id, &found);

		if (!found) {
			item = new ContactLinker(id.String(), Looper());
			item->SetProtocolLooper(_LooperFromMessage(message));
			fRosterMap.AddItem(id, item);
		}
	}

	return item;
}
