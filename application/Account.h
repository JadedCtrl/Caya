/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _ACCOUNT_H
#define _ACCOUNT_H

#include <Handler.h>
#include <Messenger.h>
#include <String.h>

#include "ChatProtocol.h"

class Account : public ChatProtocolMessengerInterface {
public:
							Account(bigtime_t instanceId, ChatProtocol* cayap,
									const char* name, const char* addOnSignature,
									BHandler* target);
	virtual					~Account();

			bigtime_t		Identifier() const;
			const char*		Name() const;

	virtual	status_t		SendMessage(BMessage* message);

private:
			bigtime_t		fIdentifier;
			ChatProtocol*	fProtocol;
			BString			fName;
			BMessenger		fMessenger;
			BMessage*		fSettings;
};

#endif	// _ACCOUNT_H
