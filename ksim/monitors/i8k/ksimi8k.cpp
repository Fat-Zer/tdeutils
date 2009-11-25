/*  Dell i8K Hardware Monitor Plug-in for KSim
 *
 *  Copyright (C) 2003 Nadeem Hasan <nhasan@kde.org>
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

#include "ksimi8k.h"

#include <qlayout.h>
#include <qtimer.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qlabel.h>
#include <qcheckbox.h>

#include <label.h>
#include <klocale.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kaboutapplication.h>
#include <knuminput.h>
#include <kdebug.h>

KSIM_INIT_PLUGIN(I8KPlugin);

I8KPlugin::I8KPlugin(const char *name)
   : KSim::PluginObject(name)
{
  setConfigFileName(instanceName());
}

I8KPlugin::~I8KPlugin()
{
}

KSim::PluginView *I8KPlugin::createView(const char *className)
{
  return new I8KView(this, className);
}

KSim::PluginPage *I8KPlugin::createConfigPage(const char *className)
{
  return new I8KConfig(this, className);
}

void I8KPlugin::showAbout()
{
  QString version = kapp->aboutData()->version();

  KAboutData aboutData(instanceName(),
     I18N_NOOP("KSim I8K Plugin"), version.latin1(),
     I18N_NOOP("Dell I8K Hardware Monitor plugin"),
     KAboutData::License_GPL, "(C) 2003 Nadeem Hasan");

  aboutData.addAuthor("Nadeem Hasan", I18N_NOOP("Author"),
     "nhasan@kde.org");

  KAboutApplication(&aboutData).exec();
}

I8KView::I8KView(KSim::PluginObject *parent, const char *name)
   : KSim::PluginView(parent, name),
     m_timer( 0L ), m_procFile( 0L ), m_procStream( 0L )
{
  initGUI();

  m_timer = new QTimer( this );

  m_reData = new QRegExp( "\\S+\\s+\\S+\\s+\\S+\\s+(\\d+)\\s+\\S+"
        "\\s+\\S+\\s+(\\d+)\\s+(\\d+)\\s+\\S+\\s+\\S+" );

  openStream();
  reparseConfig();
}

I8KView::~I8KView()
{
  closeStream();

  delete m_timer;
  delete m_reData;
}

void I8KView::reparseConfig()
{
  config()->setGroup("I8KPlugin");

  m_unit = config()->readEntry( "Unit", "C" );
  m_interval = config()->readNumEntry( "Interval", 5 );

  if ( m_procStream )
  {
    m_timer->stop();
    m_timer->start( m_interval*1000 );
  }

  updateView();
}

void I8KView::openStream()
{
  kdDebug( 2003 ) << "i8k: Trying /proc/i8k...." << endl;

  m_timer->stop();

  if ( ( m_procFile = fopen( "/proc/i8k", "r" ) ) )
  {
    m_procStream = new QTextIStream( m_procFile );
    disconnect( m_timer, 0, 0, 0 );
    connect( m_timer, SIGNAL( timeout() ), SLOT( updateView() ) );
    m_timer->start( m_interval*1000 );

    kdDebug( 2003 ) << "i8k: Success" << endl;
  }
  else
  {
    // i8k module is not loaded. Try again after 30 secs.
    disconnect( m_timer, 0, 0, 0 );
    connect( m_timer, SIGNAL( timeout() ), SLOT( openStream() ) );
    m_timer->start( 30*1000 );
    kdDebug( 2003 ) << "i8k: Failed...retry after 30 secs" << endl;
  }
}

void I8KView::closeStream()
{
  if ( m_procFile )
    fclose( m_procFile );

  if ( m_procStream )
    delete m_procStream;

  m_procFile = 0L;
  m_procStream = 0L;
  m_timer->stop();
}

void I8KView::initGUI()
{
  QVBoxLayout *layout = new QVBoxLayout( this );

  m_fan1Label = new KSim::Label( this );
  m_fan2Label = new KSim::Label( this );
  m_tempLabel = new KSim::Label( this );

  layout->addWidget( m_fan1Label );
  layout->addWidget( m_fan2Label );
  layout->addWidget( m_tempLabel );
}

void I8KView::updateView()
{
  kdDebug( 2003 ) << "i8k: Updating..." << endl;
  int cputemp=0, rightspeed=0, leftspeed=0;

  if ( m_procStream )
  {
    fseek( m_procFile, 0L, SEEK_SET );

    QString line = m_procStream->read();

    if ( line.isEmpty() )
    {
      // i8k module is no longer available, is it possible?
      closeStream();
      openStream();
      return;
    }

    if ( m_reData->search( line ) > -1 )
    {
      QStringList matches = m_reData->capturedTexts();

      cputemp = matches[ 1 ].toInt();
      leftspeed = matches[ 2 ].toInt();
      rightspeed = matches[ 3 ].toInt();

      if ( m_unit == "F" )
        cputemp = ( cputemp*9/5 ) + 32;
    }
  }

  if ( rightspeed > 0 )
    m_fan1Label->setText( i18n( "Right fan: %1 RPM" ).arg( rightspeed ) );
  else
    m_fan1Label->setText( i18n( "Right fan: Off" ) );

  if ( leftspeed > 0 )
    m_fan2Label->setText( i18n( "Left fan: %1 RPM" ).arg( leftspeed ) );
  else
    m_fan2Label->setText( i18n( "Left fan: Off" ) );

  m_tempLabel->setText( i18n( "CPU temp: %1°%2" ).arg( cputemp )
      .arg( m_unit ) );
}

I8KConfig::I8KConfig(KSim::PluginObject *parent, const char *name)
   : KSim::PluginPage(parent, name)
{
  m_unit = new QCheckBox( i18n( "Show temperature in Fahrenheit" ),
        this );
  QLabel *label = new QLabel( i18n( "Update interval:" ), this );
  m_interval = new KIntNumInput( this );
  m_interval->setRange( 2, 60, 1, true );
  m_interval->setSuffix( i18n( " sec" ) );

  QGridLayout *layout = new QGridLayout( this, 3, 2, 0, 
      KDialog::spacingHint() );

  layout->addMultiCellWidget( m_unit, 0, 0, 0, 1 );
  layout->addWidget( label, 1, 0 );
  layout->addWidget( m_interval, 1, 1 );
  layout->setColStretch( 1, 1 );
  layout->setRowStretch( 2, 1 );
}

I8KConfig::~I8KConfig()
{
}

void I8KConfig::readConfig()
{
  config()->setGroup("I8KPlugin");

  QString unit = config()->readEntry( "Unit", "C" );
  int interval = config()->readNumEntry( "Interval", 5 );

  m_unit->setChecked( unit == "F" );
  m_interval->setValue( interval );
}

void I8KConfig::saveConfig()
{
  config()->setGroup("I8KPlugin");

  config()->writeEntry( "Unit", m_unit->isChecked()? "F" : "C" );
  config()->writeEntry( "Interval", m_interval->value() );
}

#include "ksimi8k.moc"

/* vim: et sw=2 ts=2
*/
