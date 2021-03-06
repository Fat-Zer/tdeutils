//
//
// C++ Implementation: $MODULE$
//
// Description:
//
//
// Author: Gav Wood <gav@kde.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <kicondialog.h>
#include <kiconloader.h>
#include <kpushbutton.h>

#include "editmode.h"

EditMode::EditMode(TQWidget *parent, const char *name, bool modal, WFlags fl) : EditModeBase(parent, name, modal, fl)
{
	theIcon->setIconType(TDEIcon::Panel, TDEIcon::Any);
}

EditMode::~EditMode()
{
}

void EditMode::slotClearIcon()
{
	theIcon->resetIcon();
}

void EditMode::slotCheckText(const TQString &newText)
{
	theOK->setEnabled(!newText.isEmpty());
}

#include "editmode.moc"
