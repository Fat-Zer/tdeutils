/*
 * disks.h
 *
 * Copyright (c) 1998 Michael Kropfberger <michael.kropfberger@gmx.net>
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


#ifndef __DISKS_H__
#define __DISKS_H__

#include <tqobject.h>
#include <tqstring.h>
#include <tqlabel.h>
#include <tqpushbutton.h>
#include <tqprogressbar.h>
#include <tqfile.h>

#include <kio/global.h>
#include <kprogress.h>
#include <kprocess.h>
#include <klocale.h>

class DiskEntry : public TQObject
{
  Q_OBJECT
  TQ_OBJECT
public:
  DiskEntry(TQObject *parent=0, const char *name=0);
  DiskEntry(const TQString & deviceName, TQObject *parent=0, const char *name=0);
  ~DiskEntry();
  TQString lastSysError() {return sysStringErrOut; }
  TQString deviceName() const { return device; }
  // The real device (in case deviceName() is a symlink)
  TQString deviceRealName() const;
  TQString mountPoint() const { return mountedOn; }
  TQString mountOptions() const { return options; }
  // The real device (in case deviceName() is a symlink)
  TQString realMountPoint() const;
  /**
   * sets the used mountCommand for the actual DiskEntry.
   * @param mntcmd   is a string containing the executable file and
   *                 special codes which will be filled in when used: <BR>
   *                 %m : mountpoint <BR>
   *                 %d : deviceName <BR>
   *                 %t : filesystem type <BR>
   *                 %o : mount options <BR>
   *                 all this information is gained from the objects' data
   *                 if no mountCommand is set it defaults to "mount %d"
   **/
  TQString mountCommand() const { return mntcmd; }
  /**
   * sets the used umountCommand for the actual DiskEntry.
   * @param mntcmd   is a string containing the executable file and
   *                 special codes which will be filled in when used: <BR>
   *                 %m : mountpoint <BR>
   *                 %d : deviceName <BR>
   *                 all this information is gained from the objects' data
   *                 if no umountCommand is set it defaults to "umount %d"
   **/
  TQString umountCommand() const { return umntcmd; }
  TQString fsType() const { return type; }
  bool mounted() const { return isMounted; }
  int kBSize() const { return size; }
  TQString iconName();
  TQString realIconName() { return icoName; }
  TQString prettyKBSize() const { return KIO::convertSizeFromKB(size); }
  int kBUsed() const { return used; }
  TQString prettyKBUsed() const { return KIO::convertSizeFromKB(used); }
  int kBAvail() const  { return avail; }
  TQString prettyKBAvail() const { return KIO::convertSizeFromKB(avail); }
  float percentFull() const;

signals:
  void sysCallError(DiskEntry *disk, int err_no);
  void deviceNameChanged();
  void mountPointChanged();
  void mountOptionsChanged();
  void fsTypeChanged();
  void mountedChanged();
  void kBSizeChanged();
  void kBUsedChanged();
  void kBAvailChanged();
  void iconNameChanged();

public slots:

  int toggleMount();
  int mount();
  int umount();
  int remount();
  void setMountCommand(const TQString & mnt);
  void setUmountCommand(const TQString & umnt);
  void setDeviceName(const TQString & deviceName);
  void setMountPoint(const TQString & mountPoint);
  void setIconName(const TQString & iconName);
  void setMountOptions(const TQString & mountOptions);
  void setFsType(const TQString & fsType);
  void setMounted(bool nowMounted);
  void setKBSize(int kb_size);
  void setKBUsed(int kb_used);
  void setKBAvail(int kb_avail);
  TQString guessIconName();

private slots:
   void receivedSysStdErrOut(KProcess *, char *data, int len);

private:
  void init();
  int sysCall(const TQString & command);
  TQString prettyPrint(int kBValue) const;

  KShellProcess     *sysProc;
  TQString           sysStringErrOut;
  bool              readingSysStdErrOut;

  TQString     device,
              type,
              mountedOn,
              options,
              icoName,
              mntcmd,
              umntcmd;

  int         size,
              used,
              avail;       // ATTENTION: used+avail != size (clustersize!)

  bool        isMounted,
              iconSetByUser;
};

#endif
