/* This file is part of the KDE project
 *
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */ 


#ifndef _KPCMCIA_H
#define _KPCMCIA_H

class QTimer;
class KPCMCIA;

#include <tqmemarray.h>

#include <sys/types.h>

#define CARD_STATUS_PRESENT    1
#define CARD_STATUS_READY      2
#define CARD_STATUS_BUSY       4
#define CARD_STATUS_SUSPEND    8
#define CARD_STATUS_SUSPENDED  8


class KPCMCIACard {
friend class KPCMCIA;
protected:
   KPCMCIACard();
   ~KPCMCIACard();

   int _fd;

   int _interrupt;
   TQString _device, _ports;
   TQString _module;
   TQString _cardname;
   TQString _type;
   int _vcc, _vpp, _vpp2;
   TQString _stabPath;
   int _iotype;
   int _inttype;
   int _cfgbase;

   int _status;
   int _num;

   // Other fields for the future
   //  etc look in linux/cs.h for more info

public:

   /**
    *    Base address for configuration
    */
   inline int configBase() { return _cfgbase; }

   /**
    *    Interrupt type
    */
   inline int intType() { return _inttype; }

   /**
    *    16/32 bit
    */
   inline int busWidth() { return _iotype; }

   /**
    *    True if the card is present
    */
   inline bool present() { return _status & CARD_STATUS_PRESENT; }

   /**
    *    Return the card status
    */
   inline int status() { return _status; }

   /**
    *    Request to eject the card
    */
   int eject();

   /**
    *    Request to resume from suspend
    */
   int resume();

   /**
    *    Request to suspend
    */
   int suspend();

   /**
    *    Request to reset the card
    */
   int reset();

   /**
    *    Insert a card   (mostly not needed?)
    */
   int insert();

   /**
    *    Return the card number
    */
   inline int num() { return _num; }

   /**
    *    Return the interrupt in use (or -1 if none)
    */
   inline int irq() { return _interrupt; }

   /**
    *    Return the VCC status
    */
   inline int vcc() { return _vcc; }

   /**
    *    Return the card programming power (1)
    */
   inline int vpp() { return _vpp; }

   /**
    *    Return the card programming power (2)
    */
   inline int vpp2() { return _vpp2; }

   /**
    *    Return the card name
    */
   inline TQString& name() { return _cardname; }

   /**
    *    Return the port range
    */
   inline TQString& ports() { return _ports; }

   /**
    *    Return the device name
    */
   inline TQString& device() { return _device; }

   /**
    *    Return the device type  (ie network, modem, etc)
    */
   inline TQString& type() { return _type; }

   /**
    *    Return the driver (module) name
    */
   inline TQString& driver() { return _module; }

   /**
    *    Refresh the card information - return < 0 on error.
    *    (this is called automatically [by KPCMCIA] on a timer normally)
    */
   int refresh();

private:
   time_t _last;
};




class KPCMCIA : public TQObject {
Q_OBJECT 
public:

  KPCMCIA(int maxSlots = 8, const char *stabPath = "/var/run/stab");
  ~KPCMCIA();

  void setRefreshSpeed(int msec);
  int getCardCount();
  KPCMCIACard *getCard(int num);

  bool haveCardServices();

signals:
  void cardUpdated(int num);


public slots:
  void updateCardInfo();

private:
  int _refreshSpeed;
  TQTimer *_timer;
  TQMemArray<KPCMCIACard *> *_cards;
  int _cardCnt;
  bool _haveCardServices;
  int _maxSlots;
  TQString _stabPath;
  
};




#endif

