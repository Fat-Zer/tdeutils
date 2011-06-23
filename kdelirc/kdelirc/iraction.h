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
#ifndef IRACTION_H
#define IRACTION_H

#include <tqstring.h>

#include "prototype.h"
#include "arguments.h"
#include "profileserver.h"

/**
@author Gav Wood
*/

class KConfig;

class IRAction
{
	TQString theProgram, theObject, theRemote, theButton, theMode;
	Prototype theMethod;
	Arguments theArguments;
	bool theRepeat, theAutoStart, theDoBefore, theDoAfter;
	IfMulti theIfMulti;
	bool theUnique;

public:
	// load/save convenience functions
	const IRAction &loadFromConfig(KConfig &theConfig, int index);
	void saveToConfig(KConfig &theConfig, int index) const;

	// may be changed to a profile-based representation in the future.
	const TQString function() const;
	const TQString application() const;
	const TQString buttonName() const;
	const TQString remoteName() const;
	const TQString notes() const;

	// bog standard raw DCOP stuff
	const TQString &program() const { return theProgram; }
	const TQString &object() const { return theObject; }
	const Prototype &method() const { return theMethod; }
	const TQString &remote() const { return theRemote; }
	const TQString &mode() const { return theMode; }
	const TQString &button() const { return theButton; }
	const Arguments arguments() const { if(theProgram != "" && theObject != "") return theArguments; return Arguments(); }
	const bool repeat() const { return theRepeat; }
	const bool autoStart() const { return theAutoStart; }
	const IfMulti ifMulti() const { return theIfMulti; }
	const bool unique() const { return theUnique; }

	const TQString &modeChange() const { return theObject; }
	const bool doBefore() const { return theDoBefore; }
	const bool doAfter() const { return theDoAfter; }

	bool isModeChange() const { return theProgram == ""; }
	bool isJustStart() const { return theProgram != "" && theObject == ""; }

	void setProgram(const TQString &newProgram) { theProgram = newProgram; }
	void setObject(const TQString &newObject) { theObject = newObject; }
	void setMethod(const Prototype &newMethod) { theMethod = newMethod; }
	void setRemote(const TQString &newRemote) { theRemote = newRemote; }
	void setMode(const TQString &newMode) { theMode = newMode; }
	void setButton(const TQString &newButton) { theButton = newButton; }
	void setArguments(const Arguments &newArguments) { theArguments = newArguments; }
	void setRepeat(bool newRepeat) { theRepeat = newRepeat; }
	void setDoBefore(bool a) { theDoBefore = a; }
	void setDoAfter(bool a) { theDoAfter = a; }
	void setAutoStart(bool newAutoStart) { theAutoStart = newAutoStart; }
	void setModeChange(const TQString &a) { theObject = a; }
	void setIfMulti(const IfMulti a) { theIfMulti = a; }
	void setUnique(const bool a) { theUnique = a; }

	IRAction(const TQString &newProgram, const TQString &newObject, const TQString &newMethod, const Arguments &newArguments, const TQString &newRemote, const TQString &newMode, const TQString &newButton, const bool newRepeat, const bool newAutoStart, const bool newDoBefore, const bool newDoAfter, const bool newUnique, const IfMulti newIfMulti);
	IRAction() { theProgram = TQString(); }
};

#endif
