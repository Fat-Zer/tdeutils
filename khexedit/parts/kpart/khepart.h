/***************************************************************************
                          khepart.h  -  description
                             -------------------
    begin                : Don Jun 19 2003
    copyright            : (C) 2003 by Friedrich W. H. Kossebau
    email                : Friedrich.W.H@Kossebau.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License version 2 as published by the Free Software Foundation.       *
 *                                                                         *
 ***************************************************************************/


#ifndef KHEPART_H
#define KHEPART_H

// kde specific
#include <tdeparts/part.h>
// app specific
#include "kbigbuffer.h"

// forward declarations
class TDERadioAction;
class TDEToggleAction;
class TDESelectAction;

namespace KHE
{

// forward declarations
class KHexEdit;

/**
 * This is a "Part".  It that does all the real work in a KPart
 * application.
 *
 * @short Main Part
 * @author Friedrich W. H. Kossebau <Friedrich.W.H@Kossebau.de>
 * @version 0.1.0
 */
class KHexEditPart : public KParts::ReadOnlyPart
{
    Q_OBJECT
  

    friend class KHexEditBrowserExtension;

  public:
    KHexEditPart( TQWidget *ParentWidget, const char *WidgetName, TQObject *Parent, const char *Name,
                  bool BrowserViewWanted );
    virtual ~KHexEditPart();


  protected: // KParts::ReadOnlyPart API
    virtual bool openFile();

  protected:
    void setupActions( bool BrowserViewWanted );
    void fitActionSettings();

  protected slots:
    // used to catch changes in the HexEdit widget
    void slotSelectionChanged();
  protected slots: // action slots
    void slotSelectAll();
    void slotUnselect();
    void slotSetCoding( int Coding );
    void slotSetEncoding( int Encoding );
    void slotSetShowUnprintable();
    void slotSetResizeStyle( int Style );
    void slotToggleOffsetColumn();
    void slotToggleValueCharColumns( int VisibleColunms );

  private:
    KHexEdit *HexEdit;
    KBigBuffer Wrapping;

    // edit menu
    TDEAction *CopyAction;
    // view menu
    TDESelectAction *CodingAction;
    TDESelectAction *EncodingAction;
    TDEToggleAction *ShowUnprintableAction;
    // settings menu
    TDESelectAction *ResizeStyleAction;
    TDEToggleAction *ShowOffsetColumnAction;
    TDESelectAction *ToggleColumnsAction;
};

}

#endif
