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

#ifndef NETDIALOG_H
#define NETDIALOG_H

#include <qtabdialog.h>

class QVBoxLayout;
class QGridLayout;
class QCheckBox;
class QGroupBox;
class QLabel;
class QWidget;
class KComboBox;
class KLineEdit;
class KURLRequester;

class NetDialog : public QTabDialog
{
  Q_OBJECT
  public:
    NetDialog(QWidget *parent, const char *name = 0);
    ~NetDialog();

    const QString deviceName() const;
    bool timer();
    const QString format() const;
    bool commands();
    const QString cCommand() const;
    const QString dCommand() const;
    bool okClicked() { return m_clicked; }

  public slots:
    void setDeviceName(const QString &);
    void setShowTimer(bool);
    void setFormat(const QString &);
    void setShowCommands(bool);
    void setCCommand(const QString &);
    void setDCommand(const QString &);

  private slots:
    void sendClicked();

  private:
    QStringList createList() const;

    bool m_clicked;
    QWidget *m_generalTab;
    QLabel *m_deviceLabel;
    KComboBox *m_deviceCombo;
    QGroupBox *m_timerBox;
    QCheckBox *m_showTimer;
    KLineEdit *m_timerEdit;
    QLabel *m_hFormat;
    QLabel *m_mFormat;
    QLabel *m_sFormat;
    QWidget *m_commandTab;
    QCheckBox *m_enableCommands;
    QLabel *m_cCommand;
    KURLRequester *m_connectRequester;
    QLabel *m_dCommand;
    KURLRequester *m_disconnectRequester;

    QGridLayout *m_generalLayout;
    QVBoxLayout *m_timerBoxLayout;
    QGridLayout *m_commandLayout;
};
#endif // NETDIALOG_H
