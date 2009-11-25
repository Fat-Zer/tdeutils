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

class QBoxLayout;
class KAboutData;
class KInstance;
class DCOPClient;

namespace KSim
{
  class MainView;

  class PanelExtension : public KPanelExtension
  {
    Q_OBJECT
    public:
      PanelExtension( const QString & configFile, Type type,
         int actions, QWidget * parent, const char * name);

      ~PanelExtension();

      QSize sizeHint( Position, QSize maxSize ) const;
      void resizeEvent(QResizeEvent *);
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
      QBoxLayout * m_layout;
      KAboutData * m_aboutData;
      DCOPClient * m_dcopClient;
  };
}
#endif
