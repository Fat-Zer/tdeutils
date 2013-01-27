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
#include <tdeconfig.h>
#include <kdebug.h>
#include <kinputdialog.h>

#include <tqtimer.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqcheckbox.h>
#include <tqcursor.h>
#include <tqpopupmenu.h>
#include <tqpushbutton.h>

class SensorViewItem : public TQCheckListItem
{
  public:
    SensorViewItem(TQListView *parent, const TQString &text1,
       const TQString &text2, const TQString &text3,
       const TQString &text4)
       : TQCheckListItem(parent, text1, CheckBox)
    {
      setText(1, text2);
      setText(2, text3);
      setText(3, text4);
    }
};

SensorsConfig::SensorsConfig(KSim::PluginObject *parent, const char *name)
   : KSim::PluginPage(parent, name)
{
  m_layout = new TQGridLayout(this);
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
  connect(m_sensorView, TQT_SIGNAL(contextMenu(KListView *,
     TQListViewItem *, const TQPoint &)), this, TQT_SLOT(menu(KListView *,
     TQListViewItem *, const TQPoint &)));

  connect( m_sensorView, TQT_SIGNAL( doubleClicked( TQListViewItem * ) ),
     TQT_SLOT( modify( TQListViewItem * ) ) );

  m_layout->addMultiCellWidget(m_sensorView, 1, 1, 0, 3);

  m_modify = new TQPushButton( this );
  m_modify->setText( i18n( "Modify..." ) );
  connect( m_modify, TQT_SIGNAL( clicked() ), TQT_SLOT( modify() ) );
  m_layout->addMultiCellWidget( m_modify, 2, 2, 3, 3 );

  m_fahrenBox = new TQCheckBox(i18n("Display Fahrenheit"), this);
  m_layout->addMultiCellWidget(m_fahrenBox, 3, 3, 0, 3);

  m_updateLabel = new TQLabel(this);
  m_updateLabel->setText(i18n("Update interval:"));
  m_updateLabel->setSizePolicy(TQSizePolicy(TQSizePolicy::Fixed,
     TQSizePolicy::Fixed));
  m_layout->addMultiCellWidget(m_updateLabel, 4, 4, 0, 0);

  m_sensorSlider = new KIntSpinBox(this);
  m_layout->addMultiCellWidget(m_sensorSlider, 4, 4, 1, 1);

  TQLabel *intervalLabel = new TQLabel(this);
  intervalLabel->setText(i18n("seconds"));
  intervalLabel->setSizePolicy(TQSizePolicy(TQSizePolicy::Fixed,
     TQSizePolicy::Fixed));
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

  for (TQListViewItemIterator it(m_sensorView); it.current(); ++it) {
    config()->setGroup("Sensors");
    config()->writeEntry(it.current()->text(2),
       TQString::number(static_cast<TQCheckListItem *>(it.current())->isOn())
       + ":" + it.current()->text(1));
  }
}

void SensorsConfig::readConfig()
{
  config()->setGroup("Sensors");
  m_fahrenBox->setChecked(config()->readBoolEntry("displayFahrenheit", false));
  m_sensorSlider->setValue(config()->readNumEntry("sensorUpdateValue", 15));

  TQStringList names;
  for (TQListViewItemIterator it(m_sensorView); it.current(); ++it) {
    config()->setGroup("Sensors");
    names = TQStringList::split(":", config()->readEntry(it.current()->text(2), "0:"));
    if (!names[1].isNull())
      it.current()->setText(1, names[1]);
    static_cast<TQCheckListItem *>(it.current())->setOn(names[0].toInt());
  }
}

void SensorsConfig::menu(KListView *, TQListViewItem *, const TQPoint &)
{
  m_popupMenu = new TQPopupMenu(this);

  m_popupMenu->insertItem(i18n("Select All"), 1);
  m_popupMenu->insertItem(i18n("Unselect All"), 2);
  m_popupMenu->insertItem(i18n("Invert Selection"), 3);

  switch (m_popupMenu->exec(TQCursor::pos())) {
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
  for (TQListViewItemIterator it(m_sensorView); it.current(); ++it)
    static_cast<TQCheckListItem *>(it.current())->setOn(true);
}

void SensorsConfig::unSelectAll()
{
  for (TQListViewItemIterator it(m_sensorView); it.current(); ++it)
    static_cast<TQCheckListItem *>(it.current())->setOn(false);
}

void SensorsConfig::invertSelect()
{
  for (TQListViewItemIterator it(m_sensorView); it.current(); ++it) {
    TQCheckListItem *item = static_cast<TQCheckListItem *>(it.current());
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
  TQString label;
  TQStringList sensorInfo;
  SensorList::ConstIterator it;
  for (it = sensorList.begin(); it != sensorList.end(); ++it) {
    label.sprintf("%02i", ++i);
    (void) new SensorViewItem(m_sensorView, label,
       (*it).sensorName(), (*it).sensorType() + "/" + (*it).sensorName(),
       (*it).sensorValue() + (*it).sensorUnit());
  }

  TQStringList names;
  for (TQListViewItemIterator it(m_sensorView); it.current(); ++it) {
    config()->setGroup("Sensors");
    names = TQStringList::split(":", config()->readEntry(it.current()->text(2), "0:"));
    if (!names[1].isNull())
      it.current()->setText(1, names[1]);
    static_cast<TQCheckListItem *>(it.current())->setOn(names[0].toInt());
  }
}

void SensorsConfig::modify( TQListViewItem * item )
{
  if ( !item )
    return;

  bool ok = false;
  TQString text = KInputDialog::getText( i18n( "Modify Sensor Label" ), i18n( "Sensor label:" ),
     item->text( 1 ), &ok, this );

  if ( ok )
    item->setText( 1, text );
}

void SensorsConfig::modify()
{
  modify( m_sensorView->selectedItem() );
}

void SensorsConfig::showEvent(TQShowEvent *)
{
  if (m_neverShown) {
    initSensors();
    m_neverShown = false;
  }
  else {
    const SensorList &list = SensorBase::self()->sensorsList();
    SensorList::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it) {
      TQListViewItem *item = m_sensorView->findItem((*it).sensorName(), 1);
      if (item)
        item->setText(3, (*it).sensorValue() + (*it).sensorUnit());
    }
  }
}
