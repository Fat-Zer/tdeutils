/*
 *  Copyright (c) 2002-2003 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/
#ifdef TQT_ONLY
  #include "compat.h"
#else
  #include <klineeditdlg.h>
  #include <klocale.h>
  #include <kmessagebox.h>
  #include <kstandarddirs.h>
  #include <kdebug.h>
  #include "userdefinedregexps.moc"
#endif

#include "userdefinedregexps.h"
#include <tqheader.h>
#include <tqpopupmenu.h>
#include <tqdir.h>
#include "widgetfactory.h"
#include "compoundregexp.h"
#include <tqlayout.h>
#include <tqlabel.h>

UserDefinedRegExps::UserDefinedRegExps( TQWidget *tqparent, const char *name )
  : TQDockWindow( TQDockWindow::InDock, tqparent, name)
{
  TQWidget* top = new TQWidget( this );
  TQVBoxLayout* lay = new TQVBoxLayout( top, 6 );
  lay->setAutoAdd( true );

  TQLabel* label = new TQLabel( i18n("Compound regular expressions:"), top );

  // This is to avoid that the label set the minimum width for the window.
  label->setMinimumSize(1,0);

  _userDefined = new TQListView( top, "UserDefinedRegExps::_userDefined" );
  _userDefined->addColumn( TQString() );
  _userDefined->header()->hide();
  //  _userDefined->setRootIsDecorated( true );
  setWidget( top );
  slotPopulateUserRegexps();

  connect( _userDefined, TQT_SIGNAL(clicked(TQListViewItem*)), this, TQT_SLOT(slotLoad(TQListViewItem*)) );
  connect( _userDefined, TQT_SIGNAL(rightButtonPressed(TQListViewItem*,const TQPoint&, int )),
           this, TQT_SLOT( slotEdit( TQListViewItem*, const TQPoint& ) ) );
}

void UserDefinedRegExps::slotPopulateUserRegexps()
{
  _userDefined->clear();
  _regExps.clear();

  createItems( i18n("User Defined"), WidgetWinItem::path(), true );

#ifdef TQT_ONLY
  TQStringList dirs;
  dirs << TQString::tqfromLatin1( "predefined" );
#else
  TQStringList dirs = KGlobal::dirs()->findDirs( "data", TQString::fromLocal8Bit("kregexpeditor/predefined/") );
#endif

  for ( TQStringList::iterator it1 = dirs.begin(); it1 != dirs.end(); ++it1 ) {
    TQDir dir( *it1, TQString(), TQDir::Name, TQDir::Dirs );
    TQStringList subdirs = dir.entryList();
    for ( TQStringList::iterator it2 = subdirs.begin(); it2 != subdirs.end(); ++it2 ) {
      if ( *it2 == TQString::fromLocal8Bit(".") || *it2 == TQString::fromLocal8Bit("..") )
        continue;
      createItems( *it2, *it1 + TQString::fromLocal8Bit("/") + *it2, false );
    }
  }

}

void UserDefinedRegExps::createItems( const TQString& _title, const TQString& dir, bool usersRegExp )
{
  TQString title = _title;
  if (_title == TQString::tqfromLatin1("general"))
	  title = i18n("General");

  TQListViewItem* lvItem = new TQListViewItem( _userDefined, title );
  lvItem->setOpen( true );

  TQDir directory( dir );
  TQStringList files = directory.entryList( TQString::fromLocal8Bit("*.regexp") );
  for ( TQStringList::Iterator it = files.begin(); it != files.end(); ++it ) {
    TQString fileName = dir + TQString::fromLocal8Bit("/") + *it;

    TQFile file( fileName );
    if ( ! file.open(IO_ReadOnly) ) {
      KMessageBox::sorry( this, i18n("Could not open file for reading: %1").tqarg(fileName) );
      continue;
    }

    TQTextStream stream( &file );
    TQString data = stream.read();
    file.close();

    RegExp* regexp = WidgetFactory::createRegExp( data );
    if ( ! regexp ) {
      KMessageBox::sorry( this, i18n("File %1 containing user defined regular expression contained an error").tqarg( fileName ) );
      continue;
    }

    new WidgetWinItem( *it, regexp, usersRegExp, lvItem );

    // Insert the regexp into the list of compound regexps
    if ( regexp->type() == RegExp::COMPOUND ) {
      CompoundRegExp* cregexp = dynamic_cast<CompoundRegExp*>( regexp );
      if ( cregexp && cregexp->allowReplace() )
        _regExps.append( cregexp );
    }
  }
}

const TQPtrList<CompoundRegExp> UserDefinedRegExps::regExps() const
{
  return _regExps;
}


void UserDefinedRegExps::slotUnSelect()
{
  _userDefined->clearSelection();
}

void UserDefinedRegExps::slotLoad(TQListViewItem* item)
{
  if ( !item || ! dynamic_cast<WidgetWinItem*>(item) ) {
    // Mouse pressed outside a widget.
    return;
  }

  WidgetWinItem* wwi = dynamic_cast<WidgetWinItem*>(item);
  if (wwi) {
    emit load( wwi->regExp() );
  }
}

void UserDefinedRegExps::slotEdit( TQListViewItem* item, const TQPoint& pos )
{
  TQPopupMenu* menu = new TQPopupMenu( this );
  menu->insertItem(i18n("Delete"), 1 );
  menu->insertItem(i18n("Rename..."), 2 );
  if ( !item || ! dynamic_cast<WidgetWinItem*>( item ) ) {
    // menu not selected on an item
    menu->setItemEnabled( 1, false );
    menu->setItemEnabled( 2, false );
  }
  else {
    // Only allow rename and delete of users own regexps.
    WidgetWinItem* winItem = dynamic_cast<WidgetWinItem*>( item );
    if ( winItem && ! winItem->isUsersRegExp() ) {
      menu->setItemEnabled( 1, false );
      menu->setItemEnabled( 2, false );
    }
  }

  int which = menu->exec( pos );

  if ( which == 1 ) { // Delete
    WidgetWinItem* winItem = dynamic_cast<WidgetWinItem*>( item );
    Q_ASSERT( winItem );
    TQFile file( winItem->fileName() );
    Q_ASSERT( file.exists() );
    file.remove();
    delete item;
  }
  else if ( which == 2 ) { // Rename
    WidgetWinItem* winItem = dynamic_cast<WidgetWinItem*>( item );
    Q_ASSERT( winItem );

    TQString oldFile = winItem->fileName();
    TQString oldName = winItem->name();

    TQString txt;
#ifdef TQT_ONLY
    txt = TQInputDialog::getText( tr("Rename Regular Expression"), tr("New name:") );
#else
    KLineEditDlg dlg(i18n("New name:"), oldName, this);
    dlg.setCaption(i18n("Rename Item"));
    bool ok = dlg.exec();
    if ( ok )
        txt = dlg.text();
#endif
    if ( !txt.isNull() && oldName != txt ) {
      TQString fileName = WidgetWinItem::path() + TQString::fromLocal8Bit("/") + txt + TQString::fromLocal8Bit(".regexp");
      TQFileInfo finfo( fileName );
      if ( finfo.exists() ) {
        int answer = KMessageBox::warningYesNo( this, i18n("<p>Overwrite named regular expression <b>%1</b>?</p>").tqarg(txt), TQString(), i18n("Overwrite"), i18n("Do Not Overwrite") );
        if ( answer != KMessageBox::Yes )
          return;

        // An item with this name already exists.
        delete winItem;
      }
      else
        winItem->setName( txt );
      TQDir dir;
      dir.rename( oldFile, fileName  );
    }
  }


  delete menu;
}

void UserDefinedRegExps::slotSelectNewAction()
{
  slotUnSelect();
}

WidgetWinItem::WidgetWinItem( TQString fileName, RegExp* regexp, bool usersRegExp, TQListViewItem* tqparent )
  :TQListViewItem( tqparent ), _regexp( regexp ), _usersRegExp ( usersRegExp )
{
  int index = fileName.tqfindRev(TQString::fromLocal8Bit(".regexp"));
  _name = fileName.left(index);

  setText( 0, _name );
}

TQString WidgetWinItem::fileName() const
{
  return path() + TQString::fromLocal8Bit("/") +_name + TQString::fromLocal8Bit(".regexp");
}

RegExp* WidgetWinItem::regExp() const
{
  return _regexp;
}

TQString WidgetWinItem::name() const
{
  return _name;
}

void WidgetWinItem::setName( const TQString& nm )
{
  _name = nm;
  setText( 0, nm );
}

TQString WidgetWinItem::path()
{
#ifdef TQT_ONLY
    return TQString::tqfromLatin1( "predefined" );
#else
  return locateLocal("data", TQString::fromLocal8Bit("KRegExpEditor/"));
#endif
}



