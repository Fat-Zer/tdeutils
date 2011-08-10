/*
 *   khexedit - Versatile hex editor
 *   Copyright (C) 1999  Espen Sand, espensa@online.no
 *   This file is based on the work by Martynas Kunigelis (KProgress)
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

#ifndef _STATUSBAR_PROGRESS_H_
#define _STATUSBAR_PROGRESS_H_

#include <tqframe.h>
#include <tqrangecontrol.h>

class CStatusBarProgress : public TQFrame, public TQRangeControl 
{
  Q_OBJECT
  TQ_OBJECT

  public:
  /** 
  * Possible values for orientation 
  */
  enum Orientation { Horizontal, Vertical };

  /** 
  * Possible values for bar style. 
  *
  * Solid means one continuous progress bar, Blocked means a 
  * progress bar made up of several blocks. 
  */ 
  enum BarStyle { Solid, Blocked };

  /** 
  * Construct a default progress bar. Orientation is horizontal. 
  */
  CStatusBarProgress(TQWidget *parent=0, const char *name=0);

  /** 
  * Construct a KProgress bar with an orientation. 
  */
  CStatusBarProgress(Orientation, TQWidget *parent=0, const char *name=0);

  /** 
  * Construct a KProgress bar with minimum, maximum and initial value. 
  */
  CStatusBarProgress(int minValue, int maxValue, int value, Orientation, 
		     TQWidget *parent=0, const char *name=0);
	
  /** 
  * Destructor 
  */
  ~CStatusBarProgress( void );
	
  /** 
  * Set the progress bar style. Allowed values are Solid and Blocked. 
  */
  void setBarStyle(BarStyle style);  
	
  /** 
  * Set the color of the progress bar. 
  */
  void setBarColor(const TQColor &); 
  
  /** 
  * Set a pixmap to be shown in the progress bar. 
  */
  void setBarPixmap(const TQPixmap &);

  /** 
  * Set the orientation of the progress bar. 
  * Allowed values are Horizonzal andQt::Vertical. 
  */
  void setOrientation(Orientation);
	
  /** 
  * Retrieve the bar style. 
  */
  BarStyle barStyle() const;

  /** 
  * Retrieve the bar color. 
  */
  const TQColor &barColor() const;

  /** 
  * Retrieve the bar pixmap. 
  */
  const TQPixmap *barPixmap() const;

  /** 
  * Retrieve the orientation. 
  */
  Orientation orientation() const;

  /**
  * Returns TRUE if progress text will be displayed, FALSE otherwise.
  */
  bool textEnabled() const;

  /**
   * Returns the recommended width for vertical progress bars or
   * the recommended height for vertical progress bars
   */
  virtual TQSize tqsizeHint() const;
	
		
  public slots:
    void setValue( int );
    void setValue( int, int); 
    void advance( int );
    void setTextEnabled( bool state );
    void setText( const TQString &msg );
	
  signals:
    void percentageChanged(int);
    void pressed( void );
	
  protected:
    void valueChange();
    void rangeChange();
    void styleChange( GUIStyle );
    void paletteChange( const TQPalette & );
    void drawContents( TQPainter * );
    void mousePressEvent( TQMouseEvent *e );
	
  private:
    TQPixmap     *mBarPixmap;
    TQColor	mBarColor;
    TQColor	mBarTextColor;
    TQColor	mTextColor;
    TQRect       fr;
    BarStyle    mBarStyle;
    Orientation mOrientation;
    bool	mTextEnabled;
    TQString     mMsg;
    int         mCurPage;
    int         mMaxPage;

    void initialize( void );
    int recalcValue( int );
    void drawText( TQPainter * );
    void adjustStyle( void );
};


#endif
