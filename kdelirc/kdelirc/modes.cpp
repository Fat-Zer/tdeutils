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
#include <kconfig.h>
#include <kdebug.h>

#include "modes.h"
#include "mode.h"

Modes::Modes()
{
}


Modes::~Modes()
{
}

void Modes::loadFromConfig(KConfig &theConfig)
{
	clear();
	int numModes = theConfig.readNumEntry("Modes");
	for(int i = 0; i < numModes; i++)
	{
		add(Mode().loadFromConfig(theConfig, i));
	}

	for(iterator i = begin(); i != end(); ++i)
		theDefaults[i.key()] = theConfig.readEntry("Default" + i.key());
}

void Modes::generateNulls(const TQStringList &theRemotes)
{
	for(TQStringList::const_iterator i = theRemotes.begin(); i != theRemotes.end(); ++i)
	{	if(!tqcontains(*i) || !operator[](*i).tqcontains("")) operator[](*i)[""] = Mode(*i, "");
		if(!theDefaults.tqcontains(*i)) theDefaults[*i].isEmpty();
	}
}

bool Modes::isDefault(const Mode &mode) const
{
	if(theDefaults[mode.remote()] == mode.name())
		return true;
//	if(theDefaults[mode.remote()].isEmpty() || theDefaults[mode.remote()].isNull())
//		return mode.name().isEmpty();
	return false;
}

const Mode Modes::getDefault(const TQString &remote) const
{
//	if(theDefaults[remote] == TQString())
//		return Mode(remote, "");
	if(tqcontains(remote))
		if(operator[](remote).tqcontains(theDefaults[remote]))
			return operator[](remote)[theDefaults[remote]];
		else return Mode(remote, "");
	else return Mode(remote, "");

}

void Modes::purgeAllModes(KConfig &theConfig)
{
	int numModes = theConfig.readNumEntry("Modes");
	for(int i = 0; i < numModes; i++)
	{	TQString Prefix = "Mode" + TQString().setNum(i);
		theConfig.deleteEntry(Prefix + "Name");
		theConfig.deleteEntry(Prefix + "Remote");
	}
}

void Modes::saveToConfig(KConfig &theConfig)
{
	int index = 0;
	purgeAllModes(theConfig);
	for(iterator i = begin(); i != end(); ++i)
		for(TQMap<TQString, Mode>::iterator j = (*i).begin(); j != (*i).end(); ++j,index++)
			(*j).saveToConfig(theConfig, index);
	theConfig.writeEntry("Modes", index);

	for(iterator i = begin(); i != end(); ++i)
		if(theDefaults[i.key()] == TQString())
			theConfig.writeEntry("Default" + i.key(), "");
		else
			theConfig.writeEntry("Default" + i.key(), theDefaults[i.key()]);
}

const Mode &Modes::getMode(const TQString &remote, const TQString &mode) const
{
	return operator[](remote)[mode];
}

ModeList Modes::getModes(const TQString &remote) const
{
	ModeList ret;
	for(TQMap<TQString, Mode>::const_iterator i = operator[](remote).begin(); i != operator[](remote).end(); ++i)
		ret += *i;
	return ret;
}

void Modes::erase(const Mode &mode)
{
	operator[](mode.remote()).erase(mode.name());
}

void Modes::add(const Mode &mode)
{
	kdDebug() << "adding a mode " << mode.name() << " to remote " << mode.remote() << endl;
	operator[](mode.remote())[mode.name()] = mode;
}

void Modes::rename(Mode &mode, const TQString name)
{
	bool was = isDefault(mode);
	erase(mode);
	mode.setName(name);
	if(was) setDefault(mode);
	add(mode);
}

