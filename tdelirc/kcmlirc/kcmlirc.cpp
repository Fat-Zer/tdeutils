/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <tqcheckbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <tqradiobutton.h>
#include <tqcombobox.h>
#include <tqevent.h>
#include <tqlistview.h>

#include <kpushbutton.h>
#include <tdeapplication.h>
#include <tdelocale.h>
#include <tdeglobal.h>
#include <tdeconfig.h>
#include <kicondialog.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <ksimpleconfig.h>
#include <kgenericfactory.h>
#include <tdelistview.h>
#include <tdemessagebox.h>
#include <kpushbutton.h>

#include <dcopclient.h>
#include <dcopref.h>

#include <irkick_stub.h>

#include "addaction.h"
#include "newmode.h"
#include "profileserver.h"
#include "remoteserver.h"
#include "kcmlirc.h"
#include "editaction.h"
#include "editmode.h"
#include "modeslist.h"
#include "selectprofile.h"

typedef KGenericFactory<KCMLirc, TQWidget> theFactory;
K_EXPORT_COMPONENT_FACTORY(kcmlirc, theFactory("kcmlirc"))

KCMLirc::KCMLirc(TQWidget *parent, const char *name, TQStringList /*args*/) : DCOPObject("KCMLirc"), TDECModule(parent, name)
{
	TDEGlobal::locale()->insertCatalogue( "kcmlirc" );
	setAboutData(new TDEAboutData("kcmlirc", I18N_NOOP("TDE Lirc"), VERSION, I18N_NOOP("The TDE IR Remote Control System"), TDEAboutData::License_GPL_V2, "Copyright (c)2003 Gav Wood", I18N_NOOP("Use this to configure TDE's infrared remote control system in order to control any TDE application with your infrared remote control."), "http://www.kde.org"));
	setButtons(TDECModule::Help);
	setQuickHelp(i18n("<h1>Remote Controls</h1><p>This module allows you to configure bindings between your remote controls and TDE applications. Simply select your remote control and click Add under the Actions/Buttons list. If you want TDE to attempt to automatically assign buttons to a supported application's actions, try clicking the Auto-Populate button.</p><p>To view the recognised applications and remote controls, simply select the <em>Loaded Extensions</em> tab.</p>"));
	bool ok;
	TDEApplication::kApplication()->dcopClient()->remoteInterfaces("irkick", "IRKick", &ok);
	if(!ok)
		if(KMessageBox::questionYesNo(this, i18n("The Infrared Remote Control software is not currently running. This configuration module will not work properly without it. Would you like to start it now?"), i18n("Software Not Running"), i18n("Start"), i18n("Do Not Start")) == KMessageBox::Yes)
		{	kdDebug() << "S" << TDEApplication::startServiceByDesktopName("irkick") << endl;
			KSimpleConfig theConfig("irkickrc");
			theConfig.setGroup("General");
			if(theConfig.readBoolEntry("AutoStart", true) == false)
				if(KMessageBox::questionYesNo(this, i18n("Would you like the infrared remote control software to start automatically when you begin TDE?"), i18n("Automatically Start?"), i18n("Start Automatically"), i18n("Do Not Start")) == KMessageBox::Yes)
					theConfig.writeEntry("AutoStart", true);
		}

	TDEApplication::kApplication()->dcopClient()->remoteInterfaces("irkick", "IRKick", &ok);
	kdDebug() << "OK" << ok << endl;


	(new TQHBoxLayout(this))->setAutoAdd(true);
	theKCMLircBase = new KCMLircBase(this);
	connect(theKCMLircBase->theModes, TQT_SIGNAL( selectionChanged(TQListViewItem *) ), this, TQT_SLOT( updateActions() ));
	connect(theKCMLircBase->theModes, TQT_SIGNAL( selectionChanged(TQListViewItem *) ), this, TQT_SLOT( updateModesStatus(TQListViewItem *) ));
	connect(theKCMLircBase->theActions, TQT_SIGNAL( currentChanged(TQListViewItem *) ), this, TQT_SLOT( updateActionsStatus(TQListViewItem *) ));
	connect(theKCMLircBase->theExtensions, TQT_SIGNAL( selectionChanged(TQListViewItem *) ), this, TQT_SLOT( updateInformation() ));
	connect(theKCMLircBase->theModes, TQT_SIGNAL( itemRenamed(TQListViewItem *) ), this, TQT_SLOT( slotRenamed(TQListViewItem *) ));
	connect(theKCMLircBase->theModes, TQT_SIGNAL(dropped(TDEListView*, TQDropEvent*, TQListViewItem*, TQListViewItem*)), this, TQT_SLOT(slotDrop(TDEListView*, TQDropEvent*, TQListViewItem*, TQListViewItem*)));
	connect((TQObject *)(theKCMLircBase->theAddActions), TQT_SIGNAL( clicked() ), this, TQT_SLOT( slotAddActions() ));
	connect((TQObject *)(theKCMLircBase->theAddAction), TQT_SIGNAL( clicked() ), this, TQT_SLOT( slotAddAction() ));
	connect((TQObject *)(theKCMLircBase->theEditAction), TQT_SIGNAL( clicked() ), this, TQT_SLOT( slotEditAction() ));
	connect((TQObject *)(theKCMLircBase->theActions), TQT_SIGNAL( doubleClicked(TQListViewItem *) ), this, TQT_SLOT( slotEditAction() ));
	connect((TQObject *)(theKCMLircBase->theRemoveAction), TQT_SIGNAL( clicked() ), this, TQT_SLOT( slotRemoveAction() ));
	connect((TQObject *)(theKCMLircBase->theAddMode), TQT_SIGNAL( clicked() ), this, TQT_SLOT( slotAddMode() ));
	connect((TQObject *)(theKCMLircBase->theEditMode), TQT_SIGNAL( clicked() ), this, TQT_SLOT( slotEditMode() ));
	connect((TQObject *)(theKCMLircBase->theRemoveMode), TQT_SIGNAL( clicked() ), this, TQT_SLOT( slotRemoveMode() ));
	load();
}

KCMLirc::~KCMLirc()
{
}

void KCMLirc::updateModesStatus(TQListViewItem *item)
{
	theKCMLircBase->theModes->setItemsRenameable(item && item->parent());
	theKCMLircBase->theAddActions->setEnabled(ProfileServer::profileServer()->profiles().count() && theKCMLircBase->theModes->selectedItem() && RemoteServer::remoteServer()->remotes()[modeMap[theKCMLircBase->theModes->selectedItem()].remote()]);
	theKCMLircBase->theAddAction->setEnabled(item);
	theKCMLircBase->theAddMode->setEnabled(item);
	theKCMLircBase->theRemoveMode->setEnabled(item && item->parent());
	theKCMLircBase->theEditMode->setEnabled(item);
}

void KCMLirc::updateActionsStatus(TQListViewItem *item)
{
	theKCMLircBase->theRemoveAction->setEnabled(item);
	theKCMLircBase->theEditAction->setEnabled(item);
}

void KCMLirc::slotRenamed(TQListViewItem *item)
{
	if(!item) return;

	if(item->parent() && item->text(0) != modeMap[item].name())
	{	allActions.renameMode(modeMap[item], item->text(0));
		allModes.rename(modeMap[item], item->text(0));
		emit changed(true);
		updateModes();
	}
}

void KCMLirc::slotEditAction()
{
	if(!theKCMLircBase->theActions->currentItem()) return;

	EditAction theDialog(actionMap[theKCMLircBase->theActions->currentItem()], this);
	TQListViewItem *item = theKCMLircBase->theModes->currentItem();
	if(item->parent()) item = item->parent();
	theDialog.theModes->insertItem(i18n("[Exit current mode]"));
	for(item = item->firstChild(); item; item = item->nextSibling())
		theDialog.theModes->insertItem(item->text(0));
	theDialog.readFrom();
	if(theDialog.exec() == TQDialog::Accepted) { theDialog.writeBack(); emit changed(true); updateActions(); }
}

void KCMLirc::slotAddActions()
{
	if(!theKCMLircBase->theModes->selectedItem()) return;
	Mode m = modeMap[theKCMLircBase->theModes->selectedItem()];
	if(!RemoteServer::remoteServer()->remotes()[m.remote()]) return;

	SelectProfile theDialog(this, 0);

	TQMap<TQListViewItem *, Profile *> profileMap;
	TQDict<Profile> dict = ProfileServer::profileServer()->profiles();
	for(TQDictIterator<Profile> i(dict); i.current(); ++i) profileMap[new TQListViewItem(theDialog.theProfiles, i.current()->name())] = i.current();

	if(theDialog.exec() == TQDialog::Accepted && theDialog.theProfiles->currentItem())
	{	autoPopulate(*(profileMap[theDialog.theProfiles->currentItem()]), *(RemoteServer::remoteServer()->remotes()[m.remote()]), m.name());
		updateActions();
		emit changed(true);
	}
}

void KCMLirc::slotAddAction()
{
	kdDebug() << k_funcinfo << endl;
	if(!theKCMLircBase->theModes->selectedItem()) return;
	Mode m = modeMap[theKCMLircBase->theModes->selectedItem()];

	AddAction theDialog(this, 0, m);
	connect(this, TQT_SIGNAL(haveButton(const TQString &, const TQString &)), &theDialog, TQT_SLOT(updateButton(const TQString &, const TQString &)));

	// populate the modes list box
	TQListViewItem *item = theKCMLircBase->theModes->selectedItem();
	if(item->parent()) item = item->parent();
	theDialog.theModes->setEnabled(item->firstChild());
	theDialog.theSwitchMode->setEnabled(item->firstChild());
	for(item = item->firstChild(); item; item = item->nextSibling())
	{	TDEListViewItem *a = new TDEListViewItem(theDialog.theModes, item->text(0));
		if(item->isSelected()) { a->setSelected(true); theDialog.theModes->setCurrentItem(a); }
	}

	if(theDialog.exec() == TQDialog::Accepted && theDialog.theButtons->selectedItem())
	{	IRAction a;
		a.setRemote(m.remote());
		a.setMode(m.name());
		a.setButton(theDialog.buttonMap[theDialog.theButtons->selectedItem()]);
		a.setRepeat(theDialog.theRepeat->isChecked());
		a.setAutoStart(theDialog.theAutoStart->isChecked());
		a.setDoBefore(theDialog.theDoBefore->isChecked());
		a.setDoAfter(theDialog.theDoAfter->isChecked());
		a.setUnique(theDialog.isUnique);
		a.setIfMulti(theDialog.theDontSend->isChecked() ? IM_DONTSEND : theDialog.theSendToTop->isChecked() ? IM_SENDTOTOP : theDialog.theSendToBottom->isChecked() ? IM_SENDTOBOTTOM : IM_SENDTOALL);
		// change mode?
		if(theDialog.theChangeMode->isChecked())
		{
			if(theDialog.theSwitchMode->isChecked() && theDialog.theModes->selectedItem())
			{
				a.setProgram("");
				a.setObject(theDialog.theModes->selectedItem()->text(0));
			}
			else if(theDialog.theExitMode->isChecked())
			{
				a.setProgram("");
				a.setObject("");
			}
			a.setAutoStart(false);
			a.setRepeat(false);
		}
		// DCOP?
		else if(theDialog.theUseDCOP->isChecked() && theDialog.theObjects->selectedItem() && theDialog.theObjects->selectedItem()->parent() && theDialog.theFunctions->selectedItem())
		{
			a.setProgram(theDialog.program);
			a.setObject(theDialog.theObjects->selectedItem()->text(0));
			a.setMethod(theDialog.theFunctions->selectedItem()->text(2));
			theDialog.theParameters->setSorting(3);
			a.setArguments(theDialog.theArguments);
		}
		// profile?
		else if(theDialog.theUseProfile->isChecked() && theDialog.theProfiles->selectedItem() && (theDialog.theProfileFunctions->selectedItem() || theDialog.theJustStart->isChecked()))
		{
			ProfileServer *theServer = ProfileServer::profileServer();

			if(theDialog.theNotJustStart->isChecked())
			{	const ProfileAction *theAction = theServer->getAction(theDialog.profileMap[theDialog.theProfiles->selectedItem()], theDialog.profileFunctionMap[theDialog.theProfileFunctions->selectedItem()]);
				a.setProgram(theAction->profile()->id());
				a.setObject(theAction->objId());
				a.setMethod(theAction->prototype());
				theDialog.theParameters->setSorting(3);
				a.setArguments(theDialog.theArguments);
			}
			else
			{	a.setProgram(theServer->profiles()[theDialog.profileMap[theDialog.theProfiles->selectedItem()]]->id());
				a.setObject("");
			}
		}

		// save our new action
		allActions.addAction(a);
		updateActions();
		emit changed(true);
	}
}

void KCMLirc::slotRemoveAction()
{
	if(!theKCMLircBase->theActions->currentItem()) return;

	IRAIt i = actionMap[theKCMLircBase->theActions->currentItem()];
	allActions.erase(i);
	updateActions();
	emit changed(true);
}

void KCMLirc::autoPopulate(const Profile &profile, const Remote &remote, const TQString &mode)
{
	TQDict<RemoteButton> d = remote.buttons();
	for(TQDictIterator<RemoteButton> i(d); i.current(); ++i)
	{	const ProfileAction *pa = profile.searchClass(i.current()->getClass());
		if(pa)
		{
			IRAction a;
			a.setRemote(remote.id());
			a.setMode(mode);
			a.setButton(i.current()->id());
			a.setRepeat(pa->repeat());
			a.setAutoStart(pa->autoStart());
			a.setProgram(pa->profile()->id());
			a.setObject(pa->objId());
			a.setMethod(pa->prototype());
			a.setUnique(pa->profile()->unique());
			a.setIfMulti(pa->profile()->ifMulti());
			Arguments l;
			// argument count should be either 0 or 1. undefined if > 1.
			if(Prototype(pa->prototype()).argumentCount() == 1)
			{
				l.append(TQString(TQString().setNum(i.current()->parameter().toFloat() * pa->multiplier())));
				l.back().cast(TQVariant::nameToType(Prototype(pa->prototype()).type(0).utf8()));
			}
			a.setArguments(l);
			allActions.addAction(a);
		}
	}
}

void KCMLirc::slotAddMode()
{
	if(!theKCMLircBase->theModes->selectedItem()) return;

	NewMode theDialog(this, 0);
	TQMap<TQListViewItem *, TQString> remoteMap;
	TQListViewItem *tr = theKCMLircBase->theModes->selectedItem();
	if(tr) if(tr->parent()) tr = tr->parent();
	for(TQListViewItem *i = theKCMLircBase->theModes->firstChild(); i; i = i->nextSibling())
	{	TDEListViewItem *a = new TDEListViewItem(theDialog.theRemotes, i->text(0));
		remoteMap[a] = modeMap[i].remote();
		if(i == tr) { a->setSelected(true); theDialog.theRemotes->setCurrentItem(a); }
	}
	if(theDialog.exec() == TQDialog::Accepted && theDialog.theRemotes->selectedItem() && !theDialog.theName->text().isEmpty())
	{
		allModes.add(Mode(remoteMap[theDialog.theRemotes->selectedItem()], theDialog.theName->text()));
		updateModes();
		emit changed(true);
	}
}

void KCMLirc::slotEditMode()
{
	if(!theKCMLircBase->theModes->selectedItem()) return;

	EditMode theDialog(this, 0);

	Mode &mode = modeMap[theKCMLircBase->theModes->selectedItem()];
	theDialog.theName->setEnabled(theKCMLircBase->theModes->selectedItem()->parent());
	theDialog.theName->setText(mode.name().isEmpty() ? mode.remoteName() : mode.name());
	if(!mode.iconFile().isNull())
		theDialog.theIcon->setIcon(mode.iconFile());
	else
		theDialog.theIcon->resetIcon();
	theDialog.theDefault->setChecked(allModes.isDefault(mode));
	theDialog.theDefault->setEnabled(!allModes.isDefault(mode));

	if(theDialog.exec() == TQDialog::Accepted)
	{	kdDebug() << "Setting icon : " << theDialog.theIcon->icon() << endl;
		mode.setIconFile(theDialog.theIcon->icon().isEmpty() ? TQString() : theDialog.theIcon->icon());
		allModes.updateMode(mode);
		if(!mode.name().isEmpty())
		{	allActions.renameMode(mode, theDialog.theName->text());
			allModes.rename(mode, theDialog.theName->text());
		}
		if(theDialog.theDefault->isChecked()) allModes.setDefault(mode);
		emit changed(true);
		updateModes();
	}
}

void KCMLirc::slotRemoveMode()
{
	if(!theKCMLircBase->theModes->selectedItem()) return;
	if(!theKCMLircBase->theModes->selectedItem()->parent()) return;

	if(KMessageBox::warningContinueCancel(this, i18n("Are you sure you want to remove %1 and all its actions?").arg(theKCMLircBase->theModes->selectedItem()->text(0)), i18n("Erase Actions?")) == KMessageBox::Continue)
	{
		allModes.erase(modeMap[theKCMLircBase->theModes->selectedItem()]);
		updateModes();
		emit changed(true);
	}
}

void KCMLirc::slotSetDefaultMode()
{
	if(!theKCMLircBase->theModes->selectedItem()) return;
	allModes.setDefault(modeMap[theKCMLircBase->theModes->selectedItem()]);
	updateModes();
	emit changed(true);
}

void KCMLirc::slotDrop(TDEListView *, TQDropEvent *, TQListViewItem *, TQListViewItem *after)
{
	Mode m = modeMap[after];

	if(modeMap[theKCMLircBase->theModes->selectedItem()].remote() != m.remote())
	{
		KMessageBox::error(this, i18n("You may only drag the selected items onto a mode of the same remote control"), i18n("You May Not Drag Here"));
		return;
	}
	for(TQListViewItem *i = theKCMLircBase->theActions->firstChild(); i; i = i->nextSibling())
		if(i->isSelected())
			(*(actionMap[i])).setMode(m.name());

	updateActions();
	emit changed(true);
}

void KCMLirc::updateActions()
{
	IRAIt oldCurrent;
	if(theKCMLircBase->theActions->selectedItem()) oldCurrent = actionMap[theKCMLircBase->theActions->selectedItem()];

	theKCMLircBase->theActions->clear();
	actionMap.clear();

	if(!theKCMLircBase->theModes->selectedItem()) { updateActionsStatus(0); return; }

	Mode m = modeMap[theKCMLircBase->theModes->selectedItem()];
	theKCMLircBase->theModeLabel->setText(m.remoteName() + ": " + (m.name().isEmpty() ? i18n("Actions <i>always</i> available") : i18n("Actions available only in mode <b>%1</b>").arg(m.name())));
	IRAItList l = allActions.findByMode(m);
	for(IRAItList::iterator i = l.begin(); i != l.end(); ++i)
	{	TQListViewItem *b = new TDEListViewItem(theKCMLircBase->theActions, (**i).buttonName(), (**i).application(), (**i).function(), (**i).arguments().toString(), (**i).notes());
		actionMap[b] = *i;
		if(*i == oldCurrent) { b->setSelected(true); theKCMLircBase->theActions->setCurrentItem(b); }
	}

	if(theKCMLircBase->theActions->currentItem())
		theKCMLircBase->theActions->currentItem()->setSelected(true);
	updateActionsStatus(theKCMLircBase->theActions->currentItem());
}

void KCMLirc::gotButton(TQString remote, TQString button)
{
	emit haveButton(remote, button);
}

void KCMLirc::updateModes()
{
	Mode oldCurrent;
	if(theKCMLircBase->theModes->selectedItem()) oldCurrent = modeMap[theKCMLircBase->theModes->selectedItem()];

	theKCMLircBase->theModes->clear();
	modeMap.clear();

	IRKick_stub IRKick("irkick", "IRKick");
	TQStringList remotes = IRKick.remotes();
	if(remotes.begin() == remotes.end())
		theKCMLircBase->theMainLabel->setMaximumSize(32767, 32767);
	else
		theKCMLircBase->theMainLabel->setMaximumSize(0, 0);
	for(TQStringList::iterator i = remotes.begin(); i != remotes.end(); ++i)
	{	Mode mode = allModes.getMode(*i, "");
		TQListViewItem *a = new TDEListViewItem(theKCMLircBase->theModes, RemoteServer::remoteServer()->getRemoteName(*i), allModes.isDefault(mode) ? "Default" : "", mode.iconFile().isNull() ? "" : "");
		if(!mode.iconFile().isNull())
			a->setPixmap(2, TDEIconLoader().loadIcon(mode.iconFile(), TDEIcon::Panel));
		modeMap[a] = mode;	// the null mode
		if(modeMap[a] == oldCurrent) { a->setSelected(true); theKCMLircBase->theModes->setCurrentItem(a); }
		a->setOpen(true);
		ModeList l = allModes.getModes(*i);
		for(ModeList::iterator j = l.begin(); j != l.end(); ++j)
			if(!(*j).name().isEmpty())
			{	TQListViewItem *b = new TDEListViewItem(a, (*j).name(), allModes.isDefault(*j) ? i18n("Default") : "", (*j).iconFile().isNull() ? "" : "");
				if(!(*j).iconFile().isNull())
					b->setPixmap(2, TDEIconLoader().loadIcon((*j).iconFile(), TDEIcon::Panel));
				modeMap[b] = *j;
				if(*j == oldCurrent) { b->setSelected(true); theKCMLircBase->theModes->setCurrentItem(b); }
			}
	}
	if(theKCMLircBase->theModes->currentItem())
		theKCMLircBase->theModes->currentItem()->setSelected(true);
	updateModesStatus(theKCMLircBase->theModes->currentItem());
	updateActions();
}

void KCMLirc::updateExtensions()
{
	theKCMLircBase->theExtensions->clear();

	{	ProfileServer *theServer = ProfileServer::profileServer();
		TQListViewItem *a = new TQListViewItem(theKCMLircBase->theExtensions, i18n("Applications"));
		a->setOpen(true);
		profileMap.clear();
		TQDict<Profile> dict = theServer->profiles();
		TQDictIterator<Profile> i(dict);
		for(; i.current(); ++i)
			profileMap[new TQListViewItem(a, i.current()->name())] = i.currentKey();
	}
	{	RemoteServer *theServer = RemoteServer::remoteServer();
		TQListViewItem *a = new TQListViewItem(theKCMLircBase->theExtensions, i18n("Remote Controls"));
		a->setOpen(true);
		remoteMap.clear();
		TQDict<Remote> dict = theServer->remotes();
		TQDictIterator<Remote> i(dict);
		for(; i.current(); ++i)
			remoteMap[new TQListViewItem(a, i.current()->name())] = i.currentKey();
	}
	updateInformation();
}

void KCMLirc::updateInformation()
{
	theKCMLircBase->theInformation->clear();
	theKCMLircBase->theInformationLabel->setText("");

	if(!theKCMLircBase->theExtensions->selectedItem()) return;

	if(!theKCMLircBase->theExtensions->selectedItem()->parent())
	{
		theKCMLircBase->theInformationLabel->setText(i18n("Information on <b>%1</b>:").arg(theKCMLircBase->theExtensions->selectedItem()->text(0)));
		if(theKCMLircBase->theExtensions->selectedItem()->text(0) == i18n("Applications"))
			new TQListViewItem(theKCMLircBase->theInformation, i18n("Number of Applications"), TQString().setNum(theKCMLircBase->theExtensions->selectedItem()->childCount()));
		else if(theKCMLircBase->theExtensions->selectedItem()->text(0) == i18n("Remote Controls"))
			new TQListViewItem(theKCMLircBase->theInformation, i18n("Number of Remote Controls"), TQString().setNum(theKCMLircBase->theExtensions->selectedItem()->childCount()));
	}
	else if(theKCMLircBase->theExtensions->selectedItem()->parent()->text(0) == i18n("Applications"))
	{
		ProfileServer *theServer = ProfileServer::profileServer();
		const Profile *p = theServer->profiles()[profileMap[theKCMLircBase->theExtensions->selectedItem()]];
		new TQListViewItem(theKCMLircBase->theInformation, i18n("Extension Name"), p->name());
		new TQListViewItem(theKCMLircBase->theInformation, i18n("Extension Author"), p->author());
		new TQListViewItem(theKCMLircBase->theInformation, i18n("Application Identifier"), p->id());
		new TQListViewItem(theKCMLircBase->theInformation, i18n("Number of Actions"), TQString().setNum(p->actions().count()));
		theKCMLircBase->theInformationLabel->setText(i18n("Information on <b>%1</b>:").arg(p->name()));
	}
	else if(theKCMLircBase->theExtensions->selectedItem()->parent()->text(0) == i18n("Remote Controls"))
	{
		RemoteServer *theServer = RemoteServer::remoteServer();
		const Remote *p = theServer->remotes()[remoteMap[theKCMLircBase->theExtensions->selectedItem()]];
		new TQListViewItem(theKCMLircBase->theInformation, i18n("Extension Name"), p->name());
		new TQListViewItem(theKCMLircBase->theInformation, i18n("Extension Author"), p->author());
		new TQListViewItem(theKCMLircBase->theInformation, i18n("Remote Control Identifier"), p->id());
		new TQListViewItem(theKCMLircBase->theInformation, i18n("Number of Buttons"), TQString().setNum(p->buttons().count()));
		theKCMLircBase->theInformationLabel->setText(i18n("Information on <b>%1</b>:").arg(p->name()));
	}
}

void KCMLirc::load()
{
	KSimpleConfig theConfig("irkickrc");
	allActions.loadFromConfig(theConfig);
	allModes.loadFromConfig(theConfig);
	allModes.generateNulls(IRKick_stub("irkick", "IRKick").remotes());

	updateExtensions();
	updateModes();
	updateActions();
}

void KCMLirc::defaults()
{
	// insert your default settings code here...
	emit changed(true);
}

void KCMLirc::save()
{
	KSimpleConfig theConfig("irkickrc");
	allActions.saveToConfig(theConfig);
	allModes.saveToConfig(theConfig);

	theConfig.sync();
	IRKick_stub("irkick", "IRKick").reloadConfiguration();

	emit changed(true);
}

void KCMLirc::configChanged()
{
 // insert your saving code here...
    emit changed(true);
}

// TODO: Take this out when I know how
extern "C"
{
	KDE_EXPORT TDECModule *create_kcmlirc(TQWidget *parent, const char *)
	{	TDEGlobal::locale()->insertCatalogue("kcmlirc");
		return new KCMLirc(parent, "KCMLirc");
	}
}

#include <irkick_stub.cpp>

#include "kcmlirc.moc"
