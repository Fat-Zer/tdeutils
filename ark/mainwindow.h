/*

  ark -- archiver for the KDE project

  Copyright (C) 2002-2003: Georg Robbers <Georg.Robbers@urz.uni-hd.de>
  Copyright (C) 2003: Helio Chissini de Castro <helio@conectiva.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef ARKMAINWINDOW_H
#define ARKMAINWINDOW_H

// QT includes
#include <tqstring.h>
#include <tqpopupmenu.h>
#include <tqtimer.h>

// KDE includes
#include <tdemainwindow.h>
#include <tdeparts/mainwindow.h>
#include <tdeparts/part.h>
#include <kprogress.h>

class ArkWidget;

class
MainWindow: public KParts::MainWindow
{
    Q_OBJECT
  
public:
    MainWindow( TQWidget *parent=0, const char *name=0 );
    virtual ~MainWindow();

    void setExtractOnly ( bool b );
    void extractTo( const KURL & targetDirectory, const KURL & archive, bool guessName );
    void addToArchive( const KURL::List & filesToAdd, const TQString & cwd = TQString(),
                       const KURL & archive = KURL(), bool askForName = false );

public slots:
    void file_newWindow();
    void file_new();
    void openURL( const KURL & url, bool tempFile = false );
    void file_open();
    void file_reload();
    void editToolbars();
    void window_close();
    void file_quit();
    void file_close();
    void slotNewToolbarConfig();
    void slotConfigureKeyBindings();
    virtual void saveProperties( TDEConfig* config );
    virtual void readProperties( TDEConfig* config );
    void slotSaveProperties();
    void slotArchivePopup( const TQPoint &pPoint);
    void slotRemoveRecentURL( const KURL &url );
    void slotAddRecentURL( const KURL &url );
    void slotFixActionState( const bool & bHaveFiles );
    void slotDisableActions();
    void slotAddOpenArk( const KURL & _arkname );
    void slotRemoveOpenArk( const KURL & _arkname );

protected:
    virtual bool queryClose(); // SM

private: // methods
    // disabling/enabling of buttons and menu items
    void setupActions();
    void setupMenuBar();

    void newCaption(const TQString & filename);
    bool arkAlreadyOpen( const KURL & url );

    KURL getOpenURL( bool addOnly = false , const TQString & caption = TQString(),
                     const TQString & startDir = TQString(),
                     const TQString & suggestedName = TQString() );

    void startProgressDialog( const TQString & text );

private slots:
    void slotProgress();

private: // data
    KParts::ReadWritePart *m_part;
    ArkWidget *m_widget; //the parts widget

    TDEAction *newWindowAction;
    TDEAction *newArchAction;
    TDEAction *openAction;
    TDEAction *closeAction;
    TDEAction *reloadAction;
    TDERecentFilesAction *recent;

    //progress dialog for konqs service menus / commmand line
    KProgressDialog *progressDialog;
    TQTimer *timer;
};

#endif /* ARKMAINWINDOW_H*/
