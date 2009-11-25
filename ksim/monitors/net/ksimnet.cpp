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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#ifdef __FreeBSD__
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/route.h>

static int mib[] = { CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST, 0 };
#endif

#include <qpushbutton.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qdatetime.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qregexp.h>
#include <qcursor.h>
#include <qpopupmenu.h>

#include <kdebug.h>
#include <klocale.h>
#include <kaboutapplication.h>
#include <kaboutdata.h>
#include <ksimpleconfig.h>
#include <kglobal.h>
#include <krun.h>
#include <kapplication.h>
#include <kiconloader.h>

#include "ksimnet.h"
#include "netconfig.h"
#include <themetypes.h>

#define NET_UPDATE 1000
#define LED_UPDATE 125

KSIM_INIT_PLUGIN(NetPlugin)

NetPlugin::NetPlugin(const char *name)
   : KSim::PluginObject(name)
{
  setConfigFileName(instanceName());
}

NetPlugin::~NetPlugin()
{
}

KSim::PluginView *NetPlugin::createView(const char *className)
{
  return new NetView(this, className);
}

KSim::PluginPage *NetPlugin::createConfigPage(const char *className)
{
  return new NetConfig(this, className);
}

void NetPlugin::showAbout()
{
  QString version = kapp->aboutData()->version();

  KAboutData aboutData(instanceName(),
     I18N_NOOP("KSim Net Plugin"), version.latin1(),
     I18N_NOOP("A net plugin for KSim"),
     KAboutData::License_GPL, "(C) 2001 Robbie Ward");

  aboutData.addAuthor("Robbie Ward", I18N_NOOP("Author"),
     "linuxphreak@gmx.co.uk");
  aboutData.addAuthor("Heitham Omar", I18N_NOOP("FreeBSD ports"),
     "super_ice@ntlworld.com");

  KAboutApplication(&aboutData).exec();
}

NetView::NetView(KSim::PluginObject *parent, const char *name)
   : KSim::PluginView(parent, name)
{
#ifdef __linux__
  m_procStream = 0L;
  if ((m_procFile = fopen("/proc/net/dev", "r")))
    m_procStream = new QTextStream(m_procFile, IO_ReadOnly);
#endif
#ifdef __FreeBSD__
  m_buf = 0;
  m_allocSize = 0;
#endif

  m_firstTime = true;
  m_netLayout = new QVBoxLayout(this);

  m_networkList = createList();
  addDisplay();

  m_netTimer = new QTimer(this);
  connect(m_netTimer, SIGNAL(timeout()), SLOT(updateGraph()));
  m_netTimer->start(NET_UPDATE);

  m_lightTimer = new QTimer(this);
  connect(m_lightTimer, SIGNAL(timeout()), SLOT(updateLights()));
  m_lightTimer->start(LED_UPDATE);

  updateGraph();
}

NetView::~NetView()
{
#ifdef __linux__
  delete m_procStream;

  if (m_procFile)
    fclose(m_procFile);
#endif

  cleanup();
}

void NetView::reparseConfig()
{
  Network::List networkList = createList();
  if ( networkList == m_networkList )
    return;

  m_netTimer->stop();
  m_lightTimer->stop();
  m_firstTime = true;

  cleanup();

  m_networkList = networkList;
  addDisplay();

  m_netTimer->start(NET_UPDATE);
  m_lightTimer->start(LED_UPDATE);
}

void NetView::cleanup()
{
  Network::List::Iterator it;
  for ( it = m_networkList.begin(); it != m_networkList.end(); ++it )
  {
    ( *it ).cleanup();
  }

  m_networkList.clear();
}

void NetView::addDisplay()
{
  int i = 0;

  Network::List::Iterator it;
  for ( it = m_networkList.begin(); it != m_networkList.end(); ++it )
  {
    KSim::LedLabel *led = addLedLabel( ( *it ).name() );
    KSim::Label *label = ( ( *it ).showTimer() ? addLabel() : 0L );
    QPopupMenu * popup = ( ( *it ).commandsEnabled() ?
       addPopupMenu( ( *it ).name(), i ) : 0L );
    KSim::Chart *chart = addChart();
    //KSim::LedLabel *led = addLedLabel( ( *it ).name() );
    //KSim::Label *label = ( ( *it ).showTimer() ? addLabel() : 0L );
    //QPopupMenu * popup = ( ( *it ).commandsEnabled() ?
       //addPopupMenu( ( *it ).name(), i ) : 0L );

    if ( ( *it ).commandsEnabled() )
    {
      if ( chart )
      {
        chart->installEventFilter( this );
      }

      if ( led )
      {
        led->installEventFilter( this );
      }

      if ( label )
      {
        label->installEventFilter( this );
      }
    }

    ( *it ).setDisplay( chart, led, label, popup );
    ++i;
  }
}

// Run the connect command
void NetView::runConnectCommand( int value )
{
  int i = 0;
  Network::List::ConstIterator it;
  for ( it = m_networkList.begin(); it != m_networkList.end(); ++it )
  {
    if ( value == i )
    {
      // I use KRun here as it provides startup notification
      if ( !( *it ).connectCommand().isNull() )
      {
        KRun::runCommand( ( *it ).connectCommand() );
      }

      break;
    }
    ++i;
  }
}

// Run the disconnect command
void NetView::runDisconnectCommand( int value )
{
  int i = 0;
  Network::List::ConstIterator it;
  for ( it = m_networkList.begin(); it != m_networkList.end(); ++it )
  {
    if ( value == i )
    {
      // I use KRun here as it provides startup notification
      if ( !( *it ).disconnectCommand().isNull() )
      {
        KRun::runCommand( ( *it ).disconnectCommand() );
      }

      break;
    }

    ++i;
  }
}

Network::List NetView::createList() const
{
  config()->setGroup( "Net" );
  int amount = config()->readNumEntry( "deviceAmount", 0 );

  Network::List list;
  for ( int i = 0; i < amount; ++i )
  {
    if ( !config()->hasGroup( "device-" + QString::number( i ) ) )
    {
      continue;
    }

    config()->setGroup( "device-" + QString::number( i ) );

    list.append( Network( config()->readEntry( "deviceName" ),
       config()->readEntry( "deviceFormat" ),
       config()->readBoolEntry( "showTimer" ),
       config()->readBoolEntry( "commands" ),
       config()->readEntry( "cCommand" ),
       config()->readEntry("dCommand") ) );
  }

  qHeapSort( list );
  return list;
}

void NetView::updateLights()
{
  Network::List::Iterator it;
  for ( it = m_networkList.begin(); it != m_networkList.end(); ++it )
  {
    if ( isOnline( ( *it ).name() ) )
    {
      unsigned long receiveDiff = ( *it ).data().in - ( *it ).oldData().in;
      unsigned long sendDiff = ( *it ).data().out - ( *it ).oldData().out;
      unsigned long halfMax = ( *it ).maxValue() / 2;

      ( *it ).led()->setMaxValue( ( *it ).maxValue() / 1024 );
      ( *it ).led()->setValue( receiveDiff / 1024 );

      if ( receiveDiff == 0 )
      {
        ( *it ).led()->setOff( KSim::Led::First );
      }
      else if ( ( receiveDiff / 1024 ) >= halfMax )
      {
        ( *it ).led()->setOn( KSim::Led::First );
      }
      else
      {
        ( *it ).led()->toggle( KSim::Led::First );
      }

      if ( sendDiff == 0 )
      {
        ( *it ).led()->setOff( KSim::Led::Second );
      }
      else if ( ( sendDiff / 1024 ) >= halfMax )
      {
        ( *it ).led()->setOn( KSim::Led::Second );
      }
      else
      {
        ( *it ).led()->toggle( KSim::Led::Second );
      }
    }
    else
    {
      ( *it ).led()->setMaxValue( 0 );
      ( *it ).led()->setValue( 0 );
      ( *it ).led()->setOff( KSim::Led::First );
      ( *it ).led()->setOff( KSim::Led::Second );
    }
  }
}

void NetView::updateGraph()
{
  int timer = 0;
  int hours = 0;
  int minutes = 0;
  int seconds = 0;

  time_t start = 0;
  struct stat st;

  QTime netTime;
  QString timeDisplay;
  QString pid( "/var/run/%1.pid" );
  QString newPid;

  Network::List::Iterator it;
  for ( it = m_networkList.begin(); it != m_networkList.end(); ++it )
  {
    if ( isOnline( ( *it ).name() ) )
    {
      NetData data;

      if ( ( *it ).label() )
      {
        timeDisplay = ( *it ).format();
        newPid = pid.arg( ( *it ).name() );

        if ( QFile::exists( newPid ) && stat( QFile::encodeName( newPid ).data(), &st ) == 0 )
        {
          start = st.st_mtime;

          timer = static_cast<int>( difftime( time( 0 ), start ) );
          hours = timer / 3600;
          minutes = (timer - hours * 3600) / 60;
          seconds = timer % 60;
          if ( netTime.isValid( hours, minutes, seconds ) )
            netTime.setHMS( hours, minutes, seconds );
        }

        // Keep backwards compat for now
        if ( timeDisplay.contains( '%' ) > 0 )
          timeDisplay.replace( '%', "" );

        ( *it ).label()->setText( netTime.toString( timeDisplay ) );
      }

      netStatistics( ( *it ).name(), data );
      ( *it ).setData( data );

      unsigned long receiveDiff = data.in - ( *it ).oldData().in;
      unsigned long sendDiff = data.out - ( *it ).oldData().out;

      if ( m_firstTime )
      {
        receiveDiff = sendDiff = 0;
      }

      ( *it ).chart()->setValue( receiveDiff, sendDiff );
      ( *it ).setMaxValue( ( *it ).chart()->maxValue() );

      QString receiveString = KGlobal::locale()->formatNumber( ( float ) receiveDiff / 1024.0, 1 );
      QString sendString = KGlobal::locale()->formatNumber( ( float ) sendDiff / 1024.0, 1 );

      ( *it ).chart()->setText( i18n( "in: %1k" ).arg( receiveString ),
         i18n( "out: %1k" ).arg( sendString ) );
    }
    else
    {
      ( *it ).setData( NetData() );
      ( *it ).chart()->setValue( 0, 0 );

      ( *it ).chart()->setText( i18n( "in: %1k" ).arg( KGlobal::locale()->formatNumber( 0.0, 1 ) ),
         i18n( "out: %1k" ).arg( KGlobal::locale()->formatNumber( 0.0, 1 ) ) );

      if ( ( *it ).label() )
        ( *it ).label()->setText( i18n( "offline" ) );
    }
  }

  if ( m_firstTime )
    m_firstTime = false;
}

KSim::Chart *NetView::addChart()
{
  KSim::Chart *chart = new KSim::Chart(false, 0, this);
  m_netLayout->addWidget(chart);
  chart->show();
  return chart;
}

KSim::LedLabel *NetView::addLedLabel(const QString &device)
{
  KSim::LedLabel *ledLabel = new KSim::LedLabel(0, KSim::Types::Net,
     device, this);

  ledLabel->show();
  m_netLayout->addWidget(ledLabel);
  return ledLabel;
}

KSim::Label *NetView::addLabel()
{
  KSim::Label *label = new KSim::Label(KSim::Types::None, this);
  label->show();
  m_netLayout->addWidget(label);
  return label;
}

QPopupMenu *NetView::addPopupMenu(const QString &device, int value)
{
  QPopupMenu *popup = new QPopupMenu(this);
  popup->insertItem(SmallIcon("network"), i18n("Connect"), this,
     SLOT(runConnectCommand(int)), 0, 1);
  popup->setItemParameter(1, value);
  popup->insertItem(SmallIcon("network"), i18n("Disconnect"), this,
     SLOT(runDisconnectCommand(int)), 0, 2);
  popup->setItemParameter(2, value);
  menu()->insertItem(device, popup, 100 + value);
  return popup;
}

void NetView::netStatistics(const QString &device, NetData &data)
{
#ifdef __linux__
  if (m_procFile == 0) {
    data.in = 0;
    data.out = 0;
    return;
  }

  QString output;
  QString parser;
  // Parse the proc file
  while (!m_procStream->atEnd()) {
    parser = m_procStream->readLine();
    // remove all the entries apart from the line containing 'device'
    if (parser.find(device) != -1)
      output = parser;
  }

  if (output.isEmpty()) {
    data.in = 0;
    data.out = 0;
    return;
  }

  // make sure our output doesn't contain "eth0:11210107" so we dont
  // end up with netList[1] actually being netList[2]
  output.replace(QRegExp(":"), " ");
  QStringList netList = QStringList::split(' ', output);

  data.in = netList[1].toULong();
  data.out = netList[9].toULong();

  fseek(m_procFile, 0L, SEEK_SET);
#endif

#ifdef __FreeBSD__
  struct if_msghdr *ifm, *nextifm;
  struct sockaddr_dl *sdl;
  char *lim, *next;
  size_t needed;
  char s[32];

  if (sysctl(mib, 6, NULL, &needed, NULL, 0) < 0)
    return;

  if (m_allocSize < needed) {
    if (m_buf != NULL)
      delete[] m_buf;

    m_buf = new char[needed];

    if (m_buf == NULL)
      return;

    m_allocSize = needed;
  }

  if (sysctl(mib, 6, m_buf, &needed, NULL, 0) < 0)
    return;

  lim = m_buf + needed;

  next = m_buf;
  while (next < lim) {
    ifm = (struct if_msghdr *)next;
    if (ifm->ifm_type != RTM_IFINFO)
      return;

    next += ifm->ifm_msglen;

    // get an interface with a network address
    while (next < lim) {
      nextifm = (struct if_msghdr *)next;
      if (nextifm->ifm_type != RTM_NEWADDR)
        break;

      next += nextifm->ifm_msglen;
    }

    // if the interface is up
    if (ifm->ifm_flags & IFF_UP) {
      sdl = (struct sockaddr_dl *)(ifm + 1);
      if (sdl->sdl_family != AF_LINK)
        continue;

      strncpy(s, sdl->sdl_data, sdl->sdl_nlen);
      s[sdl->sdl_nlen] = '\0';

      if (strcmp(device.local8Bit().data(), s) == 0) {
        data.in = ifm->ifm_data.ifi_ibytes;
        data.out = ifm->ifm_data.ifi_obytes;
        return;
      }
    }
  }
#endif
}

bool NetView::isOnline(const QString &device)
{
#ifdef __linux__
  QFile file("/proc/net/route");
  if (!file.open(IO_ReadOnly))
    return -1;

  return (QTextStream(&file).read().find(device) != -1 ? true : false);
#endif

#ifdef __FreeBSD__
  struct if_msghdr *ifm, *nextifm;
  struct sockaddr_dl *sdl;
  char *lim, *next;
  size_t needed;
  char s[32];

  if (sysctl(mib, 6, NULL, &needed, NULL, 0) < 0)
    return false;

  if (m_allocSize < needed) {
    if (m_buf != NULL)
      delete[] m_buf;

    m_buf = new char[needed];

    if (m_buf == NULL)
      return false;

    m_allocSize = needed;
  }

  if (sysctl(mib, 6, m_buf, &needed, NULL, 0) < 0)
    return false;

  lim = m_buf + needed;

  next = m_buf;
  while (next < lim) {
    ifm = (struct if_msghdr *)next;
    if (ifm->ifm_type != RTM_IFINFO)
      return false;

    next += ifm->ifm_msglen;

    // get an interface with a network address
    while (next < lim) {
      nextifm = (struct if_msghdr *)next;
      if (nextifm->ifm_type != RTM_NEWADDR)
        break;

      next += nextifm->ifm_msglen;
    }

    // if the interface is up
    if (ifm->ifm_flags & IFF_UP) {
      sdl = (struct sockaddr_dl *)(ifm + 1);
      if (sdl->sdl_family != AF_LINK)
        continue;

      strncpy(s, sdl->sdl_data, sdl->sdl_nlen);
      s[sdl->sdl_nlen] = '\0';

    if (strcmp(s, device.local8Bit().data()) == 0)
      return true;
    }
  }

  return false;
#endif
}

// EventFilter
bool NetView::eventFilter( QObject * o, QEvent * e )
{
  // find out which interface we are
  int i = 0;
  Network::List::Iterator it;
  for ( it = m_networkList.begin(); it != m_networkList.end(); ++it )
  {
    if ( o == ( *it ).chart() || o == ( *it ).label() || o == ( *it ).led() )
    {
      break;
    }

    ++i;
  }

  if ( e->type() == QEvent::MouseButtonPress )
  {
    if ( static_cast<QMouseEvent *>( e )->button() == QMouseEvent::RightButton )
    {
      showMenu(i);
    }

    return true;
  }

  return false;
}

void NetView::showMenu(int i) {

  QPopupMenu menu;
  menu.insertItem( SmallIcon("network"), i18n("Connect"), 1);
  menu.insertItem( SmallIcon("network"), i18n("Disconnect"), 2);
  switch (menu.exec(QCursor::pos())) {
    case 1:
      runConnectCommand(i);
      break;
    case 2:
      runDisconnectCommand(i);
      break;
  }
}


#include "ksimnet.moc"
