/*
 * acpi.cpp
 *
 * Copyright (c) 1999, 2003 Paul Campbell <paul@taniwha.com>
 *
 * Requires the Qt widget libraries, available at no cost at
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
#include <kconfig.h>
#include <knuminput.h>
#include <kiconloader.h>
#include <kicondialog.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kprocess.h>

// other Qt headers:
#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qhbox.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>
#include <qgrid.h>
#include <qpushbutton.h>
#include <qtooltip.h>
extern void wake_laptop_daemon();

AcpiConfig::AcpiConfig (QWidget * parent, const char *name)
  : KCModule(parent, name)
{
    KGlobal::locale()->insertCatalogue("klaptopdaemon"); // For translation of klaptopdaemon messages

    config =  new KConfig("kcmlaptoprc");

    QVBoxLayout *top_layout = new QVBoxLayout( this, KDialog::marginHint(),
					       KDialog::spacingHint() );

    QLabel *tmp_label = new QLabel( i18n("This panel provides information about your system's ACPI implementation "
					 "and lets you have access to some of the extra features provided by ACPI"), this );
    tmp_label->setAlignment( Qt::WordBreak );
    top_layout->addWidget( tmp_label );

    tmp_label = new QLabel( i18n("NOTE: the Linux ACPI implementation is still a 'work in progress'. "
				"Some features, in particular suspend and hibernate are not yet available "
				"under 2.4 - and under 2.5 some particular ACPI implementations are still "
				"unstable, these check boxes let you only enable the things that work reliably. "
				"You should test these features very gingerly - save all your work, check them "
				"on and try a suspend/standby/hibernate from the popup menu on the battery icon "
				"in the panel if it fails to come back successfully uncheck the box again."), this );
    tmp_label->setAlignment( Qt::WordBreak );
    top_layout->addWidget( tmp_label );

    tmp_label = new QLabel( i18n("Some changes made on this page may require you to quit the laptop panel "
				"and start it again to take effect"), this );
    tmp_label->setAlignment( Qt::WordBreak );
    top_layout->addWidget( tmp_label );

    bool can_enable = laptop_portable::has_acpi(1);	// is helper ready
    enableStandby = new QCheckBox( i18n("Enable standby"), this );
    top_layout->addWidget( enableStandby );
    QToolTip::add( enableStandby, i18n( "If checked this box enables transitions to the 'standby' state - a temporary powered down state" ) );
    enableStandby->setEnabled(can_enable);
    connect( enableStandby, SIGNAL(clicked()), this, SLOT(configChanged()) );

    enableSuspend = new QCheckBox( i18n("Enable &suspend"), this );
    top_layout->addWidget( enableSuspend );
    QToolTip::add( enableSuspend, i18n( "If checked this box enables transitions to the 'suspend' state - a semi-powered down state, sometimes called 'suspend-to-ram'" ) );
    enableSuspend->setEnabled(can_enable);
    connect( enableSuspend, SIGNAL(clicked()), this, SLOT(configChanged()) );

    QHBoxLayout *ll = new QHBoxLayout();
    enableHibernate = new QCheckBox( i18n("Enable &hibernate"), this );
    ll->addWidget( enableHibernate );
    QToolTip::add( enableHibernate, i18n( "If checked this box enables transitions to the 'hibernate' state - a powered down state, sometimes called 'suspend-to-disk'" ) );
    enableHibernate->setEnabled(can_enable);
    connect( enableHibernate, SIGNAL(clicked()), this, SLOT(configChanged()) );
    if (laptop_portable::has_software_suspend()) {
	ll->addStretch(1);
    	enableSoftwareSuspendHibernate = new QCheckBox( i18n("Use software suspend for hibernate"), this );
    	ll->addWidget( enableSoftwareSuspendHibernate );
    	QToolTip::add( enableSoftwareSuspendHibernate, i18n( "If checked this box enables transitions to the 'hibernate' state - a powered down state, sometimes called 'suspend-to-disk' - the kernel 'Software Suspend' mechanism will be used instead of using ACPI directly" ) );
    	enableSoftwareSuspendHibernate->setEnabled(laptop_portable::has_software_suspend(2));
    	connect( enableSoftwareSuspendHibernate, SIGNAL(clicked()), this, SLOT(configChanged()) );
    } else {
        enableSoftwareSuspendHibernate = 0;
    }
    ll->addStretch(10);

    top_layout->addLayout(ll);

    enablePerformance = new QCheckBox( i18n("Enable &performance profiles"), this );
    top_layout->addWidget( enablePerformance );
    QToolTip::add( enablePerformance, i18n( "If checked this box enables access to ACPI performance profiles - usually OK in 2.4 and later" ) );
    enablePerformance->setEnabled(can_enable);
    connect( enablePerformance, SIGNAL(clicked()), this, SLOT(configChanged()) );

    enableThrottle = new QCheckBox( i18n("Enable &CPU throttling"), this );
    top_layout->addWidget( enableThrottle );
    QToolTip::add( enableThrottle, i18n( "If checked this box enables access to ACPI throttle speed changes - usually OK in 2.4 and later" ) );
    enableThrottle->setEnabled(can_enable);
    connect( enableThrottle, SIGNAL(clicked()), this, SLOT(configChanged()) );

    tmp_label = new QLabel(i18n("If the above boxes are disabled then there is no 'helper' "
				"application set up to help change ACPI states, there are two "
				"ways you can enable this application, either make the file "
				"/proc/acpi/sleep writeable by anyone every time your system boots "
				"or use the button below to make the KDE ACPI helper application "
				"set-uid root"), this );
    tmp_label->setAlignment( Qt::WordBreak );
    top_layout->addWidget( tmp_label );
    ll = new QHBoxLayout();
    QPushButton *setupButton = new QPushButton(i18n("Setup Helper Application"), this);
    connect( setupButton, SIGNAL(clicked()), this, SLOT(setupHelper()) );
    QToolTip::add( setupButton, i18n( "This button can be used to enable the ACPI helper application" ) );
    ll->addStretch(2);
    ll->addWidget(setupButton);
    ll->addStretch(8);
    top_layout->addLayout(ll);


    top_layout->addStretch(1);
    top_layout->addWidget( new QLabel( i18n("Version: %1").arg(LAPTOP_VERSION), this), 0, Qt::AlignRight );


    load();      
}

AcpiConfig::~AcpiConfig()
{
  delete config;
}

#include "checkcrc.h"
#include "crcresult.h"
#include <qfile.h>

void AcpiConfig::setupHelper()
{
	unsigned long len, crc;
	QString helper = KStandardDirs::findExe("klaptop_acpi_helper");
	checkcrc(QFile::encodeName(helper), len, crc);
	if (len != file_len || crc != file_crc) {
		QString str(i18n("The %1 application does not seem to have "
					"the same size or checksum as when it was compiled we do NOT recommend "
					"you proceed with making it setuid-root without further investigation").arg(helper));
		int rc = KMessageBox::warningContinueCancel(0, str, i18n("KLaptopDaemon"), i18n("Run Nevertheless"));
		if (rc != KMessageBox::Continue) 
			return;
	}

	QString kdesu = KStandardDirs::findExe("kdesu");
	if (!kdesu.isEmpty()) {
		int rc = KMessageBox::warningContinueCancel(0,
				i18n("You will need to supply a root password "
					"to allow the privileges of the klaptop_acpi_helper to change."),
				i18n("KLaptopDaemon"), KStdGuiItem::cont(),
				"");
		if (rc == KMessageBox::Continue) {
			KProcess proc;
			proc << kdesu;
			proc << "-u";
			proc << "root";
			proc <<  "chown root "+helper+"; chmod +s "+helper;
			proc.start(KProcess::Block);	// run it sync so has_acpi below sees the results
		}
	} else {
		KMessageBox::sorry(0, i18n("The ACPI helper cannot be enabled because kdesu cannot be found.  Please make sure that it is installed correctly."),
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


QString AcpiConfig::quickHelp() const
{
  return i18n("<h1>ACPI Setup</h1>This module allows you to configure ACPI for your system");
}

#include "acpi.moc"


