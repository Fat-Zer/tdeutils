/*
    
    This file is part of the KFloppy program, part of the KDE project
  
    Copyright (C) 2002 by Adriaan de Groot
  
  
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <sys/types.h>
#include <signal.h>

#include <stdlib.h>
#include <ctype.h>

#include "debug.h"
#include "zip.moc"

#include <tqcheckbox.h>
#include <tqlayout.h>
#include <tqtimer.h>
#include <tqwhatsthis.h>

#include <klocale.h>
#include <kprocess.h>
#include <kconfig.h>

ZipFormat::ZipFormat(TQWidget *w,const char *n) :
	DiskFormat(w,n),
	zeroWholeDisk(0L),
	p(0L),
	formatStep(0),
	statusTimer(0L)
{
	DEBUGSETUP;

	TQGridLayout *grid = new TQGridLayout(this,1,1,10);

	zeroWholeDisk = new TQCheckBox(i18n("Zero entire disk"),this);
	TQWhatsThis::add(zeroWholeDisk,
		i18n("Try to write zeroes to the entire disk "
			"before adding a filesystem, in order "
			"to check the disk's integrity."));
	grid->addWidget(zeroWholeDisk,0,0);
	enableSoftUpdates = new TQCheckBox(i18n("Enable softupdates"),this);
	grid->addWidget(enableSoftUpdates,1,0);

	// Remember the stretch at the bottom to clear
	// up layour problems when this widget is smaller
	// than others in the stack.
	//
	grid->addRowSpacing(2,10);
	grid->setRowStretch(2,100);

	endInit();
}

const char fslabel[] = I18N_NOOP("UFS Zip100");

FilesystemList ZipFormat::FSLabels() const
{
	FilesystemList l;
	// THis isn't a basic format anywhere, I don't think.
	l.append(new FilesystemData(i18n(fslabel),0xefc87329,(DiskFormat *)this));
	return l;
}

/* static */ bool ZipFormat::runtimeCheck()
{
	DEBUGSETUP;
	dd = findExecutable("dd");
	newfs = findExecutable("newfs");
	return !newfs.isEmpty() && !dd.isEmpty();
}

/* static */ TQString ZipFormat::dd;
/* static */ TQString ZipFormat::newfs;

/* virtual slot */ void ZipFormat::setEnabled(bool b)
{
	zeroWholeDisk->setEnabled(b);
	enableSoftUpdates->setEnabled(b);
}

/* virtual */  void ZipFormat::readSettings(KConfig *c)
{
	c->setGroup(fslabel);
	zeroWholeDisk->setChecked(
		c->readBoolEntry("ZeroDisk",false));
	enableSoftUpdates->setChecked(
		c->readBoolEntry("SoftUpdates",false));
}

/* virtual */  void ZipFormat::writeSettings(KConfig *c)
{
	c->setGroup(fslabel);
	c->writeEntry("ZeroDisk",zeroWholeDisk->isChecked());
	c->writeEntry("SoftUpdates",enableSoftUpdates->isChecked());
}

void ZipFormat::quit()
{
	DEBUGSETUP;
	if (p) delete p;
	if (statusTimer) delete statusTimer;

	p=0L;
	statusTimer=0L;
}

/* virtual slot */ void ZipFormat::format(FilesystemData *f)
{
	DEBUGSETUP;

	if (f->magic()!=0xefc87329)
	{
		complainAboutFormat(f);
		return;
	}

	formatStep=0;

	if (p) delete p;
	p = new KProcess();

	if (statusTimer) delete statusTimer;
	statusTimer = new TQTimer(this);

	connect(p,TQT_SIGNAL(processExited(KProcess *)),
		this,TQT_SLOT(transition()));
	connect(p,TQT_SIGNAL(receivedStdout(KProcess *,char *,int)),
		this,TQT_SLOT(processResult(KProcess *,char *,int)));
	connect(p,TQT_SIGNAL(receivedStderr(KProcess *,char *,int)),
		this,TQT_SLOT(processResult(KProcess *,char *,int)));
	connect(statusTimer,TQT_SIGNAL(timeout()),
		this,TQT_SLOT(statusRequest()));

	transition();
}

void ZipFormat::transition()
{
	DEBUGSETUP;

	switch(formatStep)
	{
	case 0 :
		// We're using a larger block size to speed up writing.
		// So instead of 196608 blocks of 512b, use 12288 blocks
		// of 8k instead.
		//
		// For newfs we need to set the real number of blocks.
		//
		if (zeroWholeDisk->isChecked())
		{
			// Zeroing whole disk takes about 2 min.
			// No point in making a dizzy display of it.
			statusTimer->start(10000);
			TQTimer::singleShot(1000,this,
				TQT_SLOT(statusRequest()));
			totalBlocks=12288; // 196608 * 512b = 12288 * 8192b ;
		}
		else
		{
			// Takes about 5 seconds.
			statusTimer->start(1000);
			totalBlocks=100;
		}

		*p << dd
			<< "if=/dev/zero"
			<< "of=/dev/afd0c"
			<< "bs=8192" ;
		*p << TQString("count=%1").tqarg(totalBlocks);
		if (!p->start(KProcess::NotifyOnExit,KProcess::AllOutput))
		{
			emit statusMessage(i18n("Cannot start dd to zero disk."));
			emit formatDone(-1);
			delete statusTimer;
			delete p;
			statusTimer=0L;
			p=0L;
			return;
		}

		formatStep=1;
		emit statusMessage(i18n("Zeroing disk..."));
		break;
	case 1 :
		statusTimer->stop();
		// The dd for zeroing the disk has finished.
		if (p->exitStatus())
		{
			emit statusMessage(i18n("Zeroing disk failed."));
			emit formatDone(-1);
			return;
		}

		totalBlocks=196608; // Now use the real number of 512b blocks

		p->clearArguments();
		*p << newfs << "-T" << "zip100" ;
		if (enableSoftUpdates->isChecked())
		{
			*p << "-U" ;
		}
		*p << "/dev/afd0c" ;
		if (!p->start(KProcess::NotifyOnExit,KProcess::AllOutput))
		{
			emit statusMessage(i18n("Cannot start newfs."));
			emit formatDone(-1);
		};
		formatStep=2;
		emit statusMessage(i18n("Making filesystem..."));
		break;
	case 2 :
		if (p->exitStatus())
		{
			emit statusMessage(i18n("newfs failed."));
			emit formatDone(-1);
		}
		else
		{
			emit statusMessage(i18n("Disk formatted successfully."));
			emit formatDone(0);
		}
		break;
	}
}

void ZipFormat::processResult(KProcess *, char *b, int l)
{
	DEBUGSETUP;

#ifdef DEBUG
	TQString o = TQString::fromLatin1(b,l);
	DEBUGS(TQString("  %1").tqarg(o).latin1());
#endif

	switch(formatStep)
	{
	case 1 : // These are messages from dd
		if (strchr(b,'+'))
		{
			int currentblock=atoi(b);
			emit setProgress(currentblock*100/totalBlocks);
			if (totalBlocks>10000)
			{
				emit statusMessage(i18n("Zeroing block %1 of %2...")
					.tqarg(currentblock)
					.tqarg(totalBlocks));
			}
		}
		break;
	case 2 : // These are messages from newfs
		if ((b[0]==' ') && isdigit(b[1]))
		{
			int currentblock=atoi(b+1);
			emit setProgress(currentblock*100/totalBlocks);
		}
		else
		{
			// This is the initial display message from
			// newfs. It writes a first block to sector 32.
			//
			//
			emit setProgress(1);

			// TQString myBuf = TQString::fromLatin1(b, l);
			// DEBUGS(myBuf.latin1());
		}
		break;
	}
}

void ZipFormat::statusRequest()
{
	if (formatStep!=1) // Not in dd mode?
		return;
	if (!p) // How can that happen?
		return;

#ifdef ANY_BSD
	p->kill(SIGINFO);
#endif
}


