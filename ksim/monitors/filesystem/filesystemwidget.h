/*  ksim - a system monitor for kde
 *
 *  Copyright (C) 2001  Robbie Ward <linuxphreak@gmx.co.uk>
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
 
#ifndef FSYSTEM_WIDGET_H
#define FSYSTEM_WIDGET_H

#include <progress.h>

#include <tqptrlist.h>

class TDEProcess;
class TQVBoxLayout;

class FilesystemWidget : public TQWidget
{
  Q_OBJECT
  
  public:
    FilesystemWidget(TQWidget *parent, const char *name);
    ~FilesystemWidget();

    void append(int, const TQString &);
    void setText(uint, const TQString &);
    void setValue(uint, int);
    void clear();

  protected:
    bool eventFilter(TQObject *, TQEvent *);

  private slots:
    void receivedStderr(TDEProcess *, char *, int);
    void processExited(TDEProcess *);

  private:
    class Filesystem
    {
      public:
        Filesystem();
        Filesystem(KSim::Progress *, const TQString &);
        ~Filesystem();

        KSim::Progress *display() const;
        const TQString &mountPoint() const;
        const TQString &text() const;
        int value() const;

        void setText(const TQString &);
        void setValue(int);

        TQString m_mountPoint;
        KSim::Progress *m_display;
    };

    void createProcess(const TQString &, const TQString &);
    void showMenu(uint);

    typedef TQPtrList<Filesystem> ProgressList;
    ProgressList m_list;
    TQVBoxLayout *m_layout;
    TDEProcess *m_process;
    TQString m_stderrString;
};
#endif
