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

/*
 *   Much of the linux code was migrated from:
 *   kardinfo   Copyright 1999, Mirko Sucker <mirko.sucker@unibw-hamburg.de>
 */

#include <qtimer.h>
#include <qfile.h>
#include <qregexp.h>

#include <klocale.h>
#include <kinstance.h>
#include <kstandarddirs.h>


#include "kpcmcia.h"


#ifdef __linux__
   extern "C" {
      #include <sys/types.h>
      #include <unistd.h>
      #include <fcntl.h>
      #include <sys/stat.h>
      #include <sys/ioctl.h>
      #include <sys/file.h>
      #include <sys/time.h>
      #include "linux/version.h"
      #include "linux/cs_types.h"
      #include "linux/cs.h"
      #include "linux/cistpl.h"
      #include "linux/ds.h"
   }

// Taken from cardinfo.c
typedef struct event_tag_t {
   event_t event;
   const char *name;
} event_tag_t;


static event_tag_t event_tag[] = {
   { CS_EVENT_CARD_INSERTION, "card insertion" },
   { CS_EVENT_CARD_REMOVAL, "card removal" },
   { CS_EVENT_RESET_PHYSICAL, "prepare for reset" },
   { CS_EVENT_CARD_RESET, "card reset successful" },
   { CS_EVENT_RESET_COMPLETE, "reset request complete" },
   { CS_EVENT_EJECTION_REQUEST, "user eject request" },
   { CS_EVENT_INSERTION_REQUEST, "user insert request" },
   { CS_EVENT_PM_SUSPEND, "suspend card" },
   { CS_EVENT_PM_RESUME, "resume card" },
   { CS_EVENT_REQUEST_ATTENTION, "request attention" }
};

#define NTAGS (sizeof(event_tag)/sizeof(event_tag_t))

static int lookupDevice(const char *x);
static int openDevice(dev_t dev);

#else
#include <unistd.h>
#endif



KPCMCIACard::KPCMCIACard() {
  _fd = -1;
  _num = 9999999;
  _status = 0;
  _last = 0;
        _interrupt = -1;
        _ports = "";
        _device = "";
        _module = "";
        _type = "";
        _iotype = 0;
        _cardname = i18n("Empty slot.");
        _vcc = _vpp = _vpp2 = 0;
        _inttype = 0;
        _cfgbase = 0;
}


KPCMCIACard::~KPCMCIACard() {
  if (_fd != -1) close(_fd);
}



// RETURN: <0      => error
//         =0      => no error, no update
//         >0      => no error, update happened
//
int KPCMCIACard::refresh() {
//////////////////////////////////////////////
///////////// LINUX ONLY
///////////////////////////////////////////////
#ifdef __linux__
struct timeval tv;
cs_status_t status;
fd_set rfds;
int rc;
event_t event;
struct stat sb;
config_info_t cfg;
KPCMCIACard oldValues(*this);
int updated = 0;
oldValues._fd = -1;

#define CHECK_CHANGED(x, y) do { if (x.y != y) updated = 1; } while(0)

    /**
     *   Get any events on the pcmcia device
     */
tv.tv_sec = 0;  tv.tv_usec = 0;
     FD_ZERO(&rfds);
     FD_SET(_fd, &rfds);
     rc = select(_fd+1, &rfds, NULL, NULL, &tv);
     if (rc > 0) {
        rc = read(_fd, (void *)&event, 4);
        if (rc == 4) {
	   int thisEvent = -1;  // thisEvent is the index of event in event_tag
           for (unsigned int j = 0; j < NTAGS; j++) {
              if (event_tag[j].event == event) {
                 thisEvent = j;
                 break;
              }
              if (thisEvent < 0) return -1;
           }
        } else {
           return -1;
        }
     } else return updated;

     if (event == CS_EVENT_EJECTION_REQUEST) {
        _interrupt = -1;
        _ports = "";
        _device = "";
        _module = "";
        _type = "";
        _iotype = 0;
        _inttype = 0;
        _cfgbase = 0;
        _cardname = i18n("Empty slot.");
        _vcc = _vpp = _vpp2 = 0;
        return updated;
     }

     /**
      *   Read in the stab file.
      */
     if (!stat(_stabPath.latin1(), &sb) && sb.st_mtime >= _last) {
        QFile f(_stabPath.latin1());

        if (f.open(IO_ReadOnly)) {
          QTextStream ts(&f);
          bool foundit = false;
          QString _thisreg = "^Socket %1: ";
          QRegExp thisreg ( _thisreg.arg(_num) );

          if (flock(f.handle(), LOCK_SH)) return updated;

          _last = sb.st_mtime;

          // find the card
          while(!foundit) {
            QString s;
            if (ts.eof()) break;
            s = ts.readLine();
            if (s.contains(thisreg)) {
               _cardname = s.right(s.length() - s.find(':') - 1);
               _cardname = _cardname.stripWhiteSpace();
               foundit = true;
               CHECK_CHANGED(oldValues, _cardname);
            }
          }

          // read it in
          if (foundit && !ts.eof()) {  // FIXME: ts.eof() is a bad error!!
            QString s = ts.readLine();
            int end;
            s.simplifyWhiteSpace();

            end = s.find(QRegExp("[ \r\t\n]"));
            s = s.remove(0, end+1);

            end = s.find(QRegExp("[ \r\t\n]"));
            _type = s;
            _type.truncate(end);
            s = s.remove(0, end+1);

            end = s.find(QRegExp("[ \r\t\n]"));
            _module = s;
            _module.truncate(end);
            s = s.remove(0, end+1);

            end = s.find(QRegExp("[ \r\t\n]"));
            s = s.remove(0, end+1);

            end = s.find(QRegExp("[ \r\t\n]"));
            _device = s;
            _device.truncate(end);
            s = s.remove(0, end+1);
            CHECK_CHANGED(oldValues, _type);
            CHECK_CHANGED(oldValues, _module);
            CHECK_CHANGED(oldValues, _device);
          }

          flock(f.handle(), LOCK_UN);
          f.close();
        } else return -1;
     } else return updated;


     /**
      *   Get the card's status and configuration information
      */
     status.Function = 0;
     ioctl(_fd, DS_GET_STATUS, &status);
     memset(&cfg, 0, sizeof(cfg));
     ioctl(_fd, DS_GET_CONFIGURATION_INFO, &cfg);
     // status is looked up in the table at the top
     if (cfg.Attributes & CONF_VALID_CLIENT) {
        if (cfg.AssignedIRQ == 0)
          _interrupt = -1;
        else _interrupt = cfg.AssignedIRQ;

        if (cfg.NumPorts1 > 0) {
           int stop = cfg.BasePort1 + cfg.NumPorts1;
           if (cfg.NumPorts2 > 0) {
              if (stop == cfg.BasePort2) {
                 _ports.sprintf("%#x-%#x", cfg.BasePort1, stop+cfg.NumPorts2-1);
              } else {
                 _ports.sprintf("%#x-%#x, %#x-%#x", cfg.BasePort1, stop-1,
                               cfg.BasePort2, cfg.BasePort2+cfg.NumPorts2-1);
              }
           } else {
              _ports.sprintf("%#x-%#x", cfg.BasePort1, stop-1);
           }
        }
        CHECK_CHANGED(oldValues, _ports);
        CHECK_CHANGED(oldValues, _interrupt);
     }

     _vcc = cfg.Vcc;
     _vpp = cfg.Vpp1;
     _vpp2 = cfg.Vpp2;
     CHECK_CHANGED(oldValues, _vcc);
     CHECK_CHANGED(oldValues, _vpp);
     CHECK_CHANGED(oldValues, _vpp2);
     _inttype = cfg.IntType;
     CHECK_CHANGED(oldValues, _inttype);
     _iotype = cfg.IOAddrLines;
     CHECK_CHANGED(oldValues, _iotype);
     _cfgbase = cfg.ConfigBase;
     CHECK_CHANGED(oldValues, _cfgbase);

     if (status.CardState & CS_EVENT_CARD_DETECT)
        _status |= CARD_STATUS_PRESENT;
     if (status.CardState & CS_EVENT_CARD_REMOVAL)
        _status &= ~CARD_STATUS_PRESENT;
     if (event & CS_EVENT_CARD_REMOVAL)
        _status &= ~CARD_STATUS_PRESENT;

     if (!(status.CardState & CS_EVENT_PM_SUSPEND)) {
        if (status.CardState & CS_EVENT_READY_CHANGE) {
           _status |= CARD_STATUS_READY;
           _status &= ~(CARD_STATUS_BUSY|CARD_STATUS_SUSPEND);
        } else {
           _status |= CARD_STATUS_BUSY;
           _status &= ~(CARD_STATUS_READY|CARD_STATUS_SUSPEND);
        }
     } else if (status.CardState & CS_EVENT_PM_SUSPEND) {
        _status |= CARD_STATUS_SUSPEND;
        _status &= ~(CARD_STATUS_READY|CARD_STATUS_BUSY);
     }

     CHECK_CHANGED(oldValues, _status);

return updated;
#else
return 0;
#endif
}


int KPCMCIACard::insert() {
#ifdef __linux__
  ioctl(_fd, DS_INSERT_CARD);
  return 0;
#else
  return -1;
#endif
}



int KPCMCIACard::eject() {
#ifdef __linux__
  ioctl(_fd, DS_EJECT_CARD);
  return 0;
#else
  return -1;
#endif
}



int KPCMCIACard::reset() {
#ifdef __linux__
  ioctl(_fd, DS_RESET_CARD);
  return 0;
#else
  return -1;
#endif
}



int KPCMCIACard::suspend() {
#ifdef __linux__
  ioctl(_fd, DS_SUSPEND_CARD);
  return 0;
#else
  return -1;
#endif
}



int KPCMCIACard::resume() {
#ifdef __linux__
  ioctl(_fd, DS_RESUME_CARD);
  return 0;
#else
  return -1;
#endif
}





KPCMCIA::KPCMCIA(int maxSlots, const char *stabpath) : _maxSlots(maxSlots),
                                                       _stabPath(stabpath)  {

_refreshSpeed = 750;

_haveCardServices = false;
_timer = new QTimer(this);
connect(_timer, SIGNAL(timeout()), this, SLOT(updateCardInfo()));
_cards = new QMemArray<KPCMCIACard *>(_maxSlots+1);
_cardCnt = 0;


///////////////////////////////////////////////////
//  LINUX code
///////////////////////////////////////////////////
#ifdef __linux__
servinfo_t serv;

   int device = lookupDevice("pcmcia");

   if (device >= 0) {
      for (int z = 0; z < _maxSlots; z++) {
         int fd = openDevice((device << 8) + z);
         if (fd < 0) break;
         (*_cards)[_cardCnt] = new KPCMCIACard;
         (*_cards)[_cardCnt]->_stabPath = _stabPath;
         (*_cards)[_cardCnt]->_fd = fd;
         (*_cards)[_cardCnt]->_num = _cardCnt;
         //(*_cards)[_cardCnt]->refresh();
         _cardCnt++;
         //kdDebug() << "Found a pcmcia slot" << endl;
      }

      if (_cardCnt > 0) {
         if (ioctl((*_cards)[0]->_fd, DS_GET_CARD_SERVICES_INFO, &serv) == 0) {
            // FIXME: what to do here?
         }
         _haveCardServices = true;
      }
   }



_timer->start(_refreshSpeed, true);

///////////////////////////////////////////////////
//  No supported platform.
///////////////////////////////////////////////////
#else

#endif
}








KPCMCIA::~KPCMCIA() {
///////////////////////////////////////////////////
//  LINUX code
///////////////////////////////////////////////////
#ifdef __linux__


///////////////////////////////////////////////////
//  No supported platform.
///////////////////////////////////////////////////
#else

#endif


delete _timer;
delete _cards;
}







KPCMCIACard* KPCMCIA::getCard(int num) {
  if (num >= _cardCnt || num < 0) return NULL;
  return (*_cards)[num];
}




void KPCMCIA::updateCardInfo() {
 for (int i = 0; i < _cardCnt; i++) {
   int rc = (*_cards)[i]->refresh();
  /*
   kdDebug() << "CARD UPDATED: " << i  << endl
             << " Name: " << (*_cards)[i]->_cardname << endl
             << " Device: " << (*_cards)[i]->_device << endl
             << " VCC: " << (*_cards)[i]->_vcc       << endl
             << " VPP: " << (*_cards)[i]->_vpp       << endl
             << " IRQ: " << (*_cards)[i]->_interrupt << endl
             << " Ports: " << (*_cards)[i]->_ports   << endl
             << " Type: " << (*_cards)[i]->_type     << endl
             << " Module: " << (*_cards)[i]->_module << endl
             << endl;
   */
   if (rc > 0) emit cardUpdated(i);
 }
_timer->start(_refreshSpeed, true);
}



void KPCMCIA::setRefreshSpeed(int msec) {
  _refreshSpeed = msec;
}


int KPCMCIA::getCardCount() {
   return _cardCnt;
}


bool KPCMCIA::haveCardServices() {
  return _haveCardServices;
}


#include "kpcmcia.moc"



#ifdef __linux__
static int lookupDevice(const char *x) {
QFile df("/proc/devices");
QString thisreg;

   thisreg = "^[0-9]+ %1$";
   thisreg = thisreg.arg(x);

   if (df.open(IO_ReadOnly)) {
      QTextStream t(&df);
      QString s;
      while (!t.eof()) {
         s = t.readLine();

         if (s.contains(QRegExp(thisreg))) {
            int n = (s.left(3).stripWhiteSpace()).toInt();
            df.close();
            return n;
         }
      }
      df.close();
   }
return -1;
}

static int openDevice(dev_t dev) {
QString tmp_path = locateLocal("tmp", KGlobal::instance()->instanceName());
QString ext = "_socket%1";

  tmp_path += ext.arg((int)dev);

  int rc = mknod(tmp_path.latin1(), (S_IFCHR | S_IREAD), dev);
  if (rc < 0) return -1;

  int fd = open(tmp_path.latin1(), O_RDONLY);
  if (fd < 0) {
     unlink(tmp_path.latin1());
     return -1;
  }

  if (unlink(tmp_path.latin1()) < 0) {
     close(fd);
     return -1;
  }

  return fd;
}

#endif


