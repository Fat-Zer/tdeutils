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

#include <tqtimer.h>
#include <tqtoolbutton.h>
#include <tqgroupbox.h>
#include <tqlistview.h>
#include <tqspinbox.h>
#include <tqlineedit.h>
#include <tqcheckbox.h>
#include <tqslider.h>
#include <tqlcdnumber.h>
#include <kurlrequester.h>
#include <klineedit.h>

#include <kiconloader.h>
#include <tdeapplication.h>
#include <ksystemtray.h>
#include <tdefiledialog.h>

#include "ktimer.h"
#include <tqpushbutton.h>


class KTimerJobItem : public TQListViewItem {
public:
    KTimerJobItem( KTimerJob *job, TQListView *parent )
        : TQListViewItem( parent ) {
        m_job = job;
        m_error = false;
        update();
    };

    KTimerJobItem( KTimerJob *job, TQListView *parent, TQListViewItem *after )
        : TQListViewItem( parent, after ) {
        m_job = job;
        m_error = false;
        update();
    };

    virtual ~KTimerJobItem() {
        delete m_job;
    };

    KTimerJob *job() { return m_job; };

    void setStatus( bool error ) {
        m_error = error;
        update();
    }

    void update() {
        setText( 0, TQString::number(m_job->value()) );

        if( m_error )
            setPixmap( 0, SmallIcon("stop") );
        else
            setPixmap( 0, TQPixmap() );

        setText( 1, TQString::number(m_job->delay()) );

        switch( m_job->state() ) {
            case KTimerJob::Stopped: setPixmap( 2, SmallIcon("player_stop") ); break;
            case KTimerJob::Paused: setPixmap( 2, SmallIcon("player_pause") ); break;
            case KTimerJob::Started: setPixmap( 2, SmallIcon("1rightarrow") ); break;
        }

        setText( 3, m_job->command() );
    };

private:
    bool m_error;
    KTimerJob *m_job;
};


/***************************************************************/


struct KTimerPrefPrivate
{
    TQPtrList<KTimerJob> jobs;
};

KTimerPref::KTimerPref( TQWidget *parent, const char *name )
    : PrefWidget( parent, name )
{
    d = new KTimerPrefPrivate;

    d->jobs.setAutoDelete( true );

    // set icons
    m_stop->setIconSet( SmallIconSet("player_stop") );
    m_pause->setIconSet( SmallIconSet("player_pause") );
    m_start->setIconSet( SmallIconSet("1rightarrow") );

    // create tray icon
    KSystemTray *tray = new KSystemTray( this, "TimerTray" );
    tray->show();
    tray->setPixmap( SmallIcon( "ktimer" ) );

    // connect
    connect( m_add, TQT_SIGNAL(clicked()), TQT_SLOT(add()) );
    connect( m_remove, TQT_SIGNAL(clicked()), TQT_SLOT(remove()) );
    connect( m_list, TQT_SIGNAL(currentChanged(TQListViewItem*)),
             TQT_SLOT(currentChanged(TQListViewItem*)) );
    loadJobs( kapp->config() );

    show();
}


KTimerPref::~KTimerPref()
{
    saveJobs( kapp->config() );
    delete d;
}


void KTimerPref::add()
{
    KTimerJob *job = new KTimerJob;
    KTimerJobItem *item = new KTimerJobItem( job, m_list );

    connect( job, TQT_SIGNAL(delayChanged(KTimerJob*,unsigned)),
             TQT_SLOT(jobChanged(KTimerJob*)) );
    connect( job, TQT_SIGNAL(valueChanged(KTimerJob*,unsigned)),
             TQT_SLOT(jobChanged(KTimerJob*)) );
    connect( job, TQT_SIGNAL(stateChanged(KTimerJob*,States)),
             TQT_SLOT(jobChanged(KTimerJob*)) );
    connect( job, TQT_SIGNAL(commandChanged(KTimerJob*,const TQString&)),
             TQT_SLOT(jobChanged(KTimerJob*)) );
    connect( job, TQT_SIGNAL(finished(KTimerJob*,bool)),
             TQT_SLOT(jobFinished(KTimerJob*,bool)) );

    job->setUser( item );

    // TQt drops currentChanged signals on first item (bug?)
    if( m_list->childCount()==1 )
      currentChanged( item );

    m_list->setCurrentItem( item );
    m_list->triggerUpdate();
}


void KTimerPref::remove()
{
    delete m_list->currentItem();
    m_list->triggerUpdate();
}


void KTimerPref::currentChanged( TQListViewItem *i )
{
    KTimerJobItem *item = static_cast<KTimerJobItem*>(i);
    if( item ) {
        KTimerJob *job = item->job();

        m_state->setEnabled( true );
        m_settings->setEnabled( true );
        m_remove->setEnabled( true );

        m_delay->disconnect();
        m_loop->disconnect();
        m_one->disconnect();
        m_start->disconnect();
        m_pause->disconnect();
        m_stop->disconnect();
        m_counter->disconnect();
        m_slider->disconnect();

        connect( m_commandLine->lineEdit(), TQT_SIGNAL(textChanged(const TQString &)),
                 job, TQT_SLOT(setCommand(const TQString &)) );
        connect( m_delay, TQT_SIGNAL(valueChanged(int)),
                 job, TQT_SLOT(setDelay(int)) );
        connect( m_loop, TQT_SIGNAL(toggled(bool)),
                 job, TQT_SLOT(setLoop(bool)) );
        connect( m_one, TQT_SIGNAL(toggled(bool)),
                 job, TQT_SLOT(setOneInstance(bool)) );
        connect( m_stop, TQT_SIGNAL(clicked()),
                 job, TQT_SLOT(stop()) );
        connect( m_pause, TQT_SIGNAL(clicked()),
                 job, TQT_SLOT(pause()) );
        connect( m_start, TQT_SIGNAL(clicked()),
                 job, TQT_SLOT(start()) );
        connect( m_slider, TQT_SIGNAL(valueChanged(int)),
                 job, TQT_SLOT(setValue(int)) );

        m_commandLine->lineEdit()->setText( job->command() );
        m_delay->setValue( job->delay() );
        m_loop->setChecked( job->loop() );
        m_one->setChecked( job->oneInstance() );
        m_counter->display( (int)job->value() );
        m_slider->setMaxValue( job->delay() );
        m_slider->setValue( job->value() );

    } else {
        m_state->setEnabled( false );
        m_settings->setEnabled( false );
        m_remove->setEnabled( false );
    }
}


void KTimerPref::jobChanged( KTimerJob *job )
{
    KTimerJobItem *item = static_cast<KTimerJobItem*>(job->user());
    if( item ) {
        item->update();
        m_list->triggerUpdate();

        if( item==m_list->currentItem() ) {

            // XXX optimize
            m_slider->setMaxValue( job->delay() );
            m_slider->setValue( job->value() );
            m_counter->display( (int)job->value() );
        }
    }
}


void KTimerPref::jobFinished( KTimerJob *job, bool error )
{
    KTimerJobItem *item = static_cast<KTimerJobItem*>(job->user());
    item->setStatus( error );
    m_list->triggerUpdate();
}


void KTimerPref::saveJobs( TDEConfig *cfg )
{
    int num = 0;
    KTimerJobItem *item = static_cast<KTimerJobItem*>(m_list->firstChild());
    while( item ) {
        item->job()->save( cfg, TQString("Job%1").arg( num ) );
        item = static_cast<KTimerJobItem*>(item->nextSibling());
        num++;
    }

    cfg->setGroup( "Jobs" );
    cfg->writeEntry( "Number", num );

    cfg->sync();
}


void KTimerPref::loadJobs( TDEConfig *cfg )
{
    cfg->setGroup( "Jobs" );
    int num = cfg->readNumEntry( "Number", 0 );
    for( int n=0; n<num; n++ ) {
            KTimerJob *job = new KTimerJob;
            KTimerJobItem *item = new KTimerJobItem( job, m_list );

            connect( job, TQT_SIGNAL(delayChanged(KTimerJob*,unsigned)),
                     TQT_SLOT(jobChanged(KTimerJob*)) );
            connect( job, TQT_SIGNAL(valueChanged(KTimerJob*,unsigned)),
                     TQT_SLOT(jobChanged(KTimerJob*)) );
            connect( job, TQT_SIGNAL(stateChanged(KTimerJob*,States)),
                     TQT_SLOT(jobChanged(KTimerJob*)) );
            connect( job, TQT_SIGNAL(commandChanged(KTimerJob*,const TQString&)),
                     TQT_SLOT(jobChanged(KTimerJob*)) );
            connect( job, TQT_SIGNAL(finished(KTimerJob*,bool)),
                     TQT_SLOT(jobFinished(KTimerJob*,bool)) );

            job->load( cfg, TQString( "Job%1" ).arg(n) );

            job->setUser( item );
    }

    m_list->triggerUpdate();
}


/*********************************************************************/


struct KTimerJobPrivate {
    unsigned delay;
    TQString command;
    bool loop;
    bool oneInstance;
    unsigned value;
    KTimerJob::States state;
    TQPtrList<TDEProcess> processes;
    void *user;

    TQTimer *timer;
};


KTimerJob::KTimerJob( TQObject *parent, const char *name )
    : TQObject( parent, name )
{
    d = new KTimerJobPrivate;

    d->delay = 100;
    d->loop = false;
    d->oneInstance = true;
    d->value = 100;
    d->state = Stopped;
    d->processes.setAutoDelete( true );
    d->user = 0;

    d->timer = new TQTimer( this );
    connect( d->timer, TQT_SIGNAL(timeout()), TQT_SLOT(timeout()) );
}


KTimerJob::~KTimerJob()
{
    delete d;
}


void KTimerJob::load( TDEConfig *cfg, const TQString& grp )
{
    cfg->setGroup( grp );
    cfg->writeEntry( "Delay", d->delay );
    cfg->writePathEntry( "Command", d->command );
    cfg->writeEntry( "Loop", d->loop );
    cfg->writeEntry( "OneInstance", d->oneInstance );
    cfg->writeEntry( "State", (int)d->state );
}


void KTimerJob::save( TDEConfig *cfg, const TQString& grp )
{
    cfg->setGroup( grp );
    setDelay( cfg->readNumEntry( "Delay", 100 ) );
    setCommand( cfg->readPathEntry( "Command" ) );
    setLoop( cfg->readBoolEntry( "Loop", false ) );
    setOneInstance( cfg->readBoolEntry( "OneInstance", d->oneInstance ) );
    setState( (States)cfg->readNumEntry( "State", (int)Stopped ) );
}


void *KTimerJob::user()
{
    return d->user;
}


void KTimerJob::setUser( void *user )
{
    d->user = user;
}


unsigned KTimerJob::delay()
{
    return d->delay;
}


void KTimerJob::setDelay( unsigned sec )
{
    if( d->delay!=sec ) {
        d->delay = sec;

        if( d->state==Stopped )
            setValue( sec );

        emit delayChanged( this, sec );
        emit changed( this );
    }
}


TQString KTimerJob::command()
{
    return d->command;
}


void KTimerJob::setCommand( const TQString &cmd )
{
    if( d->command!=cmd ) {
        d->command = cmd;
        emit commandChanged( this, cmd );
        emit changed( this );
    }
}


bool KTimerJob::loop()
{
    return d->loop;
}


void KTimerJob::setLoop( bool loop )
{
    if( d->loop!=loop ) {
        d->loop = loop;
        emit loopChanged( this, loop );
        emit changed( this );
    }
}


bool KTimerJob::oneInstance()
{
    return d->oneInstance;
}


void KTimerJob::setOneInstance( bool one )
{
    if( d->oneInstance!=one ) {
        d->oneInstance = one;
        emit oneInstanceChanged( this, one );
        emit changed( this );
    }
}


unsigned KTimerJob::value()
{
    return d->value;
}


void KTimerJob::setValue( unsigned value )
{
    if( d->value!=value ) {
        d->value = value;
        emit valueChanged( this, value );
        emit changed( this );
    }
}


KTimerJob::States KTimerJob::state()
{
    return d->state;
}


void KTimerJob::setState( KTimerJob::States state )
{
    if( d->state!=state ) {
        if( state==Started )
            d->timer->start( 1000 );
        else
            d->timer->stop();

        if( state==Stopped )
            setValue( d->delay );

        d->state = state;
        emit stateChanged( this, state );
        emit changed( this );
    }
}


void KTimerJob::timeout()
{
    if( d->state==Started && d->value!=0 ) {
        setValue( d->value-1 );
        if( d->value==0 ) {
            fire();
            if( d->loop )
                setValue( d->delay );
            else
                stop();
        }
    }
}


void KTimerJob::processExited(TDEProcess *proc)
{
    bool ok = proc->exitStatus()==0;
    d->processes.remove( proc );
    if( !ok ) emit error( this );
    emit finished( this, !ok );
}


void KTimerJob::fire()
{
    if( !d->oneInstance || d->processes.isEmpty() ) {
        KShellProcess *proc = new KShellProcess;
        (*proc) << d->command;
        d->processes.append( proc );
        connect( proc, TQT_SIGNAL(processExited(TDEProcess*)),
                 TQT_SLOT(processExited(TDEProcess*)) );
        bool ok = proc->start( TDEProcess::NotifyOnExit );
        emit fired( this );
        if( !ok ) {
            d->processes.remove( proc );
            emit error( this );
            emit finished( this, true );
        }
    }
}
#include "ktimer.moc"
