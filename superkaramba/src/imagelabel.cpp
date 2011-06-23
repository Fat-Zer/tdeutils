/****************************************************************************
*  imagelabel.cpp  -  ImageLabel meter
*
*  Copyright (C) 2003 Hans Karlsson <karlsson.h@home.se>
*  Copyright (c) 2004 Petri Damstén <damu@iki.fi>
*
*  This file is part of SuperKaramba.
*
*  SuperKaramba is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  SuperKaramba is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with SuperKaramba; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
****************************************************************************/

#include <tqpixmap.h>
#include <tqtimer.h>
#include <tqtooltip.h>
#include <kpixmapeffect.h>
#include <kdebug.h>
#include <kimageeffect.h>
#include <ktempfile.h>
#include <kio/job.h>
#include "karambaapp.h"
#include "imagelabel.h"

// Effect
Effect::Effect(ImageLabel* img, int msec) :
  myImage(img)
{
  if (msec > 0)
  {
    // remove the effect after the given time
    //TQTimer::singleShot (millisec, myImage, TQT_SLOT(slotEffectExpired()));
    //timer -> changeInterval(millisec);
    millisec = msec;
  }
  else
  {
    millisec = msec;
  }
}

Effect::~Effect()
{
}

void Effect::startTimer()
{
  if (millisec > 0)
  {
    TQTimer::singleShot (millisec, myImage, TQT_SLOT(slotEffectExpired()));
    millisec = 0;
  }
}

// Intensity
Intensity::Intensity(ImageLabel* img, float r, int millisec) :
  Effect(img, millisec)
{
  ratio = r;
  ratio = (ratio > 1)  ? 1 : ratio;
  ratio = (ratio < -1) ? -1 : ratio;
}

KPixmap Intensity::apply(KPixmap pixmap)
{
  return KPixmapEffect::intensity(pixmap, ratio);
}

// ChannelIntensity
ChannelIntensity::ChannelIntensity(ImageLabel* img, float r, TQString c,
                                   int millisec) :
  Effect(img, millisec)
{
  ratio = r;
  ratio = (ratio > 1)  ? 1 : ratio;
  ratio = (ratio < -1) ? -1 : ratio;

  channel = 0;
  if (c.tqfind("red", 0 , false))
  {
    channel = 0;
  }
  else if (c.tqfind("green", 0, false))
  {
    channel = 1;
  }
  else if (c.tqfind("blue", 0, false))
  {
    channel = 2;
  }
}

KPixmap ChannelIntensity::apply(KPixmap pixmap)
{
  return KPixmapEffect::channelIntensity(pixmap, ratio,
    (KPixmapEffect::RGBComponent)channel);
}

// ToGray
ToGray::ToGray(ImageLabel* img, int millisec) : Effect(img, millisec)
{
}

KPixmap ToGray::apply(KPixmap pixmap)
{
  return KPixmapEffect::toGray(pixmap);
}

/***********************************************************************/

ImageLabel::ImageLabel(karamba* k, int ix,int iy,int iw,int ih) :
  Meter(k, ix,iy,iw,ih), zoomed(false), rollover(false)
{
   background = 0;
  cblend = 0;
  //scaleMat.reset();
  //rotMat.reset();
  scale_w = 1;
  scale_h = 1;
  rot_angle = 0;

  doScale = false;
  doRotate = false;

  imageEffect = 0;
}

ImageLabel::ImageLabel(karamba* k) :
  Meter(k), zoomed(false), rollover(false)
{
  cblend = 0;
  background = 0;
}

ImageLabel::~ImageLabel()
{
  if (imageEffect != 0)
  {
    delete imageEffect;
    imageEffect = 0;
  }
  if(!old_tip_rect.isNull())
  {
    TQToolTip::remove(m_karamba, old_tip_rect);
  }
}

void ImageLabel::setValue(long v)
{
 setValue( TQString::number( v ) );
}

void ImageLabel::show()
{
  Meter::show();
  setEnabled(true);
}

void ImageLabel::hide()
{
  Meter::hide();
  setEnabled(false);
}

void ImageLabel::rotate(int deg)
{
  doRotate = !(deg == 0);

  rot_angle = deg;

  applyTransformations();
}

void ImageLabel::scale(int w, int h)
{
  doScale = !(w == realpixmap.width() && h == realpixmap.height());

  scale_w = w;
  scale_h = h;

  applyTransformations();
}

void ImageLabel::smoothScale(int w, int h)
{
    doScale = !(w == realpixmap.width() && h == realpixmap.height());

    scale_w = w;
    scale_h = h;

    applyTransformations(true);

//  double widthFactor = ((double)w) / ((double)realpixmap.width());
//  double heightFactor = ((double)h) / ((double)realpixmap.height());

//  pixmap.convertFromImage(realpixmap.convertToImage().smoothScale(w, h));

//  setWidth(pixmap.width());
//  setHeight(pixmap.height());

}

void ImageLabel::removeImageTransformations()
{
    doScale = false;
    doRotate = false;

    scale_w = 1;
    scale_h = 1;
    rot_angle = 0;
    pixmap = realpixmap;
}

void ImageLabel::applyTransformations(bool useSmoothScale)
{
    pixmap = realpixmap;
    if (doRotate)
    {
        // KDE and QT seem to miss a high quality image rotation
        TQWMatrix rotMat;
        rotMat.rotate(rot_angle);
        pixmap = pixmap.xForm(rotMat);
    }
    if (doScale)
    {
        if (m_karamba -> useSmoothTransforms() || useSmoothScale)
        {
            pixmap.convertFromImage(
              pixmap.convertToImage().smoothScale(scale_w, scale_h));
        }
        else
        {
            double widthFactor = ((double)scale_w) / ((double)pixmap.width());
            double heightFactor = ((double)scale_h) / ((double)pixmap.height());
            TQWMatrix scaleMat;
            scaleMat.scale(widthFactor, heightFactor);
            pixmap = pixmap.xForm(scaleMat);
        }
    }
    if (imageEffect != 0)
    {
        pixmap = imageEffect -> apply(pixmap);
    }
    setWidth(pixmap.width());
    setHeight(pixmap.height());
}

void ImageLabel::slotCopyResult(KIO::Job* job)
{
  TQString tempFile = ((KIO::FileCopyJob*)job)->destURL().path();
  if(job->error() == 0)
  {
    setValue(tempFile);
    imagePath = ((KIO::FileCopyJob*)job)->srcURL().path();
    emit pixmapLoaded();
  }
  else
  {
    qWarning("Error downloading (%s): %s", job->errorText().ascii(),
                                           tempFile.ascii());
  }
  KIO::NetAccess::removeTempFile(tempFile);
}

void ImageLabel::setValue(TQString fn)
{
  // use the first line
  TQStringList sList = TQStringList::split( "\n", fn );
  TQString fileName = *sList.begin();
  KURL url(fileName);
  TQRegExp rx("^[a-zA-Z]{1,5}:/",false);
  bool protocol = (rx.search(fileName)!=-1)?true:false;
  TQPixmap pm;

  if(protocol && url.isLocalFile() == false)
  {
    KTempFile tmpFile;
    KIO::FileCopyJob* copy = KIO::file_copy(fileName, tmpFile.name(), 0600,
                                            true, false, false);
    connect(copy, TQT_SIGNAL(result(KIO::Job*)),
            this, TQT_SLOT(slotCopyResult(KIO::Job*)));
    return;
  }
  else
  {
    if(m_karamba->theme().isThemeFile(fileName))
    {
      TQByteArray ba = m_karamba->theme().readThemeFile(fileName);
      pm.loadFromData(ba);
    }
    else
    {
      pm.load(fileName);
    }
    imagePath = fileName;
  }
  setValue(pm);
}

//Matthew Kay: a new version of setValue to be used by createTaskIcon()
/**
 * set the internal pixmap of this image to the given TQPixmap pix
 */
void ImageLabel::setValue(TQPixmap& pix)
{
  realpixmap = KPixmap(pix);
  pixmap = realpixmap;
  setWidth(pixmap.width());
  setHeight(pixmap.height());

  pixmapWidth = pixmap.width();
  pixmapHeight = pixmap.height();
  rect_off = TQRect(getX(),getY(),pixmapWidth,pixmapHeight);
}

void ImageLabel::mUpdate(TQPainter* p, int backgroundUpdate)
{
  if (backgroundUpdate == 1)
  {
    //only draw image if not hidden
    if (hidden == 0)
    {
      if (cblend == 0)
        //draw the pixmap
        p->drawPixmap(getX(),getY(),pixmap);
      else
      {
        //Blend this image with a color

        TQImage image = pixmap.convertToImage();

        TQImage result = KImageEffect::blend(TQColor(255,0,0), image, 0.5f);
        p->drawImage(getX(),getY(),result);

        //p->drawRect(boundingBox);
      }
    }
    // start Timer
    if (imageEffect != 0)
    {
      imageEffect -> startTimer();
    }
  }
}

void ImageLabel::mUpdate(TQPainter* p)
{
  //only draw image if not hidden
  if (hidden == 0 && background == 0)
  {
    if (cblend == 0)
    {
      //draw the pixmap
      p->drawPixmap(getX(),getY(),pixmap);
    }
    else
    {
      //Blend this image with a color

      TQImage image = pixmap.convertToImage();

      TQImage result = KImageEffect::blend(TQColor(255,0,0), image, 0.5f);
      p->drawImage(getX(),getY(),result);

      //p->drawRect(boundingBox);
    }
  }
  // start Timer
  if (imageEffect != 0)
  {
    imageEffect -> startTimer();
  }
}

bool ImageLabel::click(TQMouseEvent* e)
{
    if (getBoundingBox().tqcontains(e -> x(), e -> y()) && isEnabled())
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

void ImageLabel::parseImages(TQString fn, TQString fn_roll, int _xoff,
                             int _yoff, int _xon, int _yon)
{
  //fn = filename;
  //fn_roll = filename_roll;

  xoff = _xoff;
  yoff = _yoff;
  xon = _xon;
  yon = _yon;

  // use the first line
  TQStringList sList = TQStringList::split( "\n", fn );
  TQString fileName = *sList.begin();
  TQFileInfo fileInfo( fileName );
  TQString path;

  TQRegExp rx("^http://",false);
  bool fileOnNet = (rx.search(fileName)!=-1)?true:false;


  if( fileInfo.isRelative() && !fileOnNet )
  {
    path = m_karamba->theme().path() + "/" + fileName;
  }
  else
  {
    path = fileName;
  }

  if ( fileOnNet )
  {
    TQString tmpFile;
#if defined(KDE_3_2)
    if(KIO::NetAccess::download(KURL(path), tmpFile, karambaApp->tqparentWindow()))
#else
    if(KIO::NetAccess::download(KURL(path), tmpFile))
#endif
    {
      pixmap_off = KPixmap(tmpFile);
      KIO::NetAccess::removeTempFile(tmpFile);
      qDebug( "Downloaded: %s to %s", path.ascii(), tmpFile.ascii() );
    }
    else
    {
      qDebug( "Error Downloading: %s", path.ascii());
    }
  }
  else
  {
    pixmap_off = KPixmap( path );
  }

  pixmapOffWidth = pixmap.width();
  pixmapOffHeight = pixmap.height();

  rect_off = TQRect(xoff,yoff,pixmapWidth,pixmapHeight);
/////////////////////////////
  if (fn_roll.isEmpty())
    return;

  rollover=true;
  sList = TQStringList::split( "\n", fn_roll );
  fileName = *sList.begin();
  fileInfo = TQFileInfo( fileName );

  fileOnNet = (rx.search(fileName)!=-1)?true:false;


  if( fileInfo.isRelative() && !fileOnNet )
  {
    path = m_karamba->theme().path() + "/" + fileName;
  }
  else
  {
    path = fileName;
  }

  if ( fileOnNet )
  {
    TQString tmpFile;
#if defined(KDE_3_2)
    if(KIO::NetAccess::download(KURL(path), tmpFile, karambaApp->tqparentWindow()))
#else
    if(KIO::NetAccess::download(KURL(path), tmpFile))
#endif
    {
      pixmap_on = KPixmap(tmpFile);
      KIO::NetAccess::removeTempFile(tmpFile);
      qDebug( "Downloaded: %s to %s", path.ascii(), tmpFile.ascii());
    }
    else
    {
      qDebug( "Error Downloading: %s", path.ascii());
    }
  }
  else
  {
    pixmap_on = KPixmap( path );
  }
  pixmapOnWidth = pixmap_on.width();
  pixmapOnHeight = pixmap_on.height();

  rect_on = TQRect(xon,yon,pixmapOnWidth,pixmapOnHeight);
}

void ImageLabel::setBackground(int b)
{
  background = b;
}

void ImageLabel::rolloverImage(TQMouseEvent *e)
{
  if (!rollover)
    return;

  if (zoomed)
  {
    if (!rect_off.tqcontains(e->pos()))
    {
      // rollover the image to the zoomed image
      //setValue(fn_roll);
      setX(xoff);
      setY(yoff);
      pixmap = pixmap_off;
      pixmapWidth = pixmapOffWidth;
      pixmapHeight = pixmapOffHeight;
      zoomed = false;
      m_karamba->step();
    }
  }
  else
  {
    if (rect_off.tqcontains(e->pos()))
    {
      // rollover the image to the zoomed image
      //setValue(fn_roll);
      setX(xon);
      setY(yon);
      pixmap = pixmap_on;
      pixmapWidth = pixmapOnWidth;
      pixmapHeight = pixmapOnHeight;
      zoomed = true;
      m_karamba->step();
    }
  }
}

void ImageLabel::setTooltip(TQString txt)
{
  TQRect rect(getX(),getY(),pixmapWidth,pixmapHeight);
  TQToolTip::add(m_karamba, rect, txt);
  old_tip_rect = TQRect(rect.topLeft(), rect.bottomRight());
}


void ImageLabel::removeEffects()
{
  if (imageEffect != 0)
  {
    delete imageEffect;
    imageEffect = 0;
  }
  applyTransformations();
}

void ImageLabel::intensity(float ratio, int millisec)
{
  if (imageEffect != 0)
  {
    delete imageEffect;
    imageEffect = 0;
  }
  //KPixmapEffect::intensity(pixmap, ratio);
  imageEffect = new Intensity(this, ratio, millisec);
  applyTransformations();
}

void ImageLabel::channelIntensity(float ratio, TQString channel, int millisec)
{
  if (imageEffect != 0)
  {
    delete imageEffect;
    imageEffect = 0;
  }
  //KPixmapEffect::channelIntensity(pixmap, ratio, rgbChannel);
  imageEffect = new ChannelIntensity(this, ratio, channel, millisec);
  applyTransformations();
}

void ImageLabel::toGray(int millisec)
{
  if (imageEffect != 0)
  {
    delete imageEffect;
    imageEffect = 0;
  }
  //KPixmapEffect::toGray(pixmap);
  imageEffect = new ToGray(this, millisec);
  applyTransformations();
}

void ImageLabel::slotEffectExpired()
{
  removeEffects();
  m_karamba -> externalStep();
}

void ImageLabel::attachClickArea(TQString leftMouseButton,
                                 TQString  middleMouseButton,
                                 TQString rightMouseButton)
{
    leftButtonAction = leftMouseButton;
    middleButtonAction = middleMouseButton;
    rightButtonAction = rightMouseButton;
}

#include "imagelabel.moc"
