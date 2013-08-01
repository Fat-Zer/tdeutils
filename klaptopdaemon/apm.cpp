/*
 * battery.cpp
 *
 * Copyright (c) 1999 Paul Campbell <paul@taniwha.com>
 * Copyright (c) 2002 Marc Mutz <mutz@kde.org>
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
#include "apm.h"
#include "version.h"
#include "portable.h"
#include <stdlib.h>
#include <unistd.h>

// other KDE headers:
#include <tdelocale.h>
#include <tdeconfig.h>
#include <knuminput.h>
#include <kiconloader.h>
#include <kicondialog.h>
#include <tdeapplication.h>
#include <tdemessagebox.h>
#include <kstandarddirs.h>
#include <kprocess.h>

// other TQt headers:
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqcheckbox.h>
#include <tqhbox.h>
#include <tqvgroupbox.h>
#include <tqgrid.h>
#include <tqpushbutton.h>
#include <tqtooltip.h>
extern void wake_laptop_daemon();

ApmConfig::ApmConfig (TQWidget * parent, const char *name)
  : TDECModule(parent, name)
{
    TDEGlobal::locale()->insertCatalogue("klaptopdaemon"); // For translation of klaptopdaemon messages

    config =  new TDEConfig("kcmlaptoprc");

    TQVBoxLayout *top_layout = new TQVBoxLayout( this, KDialog::marginHint(),
					       KDialog::spacingHint() );

    TQLabel *tmp_label = new TQLabel( i18n("This panel lets you configure your APM system and lets "
					"you have access to some of the extra features provided by it"), this );
    tmp_label->setAlignment( TQt::WordBreak );
    top_layout->addWidget( tmp_label );
    
    tmp_label = new TQLabel( i18n("NOTE: some APM implementations have buggy suspend/standby "
				"implementations. You should test these features very gingerly - save "
				"all your work, check them on and try a suspend/standby from "
				"the popup menu on the battery icon in the panel if it fails to come "
				"back successfully uncheck the box again."), this );
    tmp_label->setAlignment( TQt::WordBreak );
    top_layout->addWidget( tmp_label );

    tmp_label = new TQLabel( i18n("Some changes made on this page may require you to quit the laptop panel "
				"and start it again to take effect"), this );
    tmp_label->setAlignment( TQt::WordBreak );
    top_layout->addWidget( tmp_label );

    bool can_enable = laptop_portable::has_apm(1);	// is helper ready
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
    apm_name = "/usr/bin/apm";
    if (::access(apm_name, F_OK) != 0 && ::access("/usr/sbin/apm", F_OK) == 0) 
	    apm_name = "/usr/sbin/apm";

    tmp_label = new TQLabel(i18n("If the above boxes are disabled then there is no 'helper' "
				"application set up to help change APM states, there are two "
				"ways you can enable this application, either make the file "
				"/proc/apm writeable by anyone every time your system boots "
				"or use the button below to make the %1 application "
				"set-uid root").arg(apm_name), this );
    tmp_label->setAlignment( TQt::WordBreak );
    top_layout->addWidget( tmp_label );
    TQHBoxLayout *ll = new TQHBoxLayout(top_layout);
    TQPushButton *setupButton = new TQPushButton(i18n("Setup Helper Application"), this);
    connect( setupButton, TQT_SIGNAL(clicked()), this, TQT_SLOT(setupHelper()) );
    TQToolTip::add( setupButton, i18n( "This button can be used to enable the APM helper application" ) );
    ll->addStretch(2);
    ll->addWidget(setupButton);
    ll->addStretch(8);
    if (laptop_portable::has_software_suspend()) {
	tmp_label = new TQLabel( i18n("Your system seems to have 'Software Suspend' installed, this can "
				"be used to hibernate or 'suspend to disk' your system if you want "
				"to use this for hibernation check the box below"), this );
	tmp_label->setAlignment( TQt::WordBreak );
	top_layout->addWidget( tmp_label );
     	enableSoftwareSuspendHibernate = new TQCheckBox( i18n("Enable software suspend for hibernate"), this );
    	top_layout->addWidget( enableSoftwareSuspendHibernate );
    	TQToolTip::add( enableSoftwareSuspendHibernate, i18n( "If checked this box enables transitions to the 'hibernate' state using the 'Software Suspend' mechanism" ) );
    	enableSoftwareSuspendHibernate->setEnabled(laptop_portable::has_software_suspend(2));
    	connect( enableSoftwareSuspendHibernate, TQT_SIGNAL(clicked()), this, TQT_SLOT(configChanged()) );
	tmp_label = new TQLabel( i18n("If the above box is disabled then you need to be logged in "
				"as root or need a helper application to invoke the Software "
				"Suspend utility - TDE provides a utility to do this, if you "
				"wish to use it you must make it set-uid root, the button "
				"below will do this for you"), this );
	tmp_label->setAlignment( TQt::WordBreak );
	top_layout->addWidget( tmp_label );
        ll = new TQHBoxLayout(this);
        TQPushButton *setupSSButton = new TQPushButton(i18n("Setup SS Helper Application"), this);
        connect( setupSSButton, TQT_SIGNAL(clicked()), this, TQT_SLOT(setupHelper2()) );
        TQToolTip::add( setupSSButton, i18n( "This button can be used to enable the Software Suspend helper application" ) );
        ll->addStretch(2);
        ll->addWidget(setupSSButton);
        ll->addStretch(8);
    } else {
	enableSoftwareSuspendHibernate = 0;
    }


    top_layout->addStretch(1);
    top_layout->addWidget( new TQLabel( i18n("Version: %1").arg(LAPTOP_VERSION), this), 0, TQt::AlignRight );


    load();      
}

ApmConfig::~ApmConfig()
{
  delete config;
}

void ApmConfig::setupHelper()
{
	TQString tdesu = TDEStandardDirs::findExe("tdesu");
	if (!tdesu.isEmpty()) {
		int rc = KMessageBox::warningContinueCancel(0,
				i18n("You will need to supply a root password "
					"to allow the privileges of the %1 application to change.").arg(apm_name),
				"KLaptopDaemon", KStdGuiItem::cont(),
				"");
		if (rc == KMessageBox::Continue) {
			TDEProcess proc;
			proc << tdesu;
			proc << "-u";
			proc << "root";
			proc <<  TQString("chown root ")+apm_name+TQString("; chmod +s ")+apm_name;
			proc.start(TDEProcess::Block);	// run it sync so has_apm below sees the results
		}
	} else {
		KMessageBox::sorry(0, i18n("%1 cannot be enabled because tdesu cannot be found.  Please make sure that it is installed correctly.").arg(TQString(apm_name)),
				i18n("KLaptopDaemon"));
	}
	laptop_portable::apm_set_mask(enablestandby, enablesuspend);
    	bool can_enable = laptop_portable::has_apm(1);	// is helper ready
    	enableStandby->setEnabled(can_enable);
    	enableSuspend->setEnabled(can_enable);
	wake_laptop_daemon();
}

#include "checkcrc.h"
#include "crcresult.h"
void ApmConfig::setupHelper2()	// we use the acpi helper to do software suspend
{
	unsigned long len, crc;
	TQString helper = TDEStandardDirs::findExe("klaptop_acpi_helper");
	checkcrc(helper.latin1(), len, crc);

	TQString tdesu = TDEStandardDirs::findExe("tdesu");
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
		KMessageBox::sorry(0, i18n("The Software Suspend helper cannot be enabled because tdesu cannot be found.  Please make sure that it is installed correctly."),
				i18n("KLaptopDaemon"));
	}
	laptop_portable::software_suspend_set_mask(enablesoftwaresuspend);
    	bool can_enable = laptop_portable::has_software_suspend(2);	// is helper ready
    	enableSoftwareSuspendHibernate->setEnabled(can_enable);
	wake_laptop_daemon();
}


void ApmConfig::save()
{
        enablestandby = enableStandby->isChecked();
        enablesuspend = enableSuspend->isChecked();
	laptop_portable::apm_set_mask(enablestandby, enablesuspend);
	enablesoftwaresuspend = (enableSoftwareSuspendHibernate ? enableSoftwareSuspendHibernate->isChecked():0);
	laptop_portable::software_suspend_set_mask(enablesoftwaresuspend);

        config->setGroup("ApmDefault");

        config->writeEntry("EnableStandby", enablestandby);
        config->writeEntry("EnableSuspend", enablesuspend);

       	config->setGroup("SoftwareSuspendDefault");
        config->writeEntry("EnableHibernate", enablesoftwaresuspend);
	config->sync();
        changed(false);
	wake_laptop_daemon();
}

void ApmConfig::load()
{
	load( false );
}

void ApmConfig::load(bool useDefaults)
{
	     config->setReadDefaults( useDefaults );
        config->setGroup("ApmDefault");
        enablestandby = config->readBoolEntry("EnableStandby", true);
        enableStandby->setChecked(enablestandby);
        enablesuspend = config->readBoolEntry("EnableSuspend", true);
        enableSuspend->setChecked(enablesuspend);
       	config->setGroup("SoftwareSuspendDefault");
        enablesoftwaresuspend = config->readBoolEntry("EnableHibernate", false);
	if (enableSoftwareSuspendHibernate)
        	enableSoftwareSuspendHibernate->setChecked(enablesoftwaresuspend);
        emit changed(useDefaults);
}

void ApmConfig::defaults()
{
	     load( true );
}


void ApmConfig::configChanged()
{
  emit changed(true);
}


TQString ApmConfig::quickHelp() const
{
  return i18n("<h1>APM Setup</h1>This module allows you to configure APM for your system");
}


#include "apm.moc"


