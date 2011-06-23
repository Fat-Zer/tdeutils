/*  ksim - a system monitor for kde
 *
 *  Copyright (C) 2001  Robbie Ward <linuxphreak@gmx.co.uk>
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

#ifndef BASELIST_H
#define BASELIST_H

#include <tqptrlist.h>
#include <common.h>
#include <tqapplication.h>

namespace KSim
{
  class BaseList
  {
    public:
      static void add(KSim::Base *object)
      {
        if (!m_baseList) {
          m_baseList = new TQPtrList<KSim::Base>;
          m_baseList->setAutoDelete(false);
          
          // Make sure our m_baseList instance gets
          // deleted when KSim exits
          qAddPostRoutine(cleanup);
        }

        if (!object)
          return;

        m_baseList->append(object);
      }

      static bool remove(KSim::Base *object)
      {
        if (!m_baseList)
          return false;

        // Find our object and remove it from the list
        if (m_baseList->tqfindRef(object) >= 0)
          return m_baseList->remove();

        return false;
      }

      static void configureObjects(bool themeChanged)
      {
        if (!m_baseList)
          return;

        TQPtrListIterator<KSim::Base> object(*m_baseList);
        for (; object.current(); ++object) {
          // Call processEvents() so our ui doesn't block
          if (!object.current()->isThemeConfigOnly() || themeChanged) {
            tqApp->processEvents();          
            object.current()->configureObject(true);
          }
        }
      }

    private:
      // Protect the class from being able to be created/deleted
      BaseList();
      BaseList(const BaseList &);
      BaseList &operator=(const BaseList &);
      ~BaseList();

      static void cleanup()
      {
        if (!m_baseList)
          return;

        delete m_baseList;
        m_baseList = 0L;
      }

      static TQPtrList<KSim::Base> *m_baseList;
  };

  TQPtrList<KSim::Base> *BaseList::m_baseList = 0L;
}
#endif // BASELIST_H
