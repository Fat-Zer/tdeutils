//
//
// C++ Implementation: $MODULE$
//
// Description:
//
//
// Author: Gav Wood <gav@kde.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <tqvariant.h>

#include <tdeconfig.h>
#include <klocale.h>

#include "iraction.h"
#include "profileserver.h"
#include "remoteserver.h"

IRAction::IRAction(const TQString &newProgram, const TQString &newObject, const TQString &newMethod, const Arguments &newArguments, const TQString &newRemote, const TQString &newMode, const TQString &newButton, const bool newRepeat, const bool newAutoStart, const bool newDoBefore, const bool newDoAfter, const bool newUnique, const IfMulti newIfMulti)
{
	theProgram = newProgram;
	theObject = newObject;
	theMethod = newMethod;
	theArguments = newArguments;
	theRemote = newRemote;
	theMode = newMode;
	theButton = newButton;
	theRepeat = newRepeat;
	theDoAfter = newDoAfter;
	theDoBefore = newDoBefore;
	theAutoStart = newAutoStart;
	theUnique = newUnique;
	theIfMulti = newIfMulti;
}

const IRAction &IRAction::loadFromConfig(TDEConfig &theConfig, int index)
{
	TQString Binding = "Binding" + TQString().setNum(index);
	int numArguments = theConfig.readNumEntry(Binding + "Arguments");
	theArguments.clear();
	for(int j = 0; j < numArguments; j++)
	{	TQVariant::Type theType = (TQVariant::Type)theConfig.readNumEntry(Binding + "ArgumentType" + TQString().setNum(j), TQVariant::String);
		theArguments += theConfig.readPropertyEntry(Binding + "Argument" + TQString().setNum(j), theType == TQVariant::CString ? TQVariant::String : theType);
		theArguments.last().cast(theType);
	}

	theProgram = theConfig.readEntry(Binding + "Program");
	theObject = theConfig.readEntry(Binding + "Object");
	theMethod.setPrototype(theConfig.readEntry(Binding + "Method"));
	theRemote = theConfig.readEntry(Binding + "Remote");
	theMode = theConfig.readEntry(Binding + "Mode");
	theButton = theConfig.readEntry(Binding + "Button");
	theRepeat = theConfig.readBoolEntry(Binding + "Repeat");
	theDoBefore = theConfig.readBoolEntry(Binding + "DoBefore");
	theDoAfter = theConfig.readBoolEntry(Binding + "DoAfter");
	theAutoStart = theConfig.readBoolEntry(Binding + "AutoStart");
	theUnique = theConfig.readBoolEntry(Binding + "Unique", true);
	theIfMulti = (IfMulti)theConfig.readNumEntry(Binding + "IfMulti", IM_DONTSEND);

	return *this;
}

void IRAction::saveToConfig(TDEConfig &theConfig, int index) const
{
	TQString Binding = "Binding" + TQString().setNum(index);

	theConfig.writeEntry(Binding + "Arguments", theArguments.count());
	for(unsigned j = 0; j < theArguments.count(); j++)
	{	TQVariant arg = theArguments[j];
		TQVariant::Type preType = arg.type();
		if(preType == TQVariant::CString) arg.cast(TQVariant::String);
		theConfig.writeEntry(Binding + "Argument" + TQString().setNum(j), arg);
		theConfig.writeEntry(Binding + "ArgumentType" + TQString().setNum(j), preType);
	}
	theConfig.writeEntry(Binding + "Program", theProgram);
	theConfig.writeEntry(Binding + "Object", theObject);
	theConfig.writeEntry(Binding + "Method", theMethod.prototype());
	theConfig.writeEntry(Binding + "Remote", theRemote);
	theConfig.writeEntry(Binding + "Mode", theMode);
	theConfig.writeEntry(Binding + "Button", theButton);
	theConfig.writeEntry(Binding + "Repeat", theRepeat);
	theConfig.writeEntry(Binding + "DoBefore", theDoBefore);
	theConfig.writeEntry(Binding + "DoAfter", theDoAfter);
	theConfig.writeEntry(Binding + "AutoStart", theAutoStart);
	theConfig.writeEntry(Binding + "Unique", theUnique);
	theConfig.writeEntry(Binding + "IfMulti", theIfMulti);
}

const TQString IRAction::function() const
{
	ProfileServer *theServer = ProfileServer::profileServer();
	if(theProgram.isEmpty())
		if(theObject.isEmpty())
			return i18n("Exit mode");
		else
			return i18n("Switch to %1").arg(theObject);
	else
		if(theObject.isEmpty())
			return i18n("Just start");
		else
		{
			const ProfileAction *a = theServer->getAction(theProgram, theObject, theMethod.prototype());
			if(a)
				return a->name();
			else
				return theObject + "::" + theMethod.name();
		}
}

const TQString IRAction::notes() const
{

	if(isModeChange())
		return TQString(theDoBefore ? i18n("Do actions before. ") : "") +
			TQString(theDoAfter ? i18n("Do actions after. ") : "");
	else if(isJustStart())
		return "";
	else
		return TQString(theAutoStart ? i18n("Auto-start. ") : "")
			+ TQString(theRepeat ? i18n("Repeatable. ") : "")
			+ TQString(!theUnique ? (theIfMulti == IM_DONTSEND ? i18n("Do nothing if many instances. ")
						: theIfMulti == IM_SENDTOTOP ? i18n("Send to top instance. ")
						: theIfMulti == IM_SENDTOBOTTOM ? i18n("Send to bottom instance. ") : i18n("Send to all instances. "))
						: "");
}

const TQString IRAction::application() const
{
	ProfileServer *theServer = ProfileServer::profileServer();
	if(theProgram.isEmpty())
		return "";
	else
	{
		const Profile *a = theServer->profiles()[theProgram];
		if(a)
			return a->name();
		else
			return theProgram;
	}
}

const TQString IRAction::remoteName() const
{
	return RemoteServer::remoteServer()->getRemoteName(theRemote);
}

const TQString IRAction::buttonName() const
{
	return RemoteServer::remoteServer()->getButtonName(theRemote, theButton);
}

