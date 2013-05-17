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
#include <tdeconfig.h>
#include <kdebug.h>

#include "iractions.h"
#include "iraction.h"

void IRActions::loadFromConfig(TDEConfig &theConfig)
{
	clear();
	int numBindings = theConfig.readNumEntry("Bindings");
	for(int i = 0; i < numBindings; i++)
		addAction(IRAction().loadFromConfig(theConfig, i));
}

void IRActions::purgeAllBindings(TDEConfig &theConfig)
{
	int numBindings = theConfig.readNumEntry("Bindings");
	for(int i = 0; i < numBindings; i++)
	{	TQString Binding = "Binding" + TQString().setNum(i);
		int numArguments = theConfig.readNumEntry(Binding + "Arguments");
		for(int j = 0; j < numArguments; j++)
		{	theConfig.deleteEntry(Binding + "Argument" + TQString().setNum(j));
			theConfig.deleteEntry(Binding + "ArgumentType" + TQString().setNum(j));
		}
		theConfig.deleteEntry(Binding + "Arguments"); theConfig.deleteEntry(Binding + "Program");
		theConfig.deleteEntry(Binding + "Object"); theConfig.deleteEntry(Binding + "Method");
		theConfig.deleteEntry(Binding + "Remote"); theConfig.deleteEntry(Binding + "Button");
		theConfig.deleteEntry(Binding + "Repeat"); theConfig.deleteEntry(Binding + "Mode");
	}
}

void IRActions::saveToConfig(TDEConfig &theConfig)
{
	int index = 0;
	purgeAllBindings(theConfig);
	for(iterator i = begin(); i != end(); ++i,index++)
		(*i).saveToConfig(theConfig, index);
	theConfig.writeEntry("Bindings", index);
}

IRAIt IRActions::addAction(const IRAction &theAction)
{
	return append(theAction);
}

IRAItList IRActions::findByButton(const TQString &remote, const TQString &button)
{
	IRAItList ret;
	for(iterator i = begin(); i != end(); ++i)
		if((*i).remote() == remote && (*i).button() == button)
			ret += i;
	return ret;
}

void IRActions::renameMode(const Mode &mode, const TQString &to)
{
	for(iterator i = begin(); i != end(); ++i)
	{	if((*i).remote() == mode.remote() && (*i).mode() == mode.name()) (*i).setMode(to);
		if((*i).isModeChange() && (*i).modeChange() == mode.name()) (*i).setModeChange(to);
	}
}

IRAItList IRActions::findByMode(const Mode &mode)
{
	IRAItList ret;
	for(iterator i = begin(); i != end(); ++i)
		if((*i).remote() == mode.remote() && (*i).mode() == mode.name()) ret += i;
	return ret;
}

IRAItList IRActions::findByModeButton(const Mode &mode, const TQString &button)
{
	IRAItList ret;
	for(iterator i = begin(); i != end(); ++i)
		if((*i).remote() == mode.remote() && (*i).mode() == mode.name() && (*i).button() == button)
			ret += i;
	return ret;
}
