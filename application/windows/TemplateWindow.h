/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _TEMPLATE_WINDOW_H
#define _TEMPLATE_WINDOW_H

#include <String.h>
#include <Window.h>

#include "ProtocolTemplate.h"
#include "Server.h"

class BAlert;
class BMenu;
class BMenuField;
class BTextControl;
class ProtocolSettings;
class TemplateView;


class TemplateWindow : public BWindow {
public:
						/*! Get template from selected account's protocol
						 * 	via ChatProtocol::SettingsTemplate() */
						TemplateWindow(const char* title,
							const char* templateType, BMessage* msg,
							Server* server, bigtime_t instance = -1);

						/*! Use only the given template. */
						TemplateWindow(const char* title,
							ProtocolTemplate* temp, BMessage* msg,
							Server* server, bigtime_t instance = -1);

	virtual	void		MessageReceived(BMessage* msg);

			void		SetTarget(BHandler* target);

private:
			void		_InitInterface(bigtime_t instance);
			void		_LoadTemplate();

	Server*				fServer;
	AccountInstances	fAccounts;
	int32				fSelectedAcc;
	BMenuField*			fMenuField;

	ProtocolTemplate*	fTemplate;
	BString				fTemplateType;
	TemplateView*		fTemplateView;

	BButton*			fOkButton;

	BMessage*			fMessage;
	BHandler*			fTarget;
};

#endif	// _TEMPLATE_WINDOW_H
