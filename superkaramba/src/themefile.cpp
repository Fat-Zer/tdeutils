/****************************************************************************
*  themefile.cpp - Theme file handling
*
*  Copyright (C) 2003 Hans Karlsson <karlsson.h@home.se>
*  Copyright (C) 2003-2004 Adam Geitgey <adam@rootnode.org>
*  Copyright (c) 2004 Petri Damst� <damu@iki.fi>
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
#include "themefile.h"
#include "lineparser.h"
#include "themelocale.h"
#include <kdebug.h>
#include <kurl.h>
#include <kzip.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kio/netaccess.h>
#include <tqtextstream.h>
#include <tqfileinfo.h>
#include <tqdom.h>
#include <tqdir.h>

class ZipFile
{
  public:
    ZipFile() :
      m_zip(0), m_file(0)
    {
    }
    void setFile(const TQString& filename)
    {
      m_filename = filename;
      if(filename.isEmpty())
        return;

      const KArchiveEntry* entry;

      entry = m_dir->entry(filename);
      if(entry == 0 || !entry->isFile())
      {
        m_file = 0;
        return;
      }
      m_file = static_cast<const KArchiveFile*>(entry);
    }
    void setZip(const TQString& zipfile)
    {
      closeZip();

      m_zip = new KZip(zipfile);

      if(!m_zip->open(IO_ReadOnly))
      {
        qDebug("Unable to open '%s' for reading.", zipfile.ascii());
        return;
      }
      m_dir = m_zip->directory();
      if(m_dir == 0)
      {
        qDebug("Error reading directory contents of file %s", zipfile.ascii());
        return;
      }
    }

    virtual ~ZipFile()
    {
      closeZip();
    }

    void closeZip()
    {
      if(m_zip)
      {
        m_zip->close();
        delete  m_zip;
      }
    }

    TQByteArray data()
    {
      if(m_file)
        return m_file->data();
      else
      {
        if(!m_filename.isEmpty())
          qDebug("Error reading file %s from zip", m_filename.ascii());
        return TQByteArray();
      }
    }

    bool exists()
    {
      return (m_file != 0);
    }

  private:
    KZip* m_zip;
    const KArchiveFile* m_file;
    TQString m_filename;
    const KArchiveDirectory* m_dir;
};

ThemeFile::ThemeFile(const KURL& url)
  : m_stream(0), m_locale(0), m_zip(0)
{
  if(url.isValid())
    set(url);
}

ThemeFile::~ThemeFile()
{
  delete m_stream;
  delete m_locale;
  delete m_zip;
}

bool ThemeFile::open()
{
  bool result = false;

  close();

  if(m_zipTheme)
  {
    m_zip->setFile(m_theme);
    m_ba = m_zip->data();
    if(m_ba.size() > 0)
    {
      m_stream = new TQTextStream(m_ba, IO_ReadOnly);
      result = true;
    }
  }
  else
  {
    m_fl.setName(m_file);

    if(m_fl.open(IO_ReadOnly|IO_Translate))
    {
      m_stream = new TQTextStream(&m_fl);        // use a text stream
      result = true;
    }
  }
  return result;
}

bool ThemeFile::nextLine(LineParser& parser)
{
  parser.set("");

  if(m_stream)
  {
    TQString result = m_stream->readLine();

    if(result.isNull())
      return false;
    parser.set(result);
    return true;
  }
  return false;
}

bool ThemeFile::close()
{
  if(m_stream)
  {
    delete m_stream;
    m_stream = 0;
    m_fl.close();
    m_ba.resize(0);
    return true;
  }
  return false;
}

bool ThemeFile::isValid() const
{
  return (exists() && !m_name.isEmpty() && !m_theme.isEmpty());
}

bool ThemeFile::exists() const
{
  TQFileInfo file(m_file);
  return file.exists();
}

TQPixmap ThemeFile::icon() const
{
  return TQPixmap(readThemeFile(m_icon));
}

bool ThemeFile::set(const KURL &url)
{
  if(!url.isLocalFile() && !url.protocol().isEmpty())
  {
    if(KMessageBox::warningContinueCancel(TQT_TQWIDGET(kapp->activeWindow()),
       i18n("You are about to install and run %1 SuperKaramba theme. Since "
            "themes can contain executable code you should only install themes "
            "from sources that you trust. Continue?"), i18n("Executable Code Warning"), i18n("Install")
           .arg(url.prettyURL()))
       == KMessageBox::Cancel)
    {
      return false;
    }

    TQDir themeDir(locateLocal("appdata", "themes/", true));
    TQFileInfo localFile = themeDir.filePath(url.fileName());

    if(localFile.exists())
    {
      if(KMessageBox::warningContinueCancel(TQT_TQWIDGET(kapp->activeWindow()),
         i18n("%1 already exists. Do you want to overwrite it?")
             .arg(localFile.filePath()),i18n("File Exists"),i18n("Overwrite"))
         == KMessageBox::Cancel)
      {
        return false;
      }
    }
    if(!KIO::NetAccess::file_copy(url, localFile.filePath(), -1, true,
      false, kapp->mainWidget()))
    {
      return false;
    }
    m_file = localFile.filePath();
  }
  else
  {
    if(url.directory().isEmpty() || url.directory() == "/")
      m_file = canonicalFile(TQDir::current().filePath(url.fileName()));
    else
      m_file = canonicalFile(url.path());
    if(!exists())
      return false;
  }

  TQFileInfo fi(m_file);

  m_name = fi.baseName( TRUE );
  m_theme = m_name + ".theme";
  m_python = m_name;
  m_id = m_name;

  if(isZipFile(m_file))
  {
    m_path = m_file;
    m_zipTheme = true;
    m_zip = new ZipFile();
    m_zip->setZip(m_file);
  }
  else
  {
    m_path = fi.dirPath(true) + "/";
    m_zipTheme = false;
  }
  parseXml();

  TQFileInfo fimo(m_python);
  if(m_python.isEmpty())
    fimo.setFile(m_theme);
  else
    fimo.setFile(m_python);
  m_mo = fimo.baseName( TRUE );

  m_locale = new ThemeLocale(this);
  return isValid();
}

void ThemeFile::parseXml()
{
  if(!fileExists("maindata.xml"))
    return;
  TQByteArray ba = readThemeFile("maindata.xml");
  TQDomDocument doc("superkaramba_theme");
  doc.setContent(ba);
  TQDomElement element = doc.documentElement();

  TQDomNode n = element.firstChild();
  while(!n.isNull())
  {
    TQDomElement e = n.toElement();
    if(!e.isNull())
    {
      if(e.tagName() == "name")
        m_name = e.text();
      else if(e.tagName() == "themefile")
        m_theme = e.text();
      else if(e.tagName() == "python_module")
      {
        m_python = e.text();
        if(m_python.right(3).lower() == ".py")
          m_python.remove(m_python.length() - 3, 3);
      }
      else if(e.tagName() == "description")
        m_description = e.text();
      else if(e.tagName() == "author")
        m_author = e.text();
      else if(e.tagName() == "author_email")
        m_authorEmail = e.text();
      else if(e.tagName() == "homepage")
        m_homepage = e.text();
      else if(e.tagName() == "icon")
        m_icon = e.text();
      else if(e.tagName() == "version")
        m_version = e.text();
      else if(e.tagName() == "license")
        m_license = e.text();
    }
    n = n.nextSibling();
  }
}

bool ThemeFile::canUninstall() const
{
  TQFileInfo fi(file());
  if(fi.permission(TQFileInfo::WriteUser) ||
     fi.permission(TQFileInfo::WriteGroup) ||
     fi.permission(TQFileInfo::WriteOther))
    return true;
  return false;
}

bool ThemeFile::isThemeFile(const TQString& filename) const
{
  TQFileInfo fileInfo(filename);

  return fileInfo.isRelative();
}

bool ThemeFile::fileExists(const TQString& filename) const
{
  if(isThemeFile(filename))
  {
    if(isZipTheme())
    {
      m_zip->setFile(filename);
      return m_zip->exists();
    }
    else
      return TQFileInfo(path() + "/" + filename).exists();
  }
  else
    return TQFileInfo(filename).exists();
}

TQByteArray ThemeFile::readThemeFile(const TQString& filename) const
{
  //TQTime time;
  //time.start();
  TQByteArray ba;

  if(isZipTheme())
  {
    m_zip->setFile(filename);
    ba = m_zip->data();
  }
  else
  {
    TQFile file(path() + "/" + filename);

    if(file.open(IO_ReadOnly))
    {
      ba = file.readAll();
      file.close();
    }
  }
  //kdDebug() << "Read theme file: " << filename << ", " << time.elapsed()
  //    << "ms" << endl;
  return ba;
}

bool ThemeFile::isZipFile(const TQString& filename)
{
  TQFile file(filename);

  if(file.open(IO_ReadOnly))
  {
    unsigned char buf[5];

    if(file.readBlock((char*)buf, 4) == 4)
    {
      if(buf[0] == 'P' && buf[1] == 'K' && buf[2] == 3 && buf[3] == 4)
        return true;
    }
  }
  return false;
}

bool ThemeFile::pythonModuleExists() const
{
  return (!m_python.isEmpty() && fileExists(m_python + ".py"));
}

TQString ThemeFile::canonicalFile(const TQString& file)
{
  // Get absolute path with NO symlinks
  TQFileInfo fi(file);
  return TQDir(fi.dir().canonicalPath()).filePath(fi.fileName());
}
