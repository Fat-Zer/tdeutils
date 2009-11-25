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

#include "sensorsconfig.h"
#include "sensorsconfig.moc"
#include "sensorbase.h"

#include <klocale.h>
#include <kiconloader.h>
#include <klistview.h>
#include <knuminput.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kinputdialog.h>

#include <qtimer.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qcursor.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>

class SensorViewItem : public QCheckListItem
{
  public:
    SensorViewItem(QListView *parent, const QString &text1,
       const QString &text2, const QString &text3,
       const QString &text4)
       : QCheckListItem(parent, text1, CheckBox)
    {
      setText(1, text2);
      setText(2, text3);
      setText(3, text4);
    }
};

SensorsConfig::SensorsConfig(KSim::PluginObject *parent, const char *name)
   : KSim::PluginPage(parent, name)
{
  m_layout = new QGridLayout(this);
  m_layout->setSpacing(6);
  m_neverShown = true;

  m_sensorView = new KListView(this);
  m_sensorView->addColumn(i18n("No."));
  m_sensorView->addColumn(i18n("Label"));
  m_sensorView->addColumn(i18n("Sensors"));
  m_sensorView->addColumn(i18n("Value"));
  m_sensorView->setColumnWidth(0, 40);
  m_sensorView->setColumnWidth(1, 60);
  m_sensorView->setColumnWidth(2, 80);
  m_sensorView->setAllColumnsShowFocus(true);
  connect(m_sensorView, SIGNAL(contextMenu(KListView *,
     QListViewItem *, const QPoint &)), this, SLOT(menu(KListView *,
     QListViewItem *, const QPoint &)));

  connect( m_sensorView, SIGNAL( doubleClicked( QListViewItem * ) ),
     SLOT( modify( QListViewItem * ) ) );

  m_layout->addMultiCellWidget(m_sensorView, 1, 1, 0, 3);

  m_modify = new QPushButton( this );
  m_modify->setText( i18n( "Modify..." ) );
  connect( m_modify, SIGNAL( clicked() ), SLOT( modify() ) );
  m_layout->addMultiCellWidget( m_modify, 2, 2, 3, 3 );

  m_fahrenBox = new QCheckBox(i18n("Display Fahrenheit"), this);
  m_layout->addMultiCellWidget(m_fahrenBox, 3, 3, 0, 3);

  m_updateLabel = new QLabel(this);
  m_updateLabel->setText(i18n("Update interval:"));
  m_updateLabel->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,
     QSizePolicy::Fixed));
  m_layout->addMultiCellWidget(m_updateLabel, 4, 4, 0, 0);

  m_sensorSlider = new KIntSpinBox(this);
  m_layout->addMultiCellWidget(m_sensorSlider, 4, 4, 1, 1);

  QLabel *intervalLabel = new QLabel(this);
  intervalLabel->setText(i18n("seconds"));
  intervalLabel->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,
     QSizePolicy::Fixed));
  m_layout->addMultiCellWidget(intervalLabel, 4, 4, 2, 2);
}

SensorsConfig::~SensorsConfig()
{
}

void SensorsConfig::saveConfig()
{
  config()->setGroup("Sensors");
  config()->writeEntry("sensorUpdateValue", m_sensorSlider->value());
  config()->writeEntry("displayFahrenheit", m_fahrenBox->isChecked());

  for (QListViewItemIterator it(m_sensorView); it.current(); ++it) {
    config()->setGroup("Sensors");
    config()->writeEntry(it.current()->text(2),
       QString::number(static_cast<QCheckListItem *>(it.current())->isOn())
       + ":" + it.current()->text(1));
  }
}

void SensorsConfig::readConfig()
{
  config()->setGroup("Sensors");
  m_fahrenBox->setChecked(config()->readBoolEntry("displayFahrenheit", false));
  m_sensorSlider->setValue(config()->readNumEntry("sensorUpdateValue", 15));

  QStringList names;
  for (QListViewItemIterator it(m_sensorView); it.current(); ++it) {
    config()->setGroup("Sensors");
    names = QStringList::split(":", config()->readEntry(it.current()->text(2), "0:"));
    if (!names[1].isNull())
      it.current()->setText(1, names[1]);
    static_cast<QCheckListItem *>(it.current())->setOn(names[0].toInt());
  }
}

void SensorsConfig::menu(KListView *, QListViewItem *, const QPoint &)
{
  m_popupMenu = new QPopupMenu(this);

  m_popupMenu->insertItem(i18n("Select All"), 1);
  m_popupMenu->insertItem(i18n("Unselect All"), 2);
  m_popupMenu->insertItem(i18n("Invert Selection"), 3);

  switch (m_popupMenu->exec(QCursor::pos())) {
    case 1:
      selectAll();
      break;
    case 2:
      unSelectAll();
      break;
    case 3:
      invertSelect();
      break;
  }

  delete m_popupMenu;
}

void SensorsConfig::selectAll()
{
  for (QListViewItemIterator it(m_sensorView); it.current(); ++it)
    static_cast<QCheckListItem *>(it.current())->setOn(true);
}

void SensorsConfig::unSelectAll()
{
  for (QListViewItemIterator it(m_sensorView); it.current(); ++it)
    static_cast<QCheckListItem *>(it.current())->setOn(false);
}

void SensorsConfig::invertSelect()
{
  for (QListViewItemIterator it(m_sensorView); it.current(); ++it) {
    QCheckListItem *item = static_cast<QCheckListItem *>(it.current());
    if (item->isOn())
      item->setOn(false);
    else
      item->setOn(true);
  }
}

void SensorsConfig::initSensors()
{
  const SensorList &sensorList = SensorBase::self()->sensorsList();

  int i = 0;
  QString label;
  QStringList sensorInfo;
  SensorList::ConstIterator it;
  for (it = sensorList.begin(); it != sensorList.end(); ++it) {
    label.sprintf("%02i", ++i);
    (void) new SensorViewItem(m_sensorView, label,
       (*it).sensorName(), (*it).sensorType() + "/" + (*it).sensorName(),
       (*it).sensorValue() + (*it).sensorUnit());
  }

  QStringList names;
  for (QListViewItemIterator it(m_sensorView); it.current(); ++it) {
    config()->setGroup("Sensors");
    names = QStringList::split(":", config()->readEntry(it.current()->text(2), "0:"));
    if (!names[1].isNull())
      it.current()->setText(1, names[1]);
    static_cast<QCheckListItem *>(it.current())->setOn(names[0].toInt());
  }
}

void SensorsConfig::modify( QListViewItem * item )
{
  if ( !item )
    return;

  bool ok = false;
  QString text = KInputDialog::getText( i18n( "Modify Sensor Label" ), i18n( "Sensor label:" ),
     item->text( 1 ), &ok, this );

  if ( ok )
    item->setText( 1, text );
}

void SensorsConfig::modify()
{
  modify( m_sensorView->selectedItem() );
}

void SensorsConfig::showEvent(QShowEvent *)
{
  if (m_neverShown) {
    initSensors();
    m_neverShown = false;
  }
  else {
    const SensorList &list = SensorBase::self()->sensorsList();
    SensorList::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it) {
      QListViewItem *item = m_sensorView->findItem((*it).sensorName(), 1);
      if (item)
        item->setText(3, (*it).sensorValue() + (*it).sensorUnit());
    }
  }
}
