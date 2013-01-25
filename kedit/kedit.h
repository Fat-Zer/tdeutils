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

#include <tqtextcodec.h>
#include <tqptrdict.h>

#include "version.h"

#include <kmainwindow.h>

class KEdit;
class KSpell;
class KSpellConfig;
class TQTimer;
class KAction;
class KRecentFilesAction;

namespace TDEIO { class Job; }

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

    TopLevel( TQWidget *parent=0, const char *name=0 );
    ~TopLevel();

    /**
     * Reads a file into the edit widget.
     *
     * @return KEDIT_OK on success
     */
    int openFile( const TQString& _filename, int _mode, const TQString &encoding, bool _undoAction = false );

    /**
     * Saves the edit widget to a file.
     *
     * @return KEDIT_OK on success
     */
    int saveFile( const TQString& _filename, bool backup, const TQString &encoding);

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
    static TQPtrList<TopLevel> *windowList;
    //TQPopupMenu *right_mouse_button;

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
    TQString m_caption;

    bool newWindow;
    int statusID, toolID, indentID;
    TQTimer *statusbar_timer;
    KRecentFilesAction *recent;
    KAction *cutAction;
    KAction *copyAction;
    KAction *undoAction;
    KAction *redoAction;

    int open_mode;

    TDEConfig *config;

    KSpell *kspell; // Current spell checking object

    /*
     * The source, the destination of the copy, and the open mode
     * for each job being run (job ptr is the dict key).
     */
    TQPtrDict <TQString> m_sNet;
    TQPtrDict <TQString> m_sLocal;
    TQPtrDict <int> m_openMode;

    // Session management
    void saveProperties(TDEConfig*);
    void readProperties(TDEConfig*);

public slots:
    void openRecent(const KURL&);
    void gotoLine();
    void mail();
    void setGeneralStatusField(const TQString &string);
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
    void spell_done(const TQString&);
    void spell_finished();

    void urlDrop_slot(TQDropEvent* e);

    void set_colors();

protected:
    /// Drag and Drop
    void dragEnterEvent(TQDragEnterEvent* e);
    void dropEvent(TQDropEvent* e);

private slots:
    void updateSettings();
    void readSettings();
    void showSettings();
    void slotSelectionChanged();
    TQString replaceISpell(TQString msg, int client);
};

class SettingsDialog: public TDEConfigDialog {
Q_OBJECT
  

public:
  SettingsDialog(TQWidget *parent, const char *name,TDEConfigSkeleton *config, KSpellConfig *_spellConfig);
  
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

