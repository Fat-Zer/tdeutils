/*
 * disks.cpp
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

#include <tqfileinfo.h>
#include <tqdir.h>

#include <kglobal.h>
#include <kdebug.h>

#include "disks.h"
#include "disks.moc"

/****************************************************/
/********************* DiskEntry ********************/
/****************************************************/

/**
  * Constructor
**/
void DiskEntry::init()
{
  device="";
  type="";
  mountedOn="";
  options="";
  size=0;
  used=0;
  avail=0;
  isMounted=FALSE;
  mntcmd="";
  umntcmd="";
  iconSetByUser=FALSE;
  icoName="";


 // BackgroundProcesses ****************************************

 sysProc = new KShellProcess(); Q_CHECK_PTR(sysProc);
 connect( sysProc, TQT_SIGNAL(receivedStdout(KProcess *, char *, int) ),
        this, TQT_SLOT (receivedSysStdErrOut(KProcess *, char *, int)) );
 connect( sysProc, TQT_SIGNAL(receivedStderr(KProcess *, char *, int) ),
        this, TQT_SLOT (receivedSysStdErrOut(KProcess *, char *, int)) );
 readingSysStdErrOut=FALSE;


}

DiskEntry::DiskEntry(TQObject *tqparent, const char *name)
 : TQObject (tqparent, name)
{
  init();
}

DiskEntry::DiskEntry(const TQString & deviceName, TQObject *tqparent, const char *name)
 : TQObject (tqparent, name)
{
  init();

  setDeviceName(deviceName);
}
DiskEntry::~DiskEntry()
{
  disconnect(this);
  delete sysProc;
}

int DiskEntry::toggleMount()
{
  if (!mounted())
      return mount();
  else
      return umount();
}

int DiskEntry::mount()
{
  TQString cmdS=mntcmd;
  if (cmdS.isEmpty()) // generate default mount cmd
    if (getuid()!=0 ) // user mountable
      {
      cmdS="mount %d";
      }
	else  // root mounts with all params/options
      {
      // FreeBSD's mount(8) is picky: -o _must_ go before
      // the device and mountpoint.
      cmdS=TQString::tqfromLatin1("mount -t%t -o%o %d %m");
      }

  cmdS.tqreplace(TQString::tqfromLatin1("%d"),deviceName());
  cmdS.tqreplace(TQString::tqfromLatin1("%m"),mountPoint());
  cmdS.tqreplace(TQString::tqfromLatin1("%t"),fsType());
  cmdS.tqreplace(TQString::tqfromLatin1("%o"),mountOptions());

  kdDebug() << "mount-cmd: [" << cmdS << "]" << endl;
  int e=sysCall(cmdS);
  if (!e) setMounted(TRUE);
  kdDebug() << "mount-cmd: e=" << e << endl;
  return e;
}

int DiskEntry::umount()
{
  kdDebug() << "umounting" << endl;
  TQString cmdS=umntcmd;
  if (cmdS.isEmpty()) // generate default umount cmd
      cmdS="umount %d";

  cmdS.tqreplace(TQString::tqfromLatin1("%d"),deviceName());
  cmdS.tqreplace(TQString::tqfromLatin1("%m"),mountPoint());

  kdDebug() << "umount-cmd: [" << cmdS << "]" << endl;
  int e=sysCall(cmdS);
  if (!e) setMounted(FALSE);
  kdDebug() << "umount-cmd: e=" << e << endl;

  return e;
}

int DiskEntry::remount()
{
  if (mntcmd.isEmpty() && umntcmd.isEmpty() // default mount/umount commands
      && (getuid()==0)) // you are root
    {
    TQString oldOpt=options;
    if (options.isEmpty())
       options="remount";
    else
       options+=",remount";
    int e=mount();
    options=oldOpt;
    return e;
   } else {
    if (int e=umount())
      return mount();
   else return e;
  }
}

void DiskEntry::setMountCommand(const TQString & mnt)
{
  mntcmd=mnt;
}

void DiskEntry::setUmountCommand(const TQString & umnt)
{
  umntcmd=umnt;
}

void DiskEntry::setIconName(const TQString & iconName)
{
  iconSetByUser=TRUE;
  icoName=iconName;
  if (icoName.right(6) == "_mount")
     icoName.truncate(icoName.length()-6);
  else if (icoName.right(8) == "_unmount")
     icoName.truncate(icoName.length()-8);

  emit iconNameChanged();
}

TQString DiskEntry::iconName()
{
  TQString iconName=icoName;
  if (iconSetByUser) {
    mounted() ? iconName+="_mount" : iconName+="_unmount";
   return iconName;
  } else
   return guessIconName();
}

TQString DiskEntry::guessIconName()
{
  TQString iconName;
    // try to be intelligent
    if (-1!=mountPoint().tqfind("cdrom",0,FALSE)) iconName+="cdrom";
    else if (-1!=deviceName().tqfind("cdrom",0,FALSE)) iconName+="cdrom";
    else if (-1!=mountPoint().tqfind("writer",0,FALSE)) iconName+="cdwriter";
    else if (-1!=deviceName().tqfind("writer",0,FALSE)) iconName+="cdwriter";
    else if (-1!=mountPoint().tqfind("mo",0,FALSE)) iconName+="mo";
    else if (-1!=deviceName().tqfind("mo",0,FALSE)) iconName+="mo";
    else if (-1!=deviceName().tqfind("fd",0,FALSE)) {
            if (-1!=deviceName().tqfind("360",0,FALSE)) iconName+="5floppy";
            if (-1!=deviceName().tqfind("1200",0,FALSE)) iconName+="5floppy";
            else iconName+="3floppy";
	 }
    else if (-1!=mountPoint().tqfind("floppy",0,FALSE)) iconName+="3floppy";
    else if (-1!=mountPoint().tqfind("zip",0,FALSE)) iconName+="zip";
    else if (-1!=fsType().tqfind("nfs",0,FALSE)) iconName+="nfs";
    else iconName+="hdd";
    mounted() ? iconName+="_mount" : iconName+="_unmount";
//    if ( -1==mountOptions().tqfind("user",0,FALSE) )
//      iconName.prepend("root_"); // special root icon, normal user can�t mount

    //debug("device %s is %s",deviceName().latin1(),iconName.latin1());

    //emit iconNameChanged();
  return iconName;
}


/***************************************************************************
  * starts a command on the underlying system via /bin/sh
**/
int DiskEntry::sysCall(const TQString & command)
{
  if (readingSysStdErrOut || sysProc->isRunning() )  return -1;

  sysStringErrOut=i18n("Called: %1\n\n").tqarg(command); // put the called command on ErrOut
  sysProc->clearArguments();
  (*sysProc) << command;
    if (!sysProc->start( KProcess::Block, KProcess::AllOutput ))
     kdFatal() << i18n("could not execute %1").tqarg(command.local8Bit().data()) << endl;

  if (sysProc->exitStatus()!=0) emit sysCallError(this, sysProc->exitStatus());

  return (sysProc->exitStatus());
}


/***************************************************************************
  * is called, when the Sys-command writes on StdOut or StdErr
**/
void DiskEntry::receivedSysStdErrOut(KProcess *, char *data, int len)
{
  TQString tmp = TQString::fromLocal8Bit(data, len);
  sysStringErrOut.append(tmp);
}

float DiskEntry::percentFull() const
{
   if (size != 0) {
      return 100 - ( ((float)avail / (float)size) * 100 );
   } else {
      return -1;
   }
}

void DiskEntry::setDeviceName(const TQString & deviceName)
{
 device=deviceName;
 emit deviceNameChanged();
}

TQString DiskEntry::deviceRealName() const
{
 TQFileInfo inf( device );
 TQDir dir( inf.dirPath( true ) );
 TQString relPath = inf.fileName();
 if ( inf.isSymLink() ) {
  TQString link = inf.readLink();
  if ( link.startsWith( "/" ) )
    return link;
  relPath = link;
 }
 return dir.canonicalPath() + "/" + relPath;
}

void DiskEntry::setMountPoint(const TQString & mountPoint)
{
  mountedOn=mountPoint;
 emit mountPointChanged();
}

TQString DiskEntry::realMountPoint() const
{
 TQDir dir( mountedOn );
 return dir.canonicalPath();
}

void DiskEntry::setMountOptions(const TQString & mountOptions)
{
 options=mountOptions;
 emit mountOptionsChanged();
}

void DiskEntry::setFsType(const TQString & fsType)
{
  type=fsType;
  emit fsTypeChanged();
}

void DiskEntry::setMounted(bool nowMounted)
{
  isMounted=nowMounted;
  emit mountedChanged();
}

void DiskEntry::setKBSize(int kb_size)
{
  size=kb_size;
  emit kBSizeChanged();
}

void DiskEntry::setKBUsed(int kb_used)
{
  used=kb_used;
  if ( size < (used+avail) ) {  //adjust kBAvail
     kdWarning() << "device " << device << ": kBAvail(" << avail << ")+*kBUsed(" << used << ") exceeds kBSize(" << size << ")" << endl;
     setKBAvail(size-used);
  }
  emit kBUsedChanged();
}

void DiskEntry::setKBAvail(int kb_avail)
{
  avail=kb_avail;
  if ( size < (used+avail) ) {  //adjust kBUsed
     kdWarning() << "device " << device << ": *kBAvail(" << avail << ")+kBUsed(" << used << ") exceeds kBSize(" << size << ")" << endl;
     setKBUsed(size-avail);
  }
  emit kBAvailChanged();
}


