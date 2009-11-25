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

#include "netdialog.h"
#include "netdialog.moc"

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qfile.h>
#include <qregexp.h>

#include <klocale.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <klineedit.h>
#include <kapplication.h>
#include <kurlrequester.h>
#include <kstdguiitem.h>

#ifdef __FreeBSD__
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/route.h>
#endif

#include <string.h>

NetDialog::NetDialog(QWidget *parent, const char *name)
   : QTabDialog(parent, name, true)
{
  m_clicked = false;
  setCaption(kapp->makeStdCaption(i18n("Network Interface")));

  m_generalTab = new QWidget(this);
  m_generalLayout = new QGridLayout(m_generalTab);
  m_generalLayout->setSpacing(6);
  m_generalLayout->setMargin(11);

  m_deviceLabel = new QLabel(m_generalTab);
  m_deviceLabel->setText(i18n("Interface:"));
  m_generalLayout->addMultiCellWidget(m_deviceLabel, 0, 0, 0, 0);

  m_deviceCombo = new KComboBox(true, m_generalTab);
  m_deviceCombo->setSizePolicy(QSizePolicy(QSizePolicy::Preferred,
     QSizePolicy::Fixed));
  m_deviceCombo->setFocus();
  m_deviceCombo->setDuplicatesEnabled(false);
  m_generalLayout->addMultiCellWidget(m_deviceCombo, 0, 0, 1, 1);

  QStringList output(createList());
  if (output.isEmpty()) {
    m_deviceCombo->insertItem("ppp0");
    m_deviceCombo->insertItem("eth0");
  }
  else
    m_deviceCombo->insertStringList(output);

  QSpacerItem *deviceSpacer = new QSpacerItem(20, 20,
     QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_generalLayout->addMultiCell(deviceSpacer, 0, 0, 2, 2);

  m_timerBox = new QGroupBox(m_generalTab);
  m_timerBox->setTitle(i18n("Timer"));
  m_timerBox->setColumnLayout(0, Qt::Vertical);
  m_timerBox->layout()->setSpacing(0);
  m_timerBox->layout()->setMargin(0);
  m_timerBoxLayout = new QVBoxLayout(m_timerBox->layout());
  m_timerBoxLayout->setAlignment(Qt::AlignTop);
  m_timerBoxLayout->setSpacing(6);
  m_timerBoxLayout->setMargin(11);

  m_timerEdit = new KLineEdit(m_timerBox);
  m_timerEdit->setText("hh:mm:ss");
  m_timerEdit->setEnabled(false);

  m_showTimer = new QCheckBox(m_timerBox);
  m_showTimer->setText(i18n("Show timer"));
  connect(m_showTimer, SIGNAL(toggled(bool)),
      m_timerEdit, SLOT(setEnabled(bool)));
  m_timerBoxLayout->addWidget(m_showTimer);
  m_timerBoxLayout->addWidget(m_timerEdit);

  m_hFormat = new QLabel(m_timerBox);
  m_hFormat->setText(i18n("hh - Total hours online"));
  m_timerBoxLayout->addWidget(m_hFormat);

  m_mFormat = new QLabel(m_timerBox);
  m_mFormat->setText(i18n("mm - Total minutes online"));
  m_timerBoxLayout->addWidget(m_mFormat);

  m_sFormat = new QLabel(m_timerBox);
  m_sFormat->setText(i18n("ss - Total seconds online"));
  m_timerBoxLayout->addWidget(m_sFormat);
  m_generalLayout->addMultiCellWidget(m_timerBox, 1, 1, 0, 2);

  QSpacerItem *spacer = new QSpacerItem(20, 20,
      QSizePolicy::Minimum, QSizePolicy::Expanding);
  m_generalLayout->addMultiCell(spacer, 2, 2, 0, 0);
  addTab(m_generalTab, i18n("General"));

  m_commandTab = new QWidget(this);
  m_commandLayout = new QGridLayout(m_commandTab);
  m_commandLayout->setSpacing(6);
  m_commandLayout->setMargin(11);

  m_enableCommands = new QCheckBox(m_commandTab);
  m_enableCommands->setText(i18n("Enable connect/disconnect"));
  m_commandLayout->addMultiCellWidget(m_enableCommands, 0, 0, 0, 2);

  m_cCommand = new QLabel(m_commandTab);
  m_cCommand->setText(i18n("Connect command:"));
  m_commandLayout->addMultiCellWidget(m_cCommand, 1, 1, 0, 0);

  m_connectRequester = new KURLRequester(m_commandTab);
  m_connectRequester->setMinimumSize(145, 0);
  m_connectRequester->setEnabled(false);
  connect(m_enableCommands, SIGNAL(toggled(bool)),
      m_connectRequester, SLOT(setEnabled(bool)));
  m_commandLayout->addMultiCellWidget(m_connectRequester, 1, 1, 1, 2);

  m_dCommand = new QLabel(m_commandTab);
  m_dCommand->setText(i18n("Disconnect command:"));
  m_commandLayout->addMultiCellWidget(m_dCommand, 2, 2, 0, 0);

  m_disconnectRequester = new KURLRequester(m_commandTab);
  m_disconnectRequester->setMinimumSize(145, 0);
  m_disconnectRequester->setEnabled(false);
  connect(m_enableCommands, SIGNAL(toggled(bool)),
      m_disconnectRequester, SLOT(setEnabled(bool)));
  m_commandLayout->addMultiCellWidget(m_disconnectRequester, 2, 2, 1, 2);

  QSpacerItem *commandSpacer = new QSpacerItem(20, 20,
      QSizePolicy::Minimum, QSizePolicy::Expanding);
  m_commandLayout->addItem(commandSpacer);
  addTab(m_commandTab, i18n("Commands"));

  setOkButton(KStdGuiItem::ok().text());
  setCancelButton(KStdGuiItem::cancel().text());
  connect(this, SIGNAL(applyButtonPressed()), SLOT(sendClicked()));
}

NetDialog::~NetDialog()
{
}

const QString NetDialog::deviceName() const
{
  return m_deviceCombo->currentText();
}

bool NetDialog::timer()
{
  return m_showTimer->isChecked();
}

const QString NetDialog::format() const
{
  return m_timerEdit->text();
}

bool NetDialog::commands()
{
  return m_enableCommands->isChecked();
}

const QString NetDialog::cCommand() const
{
  return m_connectRequester->url();
}

const QString NetDialog::dCommand() const
{
  return m_disconnectRequester->url();
}

void NetDialog::setDeviceName(const QString &text)
{
  m_deviceCombo->setCurrentItem(text, true);
}

void NetDialog::setShowTimer(bool value)
{
  m_showTimer->setChecked(value);
}

void NetDialog::setFormat(const QString &format)
{
  m_timerEdit->setText(format);
}

void NetDialog::setShowCommands(bool value)
{
  m_enableCommands->setChecked(value);
}

void NetDialog::setCCommand(const QString &text)
{
  m_connectRequester->setURL(text);
}

void NetDialog::setDCommand(const QString &text)
{
  m_disconnectRequester->setURL(text);
}

void NetDialog::sendClicked()
{
  m_clicked = true;
  kdDebug(2003) << "ok was clicked" << endl;
}

QStringList NetDialog::createList() const
{
#ifdef __linux__
  QFile file("/proc/net/dev");
  if (!file.open(IO_ReadOnly))
    return QStringList();

  QStringList output;
  QTextStream textStream(&file);
  while (!textStream.atEnd())
    output.append(textStream.readLine());

  if (output.isEmpty())
    return QStringList();

  output.pop_front();
  output.pop_front();

  QStringList::Iterator it;
  QStringList list;
  for (it = output.begin(); it != output.end(); ++it) {
    list = QStringList::split(' ', (*it));
    (*it) = list[0].stripWhiteSpace();
    (*it).truncate((*it).find(':'));
  }

  return output;
#endif

#ifdef __FreeBSD__
  QStringList output;
  int mib[] = { CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST, 0 };
  char *buf = 0;
  int alloc = 0;
  struct if_msghdr *ifm, *nextifm;
  struct sockaddr_dl *sdl;
  char *lim, *next;
  size_t needed;
  char s[32];

  if (sysctl(mib, 6, NULL, &needed, NULL, 0) < 0)
    return QStringList();

  if (alloc < needed) {
    buf = new char[needed];

    if (buf == NULL)
      return QStringList();

    alloc = needed;
  }

  if (sysctl(mib, 6, buf, &needed, NULL, 0) < 0)
    return QStringList();

  lim = buf + needed;

  next = buf;
  while (next < lim) {
    ifm = (struct if_msghdr *)next;
    if (ifm->ifm_type != RTM_IFINFO)
      return QStringList();

    next += ifm->ifm_msglen;

    // get an interface with a network address
    while (next < lim) {
      nextifm = (struct if_msghdr *)next;
      if (nextifm->ifm_type != RTM_NEWADDR)
        break;

      next += nextifm->ifm_msglen;
    }

    // if the interface is up
    if (ifm->ifm_flags & IFF_UP) {
      sdl = (struct sockaddr_dl *)(ifm + 1);
      if (sdl->sdl_family != AF_LINK)
        continue;

      strncpy(s, sdl->sdl_data, sdl->sdl_nlen);
      s[sdl->sdl_nlen] = '\0';

      output.append(s);
    }
  }

  if (buf)
    delete[] buf;

  return output;
#endif
}
