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

#include <tqpushbutton.h>
#include <tqlayout.h>
#include <tqcursor.h>

#include <tdelistview.h>
#include <ksimpleconfig.h>
#include <tdelocale.h>
#include <kiconloader.h>
#include <tdeapplication.h>
#include <tdepopupmenu.h>
#include <kdebug.h>
#include <tdemessagebox.h>

#include "netconfig.h"
#include "netconfig.moc"
#include "netdialog.h"

NetConfig::NetConfig(KSim::PluginObject *parent, const char *name)
   : KSim::PluginPage(parent, name)
{
  m_yes = i18n("yes");
  m_no = i18n("no");

  TQVBoxLayout *mainLayout = new TQVBoxLayout(this);
  mainLayout->setSpacing(6);

  usingBox = new TDEListView(this);
  usingBox ->addColumn(i18n("Interface"));
  usingBox ->addColumn(i18n("Timer"));
  usingBox ->addColumn(i18n("Commands"));
  usingBox->setAllColumnsShowFocus(true);

  connect(usingBox, TQT_SIGNAL(contextMenu(TDEListView *,
     TQListViewItem *, const TQPoint &)),
     TQT_SLOT(menu(TDEListView *, TQListViewItem *, const TQPoint &)));
  connect(usingBox, TQT_SIGNAL(doubleClicked(TQListViewItem *)),
     TQT_SLOT(modifyItem(TQListViewItem *)));
  mainLayout->addWidget(usingBox);

  layout = new TQHBoxLayout;
  layout->setSpacing(6);

  TQSpacerItem *spacer = new TQSpacerItem(20, 20,
     TQSizePolicy::Expanding, TQSizePolicy::Minimum);
  layout->addItem(spacer);

  insertButton = new TQPushButton(this);
  insertButton->setText(i18n("Add..."));
  connect(insertButton, TQT_SIGNAL(clicked()), TQT_SLOT(showNetDialog()));
  layout->addWidget(insertButton);

  modifyButton = new TQPushButton(this);
  modifyButton->setText(i18n("Modify..."));
  connect(modifyButton, TQT_SIGNAL(clicked()), TQT_SLOT(modifyCurrent()));
  layout->addWidget(modifyButton);

  removeButton = new TQPushButton(this);
  removeButton->setText(i18n("Remove"));
  connect(removeButton, TQT_SIGNAL(clicked()), TQT_SLOT(removeCurrent()));
  layout->addWidget(removeButton);

  mainLayout->addLayout(layout);
}

NetConfig::~NetConfig()
{
}

void NetConfig::saveConfig()
{
  qHeapSort( m_networkList);

  int i = 0;
  Network::List::Iterator it;
  for( it = m_networkList.begin(); it != m_networkList.end(); ++it )
  {
    config()->setGroup( "device-" + TQString::number( i ) );
    config()->writeEntry( "deviceName", ( *it ).name() );
    config()->writeEntry( "showTimer", ( *it ).showTimer() );
    config()->writeEntry( "deviceFormat", ( *it ).format() );
    config()->writeEntry( "commands", ( *it ).commandsEnabled() );
    config()->writeEntry( "cCommand", ( *it ).connectCommand() );
    config()->writeEntry( "dCommand", ( *it ).disconnectCommand() );
    i++;
  }

  config()->setGroup( "Net" );
  config()->writeEntry( "deviceAmount", m_networkList.count() );
}

void NetConfig::readConfig()
{
  usingBox->clear();
  m_networkList.clear();

  config()->setGroup( "Net" );
  int deviceAmount = config()->readNumEntry( "deviceAmount" );

  for ( int i = 0; i < deviceAmount; ++i )
  {
    if ( !config()->hasGroup( "device-" + TQString::number( i ) ) )
    {
      continue;
    }

    config()->setGroup("device-" + TQString::number(i));

    m_networkList.append( Network( config()->readEntry( "deviceName" ),
       config()->readEntry( "deviceFormat" ),
       config()->readBoolEntry( "showTimer" ),
       config()->readBoolEntry( "commands" ),
       config()->readEntry( "cCommand" ),
       config()->readEntry( "dCommand" ) ) );

    (void) new TQListViewItem( usingBox,
       config()->readEntry( "deviceName" ),
       boolToString( config()->readBoolEntry( "showTimer" ) ),
       boolToString( config()->readBoolEntry( "commands" ) ) );
  }
}

void NetConfig::menu(TDEListView *, TQListViewItem *item, const TQPoint &)
{
  aboutMenu = new TQPopupMenu(this);

  if (item) {
    aboutMenu->insertItem(i18n("&Add Net Device"), 3);
    aboutMenu->insertItem(i18n("&Modify '%1'").arg(item->text(0)), 2);
    aboutMenu->insertItem(i18n("&Remove '%1'").arg(item->text(0)), 1);
  }
  else {
    aboutMenu->insertItem(i18n("&Add Net Device"), 3);
    aboutMenu->insertItem(i18n("&Modify..."), 2);
    aboutMenu->insertItem(i18n("&Remove..."), 1);
    aboutMenu->setItemEnabled(1, false);
    aboutMenu->setItemEnabled(2, false);
  }

  switch (aboutMenu->exec(TQCursor::pos())) {
    case 1:
      removeItem(item);
      break;
    case 2:
      modifyItem(item);
      break;
    case 3:
      showNetDialog();
      break;
  }

  delete aboutMenu;
}

void NetConfig::modifyItem(TQListViewItem *item)
{
  if (!item)
    return;

  netDialog = new NetDialog(this);
  Network::List::Iterator it, netDevice;
  for ( it = m_networkList.begin(); it != m_networkList.end(); ++it )
  {
    if ( ( *it ).name() == item->text( 0 ) )
    {
      netDevice = it;
      netDialog->setDeviceName( ( *it ).name() );
      netDialog->setShowTimer( ( *it ).showTimer() );
      netDialog->setFormat( ( *it ).format() );
      netDialog->setShowCommands( ( *it ).commandsEnabled() );
      netDialog->setCCommand( ( *it ).connectCommand() );
      netDialog->setDCommand( ( *it ).disconnectCommand() );
      break;
    }
  }

  netDialog->exec();
  if ( netDialog->okClicked() )
  {
    m_networkList.remove( netDevice );
    m_networkList.append( Network( netDialog->deviceName(),
       netDialog->format(),
       netDialog->timer(),
       netDialog->commands(),
       netDialog->cCommand(),
       netDialog->dCommand() ) );

    item->setText( 0, netDialog->deviceName() );
    item->setText( 1, boolToString( netDialog->timer() ) );
    item->setText( 2, boolToString( netDialog->commands() ) );
  }

  delete netDialog;
}

void NetConfig::removeItem(TQListViewItem *item)
{
  if (!item)
    return;

  int result = KMessageBox::warningContinueCancel(0, i18n("Are you sure you "
     "want to remove the net interface '%1'?").arg(item->text(0)), TQString(), KStdGuiItem::del());

  if (result == KMessageBox::Cancel)
    return;

  int i = 0;
  Network::List::Iterator it;
  for ( it = m_networkList.begin(); it != m_networkList.end(); ++it )
  {
    if ( item->text( 0 ) == ( *it ).name() )
    {
      m_networkList.remove( it );
      if ( config()->deleteGroup( "device-" + TQString::number( i ) ) )
        kdDebug(2003) << "device-" << i << " was deleted" << endl;

      break;
    }
    i++;
  }

  delete item;
}

void NetConfig::removeCurrent()
{
  removeItem(usingBox->currentItem());
}

void NetConfig::modifyCurrent()
{
  modifyItem(usingBox->currentItem());
}

void NetConfig::showNetDialog()
{
  netDialog = new NetDialog(this, "netDialog");
  netDialog->exec();
  if (netDialog->okClicked())
    getStats();

  delete netDialog;
}

void NetConfig::getStats()
{
  Network::List::ConstIterator it;
  for ( it = m_networkList.begin(); it != m_networkList.end(); ++it )
  {
    if ( ( *it ).name() == netDialog->deviceName() )
    {
      KMessageBox::sorry(0, i18n("You already have a network "
        "interface by this name. Please select a different interface"));

      return;
    }
  }

  m_networkList.append( Network( netDialog->deviceName(),
     netDialog->format(),
     netDialog->timer(),
     netDialog->commands(),
     netDialog->cCommand(),
     netDialog->dCommand() ) );

  (void) new TQListViewItem( usingBox,
     netDialog->deviceName(),
     boolToString( netDialog->timer() ),
     boolToString( netDialog->commands() ) );
}

const TQString &NetConfig::boolToString(bool value) const
{
  if (value)
    return m_yes;

  return m_no;
}
