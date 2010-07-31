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
#ifndef MODE_H
#define MODE_H

#include <tqstring.h>

#include "remoteserver.h"

/**
@author Gav Wood
*/

class KConfig;

class Mode
{
	TQString theName, theRemote, theIconFile;

public:
	void setName(const TQString &a) { theName = a; }
	void setRemote(const TQString &a) { theRemote = a; }
	void setIconFile(const TQString &a) { theIconFile = a; }

	const TQString &name() const { return theName; }
	const TQString &remote() const { return theRemote; }
	const TQString &iconFile() const { return theIconFile; }
	const TQString &remoteName() const { return RemoteServer::remoteServer()->getRemoteName(theRemote); }

	const Mode &loadFromConfig(KConfig &theConfig, int index);
	void saveToConfig(KConfig &theConfig, int index);

	bool operator==(const Mode &mode) const { return mode.theName == theName && mode.theRemote == theRemote; }

	Mode();
	Mode(const TQString &remote, const TQString &name, const TQString &iconFile = TQString::null);
	~Mode();
};

#endif
