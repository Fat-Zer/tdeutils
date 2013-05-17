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
#ifndef PROTOTYPE_H
#define PROTOTYPE_H

#include <tqstringlist.h>
#include <tqpair.h>
#include <tqstring.h>

/**
@author Gav Wood
*/

class Prototype
{
	TQString original, theName, theReturn;
	TQStringList theNames, theTypes;

	void parse();

public:
	unsigned count() const{ return theTypes.count(); }
	const TQPair<TQString, TQString> operator[](int i) const { return tqMakePair(theTypes[i], theNames[i]); }
	const TQString &name(int i) const { return theNames[i]; }
	const TQString &type(int i) const { return theTypes[i]; }
	const TQString &returnType() const { return theReturn; }
	const TQString &name() const { return theName; }
	const TQString &prototype() const { return original; }
	const TQString argumentList() const;
	const TQString argumentListNN() const;
	const int argumentCount() { return theTypes.count(); }
	const TQString prototypeNR() const { return theName + "(" + argumentListNN() + ")"; }

	void setPrototype(const TQString &source) { original = source; parse(); }

	Prototype &operator=(const TQString &source) { setPrototype(source); return *this; }

	Prototype(const TQString &source);
	Prototype();
	~Prototype();

};

#endif
