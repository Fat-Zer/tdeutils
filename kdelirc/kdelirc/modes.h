//
//
// C++ Interface: $MODULE$
//
// Description:
//
//
// Author: Gav Wood <gav@kde.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MODES_H
#define MODES_H

#include <tqstring.h>
#include <tqmap.h>
#include <tqpair.h>
#include <tqvaluelist.h>

#include "mode.h"

/**
@author Gav Wood
*/

class KConfig;

typedef TQValueList<Mode> ModeList;

class Modes : protected TQMap<TQString, TQMap<TQString, Mode> >
{
	void purgeAllModes(KConfig &theConfig);
	TQMap<TQString, TQString> theDefaults;

public:
	void loadFromConfig(KConfig &theConfig);
	void saveToConfig(KConfig &theConfig);
	void generateNulls(const TQStringList &theRemotes);

	const Mode &getMode(const TQString &remote, const TQString &mode) const;
	ModeList getModes(const TQString &remote) const;
	const Mode getDefault(const TQString &remote) const;
	bool isDefault(const Mode &mode) const;

	/**
	 * Call when you've changed a previously getMode'd mode and you want the changes
	 *   to be recorded
	 **/
	void updateMode(const Mode &mode) { operator[](mode.remote())[mode.name()] = mode; }
	void setDefault(const Mode &mode) { theDefaults[mode.remote()] = mode.name(); }
	void erase(const Mode &mode);
	void add(const Mode &mode);

	// dont use this without renaming all the modes in the actions!!!
	void rename(Mode &mode, const TQString name);

	Modes();
	~Modes();
};

#endif
