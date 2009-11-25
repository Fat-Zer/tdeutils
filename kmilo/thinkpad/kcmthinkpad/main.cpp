/*
 *  main.cpp
 *
 *  Copyright (C) 2004 Jonathan Riddell <jr@jriddell.org>
 *
 *  Based on kcm_kvaio
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

#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qfile.h>
#include <qbuttongroup.h>

#include <dcopclient.h>
#include <kgenericfactory.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kurlrequester.h>
#include <kcmodule.h>

#ifdef Q_OS_FREEBSD
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#include "kcmthinkpad_general.h"

#include <dcopclient.h>

#include "main.h"
#include "main.moc"

typedef KGenericFactory<KCMThinkpadModule, QWidget> KCMThinkpadModuleFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_thinkpad, KCMThinkpadModuleFactory("kcmthinkpad"))

#define CONFIG_FILE "kmilodrc"

KCMThinkpadModule::KCMThinkpadModule(QWidget* parent, const char* name, const QStringList&)
	: KCModule(KCMThinkpadModuleFactory::instance(), parent, name) {
	KAboutData* about =
		new KAboutData(I18N_NOOP("kcmthinkpad"),
			       I18N_NOOP("KDE Control Module for IBM Thinkpad "
					 "Laptop Hardware"),
			       0, 0, KAboutData::License_GPL,
			       "(c) 2004 Jonathan Riddell");

	about->addAuthor("Jonathan Riddell",
			 I18N_NOOP("Original author"),
			 "jr@jriddell.org");
	setAboutData( about );

	QVBoxLayout* layout = new QVBoxLayout(this);
	m_KCMThinkpadGeneral = new KCMThinkpadGeneral(this);
	layout->addWidget( m_KCMThinkpadGeneral );
	layout->addStretch();

	load();

	//try and open /dev/nvram
	m_nvramReadable = false;
	m_nvramWriteable = false;

#ifdef Q_OS_FREEBSD
	// Look if the sysctl tree of acpi_ibm is in place
	u_int n = 0;
	size_t len = sizeof(n);
	if (m_nvramReadable = ( sysctlbyname("dev.acpi_ibm.0.volume", &n, &len, NULL, 0) != -1 ))
		m_nvramWriteable = ( sysctlbyname("dev.acpi_ibm.0.volume", NULL, NULL, &n, len) != -1 );

	if (!m_nvramReadable) {
		setButtons(buttons() & ~Default);
		m_KCMThinkpadGeneral->bgGeneral->setEnabled(false);
		m_KCMThinkpadGeneral->tlOff->setText(i18n("In order to use the Thinkpad Buttons KMilo Plugin, "
							  "you have to load the acpi_ibm(4) driver."));
	} else if (!m_nvramWriteable) {
		m_KCMThinkpadGeneral->tlOff->setText(i18n("Could not write to dev.acpi_ibm.0.volume. "
							  "Using software volume, required for "
							  "R30/R31 models, or using a custom volume "
							  "change step is disabled."));
#else
	QFile nvramFile(m_nvramFile);
	if ( nvramFile.open(IO_ReadOnly) )  {
		m_nvramReadable = true;
		nvramFile.close();
	}
	if ( nvramFile.open(IO_WriteOnly) )  {
		m_nvramWriteable = true;
		nvramFile.close();
	}

	if (!m_nvramReadable) {
            setButtons(buttons() & ~Default);
            m_KCMThinkpadGeneral->bgGeneral->setEnabled(false);
	} else if (!m_nvramWriteable) {
		m_KCMThinkpadGeneral->tlOff->setText(i18n("Could not write to %1. "
							  "To use the software volume, required for "
							  "R30/R31 models and to use a custom volume "
							  "change step, set the nvram device to world "
							  "writeable: <em>chmod 666 "
							  "/dev/nvram</em>").arg(m_nvramFile));
#endif
	} else {
		m_KCMThinkpadGeneral->tlOff->setText(i18n("Thinkpad Buttons KMilo Plugin Ready For Configuration"));
	}

	connect( m_KCMThinkpadGeneral, SIGNAL(changed()), SLOT(changed()));

}

void KCMThinkpadModule::save() {
	if (!m_nvramReadable)  {
		return;
	}
	DCOPClient client;

	KConfig config(CONFIG_FILE);
	config.setGroup("thinkpad");

	config.writeEntry("run", m_KCMThinkpadGeneral->mCbRun->isChecked());
	config.writeEntry("softwareVolume", m_KCMThinkpadGeneral->mCbSoftwareVolume->isChecked());
	config.writeEntry("volumeStep", m_KCMThinkpadGeneral->mSpinboxVolumeStep->value());
	config.writeEntry("buttonThinkpad", m_KCMThinkpadGeneral->commandExec->url());
	config.writeEntry("buttonHome", m_KCMThinkpadGeneral->commandExecHome->url());
	config.writeEntry("buttonSearch", m_KCMThinkpadGeneral->commandExecSearch->url());
	config.writeEntry("buttonMail", m_KCMThinkpadGeneral->commandExecMail->url());
	config.writeEntry("buttonZoom", m_KCMThinkpadGeneral->commandExecZoom->url());
	config.sync();

	if (client.attach()) {
		QByteArray data, replyData;
		QCString replyType;

		if (!client.call("kded", "kmilod", "reconfigure()", data, replyType, replyData))  {
			kdDebug() << "KCMThinkpad::showTextMsg: "
				  << "there was an error using DCOP on kmilod::reconfigure()." << endl;
		}
	} else {
		kdDebug() << "KCMThinkpadModule: cannot attach to DCOP server, "
			  << "no automatic config update." << endl;
	}
}

void KCMThinkpadModule::load() {
	load( false );
}

void KCMThinkpadModule::load(bool useDefaults) {
	KConfig config(CONFIG_FILE);

	config.setReadDefaults( useDefaults );

	config.setGroup("thinkpad");

	m_KCMThinkpadGeneral->mCbRun->setChecked(config.readBoolEntry("run", false));
	m_KCMThinkpadGeneral->mCbSoftwareVolume->setChecked(config.readBoolEntry("softwareVolume", true));
	m_KCMThinkpadGeneral->mSpinboxVolumeStep->setValue(config.readNumEntry("volumeStep", 14));
	m_KCMThinkpadGeneral->commandExec->setURL(config.readEntry("buttonThinkpad", KDE_BINDIR "/konsole"));
	m_KCMThinkpadGeneral->commandExecHome->setURL(config.readEntry("buttonHome", KDE_BINDIR "/konqueror"));
	m_KCMThinkpadGeneral->commandExecSearch->setURL(config.readEntry("buttonSearch", KDE_BINDIR "/kfind"));
	m_KCMThinkpadGeneral->commandExecMail->setURL(config.readEntry("buttonMail", KDE_BINDIR "/kmail"));
	m_KCMThinkpadGeneral->commandExecZoom->setURL(config.readEntry("buttonZoom", KDE_BINDIR "/ksnapshot"));
	m_nvramFile = config.readEntry("nvram", "/dev/nvram");

	emit changed( useDefaults );
}

void KCMThinkpadModule::defaults() {
	load( true );
}

