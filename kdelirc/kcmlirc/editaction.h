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
#ifndef EDITACTION_H
#define EDITACTION_H

#include <tqstring.h>

#include "iractions.h"
#include "editactionbase.h"
#include "arguments.h"

/**
@author Gav Wood
*/

class EditAction : public EditActionBase
{
	Q_OBJECT
  TQ_OBJECT
	IRAIt theAction;
	TQMap<TQString, TQString> applicationMap, functionMap;
	TQMap<TQString, TQString> nameProgramMap;
	TQMap<TQString, bool> uniqueProgramMap;
	Arguments arguments;
	TQString program;
	bool isUnique;

public:
	void writeBack();
	void readFrom();

	virtual void slotParameterChanged();
	virtual void updateArgument(int index);
	virtual void updateArguments();
	virtual void updateApplications();
	virtual void updateFunctions();
	virtual void updateOptions();
	virtual void updateDCOPApplications();
	virtual void updateDCOPObjects();
	virtual void updateDCOPFunctions();

	EditAction(IRAIt action, TQWidget *parent = 0, const char *name = 0);
	~EditAction();
};

#endif
