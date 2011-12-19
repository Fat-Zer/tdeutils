/*
 * Copyright (C) 2005 Petri Damstén <petri.damsten@iki.fi>
 *
 * This file is part of SuperKaramba.
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

#ifndef THEMESDLG_H
#define THEMESDLG_H

#include <themes_layout.h>
#include "karambaapp.h"

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

/**
@author See README for the list of authors
*/
#ifdef HAVE_KNEWSTUFF
class SKNewStuff;
#endif
class ThemeFile;
class KArchiveDirectory;

class ThemesDlg : public ThemesLayout
{
    Q_OBJECT
  TQ_OBJECT

  public:
    ThemesDlg(TQWidget *parent = 0, const char *name = 0);
    ~ThemesDlg();

    int addTheme(const TQString &appId, const TQString &file);
    void removeTheme(const TQString &appId, const TQString &file, int instance);
    int addThemeToList(const TQString &file);
    void addSkzThemeToDialog(const TQString &file);
    void addThemeToDialog(const KArchiveDirectory *archiveDir, const TQString& destDir);
    void writeNewStuffConfig(const TQString &file);
    void configSanityCheck();
    bool isDownloaded(const TQString &path);
    void saveUserAddedThemes();
    TQStringList runningThemes();

  protected slots:
    virtual void addToDesktop();
    virtual void selectionChanged(int);
    virtual void openLocalTheme();
    virtual void getNewStuff();
    virtual void search(const TQString& text);
    virtual void uninstall();

  protected:
    static bool filter(int index, TQWidget* widget, void* data);
    void populateListbox();
    int themeIndex(TQString file);
    TQStringList themes();

#ifdef HAVE_KNEWSTUFF
  private:
    SKNewStuff *mNewStuff;
    TQStringList m_newStuffStatus;
#endif
};

#endif
