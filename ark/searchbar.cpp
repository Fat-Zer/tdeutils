/*
 * ark: A program for modifying archives via a GUI.
 *
 * Copyright (C) 2004, Henrique Pinto <henrique.pinto@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "searchbar.h"

#include <tdelistviewsearchline.h>
#include <tdelistview.h>
#include <kcombobox.h>
#include <tdelocale.h>
#include <tdeaction.h>
#include <tdeactioncollection.h>

#include <tqlabel.h>
#include <tqapplication.h>
#include <tqvaluelist.h>

SearchBar::SearchBar( TQWidget* parent, TDEActionCollection* aC, const char * name )
	: TDEListViewSearchLine( parent, 0, name )
{
	TDEAction *resetSearch = new TDEAction( i18n( "Reset Search" ), TQApplication::reverseLayout() ? "clear_left" : "locationbar_erase", 0, TQT_TQOBJECT(this), TQT_SLOT( clear() ), aC, "reset_search" );

	resetSearch->plug( parent );
	resetSearch->setWhatsThis( i18n( "Reset Search\n"
	                                 "Resets the search bar, so that all archive entries are shown again." ) );
}

SearchBar::~SearchBar()
{
}

#include "searchbar.moc"
