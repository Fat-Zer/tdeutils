/*
 *  main.h
 *
 *  Copyright (C) 2003 Mirko Boehm (mirko@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef KCMKVAIO_MAIN_H
#define KCMKVAIO_MAIN_H

#include <kcmodule.h>
#include "../kvaiodriverinterface.h"

class QTimer;
class KCMKVaioGeneral;

class KVaioModule : public KCModule
{
    Q_OBJECT

public:
    KVaioModule(QWidget *parent, const char *name, const QStringList &);

    void  save();
    void  load();
    void  load(bool useDefaults);
    void defaults();

protected slots:
    void timeout();
    void vaioEvent (int);
private:
    KCMKVaioGeneral *mKVaioGeneral;
    KVaioDriverInterface *mDriver;
    QTimer *mTimer;
    bool mDriverAvailable;
};

#endif // KCMKVAIO_MAIN_H
