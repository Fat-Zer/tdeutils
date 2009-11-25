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

#include <qptrlist.h>

class KProcess;
class QVBoxLayout;

class FilesystemWidget : public QWidget
{
  Q_OBJECT
  public:
    FilesystemWidget(QWidget *parent, const char *name);
    ~FilesystemWidget();

    void append(int, const QString &);
    void setText(uint, const QString &);
    void setValue(uint, int);
    void clear();

  protected:
    bool eventFilter(QObject *, QEvent *);

  private slots:
    void receivedStderr(KProcess *, char *, int);
    void processExited(KProcess *);

  private:
    class Filesystem
    {
      public:
        Filesystem();
        Filesystem(KSim::Progress *, const QString &);
        ~Filesystem();

        KSim::Progress *display() const;
        const QString &mountPoint() const;
        const QString &text() const;
        int value() const;

        void setText(const QString &);
        void setValue(int);

        QString m_mountPoint;
        KSim::Progress *m_display;
    };

    void createProcess(const QString &, const QString &);
    void showMenu(uint);

    typedef QPtrList<Filesystem> ProgressList;
    ProgressList m_list;
    QVBoxLayout *m_layout;
    KProcess *m_process;
    QString m_stderrString;
};
#endif
