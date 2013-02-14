/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <tqregexp.h>
#include <tqlabel.h>
#include <tqradiobutton.h>
#include <tqwidgetstack.h>
#include <tqcheckbox.h>
#include <tqbuttongroup.h>

#include <kdebug.h>
#include <klineedit.h>
#include <tdelistview.h>
#include <tdeapplication.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <keditlistbox.h>
#include <klocale.h>

#include <dcopclient.h>
#include <irkick_stub.h>

#include "prototype.h"
#include "addaction.h"
#include "profileserver.h"
#include "remoteserver.h"

AddAction::AddAction(TQWidget *parent, const char *name, const Mode &mode): AddActionBase(parent, name), theMode(mode)
{
	connect(this, TQT_SIGNAL( selected(const TQString &) ), TQT_SLOT( updateForPageChange() ));
	connect(this, TQT_SIGNAL( selected(const TQString &) ), TQT_SLOT( slotCorrectPage() ));
	curPage = 0;
	updateProfiles();
	updateButtons();
	updateObjects();
	updateProfileFunctions();
}

AddAction::~AddAction()
{
}


void AddAction::slotNextParam()
{
	// TODO: go on to next parameter
}

void AddAction::slotModeSelected()
{
	theSwitchMode->setChecked(true);
}

void AddAction::slotCorrectPage()
{
	int lastPage = curPage;
	curPage = indexOf(currentPage());

	if(curPage == 2 && theUseProfile->isChecked())
		showPage(((TQWizard *)this)->page(lastPage > 1 ? 1 : 3));
	if((curPage == 2 || curPage == 5) && theChangeMode->isChecked())
		showPage(((TQWizard *)this)->page(lastPage > 1 ? 1 : 6));

	if(curPage == 3 && theUseDCOP->isChecked())
		showPage(((TQWizard *)this)->page(lastPage == 4 ? 2 : 4));

	if(curPage == 4 && (
	(theUseDCOP->isChecked() && theFunctions->currentItem() && !Prototype(theFunctions->currentItem()->text(2)).count()) ||
	(theUseProfile->isChecked() && (theProfileFunctions->currentItem() && !theProfileFunctions->currentItem()->text(1).toInt() || theJustStart->isChecked()))
	))
		showPage(((TQWizard *)this)->page(lastPage == 5 ? (theUseDCOP->isChecked() ? 2 : 3) : 5));
}

void AddAction::requestNextPress()
{
	IRKick_stub("irkick", "IRKick").stealNextPress(DCOPClient::mainClient()->appId(), "KCMLirc", "gotButton(TQString, TQString)");
}

void AddAction::cancelRequest()
{
	IRKick_stub("irkick", "IRKick").dontStealNextPress();
}

void AddAction::updateButton(const TQString &remote, const TQString &button)
{
	if(theMode.remote() == remote)
	{	// note this isn't the "correct" way of doing it; really i should iterate throughg the items and try to find the item which when put through buttonMap[item] returns the current button name. but i cant be arsed.
		theButtons->setCurrentItem(theButtons->findItem(RemoteServer::remoteServer()->getButtonName(remote, button), 0));
		theButtons->ensureItemVisible(theButtons->findItem(RemoteServer::remoteServer()->getButtonName(remote, button), 0));
	}
	else
		KMessageBox::error(this, i18n( "You did not select a mode of that remote control. Please use %1, "
                                       "or revert back to select a different mode." ).arg( theMode.remoteName() ),
                                       i18n( "Incorrect Remote Control Detected" ));

	if(indexOf(currentPage()) == 1)
		requestNextPress();
}

void AddAction::updateButtons()
{
	theButtons->clear();
	buttonMap.clear();
	IRKick_stub IRKick("irkick", "IRKick");
	TQStringList buttons = IRKick.buttons(theMode.remote());
	for(TQStringList::iterator j = buttons.begin(); j != buttons.end(); ++j)
		buttonMap[new TQListViewItem(theButtons, RemoteServer::remoteServer()->getButtonName(theMode.remote(), *j))] = *j;
}

void AddAction::updateForPageChange()
{
	if(indexOf(currentPage()) == 1) requestNextPress(); else cancelRequest();
	switch(indexOf(currentPage()))
	{	case 0: break;
		case 1: break;
		case 2: break;
		case 3: break;
		case 4: break;
	}
	updateButtonStates();
}

void AddAction::updateButtonStates()
{
	switch(indexOf(currentPage()))
	{	case 0: setNextEnabled(currentPage(), theProfiles->currentItem() != 0 || !theUseProfile->isChecked()); break;
		case 1: setNextEnabled(currentPage(), theButtons->currentItem() != 0); break;
		case 2: setNextEnabled(currentPage(), theFunctions->currentItem() != 0); break;
		case 3: setNextEnabled(currentPage(), theProfileFunctions->currentItem() != 0 || theJustStart->isChecked()); break;
		case 4: setNextEnabled(currentPage(), true); break;
		case 5: setNextEnabled(currentPage(), false); setFinishEnabled(currentPage(), true); break;
		case 6: setNextEnabled(currentPage(), false); setFinishEnabled(currentPage(), theModes->currentItem() || !theSwitchMode->isChecked()); break;
	}
}

const TQStringList AddAction::getFunctions(const TQString app, const TQString obj)
{
	TQStringList ret;
	DCOPClient *theClient = TDEApplication::kApplication()->dcopClient();
	QCStringList theApps = theClient->remoteFunctions(app.utf8(), obj.utf8());
	for(QCStringList::iterator i = theApps.begin(); i != theApps.end(); ++i)
		if(	*i != "QCStringList interfaces()" &&
			*i != "QCStringList functions()" &&
			*i != "QCStringList objects()" &&
			*i != "QCStringList find(TQCString)" )
			ret += TQString::fromUtf8(*i);
	return ret;
}

void AddAction::updateProfiles()
{
	ProfileServer *theServer = ProfileServer::profileServer();
	theProfiles->clear();
	profileMap.clear();

	TQDict<Profile> dict = theServer->profiles();
	TQDictIterator<Profile> i(dict);
	for(; i.current(); ++i)
		profileMap[new TQListViewItem(theProfiles, i.current()->name())] = i.currentKey();
}

void AddAction::updateOptions()
{
	IfMulti im;
	if(theUseProfile->isChecked())
	{
		ProfileServer *theServer = ProfileServer::profileServer();
		if(!theProfiles->currentItem()) return;
		const Profile *p = theServer->profiles()[profileMap[theProfiles->currentItem()]];
		im = p->ifMulti();
		isUnique = p->unique();
	}
	else if(theUseDCOP->isChecked())
	{
		if(!theObjects->selectedItem()) return;
		TQListViewItem* i = theObjects->selectedItem()->parent();
		if(!i) return;
		isUnique = uniqueProgramMap[i];
		TQRegExp r("(.*)-[0-9]+");
		program = r.exactMatch(nameProgramMap[i]) ? r.cap(1) : nameProgramMap[i];
		im = IM_DONTSEND;
	}
	else return;

	theIMLabel->setEnabled(!isUnique);
	theIMGroup->setEnabled(!isUnique);
	theIMLine->setEnabled(!isUnique);
	theIMTop->setEnabled(!isUnique);
	theDontSend->setEnabled(!isUnique);
	theSendToBottom->setEnabled(!isUnique);
	theSendToTop->setEnabled(!isUnique);
	theSendToAll->setEnabled(!isUnique);
	switch(im)
	{	case IM_DONTSEND: theDontSend->setChecked(true); break;
		case IM_SENDTOTOP: theSendToTop->setChecked(true); break;
		case IM_SENDTOBOTTOM: theSendToBottom->setChecked(true); break;
		case IM_SENDTOALL: theSendToAll->setChecked(true); break;
	}
}

void AddAction::updateProfileFunctions()
{
	ProfileServer *theServer = ProfileServer::profileServer();
	theProfileFunctions->clear();
	profileFunctionMap.clear();
	if(!theProfiles->currentItem()) return;

	const Profile *p = theServer->profiles()[profileMap[theProfiles->currentItem()]];
	TQDict<ProfileAction> dict = p->actions();
	for(TQDictIterator<ProfileAction> i(dict); i.current(); ++i)
		profileFunctionMap[new TQListViewItem(theProfileFunctions, i.current()->name(), TQString().setNum(i.current()->arguments().count()), i.current()->comment())] = i.currentKey();
	updateParameters();
	updateOptions();
}

void AddAction::updateParameters()
{
	theParameters->clear();
	theArguments.clear();
	if(theUseDCOP->isChecked() && theFunctions->currentItem())
	{
		Prototype p(theFunctions->currentItem()->text(2));
		for(unsigned k = 0; k < p.count(); k++)
		{	new TDEListViewItem(theParameters, p.name(k).isEmpty() ? i18n( "<anonymous>" ) : p.name(k), "", p.type(k), TQString().setNum(k + 1));
			theArguments.append(TQVariant(""));
			theArguments.back().cast(TQVariant::nameToType(p.type(k).utf8()));
		}
	}
	else if(theUseProfile->isChecked() && theProfiles->currentItem())
	{
		ProfileServer *theServer = ProfileServer::profileServer();

		if(!theProfiles->currentItem()) return;
		if(!theProfileFunctions->currentItem()) return;
		const Profile *p = theServer->profiles()[profileMap[theProfiles->currentItem()]];
		const ProfileAction *pa = p->actions()[profileFunctionMap[theProfileFunctions->currentItem()]];

		int index = 1;
		for(TQValueList<ProfileActionArgument>::const_iterator i = pa->arguments().begin(); i != pa->arguments().end(); ++i, index++)
		{	theArguments.append(TQVariant((*i).getDefault()));
			theArguments.back().cast(TQVariant::nameToType((*i).type().utf8()));
			new TQListViewItem(theParameters, (*i).comment(), theArguments.back().toString(), (*i).type(), TQString().setNum(index));
		}

		// quicky update options too...
		theRepeat->setChecked(pa->repeat());
		theAutoStart->setChecked(pa->autoStart());
	}

	updateParameter();
}

void AddAction::updateParameter()
{
	if(theParameters->currentItem())
	{	TQString type = theParameters->currentItem()->text(2);
		int index = theParameters->currentItem()->text(3).toInt() - 1;
		if(type.find("int") != -1 || type.find("short") != -1 || type.find("long") != -1)
		{	theValue->raiseWidget(2);
			theValueIntNumInput->setValue(theArguments[index].toInt());
		}
		else if(type.find("double") != -1 || type.find("float") != -1)
		{	theValue->raiseWidget(3);
			theValueDoubleNumInput->setValue(theArguments[index].toDouble());
		}
		else if(type.find("bool") != -1)
		{	theValue->raiseWidget(1);
			theValueCheckBox->setChecked(theArguments[index].toBool());
		}
		else if(type.find(TQSTRINGLIST_OBJECT_NAME_STRING) != -1)
		{	theValue->raiseWidget(4);
			TQStringList backup = theArguments[index].toStringList();
			// backup needed because calling clear will kill what ever has been saved.
			theValueEditListBox->clear();
			theValueEditListBox->insertStringList(backup);
			theArguments[index].asStringList() = backup;
		}
		else
		{	theValue->raiseWidget(0);
			theValueLineEdit->setText(theArguments[index].toString());
		}
		theCurParameter->setText(theParameters->currentItem()->text(0));
		theCurParameter->setEnabled(true);
		theValue->setEnabled(true);
	}
	else
	{	theCurParameter->setText("");
		theValueLineEdit->setText("");
		theValueCheckBox->setChecked(false);
		theValueIntNumInput->setValue(0);
		theValueDoubleNumInput->setValue(0.0);
		theCurParameter->setEnabled(false);
		theValue->setEnabled(false);
	}
}

// called when the textbox/checkbox/whatever changes value
void AddAction::slotParameterChanged()
{
	if(!theParameters->currentItem()) return;
	int index = theParameters->currentItem()->text(3).toInt() - 1;
	TQString type = theParameters->currentItem()->text(2);
	if(type.find("int") != -1 || type.find("short") != -1 || type.find("long") != -1)
		theArguments[index].asInt() = theValueIntNumInput->value();
	else if(type.find("double") != -1 || type.find("float") != -1)
		theArguments[index].asDouble() = theValueDoubleNumInput->value();
	else if(type.find("bool") != -1)
		theArguments[index].asBool() = theValueCheckBox->isChecked();
	else if(type.find(TQSTRINGLIST_OBJECT_NAME_STRING) != -1)
		theArguments[index].asStringList() = theValueEditListBox->items();
	else
		theArguments[index].asString() = theValueLineEdit->text();

	theArguments[theParameters->currentItem()->text(3).toInt() - 1].cast(TQVariant::nameToType(theParameters->currentItem()->text(2).utf8()));
	updateArgument(theParameters->currentItem());
}

// takes theArguments[theIndex] and puts it into theItem
void AddAction::updateArgument(TQListViewItem *theItem)
{
	theItem->setText(1, theArguments[theItem->text(3).toInt() - 1].toString());
}

void AddAction::updateObjects()
{
	TQStringList names;
	theObjects->clear();
	uniqueProgramMap.clear();
	nameProgramMap.clear();

	DCOPClient *theClient = TDEApplication::kApplication()->dcopClient();
	QCStringList theApps = theClient->registeredApplications();
	for(QCStringList::iterator i = theApps.begin(); i != theApps.end(); ++i)
	{
		if(!TQString(*i).find("anonymous")) continue;
		if(!TQString(*i).find(i18n( "anonymous" ))) continue;
		TQRegExp r("(.*)-[0-9]+");
		TQString name = r.exactMatch(TQString(*i)) ? r.cap(1) : *i;
		if(names.contains(name)) continue;
		names += name;

		TDEListViewItem *a = new TDEListViewItem(theObjects, name);
		uniqueProgramMap[a] = name == TQString(*i);
		nameProgramMap[a] = *i;

		QCStringList theObjects = theClient->remoteObjects(*i);
		for(QCStringList::iterator j = theObjects.begin(); j != theObjects.end(); ++j)
			if(*j != "tdesycoca" && *j != "qt")// && getFunctions(*i, *j).count())
				new TDEListViewItem(a, *j);
	}
	updateFunctions();
}

void AddAction::updateFunctions()
{
	theFunctions->clear();
	if(theObjects->currentItem() && theObjects->currentItem()->parent())
	{	TQStringList functions = getFunctions(nameProgramMap[theObjects->currentItem()->parent()], theObjects->currentItem()->text(0));
		for(TQStringList::iterator i = functions.begin(); i != functions.end(); ++i)
		{	Prototype p((TQString)(*i));
			new TDEListViewItem(theFunctions, p.name(), p.argumentList(), *i);
		}
	}
	updateOptions();
}


#include "addaction.moc"
