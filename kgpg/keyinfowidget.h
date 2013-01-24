/***************************************************************************
                          keyinfowidget.h  -  description
                             -------------------
    begin                : Thu Jul 4 2002
    copyright          : (C) 2002 by Jean-Baptiste Mardelle
    email                : bj@altern.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KEYINFOWIDGET_H
#define KEYINFOWIDGET_H

#include <kdialogbase.h>
#include <tqpixmap.h>

class KTempFile;
class TQLabel;
class KDialogBase;
class TQCheckBox;
class KDatePicker;
class KeyProperties;
class TDEProcess;
class KProcIO;

class KgpgKeyInfo : public KDialogBase
{
        Q_OBJECT
  

public:

	KgpgKeyInfo( TQWidget *parent = 0, const char *name = 0,TQString sigkey=0);
	~KgpgKeyInfo();
	KeyProperties *prop;

private slots:
        void slotinfoimgread(TDEProcess *);
        void slotChangePass();
        void slotPreOk();
	void slotChangeExp();
	void slotEnableDate(bool isOn);
	void slotChangeDate();
	void slotCheckDate(TQDate date);
	void openPhoto();
	void slotSetPhoto(const TQPixmap &pix);
	void finishphotoreadprocess(KProcIO *p);
	void slotMainImageRead(TDEProcess *);
	void slotSetMainPhoto(TQStringList list);
	void reloadMainPhoto(const TQString &uid);
	void slotInfoPasswordChanged();
	void slotInfoExpirationChanged(int res);
	void slotInfoTrustChanged();
	void slotChangeTrust(int newTrust);
	void loadKey(TQString Keyid);
	void slotDisableKey(bool isOn);

private:
        KTempFile *kgpginfotmp;
        TQLabel *keyinfoPhoto;
        TQString displayedKeyID;
        TQString expirationDate;
        bool hasPhoto,keyWasChanged;
	KDialogBase *chdate;
	TQCheckBox *kb;
	KDatePicker *kdt;
	int counter;

signals:
	
	void signalPhotoId(const TQPixmap&);
	void changeMainPhoto(const TQPixmap&);
	void keyNeedsRefresh();

};

#endif // KEYINFOWIDGET_H

