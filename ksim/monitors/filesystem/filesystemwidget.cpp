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

#include "filesystemwidget.h"

#include <pluginmodule.h>

#include <qlayout.h>
#include <qpopupmenu.h>
#include <qregexp.h>
#include <qcursor.h>

#include <kprocess.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>

FilesystemWidget::Filesystem::Filesystem()
{
  m_display = 0;
}

FilesystemWidget::Filesystem::Filesystem(KSim::Progress *display,
   const QString &point)
{
  m_display = display;
  m_mountPoint = point;
}

FilesystemWidget::Filesystem::~Filesystem()
{
  delete m_display;
}

KSim::Progress *FilesystemWidget::Filesystem::display() const
{
  return m_display;
}

const QString &FilesystemWidget::Filesystem::mountPoint() const
{
  return m_mountPoint;
}

const QString &FilesystemWidget::Filesystem::text() const
{
  return m_display->text();
}

int FilesystemWidget::Filesystem::value() const
{
  return m_display->value();
}

void FilesystemWidget::Filesystem::setText(const QString &text)
{
  if (!m_display)
    return;

  m_display->setText(text);
}

void FilesystemWidget::Filesystem::setValue(int value)
{
  if (!m_display)
    return;

  m_display->setValue(value);
}

FilesystemWidget::FilesystemWidget(QWidget *parent, const char *name)
   : QWidget(parent, name)
{
  m_list.setAutoDelete(true);
  m_layout = new QVBoxLayout(this);
  m_process = 0;
}

FilesystemWidget::~FilesystemWidget()
{
  delete m_process;
}

void FilesystemWidget::append(int max, const QString &mountPoint)
{
  KSim::Progress *progress = new KSim::Progress(max, this);
  progress->installEventFilter(this);
  progress->show();
  m_layout->addWidget(progress);

  m_list.append(new Filesystem(progress, mountPoint));
}

void FilesystemWidget::setText(uint id, const QString &text)
{
  if (id > m_list.count())
    return;

  m_list.at(id)->setText(text);
}

void FilesystemWidget::setValue(uint id, int value)
{
  if (id > m_list.count())
    return;

  m_list.at(id)->setValue(value);
}

void FilesystemWidget::clear()
{
  m_list.clear();
}

bool FilesystemWidget::eventFilter(QObject *o, QEvent *e)
{
  if (!o->isA("KSim::Progress"))
    return QWidget::eventFilter(o, e);

  KSim::Progress *progressBar = 0;
  int i = 0;
  QPtrListIterator<Filesystem> it(m_list);
  Filesystem *filesystem;
  while ((filesystem = it.current()) != 0) {
    ++it;

    if (filesystem->display() == o) {
      progressBar = filesystem->display();
      break;
    }

    ++i;
  }

  if (o == progressBar && e->type() == QEvent::MouseButtonPress)
  {
    switch(static_cast<QMouseEvent *>(e)->button()) {
      case QMouseEvent::RightButton:
        showMenu(i);
        break;
      default:
        break;
      case QMouseEvent::LeftButton:
        if (parentWidget()->inherits("KSim::PluginView"))
          static_cast<KSim::PluginView *>(parentWidget())->doCommand();
        break;
    }

    return true;
  }

  return QWidget::eventFilter(o, e);
}

void FilesystemWidget::receivedStderr(KProcess *, char *buffer, int length)
{
  m_stderrString.setLatin1(buffer, length);
}

void FilesystemWidget::processExited(KProcess *)
{
  delete m_process;
  m_process = 0;
  kdDebug(2003) << "Deleting KProcess pointer" << endl;

  if (m_stderrString.isEmpty())
    return;

  QStringList errorList = QStringList::split("\n", m_stderrString);
  QString message = i18n("<qt>The following errors occurred:<ul>");

  QStringList::Iterator it;
  for (it = errorList.begin(); it != errorList.end(); ++it) {
    message += QString::fromLatin1("<li>%1</li>")
       .arg((*it).replace(QRegExp("[u]?mount: "), QString::null));
  }

  message += QString::fromLatin1("</ul></qt>");
  KMessageBox::sorry(0, message);
}

void FilesystemWidget::createProcess(const QString &command, const QString &point)
{
  m_process = new KProcess();
  connect(m_process,
     SIGNAL(receivedStderr(KProcess *, char *, int)),
     SLOT(receivedStderr(KProcess *, char *, int)));
  connect(m_process,
     SIGNAL(processExited(KProcess *)),
     SLOT(processExited(KProcess *)));

  (*m_process) << command << point;
  void(m_process->start(KProcess::NotifyOnExit, KProcess::Stderr));
}

void FilesystemWidget::showMenu(uint id)
{
  if (id > m_list.count())
    return;

  QPopupMenu menu;
  menu.insertItem(SmallIcon("hdd_mount"), i18n("&Mount Device"), 1);
  menu.insertItem(SmallIcon("hdd_unmount"), i18n("&Unmount Device"), 2);

  switch (menu.exec(QCursor::pos())) {
    case 1:
      createProcess("mount", m_list.at(id)->mountPoint());
      break;
    case 2:
      createProcess("umount", m_list.at(id)->mountPoint());
      break;
  }
}

#include "filesystemwidget.moc"
