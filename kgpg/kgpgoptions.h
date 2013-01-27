/***************************************************************************
                          kgpgoptions.h  -  description
                             -------------------
    begin                : Mon Jul 8 2002
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
#ifndef KGPGOPTIONS_H
#define KGPGOPTIONS_H

#include <tdeconfigdialog.h>

#define GoodColor 0
#define BadColor 1
#define UnknownColor 2
#define RevColor 3

class TDEConfig;
class Encryption;
class Decryption;
class UIConf;
class GPGConf;
class ServerConf;
class ColorsConf;
class MiscConf;
class KFontChooser;
class TDEConfig;
class KSimpleConfig;

class kgpgOptions : public TDEConfigDialog
{
        Q_OBJECT
  
public:
        kgpgOptions(TQWidget *parent=0, const char *name=0);
        ~kgpgOptions();
        TQStringList names,ids;
        Encryption *page1;
        Decryption *page2;
        UIConf *page3;
        GPGConf *page4;
	ServerConf *page6;
	MiscConf *page7;
	KFontChooser *kfc;
        
private:
        TDEConfig *config;
        TQString alwaysKeyID,alwaysKeyName;
        bool firstDisplay;
	
	TQPixmap pixkeySingle,pixkeyDouble;
        TQString fileEncryptionKey;
        TQString gpgConfigPath;
        TQString keyServer,defaultServerList;
        TQString defaultKeyServer;
	TQFont startFont;
        bool useAgent;
        bool defaultUseAgent;
        bool encryptToAlways;
        bool defaultEncryptToAlways;
	TQStringList serverList;
	TQString defaultConfigPath,defaultHomePath;
	TQColor keyGood,keyBad,keyUnknown,keyRev;

private:
        bool hasChanged();
        bool isDefault();

private slots:
	void checkAdditionalState(bool);
	void slotAddKeyServer();
	void slotDelKeyServer();
	void slotDefaultKeyServer();
        void updateWidgets();
        void updateWidgetsDefault();
        void updateSettings();
	void insertAlwaysKey();
	void insertFileKey();

        void listkey();
        TQString namecode(TQString kid);
        TQString idcode(TQString kname);
        void slotInstallDecrypt(TQString mimetype);
        void slotInstallSign(TQString mimetype);
        void slotRemoveMenu(TQString menu);
	void slotChangeHome();
signals:
        void updateDisplay();
        void settingsUpdated();
	void changeFont(TQFont);
	void homeChanged();
	void refreshTrust(int, TQColor);
	void installShredder();
	void reloadKeyList();
};

#endif // KGPGOPTIONS_H

