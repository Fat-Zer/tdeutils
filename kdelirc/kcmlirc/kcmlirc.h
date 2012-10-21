/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef _KCMLIRC_H_
#define _KCMLIRC_H_

#include <tqstringlist.h>
#include <tqmap.h>

#include <kcmodule.h>

#include <dcopobject.h>

#include "kcmlircbase.h"
#include "iractions.h"
#include "modes.h"

class TQListViewItem;
class KListView;
class TQDropEvent;
class Profile;
class Remote;

class KCMLirc: public KCModule, virtual public DCOPObject
{
	Q_OBJECT
//	
	K_DCOP

private:
	KCMLircBase *theKCMLircBase;
	IRActions allActions;
	Modes allModes;
	TQMap<TQListViewItem *, IRAIt > actionMap;
	TQMap<TQListViewItem *, Mode> modeMap;
	TQMap<TQListViewItem *, TQString> profileMap, remoteMap;

	void autoPopulate(const Profile &profile, const Remote &remote, const TQString &mode);

public slots:
	void updateActions();
	void updateModesStatus(TQListViewItem *);
	void updateActionsStatus(TQListViewItem *);
	void updateModes();
	void updateExtensions();
	void updateInformation();
	void slotAddMode();
	void slotRemoveMode();
	void slotSetDefaultMode();
	void slotAddAction();
	void slotAddActions();
	void slotEditAction();
	void slotRemoveAction();
	void slotDrop(KListView *, TQDropEvent *, TQListViewItem *, TQListViewItem *after);
	void slotRenamed(TQListViewItem *item);
	void slotEditMode();

#ifndef Q_MOC_RUN
// MOC_SKIP_BEGIN
k_dcop:
// MOC_SKIP_END
#endif
	// now just used as a proxy to AddAction class
	virtual void gotButton(TQString remote, TQString button);
signals:
	void haveButton(const TQString &remote, const TQString &button);

public:
	virtual void load();
	virtual void save();
	virtual void defaults();
	virtual void configChanged();

	KCMLirc(TQWidget *parent = 0, const char *name = 0, TQStringList args = TQStringList());
	~KCMLirc();
};

#endif
