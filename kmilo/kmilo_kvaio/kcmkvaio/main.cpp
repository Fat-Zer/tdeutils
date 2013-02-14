/*
 *  main.cpp
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

#include <tqlabel.h>
#include <tqlayout.h>
#include <tqcheckbox.h>
#include <tqtimer.h>
#include <tqslider.h>

#include <dcopclient.h>
#include <kgenericfactory.h>
#include <tdeaboutdata.h>
#include <kdebug.h>
#include <tdeconfig.h>
#include <kled.h>
#include <kprogress.h>

#include "kcmkvaio_general.h"

#include <dcopclient.h>

#include "main.h"
#include "main.moc"

typedef KGenericFactory<KVaioModule, TQWidget> KVaioModuleFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kvaio, KVaioModuleFactory("kcmkvaio"))

#define CONFIG_FILE "kmilodrc"


KVaioModule::KVaioModule(TQWidget *parent, const char *name, const TQStringList &)
    : TDECModule(KVaioModuleFactory::instance(), parent, name)
{
    TDEAboutData *about =
        new TDEAboutData(I18N_NOOP("kcmkvaio"),
                       I18N_NOOP("TDE Control Module for Sony "
                                 "Vaio Laptop Hardware"),
                       0, 0, TDEAboutData::License_GPL,
                       "(c) 2003 Mirko Boehm");

    about->addAuthor("Mirko Boehm",
                     I18N_NOOP("Original author"),
                     "mirko@kde.org");
    setAboutData( about );

    TQVBoxLayout *layout = new TQVBoxLayout(this);
    mKVaioGeneral = new KCMKVaioGeneral(this);
    layout->addWidget( mKVaioGeneral );
    layout->addStretch();

    mDriver = new KVaioDriverInterface(TQT_TQOBJECT(this));
    mDriverAvailable = mDriver->connectToDriver(false);
    mTimer = new TQTimer(this);
    mTimer->start(231);

    load();
    if (! mDriverAvailable)
    {
        mKVaioGeneral->tlOff->show();
        mKVaioGeneral->frameMain->setEnabled(false);
        setButtons(buttons() & ~Default);
    } else {
        mKVaioGeneral->tlOff->hide();
        // mKVaioGeneral->frameMain->setEnabled (true);
    }

    connect(mKVaioGeneral, TQT_SIGNAL(changed()),
            TQT_SLOT(changed()));
    connect(mTimer, TQT_SIGNAL(timeout()), TQT_SLOT(timeout()));
    connect(mDriver, TQT_SIGNAL (vaioEvent(int)), TQT_SLOT (vaioEvent(int) ) );
}

void KVaioModule::save()
{
    if (! mDriverAvailable) return;
    DCOPClient mClient;

    kdDebug() << "KVaioModule: saving." << endl;
    TDEConfig config(CONFIG_FILE);

    config.setGroup("KVaio");

    config.writeEntry("Report_Unknown_Events",
                      mKVaioGeneral->cbReportUnknownEvents->isChecked());
    config.writeEntry("PeriodicallyReportPowerStatus",
		      mKVaioGeneral->mCbPowerMsgs->isChecked() );
    config.writeEntry("PowerStatusOnBackButton",
		      mKVaioGeneral->mCbBackButtonMsg->isChecked() );
    config.sync();

    if(mClient.attach())
    {
        TQByteArray data, replyData;
        TQCString replyType;

        if (!mClient.call("kded", "kmilod", "reconfigure()",
                          data, replyType, replyData))
        {
            kdDebug() << "KVaio::showTextMsg: "
                      << "there was some error using DCOP." << endl;
        }
    } else {
        kdDebug() << "KVaioModule: cannot attach to DCOP server, "
                  << "no automatic config update." << endl;
    }
}

void KVaioModule::load()
{
	load( false );
}

void KVaioModule::load(bool useDefaults)
{
    kdDebug() << "KVaioModule: loading." << endl;
    TDEConfig config(CONFIG_FILE);

	 config.setReadDefaults( useDefaults );

    config.setGroup("KVaio");
    mKVaioGeneral->cbReportUnknownEvents->setChecked
        (config.readBoolEntry("Report_Unknown_Events", false));
    mKVaioGeneral->mCbPowerMsgs->setChecked
	(config.readBoolEntry("PeriodicallyReportPowerStatus", false) );
    mKVaioGeneral->mCbBackButtonMsg->setChecked
	(config.readBoolEntry("PowerStatusOnBackButton", true) );

	 emit changed( useDefaults );
}

void KVaioModule::defaults()
{
	load( true );
}

void KVaioModule::timeout()
{
    bool bat1Avail = false, bat2Avail = false, acConnected = false;
    int bat1Remaining = 0, bat1Max = 0, bat2Remaining = 0, bat2Max = 0;

    if(mDriver->getBatteryStatus(bat1Avail, bat1Remaining, bat1Max,
                                 bat2Avail, bat2Remaining, bat2Max,
                                 acConnected) )
    {
    }
    int remaining = (int)(100.0*(bat1Remaining+bat2Remaining)
                          / (bat1Max+bat2Max));
    mKVaioGeneral->mKPBattery->setValue(remaining);
    mKVaioGeneral->kledBat1->setState(bat1Avail ? KLed::On : KLed::Off);
    mKVaioGeneral->kledBat2->setState(bat2Avail ? KLed::On : KLed::Off);
    mKVaioGeneral->kledAC->setState(acConnected ? KLed::On : KLed::Off);
}

void KVaioModule::vaioEvent (int e)
{
    kdDebug () << "KVaioModule::vaioEvent: event: " << e << endl;
}

