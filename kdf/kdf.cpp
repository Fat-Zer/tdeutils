/*
 * kdf.cpp - KDiskFree
 *
 * written 1998-2001 by Michael Kropfberger <michael.kropfberger@gmx.net>
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

#include <tdeapplication.h>
#include <tdestdaccel.h>
#include <tdecmdlineargs.h>
#include <tdeaboutdata.h>
#include <kstdaction.h>
#include <tdeaction.h>

#include "kdf.h"
#include <tdepopupmenu.h>

static const char description[] =
	I18N_NOOP("TDE free disk space utility");

static const char version[] = "v0.5";


/***************************************************************/
KDFTopLevel::KDFTopLevel(TQWidget *, const char *name)
  : TDEMainWindow(0, name)
{
  kdf = new KDFWidget(this,"kdf",FALSE);
  TQ_CHECK_PTR(kdf);
  (void) new TDEAction( i18n( "&Update" ), 0, TQT_TQOBJECT(kdf), TQT_SLOT( updateDF() ), actionCollection(), "updatedf" );

  KStdAction::quit(TQT_TQOBJECT(this), TQT_SLOT(close()), actionCollection());
  KStdAction::preferences(TQT_TQOBJECT(kdf), TQT_SLOT(settingsBtnClicked()), actionCollection());
  KStdAction::keyBindings(guiFactory(), TQT_SLOT(configureShortcuts()), 
actionCollection());
  setCentralWidget(kdf);
  //  kdf->setMinimumSize(kdf->sizeHint());
  kdf->resize(kdf->sizeHint());
  setupGUI(TDEMainWindow::Keys | StatusBar | Save | Create);
}


bool KDFTopLevel::queryExit( void )
{
  kdf->applySettings();
  return( true );
}


/***************************************************************/
int main(int argc, char **argv)
{
  TDEAboutData aboutData( "kdf", I18N_NOOP("KDiskFree"),
    version, description, TDEAboutData::License_GPL,
    "(c) 1998-2001, Michael Kropfberger");
  aboutData.addAuthor("Michael Kropfberger",0, "michael.kropfberger@gmx.net");
  TDECmdLineArgs::init( argc, argv, &aboutData );

  TDEApplication app;

  if( app.isRestored() ) //SessionManagement
  {
    for( int n=1; KDFTopLevel::canBeRestored(n); n++ )
    {
      KDFTopLevel *ktl = new KDFTopLevel();
      TQ_CHECK_PTR(ktl);
      app.setMainWidget(ktl);
      ktl->restore(n);
    }
  }
  else
  {
    KDFTopLevel *ktl = new KDFTopLevel();
    TQ_CHECK_PTR(ktl);
    ktl->show();
  }

  return app.exec();
}

#include "kdf.moc"

