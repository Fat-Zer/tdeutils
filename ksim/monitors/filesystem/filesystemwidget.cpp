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

#include <tqlayout.h>
#include <tqpopupmenu.h>
#include <tqregexp.h>
#include <tqcursor.h>

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
   const TQString &point)
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

const TQString &FilesystemWidget::Filesystem::mountPoint() const
{
  return m_mountPoint;
}

const TQString &FilesystemWidget::Filesystem::text() const
{
  return m_display->text();
}

int FilesystemWidget::Filesystem::value() const
{
  return m_display->value();
}

void FilesystemWidget::Filesystem::setText(const TQString &text)
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

FilesystemWidget::FilesystemWidget(TQWidget *tqparent, const char *name)
   : TQWidget(tqparent, name)
{
  m_list.setAutoDelete(true);
  m_layout = new TQVBoxLayout(this);
  m_process = 0;
}

FilesystemWidget::~FilesystemWidget()
{
  delete m_process;
}

void FilesystemWidget::append(int max, const TQString &mountPoint)
{
  KSim::Progress *progress = new KSim::Progress(max, this);
  progress->installEventFilter(this);
  progress->show();
  m_layout->addWidget(progress);

  m_list.append(new Filesystem(progress, mountPoint));
}

void FilesystemWidget::setText(uint id, const TQString &text)
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

bool FilesystemWidget::eventFilter(TQObject *o, TQEvent *e)
{
  if (!o->isA("KSim::Progress"))
    return TQWidget::eventFilter(o, e);

  KSim::Progress *progressBar = 0;
  int i = 0;
  TQPtrListIterator<Filesystem> it(m_list);
  Filesystem *filesystem;
  while ((filesystem = it.current()) != 0) {
    ++it;

    if (TQT_BASE_OBJECT(filesystem->display()) == TQT_BASE_OBJECT(o)) {
      progressBar = filesystem->display();
      break;
    }

    ++i;
  }

  if (TQT_BASE_OBJECT(o) == TQT_BASE_OBJECT(progressBar) && e->type() == TQEvent::MouseButtonPress)
  {
    switch(TQT_TQMOUSEEVENT(e)->button()) {
      case Qt::RightButton:
        showMenu(i);
        break;
      default:
        break;
      case Qt::LeftButton:
        if (parentWidget()->inherits("KSim::PluginView"))
          static_cast<KSim::PluginView *>(parentWidget())->doCommand();
        break;
    }

    return true;
  }

  return TQWidget::eventFilter(o, e);
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

  TQStringList errorList = TQStringList::split("\n", m_stderrString);
  TQString message = i18n("<qt>The following errors occurred:<ul>");

  TQStringList::Iterator it;
  for (it = errorList.begin(); it != errorList.end(); ++it) {
    message += TQString::tqfromLatin1("<li>%1</li>")
       .tqarg((*it).replace(TQRegExp("[u]?mount: "), TQString()));
  }

  message += TQString::tqfromLatin1("</ul></qt>");
  KMessageBox::sorry(0, message);
}

void FilesystemWidget::createProcess(const TQString &command, const TQString &point)
{
  m_process = new KProcess();
  connect(m_process,
     TQT_SIGNAL(receivedStderr(KProcess *, char *, int)),
     TQT_SLOT(receivedStderr(KProcess *, char *, int)));
  connect(m_process,
     TQT_SIGNAL(processExited(KProcess *)),
     TQT_SLOT(processExited(KProcess *)));

  (*m_process) << command << point;
  void(m_process->start(KProcess::NotifyOnExit, KProcess::Stderr));
}

void FilesystemWidget::showMenu(uint id)
{
  if (id > m_list.count())
    return;

  TQPopupMenu menu;
  menu.insertItem(SmallIcon("hdd_mount"), i18n("&Mount Device"), 1);
  menu.insertItem(SmallIcon("hdd_unmount"), i18n("&Unmount Device"), 2);

  switch (menu.exec(TQCursor::pos())) {
    case 1:
      createProcess("mount", m_list.at(id)->mountPoint());
      break;
    case 2:
      createProcess("umount", m_list.at(id)->mountPoint());
      break;
  }
}

#include "filesystemwidget.moc"
