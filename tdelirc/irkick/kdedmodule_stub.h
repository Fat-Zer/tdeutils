/****************************************************************************
**
** DCOP Stub Definition created by dcopidl2cpp from kdedmodule.kidl
**
** WARNING! All changes made in this file will be lost!
**
*****************************************************************************/

/* This is a machine-generated file, and may be distributed under the GPL
   licence, as described in tdelibs/COPYING.
*/

#ifndef __KDEDMODULE_STUB__
#define __KDEDMODULE_STUB__

#include <dcopstub.h>
#include <tqobject.h>
#include <dcopobject.h>
#include <dcopref.h>
#include <ksharedptr.h>


class KDEDModule_stub : virtual public DCOPStub
{
public:
    KDEDModule_stub( const TQCString& app, const TQCString& id );
    KDEDModule_stub( DCOPClient* client, const TQCString& app, const TQCString& id );
    explicit KDEDModule_stub( const DCOPRef& ref );
protected:
    KDEDModule_stub() : DCOPStub( never_use ) {};
};


#endif
