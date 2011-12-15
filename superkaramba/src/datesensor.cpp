/***************************************************************************
 *   Copyright (C) 2003 by Hans Karlsson                                   *
 *   karlsson.h@home.se                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "datesensor.h"

#include <tqapplication.h>
DateSensor::DateSensor( int interval ) : Sensor( interval )
{
	hidden = true;
}
DateSensor::~DateSensor()
{
}

void DateSensor::update()
{
    TQDateTime qdt  =  TQDateTime::currentDateTime();
    TQString format;
    SensorParams *sp;
    Meter *meter;

    TQObjectListIt it( *objList );
    while (it != 0)
    {
        sp = (SensorParams*)(*it);
        meter = sp->getMeter();
        format = sp->getParam("FORMAT");

        if (format.length() == 0 )
        {
	   format = "hh:mm";
        }
        meter->setValue(qdt.toString(format));
        ++it;
    }
}

void DateSensor::slotCalendarDeleted()
{
	hidden = true;
	cal = 0L;
}


DatePicker::DatePicker(TQWidget *parent)
    : TQVBox( parent, 0, WType_TopLevel | WDestructiveClose |
             WStyle_Customize | WStyle_StaysOnTop | WStyle_NoBorder )
{
    setFrameStyle( TQFrame::PopupPanel | TQFrame::Raised );
    //KWin::setOnAllDesktops( handle(), true );
    picker = new KDatePicker(this);
    picker->setCloseButton(true);

    /* name and icon for kicker's taskbar */
    //setCaption(i18n("Calendar"));
    //setIcon(SmallIcon("date"));
}

void DatePicker::keyReleaseEvent(TQKeyEvent *e)
{
        TQVBox::keyReleaseEvent(e);
        if (e->key() == TQt::Key_Escape)
                close();
}

void DateSensor::toggleCalendar(TQMouseEvent *ev)
{
	TQObjectListIt it(*objList);
	while (it != 0)
	{
		SensorParams *sp = (SensorParams*)(*it);
		Meter *meter = sp->getMeter();
		TQString width = sp->getParam("CALWIDTH");
		TQString height = sp->getParam("CALHEIGHT");

		TQRect rect(meter->getX(),meter->getY(),width.toInt(), height.toInt());
		if (rect.contains( ev->x(), ev->y() ))
		{
			if (hidden)
			{
				hidden = false;
				cal = new DatePicker(0);

				connect(cal, TQT_SIGNAL(destroyed()), TQT_SLOT(slotCalendarDeleted()));
				TQPoint c = (TQPoint(ev->x(), ev->y()));

				int w = cal->sizeHint().width();
				int h = cal->sizeHint().height();

				// make calendar fully visible
				 TQRect deskR = TQApplication::desktop()->screenGeometry(TQApplication::desktop()->screenNumber(c));


				if (c.y()+h > deskR.bottom())	c.setY(deskR.bottom()-h-1);
				if (c.x()+w > deskR.right())	c.setX(deskR.right()-w-1);
				cal->move(c);
				cal->show();

			}
			else
			{
				cal->close();
			}
		}

		++it;
	}
}

void DateSensor::mousePressEvent(TQMouseEvent *ev)
{
	switch (ev->button()) 
	{
		case Qt::LeftButton:
			toggleCalendar(ev);
			break;
		default:
			break;
	}
}


#include "datesensor.moc"
