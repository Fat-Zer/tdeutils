/***************************************************************************
                          kbuffersection.h  -  description
                             -------------------
    begin                : 22.06.2003
    copyright            : (C) 2003 by Friedrich W. H. Kossebau
    email                : Friedrich.W.H@Kossebau.de
****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License version 2 as published by the Free Software Foundation.       *
 *                                                                         *
 ***************************************************************************/


#ifndef KHE_KSELECTION_H
#define KHE_KSELECTION_H

#include "ksection.h"

namespace KHE
{

/** This class describes a selected section of the buffer.
  * As it is used as selection controlled by
  * mouse and keyboard commands it offers two ways to set its range:
  * - by giving the startposition (of the cursor) of an interactive selection
  *   and the subsequent end positions (until selection is finished)
  * - direct setting (as provided by KSection)
  *
  * the interactive selection takes care that
  *
  *@author Friedrich W. H.  Kossebau
  */
class TDESelection : public KSection
{
  public:
    /** creates a selection with a given start.
      * @param Index index in front of which the selection begins
      */
    TDESelection( int Index );
    /** creates an invalid selection */
    TDESelection();
    ~TDESelection();

  public:
    TDESelection &operator=( const TDESelection &S );
    TDESelection &operator=( const KSection &S );

  public: // modification access
    /** starts the selection. 
      * For this the anchor, start and end are set to the given index,
      * so the initial selection is empty.
      * @param Index index in front of which the selection begins
      */
    void setStart( int Index );
    /** sets the end of the current selection 
      * If the end is before the start the selection will reach from the given index 
      * @param Index index in front of which the selection ends
      */
    void setEnd( int Index );
    /** sets the selection to be invalid
      */
    void cancel();
    /** sets the anchor to the start 
      * If the selection has not started the behaviour is undefined.
      */
    void setForward();
    /** sets the anchor to the end
      * If the selection has not started the behaviour is undefined.
      */
    void setBackward();
    /** swaps anchor from start to end or vice versa 
      * If the selection has not started the behaviour is undefined.
      */
    void reverse();

  public: // value access
    /** 
      * @return anchor value
      */
    int anchor() const;

  public: // logic access
    /** 
      * @return @c true if the anchor has been set, otherwise @c false.
      */
    bool started() const;
    /** 
      * @return @c true if the anchor has been set and the selection is empty, otherwise @c false.
      */
    bool justStarted() const;
    /** 
      * @return @c true if the anchor is at the begin of the selection 
      */
    bool isForward() const;

  protected:
    /** cursor index where the selection starts */
    int Anchor;
};


inline TDESelection::TDESelection() : Anchor( -1 ) {}
inline TDESelection::TDESelection( int Index ) : Anchor( Index )  {}
inline TDESelection::~TDESelection() {}

inline TDESelection &TDESelection::operator=( const TDESelection &S )
{ 
  KSection::operator=(S); 
  Anchor = S.Anchor; 
  return *this; 
}

inline TDESelection &TDESelection::operator=( const KSection &S )
{ 
  KSection::operator=(S); 
  Anchor = start(); 
  return *this; 
}


inline void TDESelection::setStart( int Index )
{
  Anchor = Index;
  unset();
}


inline void TDESelection::setEnd( int Index )
{
  // nothing selected?
  if( Index == Anchor )
    unset();
  // selecting forwards?
  else if( Index > Anchor )
  {
    KSection::setStart( Anchor );
    KSection::setEnd( Index-1 );
  }
  // selecting backwards
  else
  {
    KSection::setStart( Index );
    KSection::setEnd( Anchor-1 );
  }
}

inline void TDESelection::reverse() 
{
   Anchor = isForward() ? end()+1 : start();
}

inline void TDESelection::setForward() 
{
   Anchor = start();
}

inline void TDESelection::setBackward() 
{
   Anchor = end()+1;
}

inline int TDESelection::anchor() const { return Anchor; }

inline void TDESelection::cancel() { Anchor = -1; unset(); }

inline bool TDESelection::started() const { return Anchor != -1; }

inline bool TDESelection::justStarted() const { return Anchor != -1 && start() == -1; }

inline bool TDESelection::isForward() const { return Anchor == start(); }

}

#endif
