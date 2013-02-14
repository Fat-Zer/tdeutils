//
//
// C++ Implementation: $MODULE$
//
// Description:
//
//
// Author: (C) 2002 by Malte Starostik
// Adaption : Gav Wood <gav@kde.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <sys/un.h>
#include <sys/socket.h>
#include <errno.h>

#include <tqwidget.h>
#include <tqdialog.h>
#include <tqtooltip.h>
#include <tqsocket.h>
#include <tqsocketnotifier.h>
#include <tqfile.h>

#include <tdeapplication.h>
#include <ksystemtray.h>
#include <kiconloader.h>
#include <kpassivepopup.h>
#include <kmessagebox.h>
#include <tdepopupmenu.h>
#include <kdebug.h>
#include <klocale.h>
#include <tdeaboutdialog.h>
#include <tdeabouttde.h>

#include <dcopclient.h>
#include <dcopref.h>

#include "klircclient.h"


KLircClient::KLircClient(TQWidget *parent, const char *name) : TQObject(parent, name), theSocket(0), listIsUpToDate(false)
{
	connectToLirc();
}

bool KLircClient::connectToLirc()
{
	int sock = ::socket(PF_UNIX, SOCK_STREAM, 0);
	if(sock == -1) return false;

	sockaddr_un addr;
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, "/dev/lircd");
	if(::connect(sock, (struct sockaddr *)(&addr), sizeof(addr)) == -1)
	{	::close(sock);
		// in case of mandrake...
		strcpy(addr.sun_path, "/tmp/.lircd");
		if(::connect(sock, (struct sockaddr *)(&addr), sizeof(addr)) == -1)
		{	::close(sock);
			return false;
		}
	}

	theSocket = new TQSocket;
	theSocket->setSocket(sock);
	connect(theSocket, TQT_SIGNAL(readyRead()), TQT_SLOT(slotRead()));
	connect(theSocket, TQT_SIGNAL(connectionClosed()), TQT_SLOT(slotClosed()));
	updateRemotes();
	return true;
}

KLircClient::~KLircClient()
{
//	if(theSocket)
		delete theSocket;
}

void KLircClient::slotClosed()
{
	delete theSocket;
	theSocket = 0;
	emit connectionClosed();
}

const TQStringList KLircClient::remotes() const
{
	TQStringList remotes;
	for(TQMap<TQString, TQStringList>::ConstIterator i = theRemotes.begin(); i != theRemotes.end(); ++i)
		remotes.append(i.key());
	remotes.sort();
	return remotes;
}

const TQStringList KLircClient::buttons(const TQString &theRemote) const
{
	return theRemotes[theRemote];
}

void KLircClient::slotRead()
{
	while (theSocket->bytesAvailable())
	{
		TQString line = readLine();
		if (line == "BEGIN")
		{
			// BEGIN
			// <command>
			// [SUCCESS|ERROR]
			// [DATA
			// n
			// n lines of data]
			// END
			line = readLine();
			if (line == "SIGHUP")
			{
				// Configuration changed
				do line = readLine();
				while (!line.isEmpty() && line != "END");
				updateRemotes();
				return;
			}
			else if (line == "LIST")
			{
				// remote control list
				if (readLine() != "SUCCESS" || readLine() != "DATA")
				{
					do line = readLine();
					while (!line.isEmpty() && line != "END");
					return;
				}
				TQStringList remotes;
				int count = readLine().toInt();
				for (int i = 0; i < count; ++i)
					remotes.append(readLine());
				do line = readLine();
				while (!line.isEmpty() && line != "END");
				if (line.isEmpty())
					return; // abort on corrupt data
				for (TQStringList::ConstIterator it = remotes.begin(); it != remotes.end(); ++it)
					sendCommand("LIST " + *it);
				return;
			}
			else if (line.left(4) == "LIST")
			{
				// button list
				if (readLine() != "SUCCESS" || readLine() != "DATA")
				{
					do line = readLine();
					while (!line.isEmpty() && line != "END");
					return;
				}
				TQString remote = line.mid(5);
				TQStringList buttons;
				int count = readLine().toInt();
				for (int i = 0; i < count; ++i)
				{
					// <code> <name>
					TQString btn = readLine().mid(17);
					if(btn.isNull()) break;
					if(btn.startsWith("'") && btn.endsWith("'"))
						btn = btn.mid(1, btn.length() - 2);
					buttons.append(btn);
				}
				theRemotes.insert(remote, buttons);
			}
			do line = readLine();
			while (!line.isEmpty() && line != "END");
			listIsUpToDate = true;
			emit remotesRead();
		}
		else
		{
			// <code> <repeat> <button name> <remote control name>
			line.remove(0, 17); // strip code
			int pos = line.find(' ');
			if (pos < 0) return;
			bool ok;
			int repeat = line.left(pos).toInt(&ok, 16);
			if (!ok) return;
			line.remove(0, pos + 1);

			pos = line.find(' ');
			if (pos < 0) return;
			TQString btn = line.left(pos);
			if(btn.startsWith("'") && btn.endsWith("'"))
				btn = btn.mid(1, btn.length() - 2);
			line.remove(0, pos + 1);

			emit commandReceived(line, btn, repeat);
		}
	}
}

void KLircClient::updateRemotes()
{
	listIsUpToDate = false;
	theRemotes.clear();
	sendCommand("LIST");
}

bool KLircClient::isConnected() const
{
	if(!theSocket) return false;
	return theSocket->state() == TQSocket::Connected;
}

bool KLircClient::haveFullList() const
{
	return listIsUpToDate;
}

const TQString KLircClient::readLine()
{
	if (!theSocket->canReadLine())
	{	bool timeout;
		// FIXME: possible race condition -
		// more might have arrived between canReadLine and waitForMore
		theSocket->waitForMore(500, &timeout);
		if (timeout)
		{	// something's wrong. there ain't no line comin!
			return TQString();
		}
	}
	TQString line = theSocket->readLine();
	line.truncate(line.length() - 1);
	return line;
}

void KLircClient::sendCommand(const TQString &command)
{
	TQString cmd = command + "\n";
	theSocket->writeBlock(TQFile::encodeName( cmd ), cmd.length());
}


#include "klircclient.moc"
