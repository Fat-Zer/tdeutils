/*
 * (c) 2001 Stefan Schimanski <1Stein@gmx.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KTIMER_H_INCLUDED
#define KTIMER_H_INCLUDED

#include <tqdialog.h>
#include <tqwidget.h>
#include <kprocess.h>
#include <tdeconfig.h>

#include "prefwidget.h"


class KTimerJob : public TQObject {
 Q_OBJECT
  

 public:
    KTimerJob( TQObject *parent=0, const char *name=0 );
    virtual ~KTimerJob();

    enum States { Stopped, Paused, Started };

    unsigned delay();
    TQString command();
    bool loop();
    bool oneInstance();
    unsigned value();
    States state();
    void *user();
    void setUser( void *user );

    void load( TDEConfig *cfg, const TQString& grp );
    void save( TDEConfig *cfg, const TQString& grp );

 public slots:
    void setDelay( unsigned sec );
    void setDelay( int sec ) { setDelay( (unsigned)sec ); };
    void setCommand( const TQString &cmd );
    void setLoop( bool loop );
    void setOneInstance( bool one );
    void setValue( unsigned value );
    void setValue( int value ) { setValue( (unsigned)value ); };
    void setState( States state );

    void pause() { setState( Paused ); }
    void stop() { setState( Stopped ); }
    void start() { setState( Started ); }

 signals:
    void stateChanged( KTimerJob *job, States state );
    void delayChanged( KTimerJob *job, unsigned sec );
    void commandChanged( KTimerJob *job, const TQString &cmd );
    void loopChanged( KTimerJob *job, bool loop );
    void oneInstanceChanged( KTimerJob *job, bool one );
    void valueChanged( KTimerJob *job, unsigned value );

    void changed( KTimerJob *job );
    void fired( KTimerJob *job );
    void finished( KTimerJob *job, bool error );
    void error( KTimerJob *job );

 protected slots:
    virtual void fire();

 private slots:
    void timeout();
    void processExited(TDEProcess *proc);

 private:
    struct KTimerJobPrivate *d;
};


class KTimerPref : public PrefWidget
{
    Q_OBJECT
  
 public:
    KTimerPref( TQWidget *parent=0, const char *name = 0 );
    virtual ~KTimerPref();

 protected slots:
    void add();
    void remove();
    void currentChanged( TQListViewItem * );

    void saveJobs( TDEConfig *cfg );
    void loadJobs( TDEConfig *cfg );

 private slots:
    void jobChanged( KTimerJob *job );
    void jobFinished( KTimerJob *job, bool error );

 private:
    struct KTimerPrefPrivate *d;
};

#endif
