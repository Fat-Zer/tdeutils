/***************************************************************************
    copyright (C) 2003      Adam Geitgey <adam@rootnode.org>
                  2003      Sven Leiber <s.leiber@web.de>
                  2000-2001 Matthias Ettrich <ettrich@kde.org>
                  2000-2001 Matthias Elter   <elter@kde.org>
                  2001      Carsten Pfeiffer <pfeiffer@kde.org>
                  2001      Martijn Klingens <mklingens@yahoo.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "systemtray.h"


#include <tqobject.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kwinmodule.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kwin.h>

#include <tqpopupmenu.h>
#include <tqdragobject.h>
#include <tqlayout.h>
#include <tqstringlist.h>
#include <tqpixmap.h>

#include <X11/Xlib.h>

Systemtray::Systemtray(TQWidget* tqparent)
  : TQWidget(tqparent,0,0)
{
  setBackgroundOrigin(ParentOrigin);
  setBackgroundMode(FixedPixmap);
  m_Wins.setAutoDelete(true);
}


Systemtray::~Systemtray()
{
  m_Wins.clear();
}

int Systemtray::getTraySize() {

	return (int) kwin_module->systemTrayWindows().size();
}

void Systemtray::updateBackgroundPixmap ( const TQPixmap & pixmap) {
  QXEmbed *emb;
  setPaletteBackgroundPixmap (pixmap);
    for (emb = m_Wins.first(); emb != 0L; emb = m_Wins.next()) {

    //Stupid stupid stupid work around for annoying bug
    //QXEmbed ignores setBackgroundOrigin(AncestorOrigin)....
    TQPixmap bug = TQPixmap(emb->size());
    bitBlt(TQT_TQPAINTDEVICE(&bug), 0, 0, TQT_TQPAINTDEVICE(const_cast<TQPixmap*>(&pixmap)), emb->parentWidget()->x()+emb->x(),  emb->parentWidget()->y()+emb->y(), emb->width(), emb->height(),TQt::CopyROP, false);
    emb->setPaletteBackgroundPixmap (bug);

  }

    TQPoint topPoint = mapToGlobal(TQPoint(0,0));
    Window hack = XCreateSimpleWindow(qt_xdisplay(), winId(), 0,0, width(), height(), 0, 0, 0);
    XRaiseWindow(qt_xdisplay(), hack);
    XMapWindow(qt_xdisplay(), hack);
    XUnmapWindow(qt_xdisplay(), hack);
    XDestroyWindow(qt_xdisplay(), hack);
}

void Systemtray::initSystray( void )
{
  bool existing = false;
  //bool content = false;
  Display *display = qt_xdisplay();
  no_of_systray_windows = 0;

  kwin_module = new KWinModule();
  systemTrayWindows = kwin_module->systemTrayWindows();
  TQValueList<WId>::ConstIterator end(systemTrayWindows.end());
  for (TQValueList<WId>::ConstIterator it = systemTrayWindows.begin(); it!=end; ++it)
  {
    no_of_systray_windows++;
    QXEmbed *emb;

    emb = new QXEmbed(this);
    emb->setBackgroundMode(FixedPixmap);

    emb->setAutoDelete(false);

    connect(emb, TQT_SIGNAL(embeddedWindowDestroyed()), TQT_SLOT(updateTrayWindows()));

    m_Wins.append(emb);

    emb->embed(*it);
    emb->resize(24, 24);
    emb->show();
    existing = true;
  }

  updateTrayWindows();

  connect(kwin_module, TQT_SIGNAL(systemTrayWindowAdded(WId)), TQT_SLOT(systemTrayWindowAdded(WId)));
  connect(kwin_module, TQT_SIGNAL(systemTrayWindowRemoved(WId)), TQT_SLOT(systemTrayWindowRemoved(WId)));

  TQCString screenstr;
  screenstr.setNum(qt_xscreen());
  TQCString trayatom = "_NET_SYSTEM_TRAY_S" + screenstr;

  net_system_tray_selection = XInternAtom( display, trayatom, false );
  net_system_tray_opcode = XInternAtom( display, "_NET_SYSTEM_TRAY_OPCODE", false );

  // Acquire system tray
  XSetSelectionOwner( display,
    net_system_tray_selection,
    winId(),
    CurrentTime );

  WId root = qt_xrootwin();

  if (XGetSelectionOwner(display, net_system_tray_selection) == winId())
  {
    XClientMessageEvent xev;

    xev.type = ClientMessage;
    xev.window = root;

    xev.message_type = XInternAtom(display, "MANAGER", false);
    xev.format = 32;

    xev.data.l[0] = CurrentTime;
    xev.data.l[1] = net_system_tray_selection;
    xev.data.l[2] = winId();
    xev.data.l[3] = 0;       /* Manager specific data */
    xev.data.l[4] = 0;       /* Manager specific data */

    XSendEvent( display, root, false, StructureNotifyMask, (XEvent *)&xev );
  }
}

void Systemtray::updateTrayWindows( void )
{
  QXEmbed *emb;

  emb = m_Wins.first();
  while ((emb = m_Wins.current()) != 0L)
  {
    WId wid = emb->embeddedWinId();
    if ((wid == 0) || !kwin_module->systemTrayWindows().contains(wid) )
      m_Wins.remove(emb);
    else
      m_Wins.next();
  }
  layoutSystray();
}
void Systemtray::layoutSystray()
{
  int i = 0, a = 0;

  QXEmbed* emb;
  int x = 0;
  int count = 0;

  //How many systray icons can fit on a line?
  int aa = width() / 24;

  if(aa < 1)
  {
    /* The place is to small to display a icon we make than one line with
       icons that we display at the top */
    aa = 1;
  }

  for (emb = m_Wins.first(); emb != 0L; emb = m_Wins.next()) {
    x = 2+i*24;

    emb->move(a*24, x);
    a++;

    if(a+1 > aa) {
      a = 0;
      i++;
    }

    count++;
    emb->tqrepaint();
  }
}

void Systemtray::systemTrayWindowAdded( WId w )
{
  //bool content = false;
  QXEmbed *emb;
  no_of_systray_windows++;
  emit updated();

  emb = new QXEmbed(this);

  emb->setAutoDelete(false);
  //emb->setBackgroundMode(X11ParentRelative);
  emb->setBackgroundMode(FixedPixmap);
  connect(emb, TQT_SIGNAL(embeddedWindowDestroyed()), TQT_SLOT(updateTrayWindows()));
  m_Wins.append(emb);

  emb->embed(w);
  emb->resize(24, 24);
  emb->show();

  layoutSystray();
}

void Systemtray::systemTrayWindowRemoved(WId)
{
  no_of_systray_windows--;
  emit updated();
  updateTrayWindows();
}

int Systemtray::getCurrentWindowCount()
{
  return no_of_systray_windows;
}

#include "systemtray.moc"
