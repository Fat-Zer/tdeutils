/* -*- C++ -*-

   This file implements the KVaio module for KMilo.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 1996-2003, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the TQt toolkit
         from Troll Tech, Norway. $

   $Id$

   * Portions of this code are
   * (C) 2001-2002 Stelian Pop <stelian@popies.net> and
   * (C) 2001-2002 Alcove <www.alcove.com>.
   * Thanks to Stelian for the implementation of the sonypi driver.
*/

#include <kgenericfactory.h>

#include "monitor.h"
#include "kmilo_kvaio.h"
#include "kvaio.h"
#include "kmilointerface.h"

KMiloKVaio::KMiloKVaio(TQObject *tqparent, const char *name,
		       const TQStringList& args)
    : Monitor(tqparent, name, args),
      m_kvaio(0)
{
    _poll = false;
    m_displayType = (Monitor::DisplayType)None ;
}


KMiloKVaio::~KMiloKVaio() {
}


bool KMiloKVaio::init()
{
    m_kvaio = new KVaio(this);

    return m_kvaio->driver()!=0;
}


KMilo::Monitor::DisplayType KMiloKVaio::poll() {
//    Monitor::DisplayType rc = (Monitor::DisplayType)None;
//    return rc;
    return m_displayType;
}


int KMiloKVaio::progress() const {
	return m_kvaio->progress();
}

void KMiloKVaio::reconfigure(KConfig* k)
{
    m_kvaio->loadConfiguration(k);
}

K_EXPORT_COMPONENT_FACTORY(kmilo_kvaio, KGenericFactory<KMiloKVaio>("kmilo_kvaio"))

