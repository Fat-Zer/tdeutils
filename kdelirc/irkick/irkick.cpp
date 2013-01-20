/***************************************************************************
                          irkick.cpp  -  Implementation of the main window
                             -------------------
    copyright            : (C) 2002 by Gav Wood
    email                : gav@kde.org
***************************************************************************/

// This program is free software.

#include <tqwidget.h>
#include <tqdialog.h>
#include <tqtooltip.h>
#include <tqregexp.h>
#include <tqtimer.h>
#include <tqevent.h>

#include <tdeversion.h>
#include <kapplication.h>
#include <kaction.h>
#if !(TDE_VERSION_MINOR<=1 && TDE_VERSION_RELEASE<=5)
#include <kactioncollection.h>
#endif
#include <ksimpleconfig.h>
#include <ksystemtray.h>
#include <kiconloader.h>
#include <kpassivepopup.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kdebug.h>
#include <klocale.h>
#include <kaboutdialog.h>
#include <kaboutkde.h>
#include <twinmodule.h>
#include <twin.h>
#include <khelpmenu.h>
#include <kglobal.h>
#include <kstdguiitem.h>

#include <dcopclient.h>
#include <dcopref.h>

#include "profileserver.h"
#include "irkick.h"

void IRKTrayIcon::mousePressEvent(TQMouseEvent *e)
{
	KSystemTray::mousePressEvent(new TQMouseEvent(TQEvent::MouseButtonPress, e->pos(), e->globalPos(), e->button() == Qt::LeftButton ? Qt::RightButton : e->button(), e->state()));
}

IRKick::IRKick(const TQCString &obj) : TQObject(), DCOPObject(obj), npApp(TQString())
{
    kapp->dcopClient()->setDefaultObject(obj);
	theClient = new KLircClient();

	theTrayIcon = new IRKTrayIcon();
	if(theClient->isConnected())
	{	theTrayIcon->setPixmap(SmallIcon("irkick"));
		TQToolTip::add(theTrayIcon, i18n("TDE Lirc Server: Ready."));
	}
	else
	{	theTrayIcon->setPixmap(SmallIcon("irkickoff"));
		TQToolTip::add(theTrayIcon, i18n("TDE Lirc Server: No infra-red remote controls found."));
		TQTimer::singleShot(10000, this, TQT_SLOT(checkLirc()));
	}
	theFlashOff = new TQTimer(theTrayIcon);
	connect(theFlashOff, TQT_SIGNAL(timeout()), TQT_SLOT(flashOff()));

	theResetCount = 0;
	slotReloadConfiguration();
	connect(theClient, TQT_SIGNAL(connectionClosed()), this, TQT_SLOT(slotClosed()));
	connect(theClient, TQT_SIGNAL(remotesRead()), this, TQT_SLOT(resetModes()));
	connect(theClient, TQT_SIGNAL(commandReceived(const TQString &, const TQString &, int)), this, TQT_SLOT(gotMessage(const TQString &, const TQString &, int)));

	theTrayIcon->contextMenu()->changeTitle(0, "IRKick");
	theTrayIcon->contextMenu()->insertItem(SmallIcon( "configure" ), i18n("&Configure..."), this, TQT_SLOT(slotConfigure()));
	theTrayIcon->contextMenu()->insertSeparator();
	theTrayIcon->contextMenu()->insertItem(SmallIcon( "help" ), KStdGuiItem::help().text(), (new KHelpMenu(theTrayIcon, KGlobal::instance()->aboutData()))->menu());
	theTrayIcon->actionCollection()->action("file_quit")->disconnect(TQT_SIGNAL(activated()));
	connect(theTrayIcon->actionCollection()->action("file_quit"), TQT_SIGNAL(activated()), TQT_SLOT(doQuit()));

	theTrayIcon->show();
}

IRKick::~IRKick()
{
	delete theTrayIcon;
	for(TQMap<TQString,IRKTrayIcon *>::iterator i = currentModeIcons.begin(); i != currentModeIcons.end(); ++i)
		if(*i) delete *i;
}

void IRKick::slotClosed()
{
	theTrayIcon->setPixmap(SmallIcon("irkickoff"));
	KPassivePopup::message("IRKick", i18n("The infrared system has severed its connection. Remote controls are no longer available."), SmallIcon("irkick"), theTrayIcon);
	TQTimer::singleShot(1000, this, TQT_SLOT(checkLirc()));
}

void IRKick::checkLirc()
{
	if(!theClient->isConnected())
		if(theClient->connectToLirc())
		{	KPassivePopup::message("IRKick", i18n("A connection to the infrared system has been made. Remote controls may now be available."), SmallIcon("irkick"), theTrayIcon);
			theTrayIcon->setPixmap(SmallIcon("irkick"));
		}
		else
			TQTimer::singleShot(10000, this, TQT_SLOT(checkLirc()));
}

void IRKick::flashOff()
{
	theTrayIcon->setPixmap(SmallIcon("irkick"));
}

void IRKick::doQuit()
{
	KSimpleConfig theConfig("irkickrc");
	theConfig.setGroup("General");
	switch(KMessageBox::questionYesNoCancel(0, i18n("Should the Infrared Remote Control server start automatically when you begin TDE?"), i18n("Automatically Start?"), i18n("Start Automatically"), i18n("Do Not Start")))
	{	case KMessageBox::No: theConfig.writeEntry("AutoStart", false); break;
		case KMessageBox::Yes: theConfig.writeEntry("AutoStart", true); break;
		case KMessageBox::Cancel: return;
	}
	TDEApplication::kApplication()->quit();
}

void IRKick::resetModes()
{
	if(theResetCount > 1)
		KPassivePopup::message("IRKick", i18n("Resetting all modes."), SmallIcon("irkick"), theTrayIcon);
	if(!theResetCount)
		allModes.generateNulls(theClient->remotes());

	TQStringList remotes = theClient->remotes();
	for(TQStringList::iterator i = remotes.begin(); i != remotes.end(); ++i)
	{	currentModes[*i] = allModes.getDefault(*i).name();
		if(theResetCount && currentModeIcons[*i]) delete currentModeIcons[*i];
		currentModeIcons[*i] = 0;
	}
	updateModeIcons();
	theResetCount++;
}

void IRKick::slotReloadConfiguration()
{
	// load configuration from config file
	KSimpleConfig theConfig("irkickrc");
	allActions.loadFromConfig(theConfig);
	allModes.loadFromConfig(theConfig);
	if(currentModes.count() && theResetCount)
		resetModes();
}

void IRKick::slotConfigure()
{
	TDEApplication::startServiceByDesktopName("kcmlirc");
}

void IRKick::updateModeIcons()
{
	for(TQMap<TQString,TQString>::iterator i = currentModes.begin(); i != currentModes.end(); ++i)
	{	Mode mode = allModes.getMode(i.key(), i.data());
		if(mode.iconFile().isNull() || mode.iconFile().isEmpty())
		{	if(currentModeIcons[i.key()])
			{	delete currentModeIcons[i.key()];
				currentModeIcons[i.key()] = 0;
			}
		}
		else
		{	if(!currentModeIcons[i.key()])
			{	currentModeIcons[i.key()] = new IRKTrayIcon();
				currentModeIcons[i.key()]->show();
				currentModeIcons[i.key()]->contextMenu()->changeTitle(0, mode.remoteName());
				currentModeIcons[i.key()]->actionCollection()->action("file_quit")->setEnabled(false);
			}
			currentModeIcons[i.key()]->setPixmap(KIconLoader().loadIcon(mode.iconFile(), KIcon::Panel));
			TQToolTip::add(currentModeIcons[i.key()], mode.remoteName() + ": <b>" + mode.name() + "</b>");
		}
	}
}

bool IRKick::getPrograms(const IRAction &action, TQStringList &programs)
{
	DCOPClient *theDC = TDEApplication::dcopClient();
	programs.clear();

	if(action.unique())
	{	if(theDC->isApplicationRegistered(action.program().utf8()))
			programs += action.program();
	}
	else
	{
		TQRegExp r = TQRegExp("^" + action.program() + "-(\\d+)$");
		// find all instances...
		QCStringList buf = theDC->registeredApplications();
		for(QCStringList::iterator i = buf.begin(); i != buf.end(); ++i)
		{
			TQString program = TQString::fromUtf8(*i);
			if(program.contains(r))
				programs += program;
		}
		if(programs.size() > 1 && action.ifMulti() == IM_DONTSEND)
			return false;
		else if(programs.size() > 1 && action.ifMulti() == IM_SENDTOTOP)
		{	TQValueList<WId> s = KWinModule().stackingOrder();
			// go through all the (ordered) window pids
			for(TQValueList<WId>::iterator i = s.fromLast(); i != s.end(); i--)
			{	int p = KWin::info(*i).pid;
				TQString id = action.program() + "-" + TQString().setNum(p);
				if(programs.contains(id))
				{	programs.clear();
					programs += id;
					break;
				}
			}
			while(programs.size() > 1) programs.remove(programs.begin());
		}
		else if(programs.size() > 1 && action.ifMulti() == IM_SENDTOBOTTOM)
		{	TQValueList<WId> s = KWinModule().stackingOrder();
			// go through all the (ordered) window pids
			for(TQValueList<WId>::iterator i = s.begin(); i != s.end(); ++i)
			{	int p = KWin::info(*i).pid;
				TQString id = action.program() + "-" + TQString().setNum(p);
				if(programs.contains(id))
				{	programs.clear();
					programs += id;
					break;
				}
			}
			while(programs.size() > 1) programs.remove(programs.begin());
		}
	}
	return true;
}

void IRKick::executeAction(const IRAction &action)
{
	DCOPClient *theDC = TDEApplication::dcopClient();
	TQStringList programs;

	if(!getPrograms(action, programs)) return;

	// if programs.size()==0 here, then the app is definately not running.
	if(action.autoStart() && !programs.size())
	{	TQString sname = ProfileServer::profileServer()->getServiceName(action.program());
		if(!sname.isNull())
		{
			KPassivePopup::message("IRKick", i18n("Starting <b>%1</b>...").arg(action.application()), SmallIcon("irkick"), theTrayIcon);
			TDEApplication::startServiceByDesktopName(sname);
		}
	}
	if(action.isJustStart()) return;

	if(!getPrograms(action, programs)) return;

	for(TQStringList::iterator i = programs.begin(); i != programs.end(); ++i)
	{	const TQString &program = *i;
		if(theDC->isApplicationRegistered(program.utf8()))
		{	TQByteArray data; TQDataStream arg(data, IO_WriteOnly);
			kdDebug() << "Sending data (" << program << ", " << action.object() << ", " << action.method().prototypeNR() << endl;
			for(Arguments::const_iterator j = action.arguments().begin(); j != action.arguments().end(); ++j)
			{	kdDebug() << "Got argument..." << endl;
				switch((*j).type())
				{	case TQVariant::Int: arg << (*j).toInt(); break;
					case TQVariant::CString: arg << (*j).toCString(); break;
					case TQVariant::StringList: arg << (*j).toStringList(); break;
					case TQVariant::UInt: arg << (*j).toUInt(); break;
					case TQVariant::Bool: arg << (*j).toBool(); break;
					case TQVariant::Double: arg << (*j).toDouble(); break;
					default: arg << (*j).toString(); break;
				}
			}
			theDC->send(program.utf8(), action.object().utf8(), action.method().prototypeNR().utf8(), data);
		}
	}
}

void IRKick::gotMessage(const TQString &theRemote, const TQString &theButton, int theRepeatCounter)
{
	kdDebug() << "Got message: " << theRemote << ": " << theButton << " (" << theRepeatCounter << ")" << endl;
	theTrayIcon->setPixmap(SmallIcon("irkickflash"));
	theFlashOff->start(200, true);
	if(!npApp.isNull())
	{
		TQString theApp = npApp;
		npApp = TQString();
		// send notifier by DCOP to npApp/npModule/npMethod(theRemote, theButton);
		TQByteArray data; TQDataStream arg(data, IO_WriteOnly);
		arg << theRemote << theButton;
		TDEApplication::dcopClient()->send(theApp.utf8(), npModule.utf8(), npMethod.utf8(), data);
	}
	else
	{
		if(currentModes[theRemote].isNull()) currentModes[theRemote] = "";
		IRAItList l = allActions.findByModeButton(Mode(theRemote, currentModes[theRemote]), theButton);
		if(!currentModes[theRemote].isEmpty())
			l += allActions.findByModeButton(Mode(theRemote, ""), theButton);
		bool doBefore = true, doAfter = false;
		for(IRAItList::const_iterator i = l.begin(); i != l.end(); ++i)
			if((**i).isModeChange() && !theRepeatCounter)
			{	// mode switch
				currentModes[theRemote] = (**i).modeChange();
				Mode mode = allModes.getMode(theRemote, (**i).modeChange());
				updateModeIcons();
				doBefore = (**i).doBefore();
				doAfter = (**i).doAfter();
				break;
			}

		for(int after = 0; after < 2; after++)
		{	if(doBefore && !after || doAfter && after)
				for(IRAItList::const_iterator i = l.begin(); i != l.end(); ++i)
					if(!(**i).isModeChange() && ((**i).repeat() || !theRepeatCounter))
					{	executeAction(**i);
					}
			if(!after && doAfter)
			{	l = allActions.findByModeButton(Mode(theRemote, currentModes[theRemote]), theButton);
				if(!currentModes[theRemote].isEmpty())
					l += allActions.findByModeButton(Mode(theRemote, ""), theButton);
			}
		}
	}
}

void IRKick::stealNextPress(TQString app, TQString module, TQString method)
{
	npApp = app;
	npModule = module;
	npMethod = method;
}

void IRKick::dontStealNextPress()
{
	npApp = TQString();
}

#include "irkick.moc"
