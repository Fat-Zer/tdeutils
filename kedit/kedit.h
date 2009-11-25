/*

    $Id$

    Copyright (C) 1997 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

    */

#ifndef _KEDIT_H_
#define _KEDIT_H_

#include <qtextcodec.h>
#include <qptrdict.h>

#include "version.h"

#include <kmainwindow.h>

class KEdit;
class KSpell;
class KSpellConfig;
class QTimer;
class KAction;
class KRecentFilesAction;

namespace KIO { class Job; }

// StatusBar field IDs
#define ID_LINE_COLUMN 1
#define ID_INS_OVR 2
#define ID_GENERAL 3

class TopLevel : public KMainWindow
{
    Q_OBJECT

public:
    enum { KEDIT_OK 		= 0,
	   KEDIT_OS_ERROR 	= 1,
	   KEDIT_USER_CANCEL 	= 2 ,
	   KEDIT_RETRY 		= 3,
	   KEDIT_NOPERMISSIONS 	= 4};

    enum { OPEN_READWRITE 	= 1,
	   OPEN_READONLY 	= 2,
	   OPEN_INSERT 		= 4,
	   OPEN_NEW             = 8 };

    TopLevel( QWidget *parent=0, const char *name=0 );
    ~TopLevel();

    /**
     * Reads a file into the edit widget.
     *
     * @return KEDIT_OK on success
     */
    int openFile( const QString& _filename, int _mode, const QString &encoding, bool _undoAction = false );

    /**
     * Saves the edit widget to a file.
     *
     * @return KEDIT_OK on success
     */
    int saveFile( const QString& _filename, bool backup, const QString &encoding);

    /**
     * Works like openFile but is able to open remote files
     */
    void openURL( const KURL& _url, int _mode );

    /**
     * Saves the current text to the URL '_url'.
     *
     * @return KEDIT::KEDIT_OK on success
     */

    int saveURL( const KURL& _url );

    /**
     * Only show the window when the following load action is successful.
     **/
    void setNewWindow() { newWindow = true; }

    /**
     * set url
     */
    void setUrl(const KURL &url) { m_url = url; }

    /// List of all windows
    static QPtrList<TopLevel> *windowList;
    //QPopupMenu *right_mouse_button;

    bool queryExit( void );
    bool queryClose( void );

protected:
    void setSensitivity();
    void setupEditWidget();
    void setupStatusBar();
    void setupActions();
    void initSpellConfig();

private:
    KSpellConfig *kspellconfigOptions;
    
public:  // Should not be!
    KEdit *eframe;
private:
    KURL m_url;
    QString m_caption;

    bool newWindow;
    int statusID, toolID, indentID;
    QTimer *statusbar_timer;
    KRecentFilesAction *recent;
    KAction *cutAction;
    KAction *copyAction;
    KAction *undoAction;
    KAction *redoAction;

    int open_mode;

    KConfig *config;

    KSpell *kspell; // Current spell checking object

    /*
     * The source, the destination of the copy, and the open mode
     * for each job being run (job ptr is the dict key).
     */
    QPtrDict <QString> m_sNet;
    QPtrDict <QString> m_sLocal;
    QPtrDict <int> m_openMode;

    // Session management
    void saveProperties(KConfig*);
    void readProperties(KConfig*);

public slots:
    void openRecent(const KURL&);
    void gotoLine();
    void mail();
    void setGeneralStatusField(const QString &string);
    void undo();
    void redo();
    void copy();
    void paste();
    void cut();
    void insertDate();
    void print();
    void select_all();
    void clean_space();
    void timer_slot();
    void file_open();
    void file_new();
    void file_insert();
    void setFileCaption();
    void statusbar_slot();
    void file_close();
    void file_save();
    void file_save_as();
    void helpselected();
    void search();
    void replace();
    void search_again();
    void toggle_overwrite();

    void spellcheck();
    void spell_started ( KSpell *);
    void spell_progress (unsigned int percent);
    void spell_done(const QString&);
    void spell_finished();

    void urlDrop_slot(QDropEvent* e);

    void set_colors();

protected:
    /// Drag and Drop
    void dragEnterEvent(QDragEnterEvent* e);
    void dropEvent(QDropEvent* e);

private slots:
    void updateSettings();
    void readSettings();
    void showSettings();
    void slotSelectionChanged();
    QString replaceISpell(QString msg, int client);
};

class SettingsDialog: public KConfigDialog {
Q_OBJECT

public:
  SettingsDialog(QWidget *parent, const char *name,KConfigSkeleton *config, KSpellConfig *_spellConfig);
  
protected slots:
  void updateSettings();
  void updateWidgets();
  void updateWidgetsDefault();
  void slotSpellConfigChanged();
  
protected:
  bool hasChanged();
  bool isDefault();

private:
  KSpellConfig *spellConfig;  
  bool spellConfigChanged;
};


#endif

