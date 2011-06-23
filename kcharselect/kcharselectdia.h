/******************************************************************/
/* KCharSelectDia - (c) by Reginald Stadlbauer 1999               */
/* Author: Reginald Stadlbauer                                    */
/* E-Mail: reggie@kde.org                                         */
/******************************************************************/

#ifndef kcharselectdia_h
#define kcharselectdia_h


#include <tqlayout.h>
#include <tqlineedit.h>
#include <tqclipboard.h>

#include <kpushbutton.h>
#include <kcharselect.h>
#include <kmainwindow.h>

static const char *version = "v1.1"; 

/******************************************************************/
/* class KCharSelectDia                                           */
/******************************************************************/

class KCharSelectDia : public KMainWindow
{
  Q_OBJECT
  TQ_OBJECT

public:
  KCharSelectDia(TQWidget *tqparent,const char *name,const TQChar &_chr,const TQString &_font,int _tableNum, bool direction);

  static bool selectChar(TQString &_font,TQChar &_chr,int _tableNum);

  int chr() { return vChr; }
  TQString font() { return vFont; }

protected:
  void closeEvent(TQCloseEvent *) { _exit(); }

  TQGridLayout *grid;
  KCharSelect *charSelect;
  TQLineEdit   *lined;
  KPushButton *bClip,*bClear;
  KPushButton *bHelp;

  TQChar vChr;
  TQString vFont;
  int pointSize;
  bool entryDirection;

protected slots:
  void charChanged(const TQChar &_chr);
  void fontSelected(const TQString &_font);
  void add()
  { add(vChr); }
  void add(const TQChar &_chr);
  void toClip();
  void toClipUTF8();
  void toClipHTML();
  void fromClip();
  void fromClipUTF8();
  void flipText();
  void toggleEntryDirection();
  void lineEditChanged();
  void _exit();
  void clear();
  void help();

};

#endif
