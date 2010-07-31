/*

 ark -- archiver for the KDE project

 Copyright (C) 2003 Georg Robbers <Georg.Robbers@urz.uni-hd.de>

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

#ifndef ARCHIVEFORMATINFO_H
#define ARCHIVEFORMATINFO_H

#include "arch.h"
#include <kmimetype.h>

class ArchiveFormatInfo
{
private:
    ArchiveFormatInfo();

public:
    static ArchiveFormatInfo * self();
    TQString filter();
    const TQStringList supportedMimeTypes( bool includeCompressed = true );
    TQStringList allDescriptions();
    ArchType archTypeForMimeType( const TQString & mimeType );
    ArchType archTypeByExtension( const TQString & archname );
    ArchType archTypeForURL( const KURL & url );
    TQString mimeTypeForDescription( const TQString & description );
    TQString findMimeType( const KURL & url );
    TQString descriptionForMimeType( const TQString & mimeType );
    TQString defaultExtension( const TQString & mimeType );
    bool wasUnknownExtension();

private:
    void buildFormatInfos();
    void addFormatInfo( ArchType type, TQString mime, TQString stdExt );

    struct FormatInfo
    {
        TQStringList extensions;
        TQStringList mimeTypes;
        TQStringList allDescriptions;
        TQStringList defaultExtensions;
        TQString description;
        enum ArchType type;
    };

    FormatInfo & find ( ArchType type );

    typedef TQValueList<FormatInfo> InfoList;
    InfoList m_formatInfos;

    bool m_lastExtensionUnknown;

    static ArchiveFormatInfo * m_pSelf;
};

#endif // ARCHIVEFORMATINFO_H

