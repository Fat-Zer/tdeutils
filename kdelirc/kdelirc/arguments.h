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
#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <tqvaluelist.h>
#include <tqvariant.h>

/**
@author Gav Wood
*/

class Arguments : public TQValueList<TQVariant>
{
public:
	const TQString toString() const;

	Arguments();
	~Arguments();
};

#endif
