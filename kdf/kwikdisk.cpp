/*
  kwikdisk.cpp - KDiskFree

  Copyright (C) 1999 by Michael Kropfberger <michael.kropfberger@gmx.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

  */

//
// 1999-12-03 Espen Sand
// Cleanups, improvements and fixes for KDE-2
//
// 2004-07-15 Stanislav Karchebny
// Rewrite for KDE 3
//

#include "kwikdisk.h"

#include <stdlib.h>

#include <tqpen.h>
#include <tqbitmap.h>
#include <tqpainter.h>

#include <tdemainwindow.h>
#include <tdelocale.h>
#include <tdeapplication.h>
#include <tdeaboutdata.h>
#include <tdecmdlineargs.h>
#include <tdemessagebox.h>
#include <tdepopupmenu.h>
#include <krun.h>

static const char description[] =
   I18N_NOOP("TDE Free disk space utility");

static const char version[] = "0.2";

static TDECmdLineOptions options[] =
{
   TDECmdLineLastOption
};

/*****************************************************************************/

KwikDisk::KwikDisk()
   : KSystemTray()
   , m_readingDF(FALSE)
   , m_dirty(TRUE)
   , m_menuVisible(FALSE)
   , m_inside(FALSE)
   , m_optionDialog(0)
{
   kdDebug() << k_funcinfo << endl;

   setPixmap(KSystemTray::loadIcon("kdf"));
   show();

   connect( &m_diskList, TQT_SIGNAL(readDFDone()), this, TQT_SLOT(updateDFDone()) );
   connect( &m_diskList, TQT_SIGNAL(criticallyFull(DiskEntry*)),
            this, TQT_SLOT(criticallyFull(DiskEntry*)) );

   connect( contextMenu(), TQT_SIGNAL(aboutToHide()), this, TQT_SLOT(aboutToHide()) );

   loadSettings();
   updateDF();
}

void KwikDisk::aboutToHide()
{
   kdDebug() << k_funcinfo << endl;
   if( !m_inside )
      m_menuVisible = FALSE;
}

void KwikDisk::enterEvent(TQEvent *)
{
   kdDebug() << k_funcinfo << endl;
   m_inside = TRUE;
}

void KwikDisk::leaveEvent(TQEvent *)
{
   kdDebug() << k_funcinfo << endl;
   m_inside = FALSE;
}

void KwikDisk::mousePressEvent(TQMouseEvent *me)
{
   kdDebug() << k_funcinfo << endl;

   if( m_dirty )
      updateDF();

   if( m_menuVisible )
   {
      contextMenu()->hide();
      m_menuVisible = FALSE;
      me->ignore();
      return;
   }

   contextMenuAboutToShow(contextMenu());
   contextMenu()->popup( me->globalPos() );
   m_menuVisible = TRUE;
}

void KwikDisk::loadSettings()
{
   kdDebug() << k_funcinfo << endl;

   m_options.updateConfiguration();
   setUpdateFrequency( m_options.updateFrequency() );
}

void KwikDisk::setUpdateFrequency(int frequency)
{
   kdDebug() << k_funcinfo << endl;

   //
   // Kill current timer and restart it if the frequency is
   // larger than zero.
   //
   TQT_TQOBJECT(this)->killTimers();
   if( frequency > 0 )
   {
      startTimer(frequency * 1000);
   }
}

/**
 * Mark the list as dirty thus forcing a reload the next time the
 * popup menu is about to become visible. Note: A current visible popup
 * will not be updated now.
 */
void KwikDisk::timerEvent(TQTimerEvent *)
{
   kdDebug() << k_funcinfo << endl;
   m_dirty = TRUE;
}

void KwikDisk::updateDF()
{
   kdDebug() << k_funcinfo << endl;

   m_readingDF = true;
   m_diskList.readFSTAB();
   m_diskList.readDF();
}

void KwikDisk::updateDFDone()
{
   kdDebug() << k_funcinfo << endl;

   m_readingDF = FALSE;
   m_dirty     = FALSE;

   contextMenu()->clear();
   contextMenu()->insertTitle(KSystemTray::loadIcon("kdf"), i18n("KwikDisk"));

   int itemNo = 0;
   for( DiskEntry *disk = m_diskList.first(); disk != 0; disk = m_diskList.next() )
   {
      // FIXME: tool tips are unused atm
      TQString toolTipText = i18n("%1 (%2) %3 on %4")
         .arg( disk->mounted() ? i18n("Unmount") : i18n("Mount"))
         .arg(disk->fsType()).arg(disk->deviceName()).arg(disk->mountPoint());

      TQString entryName = disk->mountPoint();
      if( disk->mounted() )
      {
         entryName += TQString("\t\t\t[%1]").arg(disk->prettyKBAvail());
      }
      int id = contextMenu()->insertItem("", this, TQT_SLOT(toggleMount(int)) );
      contextMenu()->setItemParameter(id, itemNo);
      itemNo++;

      TQPixmap *pix = new TQPixmap(KSystemTray::loadIcon(disk->iconName()));

      if( getuid() !=0 && disk->mountOptions().find("user",0, false) == -1 )
      {
         //
         // Special root icon, normal user cant mount.
         //
         // 2000-01-23 Espen Sand
         // Careful here: If the mask has not been defined we can
         // not use TQPixmap::mask() because it returns 0 => segfault
         //
         if( pix->mask() != 0 )
         {
            TQBitmap *bm = new TQBitmap(*(pix->mask()));
            if( bm != 0 )
            {
               TQPainter qp( bm );
               qp.setPen(TQPen(white,1));
               qp.drawRect(0,0,bm->width(),bm->height());
               qp.end();
               pix->setMask(*bm);
            }
            TQPainter qp( pix );
            qp.setPen(TQPen(red,1));
            qp.drawRect(0,0,pix->width(),pix->height());
            qp.end();
         }
         contextMenu()->disconnectItem(id,disk,TQT_SLOT(toggleMount()));
         toolTipText = i18n("You must login as root to mount this disk");
      }

      contextMenu()->changeItem(*pix,entryName,id);
   }

   contextMenu()->insertSeparator();

   contextMenu()->insertItem(
      KSystemTray::loadIcon("kdf"),
      i18n("&Start KDiskFree"), this, TQT_SLOT(startKDF()),0);

   contextMenu()->insertItem(
      KSystemTray::loadIcon("configure"),
      i18n("&Configure KwikDisk..."), this, TQT_SLOT(changeSettings()),0);

   contextMenu()->insertItem(
      KSystemTray::loadIcon("help"),
      KStdGuiItem::help().text(), this, TQT_SLOT(invokeHelp()),0);

   contextMenu()->insertSeparator();

   contextMenu()->insertItem(
      KSystemTray::loadIcon("exit"),
      KStdGuiItem::quit().text(), this, TQT_SIGNAL(quitSelected()) );
}

void KwikDisk::toggleMount(int item)
{
   kdDebug() << k_funcinfo << endl;

   DiskEntry *disk = m_diskList.at(item);
   if( disk == 0 )
   {
      return;
   }

   int val = disk->toggleMount();
   if( val != 0 )
   {
      KMessageBox::error(this, disk->lastSysError());
   }
   else if( (m_options.openFileManager() == true) && (disk->mounted() == true ) )
   {
      kdDebug() << "opening filemanager" << endl;
      if( m_options.fileManager().isEmpty() == false )
      {
         TQString cmd = m_options.fileManager();
         int pos = cmd.find("%m");
         if( pos > 0 )
         {
            cmd = cmd.replace( pos, 2, TDEProcess::quote(disk->mountPoint()) ) + " &";
         }
         else
         {
            cmd += " " + TDEProcess::quote(disk->mountPoint()) +" &";
         }
         system( TQFile::encodeName(cmd) );
      }
   }
   m_dirty = TRUE;
}

void KwikDisk::criticallyFull(DiskEntry *disk)
{
   kdDebug() << k_funcinfo << endl;

   if( m_options.popupIfFull() == true )
   {
      TQString msg = i18n("Device [%1] on [%2] is getting critically full!")
                    .arg(disk->deviceName()).arg(disk->mountPoint());
      KMessageBox::sorry( this, msg, i18n("Warning"));
   }
}

void KwikDisk::changeSettings()
{
   if( m_optionDialog == 0 )
   {
      m_optionDialog = new COptionDialog(this, "options", FALSE);
      if( !m_optionDialog ) return;
      connect(m_optionDialog, TQT_SIGNAL(valueChanged()),
                        this, TQT_SLOT(loadSettings()));
   }
   m_optionDialog->show();
}

void KwikDisk::startKDF()
{
   kdDebug() << k_funcinfo << endl;

   KRun::runCommand("kdf");
}

void KwikDisk::invokeHelp()
{
   kapp->invokeHelp("", "kdf");
}

/*****************************************************************************/

int main(int argc, char **argv)
{
   TDELocale::setMainCatalogue( "kdf" );

   TDEAboutData about("kwikdisk", I18N_NOOP("KwikDisk"), version, description,
                  TDEAboutData::License_GPL, "(C) 2004 Stanislav Karchebny",
                  0, 0, "Stanislav.Karchebny@kdemail.net");
   about.addAuthor( "Michael Kropfberger", I18N_NOOP("Original author"),
                    "michael.kropfberger@gmx.net" );
   about.addAuthor( "Espen Sand", I18N_NOOP("TDE 2 changes"), "" );
   about.addAuthor( "Stanislav Karchebny", I18N_NOOP("TDE 3 changes"),
                    "Stanislav.Karchebny@kdemail.net" );
   TDECmdLineArgs::init(argc, argv, &about);
   TDECmdLineArgs::addCmdLineOptions( options );
   TDEApplication app;
   KwikDisk *mainWin = 0;

   mainWin = new KwikDisk;
   TQObject::connect(mainWin, TQT_SIGNAL(quitSelected()), &app, TQT_SLOT(quit()));

   // mainWin has WDestructiveClose flag by default, so it will delete itself.
   return app.exec();
}

/*****************************************************************************/

#include "kwikdisk.moc"
