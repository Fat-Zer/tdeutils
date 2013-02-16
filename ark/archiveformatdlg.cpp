/*
 ark -- archiver for the KDE project

 Copyright (C) 2003: Georg Robbers <Georg.Robbers@urz.uni-hd.de>

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

#include "archiveformatdlg.h"
#include "archiveformatinfo.h"
#include "arch.h"

#include <tdelocale.h>

#include <tqlabel.h>
#include <tqvbox.h>

ArchiveFormatDlg::ArchiveFormatDlg( TQWidget * parent, const TQString & defaultType )
                        :KDialogBase( parent, "archiveformatdialog", true,
                          i18n( "Choose Archive Format" ),
                          KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok),
                          m_combo( 0 )
{
    TQString defaultDescription = ArchiveFormatInfo::self()->descriptionForMimeType( defaultType );
    TQString text;
    if ( defaultDescription.isNull() )
        text = i18n( "This file appears to be of type %1,\n"
                     "which is not a supported archive format.\n"
                     "In order to proceed, please choose the format\n"
                     "of the file." ).arg( defaultType );
    else
        text = i18n( "You are about to open a file that has a non-standard extension.\n"
                    "Ark has detected the format: %1\n"
                    "If this is not correct, please choose "
                    "the appropriate format." ).arg( defaultDescription );

    TQVBox * page = makeVBoxMainWidget();

    TQLabel * label;
    label = new TQLabel( text, page );

    m_combo = new KComboBox( page );
    TQStringList list = ArchiveFormatInfo::self()->allDescriptions();
    list.sort();
    m_combo->insertStringList( list );
    m_combo->setCurrentItem( list.findIndex( defaultDescription ) );
}

TQString ArchiveFormatDlg::mimeType()
{
    if (m_combo && !m_combo->currentText().isEmpty())
        return ArchiveFormatInfo::self()->mimeTypeForDescription( m_combo->currentText() );
    else
        return TQString();
}

#include "archiveformatdlg.moc"

