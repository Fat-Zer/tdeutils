/*  ksim - a system monitor for kde
 *
 *  Copyright (C) 2001  Robbie Ward <linuxphreak@gmx.co.uk>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301, USA.
 */

#include "ksim.h"
#include "ksimview.h"
#include <ksimconfig.h>
#include <common.h>

#include <tqlayout.h>

#include <kaboutapplication.h>
#include <kbugreport.h>
#include <kaboutdata.h>
#include <dcopclient.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <klocale.h>

extern "C"
{
  KDE_EXPORT KPanelExtension *init(TQWidget *parent, const TQString &configFile)
  {
    KGlobal::locale()->insertCatalogue("ksim");
    return new KSim::PanelExtension(configFile, KPanelExtension::Normal,
    KPanelExtension::About | KPanelExtension::Help |
    KPanelExtension::Preferences | KPanelExtension::ReportBug,
    parent, "ksim");
  }
}

KSim::PanelExtension::PanelExtension(const TQString &configFile,
   Type type, int actions, TQWidget *parent, const char *name)
   : KPanelExtension(configFile, type, actions, parent, name)
{
  m_dcopClient = new DCOPClient;
  m_view = new KSim::MainView(config(), true, this, "m_view");
  m_view->positionChange(orientation());

  m_dcopClient->registerAs(name, false);

  m_aboutData = new KAboutData(name, I18N_NOOP("KSim"), KSIM_VERSION_STRING,
     I18N_NOOP("A plugin based system monitor for TDE"),
     KAboutData::License_GPL, I18N_NOOP("(C) 2001-2003 Robbie Ward\n(C) 2005 Reuben Sutton"));
  m_aboutData->addAuthor("Reuben Sutton", I18N_NOOP("Maintainer"),"reuben.sutton@gmail.com");
  m_aboutData->addAuthor("Robbie Ward", I18N_NOOP("Original Author"),
     "linuxphreak@gmx.co.uk");
  m_aboutData->addAuthor("Jason Katz-Brown", I18N_NOOP("Developer"),
     "jason@katzbrown.com");
  m_aboutData->addAuthor("Heitham Omar", I18N_NOOP("Some FreeBSD ports"),
     "super_ice@ntlworld.com");
  m_aboutData->addAuthor("Otto Bruggeman", I18N_NOOP("Testing, Bug fixing and some help"),
     "bruggie@home.nl");
}

KSim::PanelExtension::~PanelExtension()
{
  delete m_aboutData;
  delete m_dcopClient;
}

TQSize KSim::PanelExtension::sizeHint(Position p, TQSize maxSize) const
{
  return m_view->sizeHint(p, maxSize);
}

void KSim::PanelExtension::resizeEvent(TQResizeEvent *)
{
  m_view->resize(size());
}

KPanelExtension::Position KSim::PanelExtension::preferedPosition() const
{
  return KPanelExtension::Right;
}

void KSim::PanelExtension::reparse()
{
  emit updateLayout();
}

void KSim::PanelExtension::show()
{
  KPanelExtension::show();

  reparse();
}

void KSim::PanelExtension::about()
{
  KAboutApplication(m_aboutData).exec();
}

void KSim::PanelExtension::help()
{
}

void KSim::PanelExtension::preferences()
{
  m_view->preferences();
}

void KSim::PanelExtension::reportBug()
{
  KBugReport(this, true, m_aboutData).exec();
}

void KSim::PanelExtension::positionChange(Position)
{
  m_view->positionChange(orientation());
}

#include "ksim.moc"
