/****************************************************************************
*  themefile.h - Theme file handling
*
*  Copyright (C) 2003 Hans Karlsson <karlsson.h@home.se>
*  Copyright (C) 2003-2004 Adam Geitgey <adam@rootnode.org>
*  Copyright (c) 2004 Petri Damstén <damu@iki.fi>
*
*  This file is part of SuperKaramba.
*
*  SuperKaramba is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  SuperKaramba is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with SuperKaramba; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
****************************************************************************/
#ifndef THEMEFILE_H
#define THEMEFILE_H

#include <kurl.h>
#include <tqstring.h>
#include <tqcstring.h>
#include <tqpixmap.h>
#include <tqfile.h>
#include <tqvaluevector.h>

class LineParser;
class TQTextStream;
class ThemeLocale;
class ZipFile;

/**
@author See README for the list of authors
*/
class ThemeFile
{
  public:
    typedef TQValueVector<ThemeFile> List;

    ThemeFile(const KURL& url = KURL());
    ~ThemeFile();

    bool isZipTheme() const { return m_zipTheme; };
    const TQString& name() const { return m_name; };
    const TQString& version() const { return m_version; };
    const TQString& license() const { return m_license; };
    const TQString& id() const { return m_id; };
    const TQString& mo() const { return m_mo; };
    const TQString& file() const { return m_file; };
    const TQString& pythonModule() const { return m_python; };
    bool pythonModuleExists() const;
    const TQString& path() const { return m_path; };
    const TQString& description() const { return m_description; };
    const TQString& author() const { return m_author; };
    const TQString& authorEmail() const { return m_authorEmail; };
    const TQString& homepage() const { return m_homepage; };
    TQPixmap icon() const;
    bool exists() const;
    bool isThemeFile(const TQString& filename) const;
    bool isValid() const;
    TQByteArray readThemeFile(const TQString& filename) const;
    bool fileExists(const TQString& filename) const;
    const ThemeLocale* locale() const { return m_locale; };
    bool canUninstall() const;

    bool set(const KURL& url);
    bool open();
    bool nextLine(LineParser& parser);
    bool close();

    static bool isZipFile(const TQString& filename);
    static TQString canonicalFile(const TQString& file);

  private:
    void parseXml();
    void mkdir(TQDir dir);

    TQString m_path;
    bool m_zipTheme;
    TQString m_file;
    TQString m_id;
    TQString m_mo;
    TQString m_name;
    TQString m_theme;
    TQString m_python;
    TQString m_icon;
    TQString m_version;
    TQString m_license;
    TQTextStream* m_stream;
    TQByteArray m_ba;
    TQFile m_fl;
    TQString m_description;
    TQString m_author;
    TQString m_authorEmail;
    TQString m_homepage;
    ThemeLocale* m_locale;
    ZipFile* m_zip;
};

#endif
