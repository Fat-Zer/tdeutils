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

#include <qprogressbar.h>
#include <qwidget.h>
#include <qwidgetstack.h>
#include <qlabel.h>
#include <qapplication.h>

#include <kwin.h>
#include <netwm.h>
#include <kglobalsettings.h>
#include <kdeversion.h>

#include "defaultskin.h"


DefaultSkin::DefaultSkin() {
	connect(&_timer, SIGNAL(timeout()), this, SLOT(timeout()));

	_widget = new DefaultWidget(0, "Screen Indicator", Qt::WX11BypassWM);
	_widget->setFocusPolicy(QWidget::NoFocus);

	KWin::setOnAllDesktops(_widget->winId(), true);
	KWin::setState( _widget->winId(), NET::StaysOnTop | NET::Sticky
				| NET::SkipTaskbar | NET::SkipPager );
	KWin::setType(_widget->winId(), NET::Override);

	_widget->hide();
}


DefaultSkin::~DefaultSkin() {
	delete _widget;
	_widget = 0;
}


void DefaultSkin::clear() {
	_timer.stop();
	_widget->hide();
}


void DefaultSkin::show() {
#if KDE_IS_VERSION(3,1,90)
	QRect r =  KGlobalSettings::splashScreenDesktopGeometry();
#else
	QRect r = QApplication::desktop()->geometry();
#endif
	//	_label->resize(_label->minimumSizeHint());
	//	_widget->resize(_label->minimumSizeHint());
	_widget->move(r.center() -
			QPoint(_widget->width()/2, _widget->height()/2));
	_widget->show();
	_timer.start(750, true);
}


void DefaultSkin::displayText(const QString& text, const QPixmap& customPixmap) {
	Q_UNUSED(customPixmap)
	_timer.stop();
	_widget->_widgetStack->raiseWidget(0);
	_widget->_textOnly->setText(text);
	show();
}


void DefaultSkin::displayProgress(const QString& text, int percent, const QPixmap& customPixmap) {
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

