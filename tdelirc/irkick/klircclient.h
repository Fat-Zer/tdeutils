//
//
// C++ Interface: $MODULE$
//
// Description:
//
//
// Author: Gav Wood <gav@kde.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KLIRCCLIENT_H
#define KLIRCCLIENT_H

#include <tqobject.h>
#include <tqmap.h>
#include <tqstring.h>
#include <tqstringlist.h>

class TQSocket;
class TQSocketNotifier;

/**
@author Malte Starostik
@author Gav Wood
*/

class KLircClient: public TQObject
{
	Q_OBJECT
  

private:
	struct lirc_config *theConfig;
	TQSocket *theSocket;
	TQSocketNotifier *theNotifier;
	TQMap<TQString, TQStringList> theRemotes;
	bool listIsUpToDate;

	void updateRemotes();
	void sendCommand(const TQString &command);
	const TQString readLine();

private slots:
	void slotRead();
	void slotClosed();

signals:
	/**
	 * Emitted when the list of controls / buttons was completely read
	 */
	void remotesRead();

	/**
	 * Emitted when a IR command was received
	 *
	 * The arguments are the name of the remote control used,
	 * the name of the button pressed and the repeat counter.
	 *
	 * The signal is emitted repeatedly as long as the button
	 * on the remote control remains pressed.
	 * The repeat counter starts with 0 and increases
	 * every time this signal is emitted.
	 */
	void commandReceived(const TQString &remote, const TQString &button, int repeatCounter);

	/**
	 * Emitted when the Lirc connection is closed.
	 */
	void connectionClosed();

public:
	/**
	 * Query status of connection.
	 *
	 * @returns true if connected to lircd.
	 */
	bool isConnected() const;

	/**
	 * Query status of remote list.
	 *
	 * Make sure this is true before calling remotes()/buttons(...).
	 *
	 * @returns true if up to date.
	 */
	bool haveFullList() const;

	/**
	 * Retrieve list of remote controls.
	 *
	 * @returns said list.
	 */
	const TQStringList remotes() const;

	/**
	 * Retrieve list of buttons of a praticular remote control.
	 *
	 * @returns said list.
	 */
	const TQStringList buttons(const TQString &theRemote) const;

	/**
	 * Connects to lirc.
	 *
	 * @returns true if connection is made.
	 */
	bool connectToLirc();

	KLircClient(TQWidget *parent = 0, const char *name = 0);
	~KLircClient();
};

#endif
