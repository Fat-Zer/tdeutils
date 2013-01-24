/*

 ark -- archiver for the KDE project

 Copyright (C)

 1997-1999: Rob Palmbos palm9744@kettering.edu
 1999: Francois-Xavier Duranceau duranceau@kde.org
 1999-2000: Corel Corporation (author: Emily Ezust, emilye@corel.com)
 2001: Corel Corporation (author: Michael Jarrett, michaelj@corel.com)

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

/* The following class is the base class for all of the archive types.
 * In order for it to work properly with the TDEProcess, you have to
 * connect the ProcessExited signal appropriately before spawning
 * the core operations. Then the signal that the process exited can
 * be intercepted by the viewer (in ark, ArkWidget) and dealt with
 * appropriately. See LhaArch or ZipArch for a good model. Don't use
 * TarArch or CompressedFile as models - they're too complicated!
 *
 * Don't forget to set m_archiver_program and m_unarchiver_program
 * and add a call to
 *     verifyUtilityIsAvailable(m_archiver_program, m_unarchiver_program);
 * in the constructor of your class. It's OK to leave out the second argument.
 *
 * To add a new archive type:
 * 1. Create a new header file and a source code module
 * 2. Add an entry to the ArchType enum in arch.h.
 * 3. Add appropriate types to buildFormatInfo() in archiveformatinfo.cpp
 *    and archFactory() in arch.cpp
 */


#ifndef ARCH_H
#define ARCH_H

#include <tqobject.h>
#include <tqptrlist.h> // Some very annoying hackery in arkwidgetpart
#include <tqregexp.h>
#include <tqstring.h>
#include <kurl.h>
#include <tqpair.h>

class TQCString;
class TQStringList;
class TDEProcess;

class FileListView;
class ArkWidget;

enum ArchType { UNKNOWN_FORMAT, ZIP_FORMAT, TAR_FORMAT, AA_FORMAT,
                LHA_FORMAT, RAR_FORMAT, ZOO_FORMAT, COMPRESSED_FORMAT,
                SEVENZIP_FORMAT, ACE_FORMAT, ARJ_FORMAT };

typedef TQValueList< TQPair< TQString, TQt::AlignmentFlags > > ColumnList;

/**
 * Pure virtual base class for archives - provides a framework as well as
 * useful common functionality.
 */
class Arch : public TQObject
{
  Q_OBJECT
  

  protected:
    /**
     * A struct representing column data. This makes it possible to abstract
     * archive output, and save writing the same function for every archive
     * type. It is also much more robust than sscanf (which was breaking).
     */
    struct ArchColumns
    {
      int colRef; // Which column to load to in processLine
      TQRegExp pattern;
      int maxLength;
      bool optional;

      ArchColumns( int col, TQRegExp reg, int length = 64, bool opt = false );
    };

  public:
    Arch( ArkWidget *_viewer, const TQString & _fileName );
    virtual ~Arch();

    virtual void open() = 0;
    virtual void create() = 0;
    virtual void remove( TQStringList * ) = 0;
    virtual void test();

    virtual void addFile( const TQStringList & ) = 0;
    virtual void addDir( const TQString & ) = 0;

    // unarch the files in m_fileList or all files if m_fileList is empty.
    // if m_destDir is empty, abort with error.
    // m_viewFriendly forces certain options like directory junking required by view/edit
    virtual void unarchFileInternal() = 0;
    // returns true if a password is required
    virtual bool passwordRequired() { return false; }

    void unarchFile( TQStringList *, const TQString & _destDir,
                             bool viewFriendly = false );

    TQString fileName() const { return m_filename; }

    enum EditProperties{ Add = 1, Delete = 2, Extract = 4,
                         View = 8, Integrity = 16 };

    // is the archive readonly?
    bool isReadOnly() { return m_bReadOnly; }
    void setReadOnly( bool bVal ) { m_bReadOnly = bVal; }

    bool isError() { return m_error; }
    void resetError() { m_error = false; }

    // check to see if the utility exists in the PATH of the user
    void verifyUtilityIsAvailable( const TQString &,
                                   const TQString & = TQString() );

    void verifyCompressUtilityIsAvailable( const TQString &utility );

    void verifyUncompressUtilityIsAvailable( const TQString &utility );

    bool archUtilityIsAvailable() { return m_bArchUtilityIsAvailable; }

    bool unarchUtilityIsAvailable() { return m_bUnarchUtilityIsAvailable; }

    TQString getArchUtility() { return m_archiver_program; }

    TQString getUnarchUtility() { return m_unarchiver_program; }

    void appendShellOutputData( const char * data ) { m_lastShellOutput.append( TQString::fromLocal8Bit( data ) ); }
    void clearShellOutput() { m_lastShellOutput.truncate( 0 ); }
    const TQString& getLastShellOutput() const { return m_lastShellOutput; }

    static Arch *archFactory( ArchType aType, ArkWidget *parent,
                              const TQString &filename,
                              const TQString &openAsMimeType = TQString() );
    TQString password() { return m_password; }
    void setPassword(const TQString & pw) { m_password = pw.local8Bit(); }
    virtual void createPassword() {}

  protected slots:
    void slotOpenExited( TDEProcess* );
    void slotExtractExited( TDEProcess* );
    void slotDeleteExited( TDEProcess* );
    void slotAddExited( TDEProcess* );
    void slotTestExited( TDEProcess* );

    void slotReceivedOutput( TDEProcess *, char*, int );

    virtual bool processLine( const TQCString &line );
    virtual void slotReceivedTOC( TDEProcess *, char *, int );

  signals:
    void sigOpen( Arch * archive, bool success, const TQString &filename, int );
    void sigCreate( Arch *, bool, const TQString &, int );
    void sigDelete( bool );
    void sigExtract( bool );
    void sigAdd( bool );
    void sigTest( bool );
    void headers( const ColumnList& columns );

  protected:  // data
    TQString m_filename;
    TQString m_lastShellOutput;
    TQCString m_buffer;
    ArkWidget *m_gui;
    bool m_bReadOnly; // for readonly archives
    bool m_error;

    // lets tar delete unsuccessfully before adding without confusing the user
    bool m_bNotifyWhenDeleteFails;

    // set to whether if the compressing utility is in the user's PATH
    bool m_bArchUtilityIsAvailable;

    // set to whether if the uncompressing utility is in the user's PATH
    bool m_bUnarchUtilityIsAvailable;

    TQString m_archiver_program;
    TQString m_unarchiver_program;

    // Archive parsing information
    TQCString m_headerString;
    bool m_header_removed, m_finished;
    TQPtrList<ArchColumns> m_archCols;
    int m_numCols, m_dateCol, m_fixYear, m_fixMonth, m_fixDay, m_fixTime;
    int m_repairYear, m_repairMonth, m_repairTime;
    TDEProcess *m_currentProcess;
    TQStringList *m_fileList;
    TQString m_destDir;
    bool m_viewFriendly;
    TQCString m_password;
};

// Columns
// don't forget to change common_texts.cpp if you change something here
#define FILENAME_COLUMN    tqMakePair( i18n(" Filename "),    TQt::AlignLeft  )
#define PERMISSION_COLUMN  tqMakePair( i18n(" Permissions "), TQt::AlignLeft  )
#define OWNER_GROUP_COLUMN tqMakePair( i18n(" Owner/Group "), TQt::AlignLeft  )
#define SIZE_COLUMN        tqMakePair( i18n(" Size "),        TQt::AlignRight )
#define TIMESTAMP_COLUMN   tqMakePair( i18n(" Timestamp "),   TQt::AlignRight )
#define LINK_COLUMN        tqMakePair( i18n(" Link "),        TQt::AlignLeft  )
#define PACKED_COLUMN      tqMakePair( i18n(" Size Now "),    TQt::AlignRight )
#define RATIO_COLUMN       tqMakePair( i18n(" Ratio "),       TQt::AlignRight )
#define CRC_COLUMN         tqMakePair( i18n("Cyclic Redundancy Check"," CRC "), TQt::AlignLeft )
#define METHOD_COLUMN      tqMakePair( i18n(" Method "),  TQt::AlignLeft  )
#define VERSION_COLUMN     tqMakePair( i18n(" Version "), TQt::AlignLeft  )
#define OWNER_COLUMN       tqMakePair( i18n(" Owner "),   TQt::AlignLeft  )
#define GROUP_COLUMN       tqMakePair( i18n(" Group "),   TQt::AlignLeft  )

#endif /* ARCH_H */
