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
#ifndef IRACTIONS_H
#define IRACTIONS_H

#include <tqvaluelist.h>
#include <tqpair.h>
#include <tqstring.h>
#include <tqmap.h>

#include "iraction.h"
#include "mode.h"

/**
@author Gav Wood
*/

class KConfig;

typedef TQValueListIterator<IRAction> IRAIt;
typedef TQValueList<IRAIt> IRAItList;

class IRActions: protected TQValueList<IRAction>
{
private:
	void purgeAllBindings(KConfig &theConfig);

public:
	IRAIt addAction(const IRAction &theAction);
	IRAItList findByButton(const TQString &remote, const TQString &button);
	IRAItList findByMode(const Mode &mode);
	IRAItList findByModeButton(const Mode &mode, const TQString &button);

	void erase(const IRAIt &action) { TQValueList<IRAction>::erase(action); }
	void renameMode(const Mode &mode, const TQString &to);

	void loadFromConfig(KConfig &theConfig);
	void saveToConfig(KConfig &theConfig);
};


#endif
