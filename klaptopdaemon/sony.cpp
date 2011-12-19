/*
 * sony.cpp
 *
 * Copyright (c) 2002 Paul Campbell <paul@taniwha.com>
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
#include "sony.h"
#include "version.h"
#include "portable.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

// other KDE headers:
#include <klocale.h>
#include <kconfig.h>
#include <knuminput.h>
#include <kiconloader.h>
#include <kicondialog.h>
#include <kapplication.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <krichtextlabel.h>

// other TQt headers:
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqcheckbox.h>
#include <tqhbox.h>
#include <tqvgroupbox.h>
#include <tqgrid.h>
#include <tqpushbutton.h>
#include <tqslider.h>
#include <tqtooltip.h>

extern void wake_laptop_daemon();

SonyConfig::SonyConfig(TQWidget * parent, const char *name)
  : KCModule(parent, name)
{
    KGlobal::locale()->insertCatalogue("klaptopdaemon"); // For translation of klaptopdaemon messages

    config =  new KConfig("kcmlaptoprc");

    TQVBoxLayout *top_layout = new TQVBoxLayout( this, KDialog::marginHint(),
					       KDialog::spacingHint() );

    // TODO: remove linefeed from string, can't do it right now coz we have a string freeze
    top_layout->addWidget(new KRichTextLabel(i18n("This panel allows you to control some of the features of the\n"
		    	"'sonypi' device for your laptop - you should not enable the options below if you\nalso "
			"use the 'sonypid' program in your system").replace("\n", " "), this));

    enableScrollBar = new TQCheckBox( i18n("Enable &scroll bar"), this );
    TQToolTip::add( enableScrollBar, i18n( "When checked this box enables the scrollbar so that it works under KDE" ) );
    top_layout->addWidget( enableScrollBar );
    connect( enableScrollBar, TQT_SIGNAL(clicked()), this, TQT_SLOT(configChanged()) );

    enableMiddleEmulation = new TQCheckBox( i18n("&Emulate middle mouse button with scroll bar press"), this );
    TQToolTip::add( enableMiddleEmulation, i18n( "When checked this box enables pressing the scroll bar to act in the same way as pressing the middle button on a 3 button mouse" ) );
    top_layout->addWidget( enableMiddleEmulation );
    connect( enableMiddleEmulation, TQT_SIGNAL(clicked()), this, TQT_SLOT(configChanged()) );

    if (::access("/dev/sonypi", R_OK) != 0) {
	enableMiddleEmulation->setEnabled(0);
	enableScrollBar->setEnabled(0);
	
        // TODO: remove linefeed from string, can't do it right now coz we have a string freeze
    	top_layout->addWidget(new KRichTextLabel(i18n("The /dev/sonypi is not accessable, if you wish to use the above features its\n"
					      "protections need to be changed. Clicking on the button below will change them\n").replace("\n", " "), this));
        TQHBoxLayout *ll = new TQHBoxLayout();
        TQPushButton *setupButton = new TQPushButton(i18n("Setup /dev/sonypi"), this);
        connect( setupButton, TQT_SIGNAL(clicked()), this, TQT_SLOT(setupHelper()) );
        TQToolTip::add( setupButton, i18n( "This button can be used to enable the sony specific features" ) );
        ll->addStretch(2);
        ll->addWidget(setupButton);
        ll->addStretch(8);
        top_layout->addLayout(ll);
    }
    

    top_layout->addStretch(1);
    top_layout->addWidget( new TQLabel( i18n("Version: %1").arg(LAPTOP_VERSION), this), 0, TQt::AlignRight );


    load();      
}

void SonyConfig::setupHelper()
{
	TQString tdesu = KStandardDirs::findExe("tdesu");
	if (!tdesu.isEmpty()) {
		int rc = KMessageBox::warningContinueCancel(0,
				i18n("You will need to supply a root password "
					"to allow the protections of /dev/sonypi to be changed."),
				i18n("KLaptopDaemon"), KStdGuiItem::cont(),
				"");
		if (rc == KMessageBox::Continue) {
			KProcess proc;
			proc << tdesu;
			proc << "-u";
			proc << "root";
			proc <<  "chmod +r /dev/sonypi";
			proc.start(KProcess::Block);	// run it sync so has_acpi below sees the results
		}
	} else {
		KMessageBox::sorry(0, i18n("The /dev/sonypi protections cannot be changed because tdesu cannot be found.  Please make sure that it is installed correctly."),
				i18n("KLaptopDaemon"));
	}
    	bool enable = ::access("/dev/sonypi", R_OK) == 0;
	enableMiddleEmulation->setEnabled(enable);
	enableScrollBar->setEnabled(enable);
	wake_laptop_daemon();
}

SonyConfig::~SonyConfig()
{
  delete config;
}

void SonyConfig::save()
{
        enablescrollbar = enableScrollBar->isChecked();
        middleemulation = enableMiddleEmulation->isChecked();

        config->setGroup("SonyDefault");

        config->writeEntry("EnableScrollBar", enablescrollbar);
        config->writeEntry("EnableMiddleEmulation", middleemulation);
	config->sync();
        changed(false);
  	wake_laptop_daemon();
}

void SonyConfig::load()
{
	load( false );
}

void SonyConfig::load(bool useDefaults)
{
	     config->setReadDefaults( useDefaults );

        config->setGroup("SonyDefault");

        enablescrollbar = config->readBoolEntry("EnableScrollBar", false);
        enableScrollBar->setChecked(enablescrollbar);
        middleemulation = config->readBoolEntry("EnableMiddleEmulation", false);
        enableMiddleEmulation->setChecked(middleemulation);

        emit changed( useDefaults );
}

void SonyConfig::defaults()
{
	load( true );
}


void SonyConfig::configChanged()
{
  emit changed(true);
}


TQString SonyConfig::quickHelp() const
{
  return i18n("<h1>Sony Laptop Hardware Setup</h1>This module allows you to configure "
	"some Sony laptop hardware for your system");
}


void SonyConfig::slotStartMonitor()
{
	wake_laptop_daemon();
}


#include "sony.moc"


