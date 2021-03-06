/******************************************************************/
/* KCharSelectDia - (c) by Reginald Stadlbauer 1999               */
/* Author: Reginald Stadlbauer                                    */
/* E-Mail: reggie@kde.org                                         */
/******************************************************************/

#include "kcharselectdia.h"

#include <tdeapplication.h>
#include <tdeconfig.h>
#include <tdecmdlineargs.h>
#include <tdeaboutdata.h>
#include <tdeglobalsettings.h>

static const char description[] = 
	I18N_NOOP("TDE character selection utility");

/*================================================================*/
int main(int argc, char **argv)
{
  TDEAboutData aboutData( "kcharselect", I18N_NOOP("KCharSelect"),
    version, description, TDEAboutData::License_GPL,
    "(c) 1999, Reginald Stadlbauer");
  aboutData.addAuthor("Reginald Stadlbauer",0, "reggie@kde.org");
  aboutData.addCredit( "Nadeem Hasan", I18N_NOOP( "GUI cleanup and fixes" ),
        "nhasan@kde.org" );
  aboutData.addCredit( "Ryan Cumming", I18N_NOOP( "GUI cleanup and fixes" ),
        "bodnar42@phalynx.dhs.org" );
  aboutData.addCredit("Benjamin C. Meyer",I18N_NOOP("XMLUI conversion"),"ben+kcharselect@meyerhome.net");
	TDECmdLineArgs::init( argc, argv, &aboutData );

  TDEApplication app;

  TDEConfig *config = kapp->config();

  config->setGroup("General");
  TQString font(config->readEntry("selectedFont", TDEGlobalSettings::generalFont().family()));
  TQChar c = TQChar(static_cast<unsigned short>(config->readNumEntry("char",33)));
  int tn = config->readNumEntry("table",0);
  bool direction = config->readNumEntry("entryDirection",0);
  
  KCharSelectDia *dia = new KCharSelectDia(0L,"",c,font,tn,direction);

  app.setMainWidget(dia);
  dia->show();

  return app.exec();
}

