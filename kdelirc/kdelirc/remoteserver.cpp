//
//
// C++ Implementation: $MODULE$
//
// Description:
//
//
// Author: Gav Wood <gav@kde.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <tqfile.h>
#include <tqxml.h>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include "remoteserver.h"

RemoteServer *RemoteServer::theInstance = 0;

RemoteServer::RemoteServer()
{
	theRemotes.setAutoDelete(true);
	loadRemotes();
}

RemoteServer::~RemoteServer()
{
}

void RemoteServer::loadRemotes()
{
	TQStringList theFiles = TDEGlobal::dirs()->findAllResources("data", "remotes/*.remote.xml");
	for(TQStringList::iterator i = theFiles.begin(); i != theFiles.end(); ++i)
	{	kdDebug() << "Found data file: " << *i << endl;
		Remote *p = new Remote();
		p->loadFromFile(*i);
		theRemotes.insert(p->id(), p);
	}
}

Remote::Remote()
{
	theButtons.setAutoDelete(true);
}

Remote::~Remote()
{
}

void Remote::loadFromFile(const TQString &fileName)
{
	charBuffer = "";
	curRB = 0;

	TQFile xmlFile(fileName);
	TQXmlInputSource source(TQT_TQIODEVICE(&xmlFile));
	TQXmlSimpleReader reader;
	reader.setContentHandler(this);
	reader.parse(source);
}

bool Remote::characters(const TQString &data)
{
	charBuffer += data;
	return true;
}

bool Remote::startElement(const TQString &, const TQString &, const TQString &name, const TQXmlAttributes &attributes)
{
	if(name == "remote")
		theId = theName = attributes.value("id");
	else if(name == "button")
	{
		curRB = new RemoteButton();
		curRB->setId(attributes.value("id"));
		curRB->setClass(attributes.value("id"));
		if(attributes.index("class") > -1)
			curRB->setClass(attributes.value("class"));
		curRB->setParameter(attributes.value("parameter"));
		curRB->setName(attributes.value("id"));
	}

	charBuffer = "";
	return true;
}

bool Remote::endElement(const TQString &, const TQString &, const TQString &name)
{
	if(name == "name")
		if(curRB)
			curRB->setName(charBuffer);
		else
			theName = charBuffer;
	else if(name == "author")
		theAuthor = charBuffer;
	else if(name == "button")
	{
		theButtons.insert(curRB->id(), curRB);
		curRB = 0;
	}

	charBuffer = "";
	return true;
}
