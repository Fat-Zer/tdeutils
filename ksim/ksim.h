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

#ifndef KSIMEXTENSION_H
#define KSIMEXTENSION_H

#include <kpanelextension.h>

class TQBoxLayout;
class TDEAboutData;
class TDEInstance;
class DCOPClient;

namespace KSim
{
  class MainView;

  class PanelExtension : public KPanelExtension
  {
    Q_OBJECT
  
    public:
      PanelExtension( const TQString & configFile, Type type,
         int actions, TQWidget * parent, const char * name);

      ~PanelExtension();

      TQSize sizeHint( Position, TQSize maxSize ) const;
      void resizeEvent(TQResizeEvent *);
      Position preferedPosition() const;

      void reparse();

    public slots:
      void show();

    protected:
      void about();
      void help();
      void preferences();
      void reportBug();
      void positionChange( Position );

    private:
      KSim::MainView * m_view;
      TQBoxLayout * m_layout;
      TDEAboutData * m_aboutData;
      DCOPClient * m_dcopClient;
  };
}
#endif
