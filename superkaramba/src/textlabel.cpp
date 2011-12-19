/***************************************************************************
 *   Copyright (C) 2003 by Hans Karlsson                                   *
 *   karlsson.h@home.se                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <krun.h>
#include <stdlib.h>
#include "textlabel.h"

TextLabel::TextLabel(karamba *k, int x,int y,int w,int h):
  Meter(k, x,y,w,h), tqalignment(TQt::AlignLeft), clip(0), bgColor(0, 0, 0),
  lineHeight(0), shadow(0), scrollSpeed(0, 0), scrollPos(0, 0), scrollGap(0),
  scrollPause(0), pauseCounter(0), scrollType(ScrollNone)
{
    calculateTextSize();
    if( h != 0 || w != 0)
        clip = 0;
    else
        clip = TQt::DontClip;

    if( h == 0 || w == 0)
    {
        setWidth(-1);
        setHeight(-1);
    }
}

TextLabel::TextLabel(karamba *k):
  Meter(k, 0, 0, 0, 0), tqalignment(TQt::AlignLeft), clip(0), bgColor(0, 0, 0),
  lineHeight(0), shadow(0), scrollSpeed(0, 0), scrollPos(0, 0), scrollGap(0),
  scrollPause(0), pauseCounter(0), scrollType(ScrollNone)
{
}

TextLabel::~TextLabel()
{
}

void TextLabel::show()
{
  Meter::show();
  setEnabled(true);
}

void TextLabel::hide()
{
  Meter::hide();
  setEnabled(false);
}

void TextLabel::setTextProps( TextField* t )
{
  if(t)
  {
    text = *t;
    //lineHeight = t->getLineHeight();
    shadow = t->getShadow();
    tqalignment = t->getAlignment();
    setFontSize(t->getFontSize());
    setFont(t->getFont());

    setColor(t->getColor());
    setBGColor(t->getBGColor());
  }
  calculateTextSize();
}

void TextLabel::calculateTextSize()
{
  int tmp;
  TQFontMetrics fm(font);
  lineHeight = fm.height();
  textSize.setWidth(0);
  textSize.setHeight(lineHeight * value.count());
  TQStringList::Iterator it = value.begin();
  while(it != value.end())
  {
    tmp = fm.width(*it);
    if(tmp > textSize.width())
      textSize.setWidth(tmp);
    ++it;
  }
}

void TextLabel::setValue( TQString text)
{
    value = TQStringList::split('\n',text);
    calculateTextSize();
}

void TextLabel::setValue( long v)
{
    value = TQStringList( TQString::number( v ) );
    calculateTextSize();
}

void TextLabel::setBGColor(TQColor clr)
{
    bgColor = clr;
}

TQColor TextLabel::getBGColor() const
{
    return bgColor;
}

void TextLabel::setFont(TQString f)
{
    font.setFamily(f);
    calculateTextSize();
}

TQString TextLabel::getFont() const
{
    return font.family();
}

void TextLabel::setFontSize(int size)
{
    font.setPixelSize(size);
    calculateTextSize();
}

int TextLabel::getFontSize() const
{
    return font.pixelSize();
}

void TextLabel::setAlignment( TQString align )
{
    TQString a = align.upper();
    if( a == "LEFT" || a.isEmpty() )
        tqalignment = TQt::AlignLeft;
    if( a == "RIGHT" )
        tqalignment = TQt::AlignRight;
    if( a == "CENTER" )
        tqalignment = TQt::AlignHCenter;
}

TQString TextLabel::getAlignment() const
{
    if( tqalignment == TQt::AlignHCenter )
        return "CENTER";
    else if( tqalignment == TQt::AlignRight )
        return "RIGHT";
    else
        return "LEFT";
}

void TextLabel::setFixedPitch( bool fp)
{
    font.setFixedPitch( fp );
}

bool TextLabel::getFixedPitch() const
{
    return font.fixedPitch();
}

void TextLabel::setShadow ( int s )
{
    shadow = s;
}

int TextLabel::getShadow() const
{
    return shadow;
}

void TextLabel::setScroll(char* type, TQPoint speed, int gap, int pause)
{
  ScrollType t = TextLabel::ScrollNone;
  TQString a = type;
  a = a.upper();
  if(a == "NONE")
    t = TextLabel::ScrollNone;
  else if( a == "NORMAL" )
    t = TextLabel::ScrollNormal;
  else if( a == "BACKANDFORTH" )
    t = TextLabel::ScrollBackAndForth;
  else if( a == "ONEPASS" )
    t = TextLabel::ScrollOnePass;
  setScroll(t, speed, gap, pause);
}

void TextLabel::setScroll(ScrollType type, TQPoint speed, int gap, int pause)
{
  scrollType = type;
  scrollSpeed = speed;
  switch(scrollType)
  {
    case ScrollNormal:
    case ScrollOnePass:
    {
      int x = 0, y = 0;

      if(speed.x() > 0)
        x = -1 * textSize.width();
      else if(speed.x() < 0)
        x = getWidth()-1;
      if(speed.y() > 0)
        x = -1 * textSize.height();
      else if(speed.y() < 0)
        x = getHeight()-1;
      scrollPos = TQPoint(x,y);
      break;
    }
    case ScrollNone:
    case ScrollBackAndForth:
    default:
      scrollPos = TQPoint(0,0);
      break;
  }
  scrollGap = gap;
  scrollPause = pause;
  pauseCounter = 1;
}

int TextLabel::drawText(TQPainter *p, int x, int y, int width, int height,
                        TQString text)
{
  if( shadow != 0)
  {
    p->setPen(getBGColor());
    p->drawText(x + shadow, y + shadow, width, height,
                tqalignment | clip | TQt::ExpandTabs, text);
  }
  p->setPen(getColor());
  p->drawText(x, y, width, height, tqalignment | clip | TQt::ExpandTabs, text);
  return 0;
}

bool TextLabel::calculateScrollCoords(TQRect meterRect, TQRect &textRect,
                                        TQPoint &next, int &x, int &y)
{
  if(scrollType == ScrollBackAndForth &&
     (scrollSpeed.x() != 0 && textSize.width() < getWidth() ||
      scrollSpeed.y() != 0 && textSize.height() < getHeight()))
    return true;

  x += scrollPos.x();
  y += scrollPos.y();

  if(pauseCounter < 1)
  {
    scrollPos += scrollSpeed;

    // -1 | 0 | 1
    TQPoint direction(scrollSpeed.x()/abs((scrollSpeed.x() == 0)?
                       1:scrollSpeed.x()),
                     scrollSpeed.y()/abs((scrollSpeed.y() == 0)?
                       1:scrollSpeed.y()));
    next = TQPoint(-1 * direction.x() * (scrollGap + textSize.width()),
                  -1 * direction.y() * (scrollGap + textSize.height()));
    textRect.setCoords(x, y, x + textSize.width(), y + textSize.height());

    if(scrollType == ScrollBackAndForth)
    {
      if(direction.x() < 0 && textRect.right() <= meterRect.right() ||
         direction.x() > 0 && textRect.left() >= meterRect.left())
      {
        scrollSpeed.setX(scrollSpeed.x() * -1);
        pauseCounter = scrollPause;
      }
      if(direction.y() < 0 && textRect.bottom() <= meterRect.bottom() ||
         direction.y() > 0 && textRect.top() >= meterRect.top())
      {
        scrollSpeed.setY(scrollSpeed.y() * -1);
        pauseCounter = scrollPause;
      }
    }
    else if(!textRect.intersects(meterRect))
    {
      if(scrollType == ScrollNormal)
        scrollPos += next;
      else if(scrollType == ScrollOnePass)
        return false;
    }
  }
  else
    --pauseCounter;
  return true;
}

void TextLabel::mUpdate(TQPainter *p)
{
  if (hidden != 1)
  {
    int i = 0; //lineHeight;
    int row = 1;
    int x = getX();
    int y = getY();
    int width = getWidth();
    int height = getHeight();
    TQRect meterRect(x, y, width, height);
    TQRect textRect;
    TQPoint next;

    p->setFont(font);
    if(scrollType != ScrollNone)
    {
      p->setClipRect(x, y, width, height);
      if(!calculateScrollCoords(meterRect, textRect, next, x, y))
      {
        p->setClipping(false);
        return;
      }
      width = textSize.width();
      height = textSize.height();
    }
    TQStringList::Iterator it = value.begin();
    while(it != value.end() && (row <= height || height == -1 ))
    {
      drawText(p, x, y + i, width, height, *it);

      // Draw more instances of text if scroll type is normal scroll
      if(scrollType == ScrollNormal)
      {
        textRect.addCoords(next.x(), next.y(), next.x(), next.y());
        while(textRect.intersects(meterRect))
        {
          drawText(p, textRect.x(), textRect.y() + i, width, height, *it);
          textRect.addCoords(next.x(), next.y(), next.x(), next.y());
        }
      }
      i += lineHeight;
      it++;
      row++;
    }
    if(scrollType != ScrollNone)
      p->setClipping(false);
  }
}

bool TextLabel::click(TQMouseEvent* e)
{
    if (getBoundingBox().contains(e -> x(), e -> y()) && isEnabled())
    {
        TQString program;
        if (e -> button() == Qt::LeftButton)
        {
            program = leftButtonAction;
        }
        else if (e -> button() == Qt::MidButton)
        {
            program = middleButtonAction;
        }
        else if (e -> button() == Qt::RightButton)
        {
            program = rightButtonAction;
        }

        if( !program.isEmpty() )
        {
            KRun::runCommand(program);
        }
        else
        {
          return true;
        }
    }
    return false;
}

void TextLabel::attachClickArea(TQString leftMouseButton,
                                TQString middleMouseButton,
                                TQString rightMouseButton)
{
    leftButtonAction = leftMouseButton;
    middleButtonAction = middleMouseButton;
    rightButtonAction = rightMouseButton;
}

#include "textlabel.moc"
