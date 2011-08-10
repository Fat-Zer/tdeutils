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

#include <tqtabdialog.h>

class TQVBoxLayout;
class TQGridLayout;
class TQCheckBox;
class TQGroupBox;
class TQLabel;
class TQWidget;
class KComboBox;
class KLineEdit;
class KURLRequester;

class NetDialog : public TQTabDialog
{
  Q_OBJECT
  TQ_OBJECT
  public:
    NetDialog(TQWidget *parent, const char *name = 0);
    ~NetDialog();

    const TQString deviceName() const;
    bool timer();
    const TQString format() const;
    bool commands();
    const TQString cCommand() const;
    const TQString dCommand() const;
    bool okClicked() { return m_clicked; }

  public slots:
    void setDeviceName(const TQString &);
    void setShowTimer(bool);
    void setFormat(const TQString &);
    void setShowCommands(bool);
    void setCCommand(const TQString &);
    void setDCommand(const TQString &);

  private slots:
    void sendClicked();

  private:
    TQStringList createList() const;

    bool m_clicked;
    TQWidget *m_generalTab;
    TQLabel *m_deviceLabel;
    KComboBox *m_deviceCombo;
    TQGroupBox *m_timerBox;
    TQCheckBox *m_showTimer;
    KLineEdit *m_timerEdit;
    TQLabel *m_hFormat;
    TQLabel *m_mFormat;
    TQLabel *m_sFormat;
    TQWidget *m_commandTab;
    TQCheckBox *m_enableCommands;
    TQLabel *m_cCommand;
    KURLRequester *m_connectRequester;
    TQLabel *m_dCommand;
    KURLRequester *m_disconnectRequester;

    TQGridLayout *m_generalLayout;
    TQVBoxLayout *m_timerBoxLayout;
    TQGridLayout *m_commandLayout;
};
#endif // NETDIALOG_H
