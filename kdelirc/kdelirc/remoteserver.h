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
#ifndef REMOTESERVER_H
#define REMOTESERVER_H

#include <tqstring.h>
#include <tqxml.h>
#include <tqdict.h>

/**
@author Gav Wood
*/


class RemoteButton
{
	TQString theName, theId, theClass, theParameter;

	friend class Remote;
public:
	void setName(const TQString &a) { theName = a; }
	const TQString &name(void) const { return theName; }
	void setClass(const TQString &a) { theClass = a; }
	const TQString &getClass(void) const { return theClass; }
	void setParameter(const TQString &a) { theParameter = a; }
	const TQString &parameter(void) const { return theParameter; }
	void setId(const TQString &a) { theId = a; }
	const TQString &id(void) const { return theId; }
};

class Remote : public TQXmlDefaultHandler
{
	TQString theName, theId, theAuthor;
	TQDict<RemoteButton> theButtons;

	TQString charBuffer;
	RemoteButton *curRB;

	friend class RemoteServer;
public:
	bool characters(const TQString &data);
	bool startElement(const TQString &, const TQString &, const TQString &name, const TQXmlAttributes &attributes);
	bool endElement(const TQString &, const TQString &, const TQString &name);

	void setName(const TQString &a) { theName = a; }
	const TQString &name(void) const { return theName; }
	void setId(const TQString &a) { theId = a; }
	const TQString &id(void) const { return theId; }
	void setAuthor(const TQString &a) { theAuthor = a; }
	const TQString &author(void) const { return theAuthor; }
	const TQDict<RemoteButton> &buttons() const { return theButtons; }

	void loadFromFile(const TQString &fileName);

	const TQString &getButtonName(const TQString &id) const { if(theButtons[id]) return theButtons[id]->name(); return id; }

	Remote();
	~Remote();
};

class RemoteServer
{
	static RemoteServer *theInstance;
	void loadRemotes();
	TQDict<Remote> theRemotes;

public:
	static RemoteServer *remoteServer() { if(!theInstance) theInstance = new RemoteServer(); return theInstance; }

	const TQDict<Remote> &remotes() const { return theRemotes; }

	const TQString &getRemoteName(const TQString &id) const { if(theRemotes[id]) return theRemotes[id]->name(); return id; }
	const TQString &getButtonName(const TQString &remote, const TQString &button) const { if(theRemotes[remote]) return theRemotes[remote]->getButtonName(button); return button; }

	RemoteServer();
	~RemoteServer();
};

#endif
