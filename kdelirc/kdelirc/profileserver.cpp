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

#include "profileserver.h"

ProfileServer *ProfileServer::theInstance = 0;

ProfileServer::ProfileServer()
{
	theProfiles.setAutoDelete(true);
	loadProfiles();
}

ProfileServer::~ProfileServer()
{
}

void ProfileServer::loadProfiles()
{
	TQStringList theFiles = TDEGlobal::dirs()->findAllResources("data", "profiles/*.profile.xml");
	for(TQStringList::iterator i = theFiles.begin(); i != theFiles.end(); ++i)
	{	kdDebug() << "Found data file: " << *i << endl;
		Profile *p = new Profile();
		p->loadFromFile(*i);
		theProfiles.insert(p->id(), p);
	}
}

Profile::Profile()
{
	// set up defaults
	theUnique = true;
	theIfMulti = IM_DONTSEND;

	theActions.setAutoDelete(true);
}

const ProfileAction *Profile::searchClass(const TQString &c) const
{
	for(TQDictIterator<ProfileAction> i(theActions); i.current(); ++i)
		if(i.current()->getClass() == c) return i;
	return 0;
}

void Profile::loadFromFile(const TQString &fileName)
{
	charBuffer = "";
	curPA = 0;
	curPAA = 0;

	TQFile xmlFile(fileName);
	TQXmlInputSource source(TQT_TQIODEVICE(&xmlFile));
	TQXmlSimpleReader reader;
	reader.setContentHandler(this);
	reader.parse(source);
}

const ProfileAction *ProfileServer::getAction(const TQString &appId, const TQString &actionId) const
{
	if(theProfiles[appId])
		if(theProfiles[appId]->theActions[actionId])
			return theProfiles[appId]->theActions[actionId];
	return 0;
}

const TQString &ProfileServer::getServiceName(const TQString &appId) const
{
	if(theProfiles[appId])
		return theProfiles[appId]->serviceName();
	return TQString();
}

const ProfileAction *ProfileServer::getAction(const TQString &appId, const TQString &objId, const TQString &prototype) const
{
	return getAction(appId, objId + "::" + prototype);
}

bool Profile::characters(const TQString &data)
{
	charBuffer += data;
	return true;
}

bool Profile::startElement(const TQString &, const TQString &, const TQString &name, const TQXmlAttributes &attributes)
{
	if(name == "profile")
	{	theId = attributes.value("id");
		theServiceName = attributes.value("servicename");
	}
	else if(name == "action")
	{	curPA = new ProfileAction;
		curPA->setObjId(attributes.value("objid"));
		curPA->setPrototype(attributes.value("prototype"));
		curPA->setClass(attributes.value("class"));
		curPA->setMultiplier(attributes.value("multiplier").isEmpty() ? 1.0 : attributes.value("multiplier").toFloat());
		curPA->setRepeat(attributes.value("repeat") == "1");
		curPA->setAutoStart(attributes.value("autostart") == "1");
	}
	else if(name == "instances")
	{	theUnique = attributes.value("unique") == "1";
		theIfMulti = attributes.value("ifmulti") == "sendtotop" ? IM_SENDTOTOP : attributes.value("ifmulti") == "sendtobottom" ? IM_SENDTOBOTTOM : attributes.value("ifmulti") == "sendtoall" ? IM_SENDTOALL : IM_DONTSEND;
	}
	else if(name == "argument")
	{	curPA->theArguments.append(ProfileActionArgument());
		curPAA = &(curPA->theArguments.last());
		curPAA->setAction(curPA);
		curPAA->setType(attributes.value("type"));
	}
	else if(name == "range" && curPAA)
		curPAA->setRange(tqMakePair(attributes.value("min").toInt(), attributes.value("max").toInt()));

	charBuffer = "";
	return true;
}

bool Profile::endElement(const TQString &, const TQString &, const TQString &name)
{
	if(name == "name")
		if(curPA)
			curPA->setName(charBuffer);
		else
			theName = charBuffer;
	else if(name == "author")
		theAuthor = charBuffer;
	else if(name == "comment" && curPA && !curPAA)
		curPA->setComment(charBuffer);
	else if(name == "default" && curPA && curPAA)
		curPAA->setDefault(charBuffer);
	else if(name == "comment" && curPA && curPAA)
		curPAA->setComment(charBuffer);
	else if(name == "action")
	{
		curPA->setProfile(this);
		theActions.insert(curPA->objId() + "::" + curPA->prototype(), curPA);
		curPA = 0;
	}
	else if(name == "argument")
		curPAA = 0;

	charBuffer = "";
	return true;
}
