/*
 * Copyright (C) 2003-2004 Adam Geitgey <adam@rootnode.org>
 * Copyright (c) 2005 Ryan Nickell <p0z3r@earthlink.net>
 *
 * This file is part of SuperKaramba.
 *
 *  SuperKaramba is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  SuperKaramba is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SuperKaramba; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 ****************************************************************************/
#include <twinmodule.h>
#include <netwm.h>
#include <twin.h>

#include "karambaapp.h"
#include "showdesktop.h"
#include "showdesktop.moc"

ShowDesktop* ShowDesktop::the()
{
    static ShowDesktop showDesktop;
    return &showDesktop;
}

ShowDesktop::ShowDesktop()
  : TQObject()
  , showingDesktop( false )
  , kWinModule( 0 )
{
    kWinModule = new KWinModule( this );

    // on desktop changes or when a window is deiconified, we abort the show desktop mode
    connect( kWinModule, TQT_SIGNAL(currentDesktopChanged(int)),
             TQT_SLOT(slotCurrentDesktopChanged(int)));
    connect( kWinModule, TQT_SIGNAL(windowChanged(WId,unsigned int)),
             TQT_SLOT(slotWindowChanged(WId,unsigned int)));
}

void ShowDesktop::slotCurrentDesktopChanged(int)
{
    showDesktop( false );
}

#ifdef KDE_3_3
#define NET_ALL_TYPES_MASK (NET::AllTypesMask)
#else
#define NET_ALL_TYPES_MASK (-1LU)
#endif

void ShowDesktop::slotWindowChanged(WId w, unsigned int dirty)
{
  if (!showingDesktop)
    return;

  // SELI this needs checking for twin_iii (_NET_SHOWING_DESKTOP)
  if ( dirty & NET::XAWMState )
  {
    NETWinInfo inf(tqt_xdisplay(), w, tqt_xrootwin(),
                   NET::XAWMState | NET::WMWindowType);
#ifdef KDE_3_2
    NET::WindowType windowType = inf.windowType(NET_ALL_TYPES_MASK);
#else
    NET::WindowType windowType = inf.windowType();
#endif
    if ((windowType == NET::Normal || windowType == NET::Unknown)
        && inf.mappingState() == NET::Visible )
    {
      // a window was deiconified, abort the show desktop mode.
      iconifiedList.clear();
      showingDesktop = false;
      emit desktopShown( false );
    }
  }
}

void ShowDesktop::showDesktop( bool b )
{
    if( b == showingDesktop ) return;
    showingDesktop = b;

    if ( b ) {
        // this code should move to KWin after supporting NETWM1.2
        iconifiedList.clear();
        const TQValueList<WId> windows = kWinModule->windows();
        TQValueList<WId>::ConstIterator it;
        TQValueList<WId>::ConstIterator end( windows.end() );
        for ( it=windows.begin(); it!=end; ++it ) {
            WId w = *it;
            NETWinInfo info( tqt_xdisplay(), w, tqt_xrootwin(),
                             NET::XAWMState | NET::WMDesktop );
            if ( info.mappingState() == NET::Visible &&
                 ( info.desktop() == NETWinInfo::OnAllDesktops
                   || info.desktop() == (int) kWinModule->currentDesktop() )
                ) {
                iconifiedList.append( w );
            }
        }
        // find first, hide later, otherwise transients may get minimized
        // with the window they're transient for
        TQValueList<WId>::ConstIterator endInconifiedList( iconifiedList.end() );
        for ( it=iconifiedList.begin(); it!=endInconifiedList; ++it ) {
            KWin::iconifyWindow( *it, false );
        }
    } else {
        TQValueList<WId>::ConstIterator it;
        TQValueList<WId>::ConstIterator end( iconifiedList.end() );
        for ( it=iconifiedList.begin(); it!=end; ++it ) {
            KWin::deIconifyWindow( *it, false  );
        }
    }

    emit desktopShown( showingDesktop );
}
