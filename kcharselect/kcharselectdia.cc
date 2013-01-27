/******************************************************************/
/* KCharSelectDia - (c) by Reginald Stadlbauer 1999               */
/* Author: Reginald Stadlbauer                                    */
/* E-Mail: reggie@kde.org                                         */
/*         RTL support by Bryce Nesbitt                           */
/******************************************************************/

#include "kcharselectdia.moc"

#include <stdlib.h>

#include <kdialog.h>
#include <kapplication.h>
#include <kaccel.h>
#include <tdeconfig.h>
#include <klocale.h>
#include <kaction.h>

/******************************************************************/
/* class KCharSelectDia                                           */
/******************************************************************/

//==================================================================
KCharSelectDia::KCharSelectDia(TQWidget *parent,const char *name,
			       const TQChar &_chr,const TQString &_font,
			       int _tableNum, bool direction)
  : KMainWindow(parent,name), vChr(_chr), vFont(_font)
{
  setCaption(TQString()); // Standard caption

  TQWidget *mainWidget = new TQWidget(this);
  setCentralWidget(mainWidget);

  grid = new TQGridLayout( mainWidget, 3, 4, KDialog::marginHint(), KDialog::spacingHint() );

  // Add character selection widget from library tdeui
  charSelect = new KCharSelect(mainWidget,"",vFont,vChr,_tableNum);
  charSelect->resize(charSelect->sizeHint());
  connect(charSelect,TQT_SIGNAL(highlighted(const TQChar &)),
	  TQT_SLOT(charChanged(const TQChar &)));
  connect(charSelect,TQT_SIGNAL(activated(const TQChar &)),
	  TQT_SLOT(add(const TQChar &)));
  connect(charSelect,TQT_SIGNAL(fontChanged(const TQString &)),
	  TQT_SLOT(fontSelected(const TQString &)));
  grid->addMultiCellWidget(charSelect, 0, 0, 0, 3);

  // Build line editor
  lined = new TQLineEdit(mainWidget);
  lined->resize(lined->sizeHint());

  TQFont font = lined->font();
  font.setFamily( vFont );
  lined->setFont( font );

  connect(lined,TQT_SIGNAL(textChanged(const TQString &)),
	  TQT_SLOT(lineEditChanged()));
  grid->addMultiCellWidget(lined, 1, 1, 0, 3);

  // Build some buttons
  bHelp = new KPushButton( KStdGuiItem::help(), mainWidget );
  connect(bHelp,TQT_SIGNAL(clicked()),this,TQT_SLOT(help()));
  bHelp->setFixedSize( bHelp->sizeHint() );
  grid->addWidget( bHelp, 2, 0 );

  TQSpacerItem *space = new TQSpacerItem( 20, 20, TQSizePolicy::Expanding );
  grid->addItem( space, 2, 1 );

  bClear = new KPushButton( KStdGuiItem::clear(), mainWidget );
  connect(bClear,TQT_SIGNAL(clicked()),this,TQT_SLOT(clear()));
  bClear->setFixedSize( bClear->sizeHint() );
  grid->addWidget( bClear, 2, 2 );

  bClip = new KPushButton( KGuiItem( i18n( "&To Clipboard" ),
            "editcopy" ), mainWidget );
  bClip->setFixedSize( bClip->sizeHint() );
  connect(bClip,TQT_SIGNAL(clicked()),this,TQT_SLOT(toClip()));
  grid->addWidget( bClip, 2, 3 );

  // Build menu
  KStdAction::quit( TQT_TQOBJECT(this), TQT_SLOT(_exit()), actionCollection() );
  
  new KAction(i18n("&To Clipboard"), "editcopy",
         KStdAccel::shortcut(KStdAccel::Copy), TQT_TQOBJECT(this), TQT_SLOT(toClip()), actionCollection(), "copy_clip" );

  (void)new KAction(i18n("To Clipboard &UTF-8"), 0, TQT_TQOBJECT(this),
    TQT_SLOT(toClipUTF8()), actionCollection(), "copy_utf_8" );
  (void)new KAction(i18n("To Clipboard &HTML"), 0, TQT_TQOBJECT(this),
      TQT_SLOT(toClipHTML()), actionCollection(), "copy_html" );
 
  new KAction(i18n("&From Clipboard"), "editpaste",
         KStdAccel::shortcut(KStdAccel::Paste), TQT_TQOBJECT(this), TQT_SLOT(fromClip()), actionCollection(), "from_clip" );
  (void)new KAction(i18n("From Clipboard UTF-8"), 0, TQT_TQOBJECT(this),
      TQT_SLOT(fromClipUTF8()), actionCollection(), "from_clip_utf8" );
  
  i18n("From Clipboard HTML");      // Intended for future use
  
  KStdAction::clear(TQT_TQOBJECT(this), TQT_SLOT(clear()), actionCollection(), "clear");
  (void)new KAction(i18n("&Flip"), 0, TQT_TQOBJECT(this),
      TQT_SLOT(flipText()), actionCollection(), "flip" );
  (void)new KAction(i18n("&Alignment"), 0, TQT_TQOBJECT(this),
      TQT_SLOT(toggleEntryDirection()), actionCollection(), "alignment" );
  
  charSelect->setFocus();

  entryDirection = direction;
  if( entryDirection )
    lined->setAlignment( TQt::AlignRight );
  else
    lined->setAlignment( TQt::AlignLeft );

  setupGUI(Keys|StatusBar|Save|Create);
}

//==================================================================
void KCharSelectDia::charChanged(const TQChar &_chr)
{
  vChr = _chr;
}

//==================================================================
void KCharSelectDia::fontSelected(const TQString &_font)
{
  charSelect->setFont(_font);

  TQFont font = lined->font();
  font.setFamily( _font );
  lined->setFont( font );

  vFont = _font;
}

//==================================================================
void KCharSelectDia::add(const TQChar &_chr)
{
  TQString str;
  int cursorPos;

  charChanged(_chr);

  str       = lined->text();
  cursorPos = lined->cursorPosition();
  str.insert( cursorPos, vChr );
  lined->setText(str);
  cursorPos++;
  lined->setCursorPosition( cursorPos );
}

//==================================================================
void KCharSelectDia::toClip()
{
  TQClipboard *cb = TQApplication::clipboard();
  cb->setSelectionMode( true );
  cb->setText(lined->text());
  cb->setSelectionMode( false );
  cb->setText(lined->text());
}

//==================================================================
// UTF-8 is rapidly becoming the favored 8-bit encoding for
// Unicode (iso10646-1).
//
void KCharSelectDia::toClipUTF8()
{
  TQClipboard *cb = TQApplication::clipboard();
  TQString str = lined->text();
  cb->setText(str.utf8());
}

//==================================================================
//  Put valid HTML 4.0 into the clipboard.  Valid ISO-8859-1 Latin 1
//  characters are left undisturbed.  Everything else, including the
//  "c0 control characters" often used by Windows, are clipped
//  as a HTML entity.
//
void KCharSelectDia::toClipHTML()
{
  TQClipboard *cb = TQApplication::clipboard();
  TQString input;
  TQString html;
  TQString tempstring;
  TQChar   tempchar;
  uint i;

  input = lined->text();
  for(i=0; i< input.length(); i++ )
    {
      tempchar = input.at(i);
      if(  tempchar.latin1() && ((tempchar.unicode() < 128) || (tempchar.unicode() >= 128+32)) )
        {
          html.append(input.at(i));
        }
      else
        {
          html.append(tempstring.sprintf("&#x%x;", tempchar.unicode()));
        }
    }
  cb->setText(html);
}

//==================================================================
//
void KCharSelectDia::fromClip()
{
  TQClipboard *cb = TQApplication::clipboard();
  lined->setText( cb->text() );
}

//==================================================================
// UTF-8 is rapidly becoming the favored 8-bit encoding for
// Unicode (iso10646-1).  This function is handy for decoding
// UTF-8 found in legacy applications, consoles, filenames, webpages,
// etc.
//
void KCharSelectDia::fromClipUTF8()
{
  TQClipboard *cb = TQApplication::clipboard();
  TQString str = cb->text();

  lined->setText( str.fromUtf8( str.latin1() ) );
}

//==================================================================
//  Reverse the text held in the line edit buffer.  This is crucial
//  for dealing with visual vs. logical representations of
//  right to left languages, and handy for working around all
//  manner of legacy character order issues.
//
void KCharSelectDia::flipText()
{
  TQString input;
  TQString output;
  uint i;

  input = lined->text();
  for(i=0; i< input.length(); i++ )
    {
      output.prepend( input.at(i) );
    }
  lined->setText(output);
}

//==================================================================
void KCharSelectDia::toggleEntryDirection()
{
    entryDirection ^= 1;
    if( entryDirection )
        lined->setAlignment( TQt::AlignRight );
    else
        lined->setAlignment( TQt::AlignLeft );
}

//==================================================================
void KCharSelectDia::lineEditChanged()
{
    if( entryDirection )
      {
        if(lined->cursorPosition())
            lined->setCursorPosition( lined->cursorPosition() - 1 );
      }
}

//==================================================================
void KCharSelectDia::_exit()
{
  TDEConfig *config = kapp->config();

  config->setGroup("General");
  config->writeEntry("selectedFont",vFont);
  config->writeEntry("char",static_cast<int>(vChr.unicode()));
  config->writeEntry("table",charSelect->tableNum());
  config->writeEntry("entryDirection",entryDirection);
  config->sync();

  delete this;
  exit(0);
}

//==================================================================
void KCharSelectDia::clear()
{
  lined->clear();
}

//==================================================================
void KCharSelectDia::help()
{
  kapp->invokeHelp();
}

