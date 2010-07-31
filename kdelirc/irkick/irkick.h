/***************************************************************************
                          irkick.h  -  Interface of the main window
    copyright            : (C) 2002 by Gav Wood
    email                : gav@kde.org

***************************************************************************/

// This program is free software.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef __IRKICK_H
#define __IRKICK_H

#include <tqstring.h>

#include <ksystemtray.h>
#include <kdedmodule.h>
#include <kaboutdata.h>

#include <dcopobject.h>

#include "modes.h"
#include "iractions.h"
#include "klircclient.h"

class QMouseEvent;

class IRKTrayIcon: public KSystemTray
{
	void mousePressEvent(TQMouseEvent *e);

public:
	KPopupMenu* contextMenu() const { return KSystemTray::contextMenu(); }
	KActionCollection* actionCollection() { return KSystemTray::actionCollection(); }

	IRKTrayIcon(TQWidget *parent = 0, const char *name = 0): KSystemTray(parent, name) {}
};

class IRKick: public TQObject, public DCOPObject
{
	Q_OBJECT
	K_DCOP

	TQString npApp, npModule, npMethod;
	TQMap<TQString, TQString> currentModes;
	TQMap<TQString, IRKTrayIcon *> currentModeIcons;
	IRActions allActions;
	int theResetCount;
	Modes allModes;

	IRKTrayIcon *theTrayIcon;
	KAboutData *aboutData;
	TQTimer *theFlashOff;

	void updateModeIcons();

protected:
	KLircClient *theClient;

k_dcop:
	/**
	 * Query status of connection.
	 *
	 * @returns true if connected to lircd.
	 */
	virtual bool isConnected() { return theClient->isConnected(); }

	/**
	 * Query status of remote list.
	 *
	 * Make sure this is true before calling remotes()/buttons(...).
	 *
	 * @returns true if up to date.
	 */
	virtual bool haveFullList() { return theClient->haveFullList(); }

	/**
	 * Retrieve list of remote controls.
	 *
	 * @returns said list.
	 */
	virtual const TQStringList remotes() { return theClient->remotes(); }

	/**
	 * Retrieve list of buttons of a praticular remote control.
	 *
	 * @returns said list.
	 */
	virtual const TQStringList buttons(TQString theRemote) { return theClient->buttons(theRemote); }

	/**
	 * Sends next keypress to given application by DCOP.
	 *
	 * @param The receiving application.
	 * @param The receiving application module.
	 * @param The method name. Must have two TQString parameters.
	 */
	virtual void stealNextPress(TQString app, TQString module, TQString method);

	/**
	 * Cancels the proceedings of the previous stealNextPress call, if any.
	 */
	virtual void dontStealNextPress();

	/**
	 * Reloads configuration from file(s)
	 */
	virtual void reloadConfiguration() { slotReloadConfiguration(); }

private slots:
	void gotMessage(const TQString &theRemote, const TQString &theButton, int theRepeatCounter);
	void resetModes();
	void doQuit();
	void flashOff();
	void checkLirc();

	void slotConfigure();
	void slotReloadConfiguration();
	void slotClosed();
private:
	void executeAction(const IRAction &action);
	bool getPrograms(const IRAction &action, TQStringList &populous);

public:
	IRKick(const TQCString &obj);
	virtual ~IRKick();
};

#endif
