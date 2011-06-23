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

#include "modes.h"
#include "mode.h"

Mode::Mode() : theName(TQString())
{
}

Mode::Mode(const TQString &remote, const TQString &name, const TQString &iconFile)
{
	theRemote = remote;
	theName = name;
	theIconFile = iconFile;
}

Mode::~Mode()
{
}

const Mode &Mode::loadFromConfig(KConfig &theConfig, int index)
{
	TQString Prefix = "Mode" + TQString().setNum(index);
	theName = theConfig.readEntry(Prefix + "Name");
	theRemote = theConfig.readEntry(Prefix + "Remote");
	theIconFile = theConfig.readEntry(Prefix + "IconFile");
	if(theIconFile.isEmpty()) theIconFile = TQString();
	return *this;
}

void Mode::saveToConfig(KConfig &theConfig, int index)
{
	TQString Prefix = "Mode" + TQString().setNum(index);
	theConfig.writeEntry(Prefix + "Name", theName);
	theConfig.writeEntry(Prefix + "Remote", theRemote);
	theConfig.writeEntry(Prefix + "IconFile", theIconFile);
}

