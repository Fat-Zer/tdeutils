/*
    KCalc, a scientific calculator for the X window system using the
    Qt widget libraries, available at no cost at http://www.troll.no

    Copyright (C) 1996 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef KCALC_H
#define KCALC_H

class QPushButton;
class QRadioButton;
class QButtonGroup;
class QHButtonGroup;
class QWidget;
class DispLogic;
class Constants;
#include <kmainwindow.h>

/*
  Kcalc basically consist of a class for the GUI (here), a class for
  the display (dlabel.h), and one for the mathematics core
  (kcalc_core.h).

  When for example '+' is pressed, one sends the contents of the
  Display and the '+' to the core via "core.Plus(DISPLAY_AMOUNT)".
  This only updates the core. To bring the changes to the display,
  use afterwards "UpdateDisplay(true)".

  "UpdateDisplay(true)" means that the amount to be displayed should
  be taken from the core (get the result of some operation that was
  performed), "UpdateDisplay(false)" has already the information, what
  to be display (e.g. user is typing in a number).  Note that in the
  last case the core does not know the number typed in until some
  operation button is pressed, e.g. "core.Plus(display_number)".
 */

#include "config.h"

// IMPORTANT this has to come after ../config.h
#include "kcalc_core.h"
#include "kcalc_button.h"
#include "kcalc_const_button.h"

class KCalculator : public KMainWindow
{
    Q_OBJECT

public:
	KCalculator(TQWidget *parent = 0, const char *name = 0);
	~KCalculator();

signals:
	void switchInverse(bool);
	void switchMode(ButtonModeFlags,bool);
	void switchShowAccels(bool);

private:
	virtual bool eventFilter( TQObject *o, TQEvent *e );
	void updateGeometry();
	void setupMainActions(void);
	void setupStatusbar(void);
	TQWidget *setupNumericKeys(TQWidget *parent);
	void setupLogicKeys(TQWidget *parent);
	void setupScientificKeys(TQWidget *parent);
	void setupStatisticKeys(TQWidget *parent);
	void setupConstantsKeys(TQWidget *parent);
	void keyPressEvent(TQKeyEvent *e);
	void keyReleaseEvent(TQKeyEvent *e);
	void set_precision();
	void set_style();
	void resetBase(void) { (BaseChooseGroup->find(1))->animateClick();};

	void UpdateDisplay(bool get_amount_from_core = false,
			   bool store_result_in_history = false);

protected slots:
    void changeButtonNames();
    void updateSettings();
    void set_colors();
    void EnterEqual();
    void showSettings();
    void slotStatshow(bool toggled);
    void slotScientificshow(bool toggled);
    void slotLogicshow(bool toggled);
    void slotConstantsShow(bool toggled);   
    void slotShowAll(void);
    void slotHideAll(void);
    void slotAngleSelected(int number);
    void slotBaseSelected(int number);
    void slotNumberclicked(int number_clicked);
    void slotEEclicked(void);
    void slotInvtoggled(bool myboolean);
    void slotMemRecallclicked(void);
    void slotMemStoreclicked(void);
    void slotSinclicked(void);
    void slotPlusMinusclicked(void);
    void slotMemPlusMinusclicked(void);
    void slotCosclicked(void);
    void slotReciclicked(void);
    void slotTanclicked(void);
    void slotFactorialclicked(void);
    void slotLogclicked(void);
    void slotSquareclicked(void);
    void slotLnclicked(void);
    void slotPowerclicked(void);
    void slotMCclicked(void);
    void slotClearclicked(void);
    void slotACclicked(void);
    void slotParenOpenclicked(void);
    void slotParenCloseclicked(void);
    void slotANDclicked(void);
    void slotXclicked(void);
    void slotDivisionclicked(void);
    void slotORclicked(void);
    void slotXORclicked(void);
    void slotPlusclicked(void);
    void slotMinusclicked(void);
    void slotLeftShiftclicked(void);
    void slotRightShiftclicked(void);
    void slotPeriodclicked(void);
    void slotEqualclicked(void);
    void slotPercentclicked(void);
    void slotRootclicked(void);
    void slotNegateclicked(void);
    void slotModclicked(void);
    void slotStatNumclicked(void);
    void slotStatMeanclicked(void);
    void slotStatStdDevclicked(void);
    void slotStatMedianclicked(void);
    void slotStatDataInputclicked(void);
    void slotStatClearDataclicked(void);
    void slotHyptoggled(bool flag);
    void slotConstclicked(int);

    void slotConstantToDisplay(int constant);
    void slotChooseScientificConst0(int option);
    void slotChooseScientificConst1(int option);
    void slotChooseScientificConst2(int option);
    void slotChooseScientificConst3(int option);
    void slotChooseScientificConst4(int option);
    void slotChooseScientificConst5(int option);

private:
	bool inverse;
	bool hyp_mode;
	KNumber memory_num;
	KNumber setvalue;

	// angle modes for trigonometric values
	enum {
	  DegMode,
	  RadMode,
	  GradMode
	} _angle_mode;


private:
    TQWidget *mSmallPage;
    TQWidget *mLargePage;
    TQWidget *mNumericPage;

    DispLogic*	calc_display; // for historic reasons in "dlabel.h"
    TQRadioButton*	pbBaseChoose[4];
    TQPushButton*	pbAngleChoose;
    TQDict<KCalcButton>	pbStat;
    TQDict<KCalcButton>	pbScientific;
    TQDict<KCalcButton>	pbLogic;
    KCalcConstButton*	pbConstant[10];
    KCalcButton* 	pbAC;
    KCalcButton* 	pbAND;
    KCalcButton* 	pbClear;
    KCalcButton* 	pbDivision;
    KCalcButton* 	pbEE;
    KCalcButton* 	pbEqual;
    KCalcButton* 	pbFactorial;
    KCalcButton* 	pbInv;
    KCalcButton* 	pbMC;
    KCalcButton* 	pbMinus;
    KCalcButton* 	pbMod;
    KCalcButton* 	pbMemPlusMinus;
    KCalcButton* 	pbMemRecall;
    KCalcButton*	pbMemStore;
    KCalcButton* 	pbOR;
    KCalcButton* 	pbParenClose;
    KCalcButton* 	pbParenOpen;
    KCalcButton* 	pbPercent;
    KCalcButton* 	pbPeriod;
    KCalcButton* 	pbPlus;
    KCalcButton* 	pbPlusMinus;
    KCalcButton* 	pbPower;
    KCalcButton* 	pbReci;
    KSquareButton* 	pbRoot;
    KCalcButton* 	pbSquare;
    KCalcButton* 	pbX;
    KCalcButton* 	pbXOR;

    Constants * tmp_const; // this is the dialog for configuring const
			   // buttons would like to remove this, but
			   // don't know how
	
    TQHButtonGroup*      BaseChooseGroup;
    // NumButtonGroup: 0-9 = digits, 0xA-0xF = hex-keys
    TQButtonGroup* 	NumButtonGroup;
    // ConstButtonGroup C1-C6
    TQButtonGroup*	ConstButtonGroup;

    KToggleAction *actionStatshow;
    KToggleAction *actionScientificshow;
    KToggleAction *actionLogicshow;
    KToggleAction *actionConstantsShow;

    TQPtrList<KCalcButton> mFunctionButtonList;
    TQPtrList<KCalcButton> mStatButtonList;
    TQPtrList<KCalcButton> mMemButtonList;
    TQPtrList<KCalcButton> mOperationButtonList;

    int				mInternalSpacing;

    CalcEngine core;
};

#endif  // KCALC_H
