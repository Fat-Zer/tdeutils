/*  Dell i8K Hardware Monitor Plug-in for KSim
 *
 *  Copyright (C) 2003 Nadeem Hasan <nhasan@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KSIMI8K_H
#define KSIMI8K_H

#include <stdio.h>

#include <pluginmodule.h>

class TQTimer;
class TQTextIStream;
class TQRegExp;
class TQCheckBox;
class KIntNumInput;

namespace KSim
{
  class Label;
}

class I8KPlugin : public KSim::PluginObject
{
  public:
    I8KPlugin(const char *name);
    ~I8KPlugin();

    virtual KSim::PluginView *createView(const char *);
    virtual KSim::PluginPage *createConfigPage(const char *);

    virtual void showAbout();
};

class I8KView : public KSim::PluginView
{
  Q_OBJECT
  
  public:
    I8KView(KSim::PluginObject *parent, const char *name);
    ~I8KView();

    virtual void reparseConfig();

  protected:

    void initGUI();
    void closeStream();

  protected slots:

    void openStream();
    void updateView();

  private:

    TQString m_unit;
    int m_interval;

    KSim::Label *m_fan1Label, *m_fan2Label, *m_tempLabel;
    TQTimer *m_timer;
    FILE *m_procFile;
    TQTextIStream *m_procStream;
    TQRegExp *m_reData;
};

class I8TDEConfig : public KSim::PluginPage
{
  Q_OBJECT
  
  public:
    I8TDEConfig(KSim::PluginObject *parent, const char *name);
    ~I8TDEConfig();

    virtual void saveConfig();
    virtual void readConfig();

  private slots:

  private:

    TQCheckBox *m_unit;
    KIntNumInput *m_interval;
};

#endif // KSIMI8K_H

/* vim: et sw=2 ts=2
*/
