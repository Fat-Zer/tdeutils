/*

 $Id$

 KCalc 

 Copyright (C) Bernd Johannes Wuebben
               wuebben@math.cornell.edu
	       wuebben@kde.org

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 */


#ifndef _D_LABEL_H_
#define _D_LABEL_H_

#include <tqvaluevector.h>
#include "kcalcdisplay.h"

class CalcEngine;
class TDEAction;
class TDEActionCollection;

class DispLogic : public KCalcDisplay
{
Q_OBJECT
  

public:
	DispLogic(TQWidget *parent, const char *name,
		  TDEActionCollection *coll);
	~DispLogic();

	void changeSettings();
	void EnterDigit(int data);
	void update_from_core(CalcEngine const &core,
			      bool store_result_in_history = false);

private slots:
	void history_back(void);
	void history_forward(void);

private:
	TQValueVector<KNumber> _history_list;
	int _history_index;

	TDEAction *_forward;
	TDEAction *_back;
};

#endif
