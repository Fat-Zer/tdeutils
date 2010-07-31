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
#ifndef PROFILESERVER_H
#define PROFILESERVER_H

#include <tqpair.h>
#include <tqstring.h>
#include <tqvaluelist.h>
#include <tqmap.h>
#include <tqxml.h>
#include <tqdict.h>

/**
@author Gav Wood
*/

enum IfMulti {IM_DONTSEND, IM_SENDTOALL, IM_SENDTOTOP, IM_SENDTOBOTTOM};

typedef QPair<int,int> Range;

class ProfileAction;
class Profile;

class ProfileActionArgument
{
	TQString theComment, theType;
	Range theRange;
	TQString theDefault;		// should be QVariant?
	const ProfileAction *parent;

	friend class Profile;
public:
	const TQString &comment() const { return theComment; }
	void setComment(const TQString &a) { theComment = a; }
	const TQString &type() const { return theType; }
	void setType(const TQString &a) { theType = a; }
	const TQString &getDefault() const { return theDefault; }
	void setDefault(const TQString &a) { theDefault = a; }
	const Range &range() const { return theRange; }
	void setRange(const Range &a) { theRange = a; }

	const ProfileAction *action() const { return parent; }
	void setAction(const ProfileAction *a) { parent = a; }
};

class ProfileAction
{
	TQString theObjId, thePrototype, theName, theComment, theClass;
	float theMultiplier;
	const Profile *parent;
	bool theRepeat, theAutoStart;
	TQValueList<ProfileActionArgument> theArguments;

	friend class Profile;
public:
	const TQString &objId() const { return theObjId; }
	void setObjId(const TQString &a) { theObjId = a; }
	const TQString &prototype() const { return thePrototype; }
	void setPrototype(const TQString &a) { thePrototype = a; }
	const TQString &name() const { return theName; }
	void setName(const TQString &a) { theName = a; }
	const TQString &comment() const { return theComment; }
	void setComment(const TQString &a) { theComment = a; }
	const TQString &getClass() const { return theClass; }
	void setClass(const TQString &a) { theClass = a; }
	const float multiplier() const { return theMultiplier; }
	void setMultiplier(const float a) { theMultiplier = a; }
	bool repeat() const { return theRepeat; }
	void setRepeat(bool a) { theRepeat = a; }
	bool autoStart() const { return theAutoStart; }
	void setAutoStart(bool a) { theAutoStart = a; }
	const TQValueList<ProfileActionArgument> &arguments() const { return theArguments; }

	const Profile *profile() const { return parent; }
	void setProfile(const Profile *a) { parent = a; }
};

class Profile : public QXmlDefaultHandler
{
	TQString theId, theName, theAuthor, theServiceName;
	IfMulti theIfMulti;
	bool theUnique;
	TQString charBuffer;

	ProfileAction *curPA;
	ProfileActionArgument *curPAA;
	TQDict<ProfileAction> theActions;		// objid+"::"+prototype => ProfileAction

	friend class ProfileServer;
public:
	bool characters(const TQString &data);
	bool startElement(const TQString &, const TQString &, const TQString &name, const TQXmlAttributes &attributes);
	bool endElement(const TQString &, const TQString &, const TQString &name);

	const TQString &id() const { return theId; }
	void setId(const TQString &a) { theId = a; }
	const TQString &name() const { return theName; }
	void setName(const TQString &a) { theName = a; }
	const TQString &author() const { return theAuthor; }
	void setAuthor(const TQString &a) { theAuthor = a; }
	const bool unique() const { return theUnique; }
	void setUnique(const bool a) { theUnique = a; }
	const IfMulti ifMulti() const { return theIfMulti; }
	void setIfMulti(const IfMulti a) { theIfMulti = a; }
	const TQString &serviceName() const { if(theServiceName != TQString::null) return theServiceName; return theName; }
	void setServiceName(const TQString &a) { theServiceName = a; }
	const TQDict<ProfileAction> &actions() const { return theActions; }
	const ProfileAction *searchClass(const TQString &c) const;

	void loadFromFile(const TQString &fileName);

	Profile();
};

class ProfileServer
{
	static ProfileServer *theInstance;
	void loadProfiles();
	TQDict<Profile> theProfiles;			// id => Profile

public:
	static ProfileServer *profileServer() { if(!theInstance) theInstance = new ProfileServer(); return theInstance; }
	const TQDict<Profile> profiles() const { return theProfiles; }
	const ProfileAction *getAction(const TQString &appId, const TQString &objId, const TQString &prototype) const;
	const ProfileAction *getAction(const TQString &appId, const TQString &actionId) const;
	const TQString &getServiceName(const TQString &appId) const;

	ProfileServer();
	~ProfileServer();
};

#endif
