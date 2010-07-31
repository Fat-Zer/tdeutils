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
#include <tqstring.h>

#include "arguments.h"

Arguments::Arguments()
{
}

Arguments::~Arguments()
{
}

const TQString Arguments::toString() const
{
	TQString ret = "";
	for(Arguments::const_iterator i = begin(); i != end(); ++i)
	{	TQString s = (*i).toString();
		if(s.isNull()) s = "...";
		if(i != begin()) ret += ", ";
		ret += s;
	}
	return ret;
}

