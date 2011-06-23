// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
/*
   This file is part of the KDE project

   Copyright (c) 2003 George Staikos <staikos@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

*/

#include "defaultwidget.h"

#include <tqprogressbar.h>
#include <tqwidget.h>
#include <tqwidgetstack.h>
#include <tqlabel.h>
#include <tqapplication.h>

#include <kwin.h>
#include <netwm.h>
#include <kglobalsettings.h>
#include <kdeversion.h>
#include <kconfig.h>

#include "defaultskin.h"


DefaultSkin::DefaultSkin() {
	connect(&_timer, TQT_SIGNAL(timeout()), this, TQT_SLOT(timeout()));

	_widget = new DefaultWidget(0, "Screen Indicator", TQt::WX11BypassWM);
	_widget->setFocusPolicy(TQ_NoFocus);

	KWin::setOnAllDesktops(_widget->winId(), true);
	KWin::setState( _widget->winId(), NET::StaysOnTop | NET::Sticky
				| NET::SkipTaskbar | NET::SkipPager );
	KWin::setType(_widget->winId(), NET::Override);

	_widget->hide();

	KConfig config("kmilodrc");
	reconfigure( &config );
}


DefaultSkin::~DefaultSkin() {
	delete _widget;
	_widget = 0;
}

void DefaultSkin::reconfigure( KConfig *config ) {
	
	config->setGroup("DefaultSkin");
	
	TQFont *defaultFont = new TQFont("Sans", 10, TQFont::Bold );
	TQSize *defaultSize = new TQSize( 80, 30 );
	TQColor *defaultpaletteForegroundColor = new TQColor( 200, 200, 200 );
	TQColor *defaultpaletteBackgroundColor = new TQColor( 100, 100, 100 );
	TQFont *defaultProgressFont = new TQFont("Sans", 8, TQFont::Bold );
	
	_widget->resize( config->readSizeEntry("Size", defaultSize ) );
	_widget->setFont( config->readFontEntry("Font", defaultFont ) );
	_widget->setPaletteForegroundColor( config->readColorEntry("paletteForegroundColor", defaultpaletteForegroundColor ) );
	_widget->setPaletteBackgroundColor( config->readColorEntry("paletteBackgroundColor", defaultpaletteBackgroundColor ) );
	_widget->_progress->setFont( config->readFontEntry("ProgressFont", defaultProgressFont ) );

	if ( ! config->hasGroup("DefaultSkin") )
	{
		
		config->writeEntry("Size", *defaultSize );
		config->writeEntry("Font", *defaultFont );
		config->writeEntry("paletteForegroundColor", *defaultpaletteForegroundColor );
		config->writeEntry("paletteBackgroundColor", *defaultpaletteBackgroundColor );
		config->writeEntry("ProgressFont", *defaultProgressFont );
		
	}
	
}

void DefaultSkin::clear() {
	_timer.stop();
	_widget->hide();
}


void DefaultSkin::show() {
#if KDE_IS_VERSION(3,1,90)
	TQRect r =  KGlobalSettings::splashScreenDesktopGeometry();
#else
	TQRect r = TQApplication::desktop()->tqgeometry();
#endif
	//	_label->resize(_label->tqminimumSizeHint());
	//	_widget->resize(_label->tqminimumSizeHint());
	_widget->move(r.center() -
			TQPoint(_widget->width()/2, _widget->height()/2));
	_widget->show();
	_timer.start(750, true);
}


void DefaultSkin::displayText(const TQString& text, const TQPixmap& customPixmap) {
	Q_UNUSED(customPixmap)
	_timer.stop();
	_widget->_widgetStack->raiseWidget(0);
	_widget->_textOnly->setText(text);
	show();
}


void DefaultSkin::displayProgress(const TQString& text, int percent, const TQPixmap& customPixmap) {
	Q_UNUSED(customPixmap)
	_timer.stop();
	_widget->_progressText->setText(text);
	_widget->_progress->setProgress(percent);
	_widget->_widgetStack->raiseWidget(1);
	show();
}


void DefaultSkin::timeout() {
	clear();
}


#include "defaultskin.moc"

