/*
 * sony.cpp
 *
 * Copyright (c) 2002 Paul Campbell <paul@taniwha.com>
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

// other Qt headers:
#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qhbox.h>
#include <qvgroupbox.h>
#include <qgrid.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qtooltip.h>

extern void wake_laptop_daemon();

SonyConfig::SonyConfig(QWidget * parent, const char *name)
  : KCModule(parent, name)
{
    KGlobal::locale()->insertCatalogue("klaptopdaemon"); // For translation of klaptopdaemon messages

    config =  new KConfig("kcmlaptoprc");

    QVBoxLayout *top_layout = new QVBoxLayout( this, KDialog::marginHint(),
					       KDialog::spacingHint() );

    // TODO: remove linefeed from string, can't do it right now coz we have a string freeze
    top_layout->addWidget(new KRichTextLabel(i18n("This panel allows you to control some of the features of the\n"
		    	"'sonypi' device for your laptop - you should not enable the options below if you\nalso "
			"use the 'sonypid' program in your system").replace("\n", " "), this));

    enableScrollBar = new QCheckBox( i18n("Enable &scroll bar"), this );
    QToolTip::add( enableScrollBar, i18n( "When checked this box enables the scrollbar so that it works under KDE" ) );
    top_layout->addWidget( enableScrollBar );
    connect( enableScrollBar, SIGNAL(clicked()), this, SLOT(configChanged()) );

    enableMiddleEmulation = new QCheckBox( i18n("&Emulate middle mouse button with scroll bar press"), this );
    QToolTip::add( enableMiddleEmulation, i18n( "When checked this box enables pressing the scroll bar to act in the same way as pressing the middle button on a 3 button mouse" ) );
    top_layout->addWidget( enableMiddleEmulation );
    connect( enableMiddleEmulation, SIGNAL(clicked()), this, SLOT(configChanged()) );

    if (::access("/dev/sonypi", R_OK) != 0) {
	enableMiddleEmulation->setEnabled(0);
	enableScrollBar->setEnabled(0);
	
        // TODO: remove linefeed from string, can't do it right now coz we have a string freeze
    	top_layout->addWidget(new KRichTextLabel(i18n("The /dev/sonypi is not accessable, if you wish to use the above features its\n"
					      "protections need to be changed. Clicking on the button below will change them\n").replace("\n", " "), this));
        QHBoxLayout *ll = new QHBoxLayout();
        QPushButton *setupButton = new QPushButton(i18n("Setup /dev/sonypi"), this);
        connect( setupButton, SIGNAL(clicked()), this, SLOT(setupHelper()) );
        QToolTip::add( setupButton, i18n( "This button can be used to enable the sony specific features" ) );
        ll->addStretch(2);
        ll->addWidget(setupButton);
        ll->addStretch(8);
        top_layout->addLayout(ll);
    }
    

    top_layout->addStretch(1);
    top_layout->addWidget( new QLabel( i18n("Version: %1").arg(LAPTOP_VERSION), this), 0, Qt::AlignRight );


    load();      
}

void SonyConfig::setupHelper()
{
	QString kdesu = KStandardDirs::findExe("kdesu");
	if (!kdesu.isEmpty()) {
		int rc = KMessageBox::warningContinueCancel(0,
				i18n("You will need to supply a root password "
					"to allow the protections of /dev/sonypi to be changed."),
				i18n("KLaptopDaemon"), KStdGuiItem::cont(),
				"");
		if (rc == KMessageBox::Continue) {
			KProcess proc;
			proc << kdesu;
			proc << "-u";
			proc << "root";
			proc <<  "chmod +r /dev/sonypi";
			proc.start(KProcess::Block);	// run it sync so has_acpi below sees the results
		}
	} else {
		KMessageBox::sorry(0, i18n("The /dev/sonypi protections cannot be changed because kdesu cannot be found.  Please make sure that it is installed correctly."),
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


QString SonyConfig::quickHelp() const
{
  return i18n("<h1>Sony Laptop Hardware Setup</h1>This module allows you to configure "
	"some Sony laptop hardware for your system");
}


void SonyConfig::slotStartMonitor()
{
	wake_laptop_daemon();
}


#include "sony.moc"


