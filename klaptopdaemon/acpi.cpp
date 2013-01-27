/*
 * acpi.cpp
 *
 * Copyright (c) 1999, 2003 Paul Campbell <paul@taniwha.com>
 *
 * Requires the TQt widget libraries, available at no cost at
 * http://www.troll.no/
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
 */

// my headers:
#include "acpi.h"
#include "version.h"
#include "portable.h"
#include <stdlib.h>
#include <unistd.h>

// other KDE headers:
#include <klocale.h>
#include <tdeconfig.h>
#include <knuminput.h>
#include <kiconloader.h>
#include <kicondialog.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kprocess.h>

// other TQt headers:
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqcheckbox.h>
#include <tqhbox.h>
#include <tqvgroupbox.h>
#include <tqhgroupbox.h>
#include <tqgrid.h>
#include <tqpushbutton.h>
#include <tqtooltip.h>
extern void wake_laptop_daemon();

AcpiConfig::AcpiConfig (TQWidget * parent, const char *name)
  : TDECModule(parent, name)
{
    TDEGlobal::locale()->insertCatalogue("klaptopdaemon"); // For translation of klaptopdaemon messages

    config =  new TDEConfig("kcmlaptoprc");

    TQVBoxLayout *top_layout = new TQVBoxLayout( this, KDialog::marginHint(),
					       KDialog::spacingHint() );

    TQLabel *tmp_label = new TQLabel( i18n("This panel provides information about your system's ACPI implementation "
					 "and lets you have access to some of the extra features provided by ACPI"), this );
    tmp_label->setAlignment( TQt::WordBreak );
    top_layout->addWidget( tmp_label );

    tmp_label = new TQLabel( i18n("NOTE: the Linux ACPI implementation is still a 'work in progress'. "
				"Some features, in particular suspend and hibernate are not yet available "
				"under 2.4 - and under 2.5 some particular ACPI implementations are still "
				"unstable, these check boxes let you only enable the things that work reliably. "
				"You should test these features very gingerly - save all your work, check them "
				"on and try a suspend/standby/hibernate from the popup menu on the battery icon "
				"in the panel if it fails to come back successfully uncheck the box again."), this );
    tmp_label->setAlignment( TQt::WordBreak );
    top_layout->addWidget( tmp_label );

    tmp_label = new TQLabel( i18n("Some changes made on this page may require you to quit the laptop panel "
				"and start it again to take effect"), this );
    tmp_label->setAlignment( TQt::WordBreak );
    top_layout->addWidget( tmp_label );

    bool can_enable = laptop_portable::has_acpi(1);	// is helper ready
    enableStandby = new TQCheckBox( i18n("Enable standby"), this );
    top_layout->addWidget( enableStandby );
    TQToolTip::add( enableStandby, i18n( "If checked this box enables transitions to the 'standby' state - a temporary powered down state" ) );
    enableStandby->setEnabled(can_enable);
    connect( enableStandby, TQT_SIGNAL(clicked()), this, TQT_SLOT(configChanged()) );

    enableSuspend = new TQCheckBox( i18n("Enable &suspend"), this );
    top_layout->addWidget( enableSuspend );
    TQToolTip::add( enableSuspend, i18n( "If checked this box enables transitions to the 'suspend' state - a semi-powered down state, sometimes called 'suspend-to-ram'" ) );
    enableSuspend->setEnabled(can_enable);
    connect( enableSuspend, TQT_SIGNAL(clicked()), this, TQT_SLOT(configChanged()) );

    TQHBoxLayout *ll = new TQHBoxLayout();
    enableHibernate = new TQCheckBox( i18n("Enable &hibernate"), this );
    ll->addWidget( enableHibernate );
    TQToolTip::add( enableHibernate, i18n( "If checked this box enables transitions to the 'hibernate' state - a powered down state, sometimes called 'suspend-to-disk'" ) );
    enableHibernate->setEnabled(can_enable);
    connect( enableHibernate, TQT_SIGNAL(clicked()), this, TQT_SLOT(configChanged()) );
    if (laptop_portable::has_software_suspend()) {
	ll->addStretch(1);
    	enableSoftwareSuspendHibernate = new TQCheckBox( i18n("Use software suspend for hibernate"), this );
    	ll->addWidget( enableSoftwareSuspendHibernate );
    	TQToolTip::add( enableSoftwareSuspendHibernate, i18n( "If checked this box enables transitions to the 'hibernate' state - a powered down state, sometimes called 'suspend-to-disk' - the kernel 'Software Suspend' mechanism will be used instead of using ACPI directly" ) );
    	enableSoftwareSuspendHibernate->setEnabled(laptop_portable::has_software_suspend(2));
    	connect( enableSoftwareSuspendHibernate, TQT_SIGNAL(clicked()), this, TQT_SLOT(configChanged()) );
    } else {
        enableSoftwareSuspendHibernate = 0;
    }
    ll->addStretch(10);

    top_layout->addLayout(ll);

    enablePerformance = new TQCheckBox( i18n("Enable &performance profiles"), this );
    top_layout->addWidget( enablePerformance );
    TQToolTip::add( enablePerformance, i18n( "If checked this box enables access to ACPI performance profiles - usually OK in 2.4 and later" ) );
    enablePerformance->setEnabled(can_enable);
    connect( enablePerformance, TQT_SIGNAL(clicked()), this, TQT_SLOT(configChanged()) );

    enableThrottle = new TQCheckBox( i18n("Enable &CPU throttling"), this );
    top_layout->addWidget( enableThrottle );
    TQToolTip::add( enableThrottle, i18n( "If checked this box enables access to ACPI throttle speed changes - usually OK in 2.4 and later" ) );
    enableThrottle->setEnabled(can_enable);
    connect( enableThrottle, TQT_SIGNAL(clicked()), this, TQT_SLOT(configChanged()) );

    tmp_label = new TQLabel(i18n("If the above boxes are disabled then there is no 'helper' "
				"application set up to help change ACPI states, there are two "
				"ways you can enable this application, either make the file "
				"/proc/acpi/sleep writeable by anyone every time your system boots "
				"or use the button below to make the KDE ACPI helper application "
				"set-uid root"), this );
    tmp_label->setAlignment( TQt::WordBreak );
    top_layout->addWidget( tmp_label );
    ll = new TQHBoxLayout();
    TQPushButton *setupButton = new TQPushButton(i18n("Setup Helper Application"), this);
    connect( setupButton, TQT_SIGNAL(clicked()), this, TQT_SLOT(setupHelper()) );
    TQToolTip::add( setupButton, i18n( "This button can be used to enable the ACPI helper application" ) );
    ll->addStretch(2);
    ll->addWidget(setupButton);
    ll->addStretch(8);
    top_layout->addLayout(ll);


    top_layout->addStretch(1);
    top_layout->addWidget( new TQLabel( i18n("Version: %1").arg(LAPTOP_VERSION), this), 0, TQt::AlignRight );


    load();      
}

AcpiConfig::~AcpiConfig()
{
  delete config;
}

#include "checkcrc.h"
#include "crcresult.h"
#include <tqfile.h>

void AcpiConfig::setupHelper()
{
	unsigned long len, crc;
	TQString helper = KStandardDirs::findExe("klaptop_acpi_helper");
	checkcrc(TQFile::encodeName(helper), len, crc);

	TQString tdesu = KStandardDirs::findExe("tdesu");
	if (!tdesu.isEmpty()) {
		int rc = KMessageBox::warningContinueCancel(0,
				i18n("You will need to supply a root password "
					"to allow the privileges of the klaptop_acpi_helper to change."),
				i18n("KLaptopDaemon"), KStdGuiItem::cont(),
				"");
		if (rc == KMessageBox::Continue) {
			TDEProcess proc;
			proc << tdesu;
			proc << "-u";
			proc << "root";
			proc <<  "chown root "+helper+"; chmod +s "+helper;
			proc.start(TDEProcess::Block);	// run it sync so has_acpi below sees the results
		}
	} else {
		KMessageBox::sorry(0, i18n("The ACPI helper cannot be enabled because tdesu cannot be found.  Please make sure that it is installed correctly."),
				i18n("KLaptopDaemon"));
	}
	laptop_portable::acpi_set_mask(enablestandby, enablesuspend, enablehibernate, enableperformance, enablethrottle);
    	bool can_enable = laptop_portable::has_acpi(1);	// is helper ready
    	enableStandby->setEnabled(can_enable);
    	enableSuspend->setEnabled(can_enable);
    	enableHibernate->setEnabled(can_enable);
    	enablePerformance->setEnabled(can_enable);
    	enableThrottle->setEnabled(can_enable);
	if (enableSoftwareSuspendHibernate)
		enableSoftwareSuspendHibernate->setEnabled(laptop_portable::has_software_suspend(2));
	wake_laptop_daemon();
}


void AcpiConfig::save()
{
        enablestandby = enableStandby->isChecked();
        enablesuspend = enableSuspend->isChecked();
        enablehibernate = enableHibernate->isChecked();
        enablesoftwaresuspend = (enableSoftwareSuspendHibernate?enableSoftwareSuspendHibernate->isChecked():0);
        enableperformance = enablePerformance->isChecked();
        enablethrottle = enableThrottle->isChecked();
	laptop_portable::acpi_set_mask(enablestandby, enablesuspend, enablehibernate, enableperformance, enablethrottle);

        config->setGroup("AcpiDefault");

        config->writeEntry("EnableStandby", enablestandby);
        config->writeEntry("EnableSuspend", enablesuspend);
        config->writeEntry("EnableHibernate", enablehibernate);
        config->writeEntry("EnableThrottle", enablethrottle);
        config->writeEntry("EnablePerformance", enableperformance);
        config->setGroup("SoftwareSuspendDefault");
        config->writeEntry("EnableHibernate", enablesoftwaresuspend);
	config->sync();
        changed(false);
	wake_laptop_daemon();
}

void AcpiConfig::load()
{
	load( false );
}

void AcpiConfig::load(bool useDefaults)
{
	     config->setReadDefaults( useDefaults );

        config->setGroup("AcpiDefault");

        enablestandby = config->readBoolEntry("EnableStandby", false);
        enableStandby->setChecked(enablestandby);
        enablesuspend = config->readBoolEntry("EnableSuspend", false);
        enableSuspend->setChecked(enablesuspend);
        enablehibernate = config->readBoolEntry("EnableHibernate", false);
        enableHibernate->setChecked(enablehibernate);
        enableperformance = config->readBoolEntry("EnablePerformance", false);
        enablePerformance->setChecked(enableperformance);
        enablethrottle = config->readBoolEntry("EnableThrottle", false);
        enableThrottle->setChecked(enablethrottle);
       	config->setGroup("SoftwareSuspendDefault");
        enablesoftwaresuspend = config->readBoolEntry("EnableHibernate", false);
        if (enableSoftwareSuspendHibernate)
			enableSoftwareSuspendHibernate->setChecked(enablesoftwaresuspend);
        
		  emit changed(useDefaults);
}

void AcpiConfig::defaults()
{
	load( true );
}


void AcpiConfig::configChanged()
{
  emit changed(true);
}


TQString AcpiConfig::quickHelp() const
{
  return i18n("<h1>ACPI Setup</h1>This module allows you to configure ACPI for your system");
}

#include "acpi.moc"


