/***************************************************************************
                          popuppublic.h  -  description
                             -------------------
    begin                : Sat Jun 29 2002
    copyright            : (C) 2002 by Jean-Baptiste Mardelle
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
#ifndef POPUPPUBLIC_H
#define POPUPPUBLIC_H

#include <kdialogbase.h>

//#include <kiconloader.h>
#include <tdeshortcut.h>


class TQPushButton;
class TQCheckBox;
class TDEListView;
class TQButtonGroup;
class KProcIO;

class popupPublic : public KDialogBase //TQDialog
{
        Q_OBJECT
  
public:

        popupPublic(TQWidget *parent=0, const char *name=0,TQString sfile="",bool filemode=false,TDEShortcut goDefaultKey=TQKeySequence(CTRL+TQt::Key_Home));
	~popupPublic();
        TDEListView *keysList;
        TQCheckBox *CBarmor,*CBuntrusted,*CBshred,*CBsymmetric,*CBhideid;
        bool fmode,trusted;
        TQPixmap keyPair,keySingle,keyGroup;
        TQString seclist;
	TQStringList untrustedList;

private:
        TDEConfig *config;
        TQButtonGroup *boutonboxoptions;
        TQString customOptions;

private slots:
        void customOpts(const TQString &);
        void slotprocread(KProcIO *);
        void slotpreselect();
        void refreshkeys();
        void refresh(bool state);
        void isSymetric(bool state);
        void sort();
        void enable();
	void slotGotoDefaultKey();
	
public slots:
void slotAccept();
void slotSetVisible();

protected slots:
virtual void slotOk();
	
signals:
        void selectedKey(TQStringList ,TQStringList,bool,bool);
	void keyListFilled();

};

#endif // POPUPPUBLIC_H

