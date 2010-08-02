/*

  ark -- archiver for the KDE project

  Copyright (C)

  2003: Georg Robbers <Georg.Robbers@urz.uni-hd.de>
  2002: Helio Chissini de Castro <helio@conectiva.com.br>
  2001: Corel Corporation (author: Michael Jarrett, michaelj@corel.com)
  1999-2000: Corel Corporation (author: Emily Ezust, emilye@corel.com)
  1999: Francois-Xavier Duranceau duranceau@kde.org
  1997-1999: Rob Palmbos palm9744@kettering.edu

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

#ifndef ARKWIDGET_H
#define ARKWIDGET_H

#include <kio/job.h>
#include <ktempdir.h>

#include <tqvbox.h>
#include "arch.h"

class TQPoint;
class TQString;
class TQStringList;
class TQLabel;
class TQListViewItem;
class TQDragMoveEvent;
class TQDropEvent;

class KPopupMenu;
class KProcess;
class KURL;
class KRun;
class KTempFile;
class KTempDir;
class KToolBar;

class FileListView;
class SearchBar;


class ArkWidget : public QVBox
{
    Q_OBJECT
public:
    ArkWidget( TQWidget *parent=0, const char *name=0 );
    virtual ~ArkWidget();

    bool isArchiveOpen() const { return m_bIsArchiveOpen; }
    int getNumFilesInArchive() const { return m_nNumFiles; }

    int getArkInstanceId() const { return m_arkInstanceId; }
    void setArkInstanceId( int aid ) { m_arkInstanceId = aid; }

    void cleanArkTmpDir();
    virtual TQString getArchName() const { return m_strArchName; }

    const KURL& realURL() const { return m_realURL; }
    void setRealURL( const KURL& url ) { m_realURL = url; }

    TQString tmpDir() const { return m_tmpDir ? m_tmpDir->name() : TQString::null; }

    FileListView * fileList() const { return m_fileListView; }
    SearchBar    * searchBar() const { return m_searchBar; }
    Arch * archive() const { return arch; }
    ArchType archiveType() const { return m_archType; }
    int numSelectedFiles() const { return m_nNumSelectedFiles; }

    /**
     * Miscellaneous tasks involved in closing an archive.
     */
    void closeArch();

    virtual void setExtractOnly(bool extOnly) { m_extractOnly = extOnly; }
    virtual void deleteAfterUse( const TQString& path );
    bool allowedArchiveName( const KURL & u );
    bool file_save_as( const KURL & u );
    virtual KURL getSaveAsFileName();
    virtual void setOpenAsMimeType( const TQString & mimeType );
    TQString & openAsMimeType(){ return m_openAsMimeType; }
    void prepareViewFiles( const TQStringList & fileList );
    virtual void setArchivePopupEnabled( bool b );

    virtual void extractTo( const KURL & targetDirectory, const KURL & archive, bool bGuessName );
    virtual bool addToArchive( const KURL::List & filesToAdd, const KURL & archive = KURL() );
    void convertTo( const KURL & u );

    bool isModified() { return m_modified; }
    void setModified( bool b ) { m_modified = b; }

public slots:
    void file_open( const KURL& url);
    virtual void file_close();
    virtual void file_new();
    void slotShowSearchBarToggled( bool b );
    void showSettings();

protected slots:
    void action_add();
    void action_add_dir();
    void action_view();
    void action_delete();
    bool action_extract();
    void slotOpenWith();
    void action_edit();

    void doPopup(TQListViewItem *, const TQPoint &, int); // right-click menus
    void viewFile(TQListViewItem*); // doubleClick view files

    void slotSelectionChanged();
    void slotOpen(Arch *, bool, const TQString &, int);
    void slotCreate(Arch *, bool, const TQString &, int);
    void slotDeleteDone(bool);
    void slotExtractDone(bool);
    void slotExtractRemoteDone(KIO::Job *job);
    void slotAddDone(bool);
    void slotEditFinished(KProcess *);
signals:
    void openURLRequest( const KURL & url );
    void request_file_quit();
    void setBusy( const TQString & );
    void setReady();
    void fixActions();
    void disableAllActions();
    void signalFilePopup( const TQPoint & pPoint );
    void signalArchivePopup( const TQPoint & pPoint );
    void setStatusBarText( const TQString & text );
    void setStatusBarSelectedFiles( const TQString & text );
    void removeRecentURL( const KURL & url );
    void addRecentURL( const KURL & url );
    void setWindowCaption( const TQString &caption );
    void removeOpenArk( const KURL & );
    void addOpenArk( const KURL & );
    void createDone( bool );
    void openDone( bool );
    void createRealArchiveDone( bool );
    void extractRemoteMovingDone();

protected:

    // DND
    void dragMoveEvent(TQDragMoveEvent *e);
    void dropEvent(TQDropEvent* event);
    void dropAction(TQStringList & list);

private: // methods
    // disabling/enabling of buttons and menu items
    void fixEnables();

    // disable all (temporarily, during operations)
    void disableAll();
    void updateStatusSelection();
    void updateStatusTotals();
    void addFile(TQStringList *list);
    void removeDownloadedFiles();

    // make sure that str is a local file/dir
    KURL toLocalFile(const KURL& url);

    // ask user whether to create a real archive from a compressed file
    // returns filename if so. Otherwise, empty.
    KURL askToCreateRealArchive();
    Arch * getNewArchive( const TQString & _fileName, const TQString& _mimetype = TQString() );
    void createRealArchive( const TQString &strFilename,
                            const TQStringList & filesToAdd = TQStringList() );
    KURL getCreateFilename( const TQString & _caption,
                            const TQString & _defaultMimeType = TQString::null,
                            bool allowCompressed = true,
                            const TQString & _suggestedName = TQString::null );

    bool reportExtractFailures(const TQString & _dest, TQStringList *_list);
    TQStringList existingFiles( const TQString & _dest, TQStringList & _list );

    void extractOnlyOpenDone();
    void extractRemoteInitiateMoving( const KURL & target );
    void editStart();

    void busy( const TQString & text );
    void holdBusy();
    void resumeBusy();
    void ready();

    //suggests an extract directory based on archive name
    const TQString guessName( const KURL & archive );

private slots:
    void startDrag( const TQStringList & fileList );
    void startDragSlotExtractDone( bool );
    void editSlotExtractDone();
    void editSlotAddDone( bool success );
    void viewSlotExtractDone( bool success );
    void openWithSlotExtractDone( bool success );

    void createRealArchiveSlotCreate( Arch * newArch, bool success,
                                      const TQString & fileName, int nbr );
    void createRealArchiveSlotAddDone( bool success );
    void createRealArchiveSlotAddFilesDone( bool success );

    void convertSlotExtractDone( bool success );
    void convertSlotCreate();
    void convertSlotCreateDone( bool success );
    void convertSlotAddDone( bool success );
    void convertFinish();

    void extractToSlotOpenDone( bool success );
    void extractToSlotExtractDone( bool success );

    void addToArchiveSlotOpenDone( bool success );
    void addToArchiveSlotCreateDone( bool success );
    void addToArchiveSlotAddDone( bool success );

protected:
    void arkWarning(const TQString& msg);
    void arkError(const TQString& msg);

    void newCaption(const TQString& filename);
    void createFileListView();

    bool createArchive(const TQString & name);
    void openArchive(const TQString & name);

    void showCurrentFile();

private: // data

    bool m_bBusy;
    bool m_bBusyHold;
    bool m_settingsAltered;

    // for use in the edit methods: the url.
    TQString m_strFileToView;

    // the compressed file to be added into the new archive
    TQString m_compressedFile;

    // Set to true if we are doing an "Extract to Folder"
    bool m_extractOnly;

    // Set to true if we are extracting to a remote location
    bool m_extractRemote;

    // URL to extract to.
    KURL m_extractURL;

     // URL to view
    KURL m_viewURL;

    // the mimetype the user wants to open this archive as
    TQString m_openAsMimeType;

    // if they're dragging in files, this is the temporary list for when
    // we have to create an archive:
    TQStringList *m_pTempAddList;

    KRun *m_pKRunPtr;

    TQStringList mpDownloadedList;

    bool m_bArchivePopupEnabled;

    KTempDir * m_convert_tmpDir;
    KURL m_convert_saveAsURL;
    bool m_convertSuccess;

    KURL m_extractTo_targetDirectory;

    KURL::List m_addToArchive_filesToAdd;
    KURL m_addToArchive_archive;

    KTempDir * m_createRealArchTmpDir;
    KTempDir * m_extractRemoteTmpDir;

    bool m_modified;

    KToolBar  * m_searchToolBar;
    SearchBar * m_searchBar;

    Arch   * arch;
    TQString  m_strArchName;
    KURL     m_realURL;
    KURL     m_url;
    ArchType m_archType;
    FileListView * m_fileListView;

    KIO::filesize_t m_nSizeOfFiles;
    KIO::filesize_t m_nSizeOfSelectedFiles;
    unsigned int m_nNumFiles;
    int m_nNumSelectedFiles;
    int m_arkInstanceId;

    bool m_bIsArchiveOpen;
    bool m_bIsSimpleCompressedFile;
    bool m_bDropSourceIsSelf;

    TQStringList mDragFiles;
    TQStringList *m_extractList;
    TQStringList *m_viewList;
    KTempDir *m_tmpDir;
};

#endif /* ARKWIDGET_H*/

