/*
 * pcmcia.h
 *
 * Copyright (c) 1999 Paul Campbell <paul@taniwha.com>
 *
 * Requires the TQt widget libraries, available at no cost at
 * http://www.troll.no/
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


#ifndef __PCMCIACONFIG_H__
#define __PCMCIACONFIG_H__

#include <tqdialog.h>
#include <tqpushbutton.h>
#include <tqlabel.h>
#include <tqlcdnumber.h>
#include <tqradiobutton.h>
#include <tqbuttongroup.h>

#include <kapplication.h>
#include <knuminput.h>

#include <kcmodule.h>
#include <kaboutdata.h>

class PcmciaConfig : public KCModule
{
  Q_OBJECT
  
public:
  PcmciaConfig( TQWidget *parent=0, const char* name=0);

  void save( void );
  void load();
  void defaults();

  virtual TQString quickHelp() const;

private slots:

  void changed();

    
private:
  void GetSettings( void );

  TQLabel *label0;
  TQLabel *label1;
  TQLabel *label0_text;
  TQLabel *label1_text;
                               

};

#endif

