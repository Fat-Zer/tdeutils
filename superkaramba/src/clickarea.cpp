/***************************************************************************
 *   Copyright (C) 2003 by Hans Karlsson                                   *
 *   karlsson.h@home.se                                                    *
 *                                                                         *
 *   Copyright (C) 2004,2005 Luke Kenneth Casson Leighton <lkcl@lkcl.net>  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "clickarea.h"

#include <kservicegroup.h>


ClickArea::ClickArea(karamba* k, int x, int y, int w, int h )
        : Meter(k, x, y, w, h )
{
    value = "";
    rect = TQRect( x, y, w, h );
}

ClickArea::~ClickArea()
{}


bool ClickArea::click( TQMouseEvent *e )
{
    if( rect.tqcontains( e->x(), e->y() ) )
    {
        //qDebug(TQString::number(e->type()));
	//KShellProcess ksp;
        if( e->button() != Qt::LeftButton )
			return false;
	if (!svc_name.isEmpty())
	{
		KService sv(svc_name, svc_onClick, svc_icon);
		KURL::List l;
		KRun::run(sv, l);
		return false;
	}
	else
	{
		TQString program;
		program = onClick;
		program.tqreplace( TQRegExp("%v", false), value );

		if( !program.isEmpty() )
		{
			//qDebug(program);
			KRun::runCommand(program);
		}
	}
    }
    return false;
}

void ClickArea::setOnClick( TQString oc )
{
    onClick = oc;
}

void ClickArea::setServiceOnClick( TQString name , TQString exec, TQString icon )
{
    svc_name = name;
    svc_onClick = exec;
    svc_icon = icon;
}

void ClickArea::setOnMiddleClick( TQString oc )
{
    onMiddleClick = oc;
}


TQRect ClickArea::getRectangle()
{
    return rect;
}

void ClickArea::mUpdate( TQPainter *p )
{

    p->drawRect(boundingBox);

}


void ClickArea::setValue( long v)
{

    setValue( TQString::number( v ) );

}


void ClickArea::setValue( TQString v )
{
    value = v;
}





#include "clickarea.moc"
