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
#ifndef EDITMODE_H
#define EDITMODE_H

#include "editmodebase.h"

/**
@author Gav Wood
*/
class EditMode : public EditModeBase
{
	Q_OBJECT
  TQ_OBJECT

public:
	void slotCheckText(const TQString &newText);
	void slotClearIcon();

	EditMode(TQWidget *parent = 0, const char *name = 0, bool modal = false, WFlags fl = 0);
	~EditMode();
};

#endif
