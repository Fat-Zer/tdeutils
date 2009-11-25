/*
 *   khexedit - Versatile hex editor
 *   Copyright (C) 1999-2000 Espen Sand, espensa@online.no
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef _DRAG_SOURCE_H_
#define _DRAG_SOURCE_H_

#include <qdropsite.h>
#include <qlabel.h>
#include <qbitmap.h>
#include <qstring.h>
#include <qstrlist.h>

class KURLDrag;

class CDragLabel: public QLabel
{
  Q_OBJECT

  public:
    CDragLabel( QWidget *parent );
    ~CDragLabel( void );

    void setUrl( const QString &url );
    void setDragMask( const QPixmap pix );

  protected:
    void mousePressEvent( QMouseEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );

  private:
    void prepPixmap( KURLDrag &uriDrag );

  private:
    bool    mValid;
    QString mUrl;
    QBitmap mDragMask;
    bool    mDragPending;
    QPoint  mDragOrigin;

};

#endif
