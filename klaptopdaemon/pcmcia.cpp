/*
 * pcmcia.cpp
 *
 * Copyright (c) 1999 Paul Campbell <paul@taniwha.com>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
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

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>

#include <qfileinfo.h> 
#include <qlayout.h>

#include <kglobal.h>
#include <klocale.h>
#include <stdio.h>

#include "pcmcia.h"
#include "portable.h"
#include "version.h"

PcmciaConfig::PcmciaConfig (QWidget * parent, const char *name)
  : KCModule(parent, name)
{
       KAboutData *about =
       new KAboutData(I18N_NOOP("kcmlaptop"),
		   I18N_NOOP("KDE Panel System Information Control Module"),
		   0, 0, KAboutData::License_GPL,
		   I18N_NOOP("(c) 1999 - 2002 Paul Campbell"));
       about->addAuthor("Paul Campbell", 0, "paul@taniwha.com");
       setAboutData( about );

       KGlobal::locale()->insertCatalogue("klaptopdaemon"); // For translation of klaptopdaemon messages

       label0 = laptop_portable::pcmcia_info(0, this);
       label0_text = laptop_portable::pcmcia_info(1, this);
       label1 = laptop_portable::pcmcia_info(2, this);
       label1_text = laptop_portable::pcmcia_info(3, this);
	

       QVBoxLayout *top_layout = new QVBoxLayout(this, 15, 5);
        QGridLayout *top_grid = new QGridLayout(2, 2);
        top_layout->addLayout(top_grid);

        top_grid->setColStretch(0, 0);
        top_grid->setColStretch(1, 1);
        top_grid->addRowSpacing(0, 40);
        top_grid->addRowSpacing(1, 40);

        label0->setFixedSize(80, 24);
        top_grid->addWidget(label0, 0, 0);
        label0_text->adjustSize();
        top_grid->addWidget(label0_text, 0, 1);

        label1->setFixedSize(80, 24);
        top_grid->addWidget(label1, 1, 0);
        label1_text->adjustSize();
        top_grid->addWidget(label1_text, 1, 1);


        top_layout->addStretch(1);

        QHBoxLayout *v1 = new QHBoxLayout;
        top_layout->addLayout(v1, 0);
        v1->addStretch(1);
        QString s1 = LAPTOP_VERSION;
        QString s2 = i18n("Version: ")+s1;
        QLabel* vers = new QLabel(s2, this);
        vers->setMinimumSize(vers->sizeHint());
        v1->addWidget(vers, 0);

        top_layout->activate();          

	load();
	setButtons(Help);
}


void PcmciaConfig::save()
{
}

void PcmciaConfig::load()
{
}

void PcmciaConfig::defaults()
{
}


void PcmciaConfig::changed()
{
  emit KCModule::changed(true);
}


QString PcmciaConfig::quickHelp() const
{
  return i18n("<h1>PCMCIA Config</h1>This module shows information about "
	"the PCMCIA cards in your system, if there are PCMCIA cards.");
}

#include "pcmcia.moc"
