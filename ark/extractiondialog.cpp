/*
 * ark -- archiver for the KDE project
 *
 * Copyright (C)
 *
 * 2005: Henrique Pinto <henrique.pinto@kdemail.net>
 * 2002: Helio Chissini de Castro <helio@conectiva.com.br>
 * 2001: Roberto Selbach Teixeira <maragato@conectiva.com.br>
 * 2001: Corel Corporation (author: Michael Jarrett, michaelj@corel.com)
 * 1999-2000: Corel Corporation (author: Emily Ezust emilye@corel.com)
 * 1999: Francois-Xavier Duranceau duranceau@kde.org
 * 1997-1999: Rob Palmbos palm9744@kettering.edu
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "extractiondialog.h"

#include <tqvbox.h>
#include <tqhbox.h>
#include <tqhbuttongroup.h>
#include <tqlabel.h>
#include <tqradiobutton.h>
#include <tqlayout.h>

#include <tdelocale.h>
#include <tdeglobal.h>
#include <kiconloader.h>
#include <kurlrequester.h>
#include <kurlcompletion.h>
#include <kstandarddirs.h>
#include <tdemessagebox.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <kurlpixmapprovider.h>
#include <kdebug.h>

#include "artdeutils.h"
#include "settings.h"

ExtractionDialog::ExtractionDialog( TQWidget *parent, const char *name,
                                    bool enableSelected,
                                    const KURL& defaultExtractionDir,
                                    const TQString &prefix,
                                    const TQString &archiveName )
	: KDialogBase( parent, name, true, i18n( "Extract" ), Ok | Cancel, Ok ),
	  m_selectedButton( 0 ), m_allButton( 0 ),
	  m_selectedOnly( enableSelected ), m_extractionDirectory( defaultExtractionDir ),
	  m_defaultExtractionDir( defaultExtractionDir.prettyURL() ), m_prefix( prefix )
{
	if ( !archiveName.isNull() )
	{
		setCaption( i18n( "Extract Files From %1" ).arg( archiveName ) );
	}

	TQVBox *vbox = makeVBoxMainWidget();

	TQHBox *header = new TQHBox( vbox );
	header->layout()->setSpacing( 10 );

	TQLabel *icon = new TQLabel( header );
	icon->setPixmap( DesktopIcon( "ark_extract" ) );
	icon->setSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Minimum );

	if ( enableSelected )
	{
		TQVBox *whichFiles = new TQVBox( header );
		whichFiles->layout()->setSpacing( 6 );
		new TQLabel( TQString( "<qt><b><font size=\"+1\">%1</font></b></qt>" )
		            .arg( i18n( "Extract:" ) ), whichFiles );
		TQHButtonGroup *filesGroup = new TQHButtonGroup( whichFiles );
		m_selectedButton = new TQRadioButton( i18n( "Selected files only" ), filesGroup );
		m_allButton      = new TQRadioButton( i18n( "All files" ), filesGroup );

		m_selectedButton->setChecked( true );
	}
	else
	{
		new TQLabel( TQString( "<qt><b><font size=\"+2\">%1</font></b></qt>" )
		            .arg( i18n( "Extract all files" ) ), header );
	}

	TQHBox *destDirBox = new TQHBox( vbox );

	TQLabel *destFolderLabel = new TQLabel( i18n( "Destination folder: " ), destDirBox );
	destFolderLabel->setSizePolicy( TQSizePolicy::Minimum, TQSizePolicy::Fixed );

	KHistoryCombo *combobox = new KHistoryCombo( true, destDirBox );
	combobox->setPixmapProvider( new KURLPixmapProvider );
	combobox->setHistoryItems( ArkSettings::extractionHistory() );
	destFolderLabel->setBuddy( combobox );

	KURLCompletion *comp = new KURLCompletion();
	comp->setReplaceHome( true );
	comp->setCompletionMode( TDEGlobalSettings::CompletionAuto );
	combobox->setCompletionObject( comp );
	combobox->setMaxCount( 20 );
	combobox->setInsertionPolicy( TQComboBox::AtTop );

	m_urlRequester = new KURLRequester( combobox, destDirBox );
	m_urlRequester->setSizePolicy( TQSizePolicy::Expanding, TQSizePolicy::Fixed );
	m_urlRequester->setMode( KFile::Directory );

	if (!defaultExtractionDir.prettyURL().isEmpty() )
	{
		m_urlRequester->setKURL( defaultExtractionDir.prettyURL() + prefix );
	}

	m_viewFolderAfterExtraction = new TQCheckBox( i18n( "Open destination folder after extraction" ), vbox );
	m_viewFolderAfterExtraction->setChecked( ArkSettings::openDestinationFolder() );

	connect( combobox, TQT_SIGNAL( returnPressed( const TQString& ) ), combobox, TQT_SLOT( addToHistory( const TQString& ) ) );
	connect( combobox->lineEdit(), TQT_SIGNAL( textChanged( const TQString& ) ),
	         this, TQT_SLOT( extractDirChanged( const TQString & ) ) );
}

ExtractionDialog::~ExtractionDialog()
{
	ArkSettings::setExtractionHistory( ( static_cast<KHistoryCombo*>( m_urlRequester->comboBox() ) )->historyItems() );
}

void ExtractionDialog::accept()
{

	KURLCompletion uc;
	uc.setReplaceHome( true );
	KURL p( uc.replacedPath( m_urlRequester->comboBox()->currentText() ) );

	//if p isn't local KIO and friends will complain later on
	if ( p.isLocalFile() )
	{
		TQFileInfo fi( p.path() );
		if ( !fi.isDir() && !fi.exists() )
		{
			TQString ltext = i18n( "Create folder %1?").arg(p.path());
			int createDir =  KMessageBox::questionYesNo( this, ltext, i18n( "Missing Folder" ) , i18n("Create Folder"), i18n("Do Not Create"));
			if( createDir == 4 )
			{
				return;
			}
			// create directory using filename, make sure it has trailing slash
			p.adjustPath(1);
			if( !TDEStandardDirs::makeDir( p.path() ) )
			{
				KMessageBox::error( this, i18n( "The folder could not be created. Please check permissions." ) );
				return;
			}
		}
		if ( !ArkUtils::haveDirPermissions( p.path() ) )
		{
			KMessageBox::error( this, i18n( "You do not have write permission to this folder. Please provide another folder." ) );
			return;
		}
	}

	m_extractionDirectory = p;
	m_selectedOnly = m_selectedButton == 0? false : m_selectedButton->isChecked();

	// Determine what exactly should be added to the extraction combo list
	TQString historyURL = p.prettyURL();
	if ( historyURL == KURL( m_defaultExtractionDir + m_prefix ).prettyURL() )
	{
		historyURL = m_defaultExtractionDir;
	}

	KHistoryCombo *combo = static_cast<KHistoryCombo*>( m_urlRequester->comboBox() );
	// If the item was already in the list, delete it from the list and readd it at the top
	combo->removeFromHistory( historyURL );
	combo->addToHistory( historyURL );

	ArkSettings::setOpenDestinationFolder( m_viewFolderAfterExtraction->isChecked() );

	KDialogBase::accept();
}

void ExtractionDialog::extractDirChanged(const TQString &text )
{
	enableButtonOK(!text.isEmpty());
}

#include "extractiondialog.moc"
// kate: space-indent off; tab-width 4;
