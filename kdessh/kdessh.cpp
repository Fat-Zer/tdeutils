/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module tdesu.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 */

#include <config.h>

#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/resource.h>


#include <kdebug.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <tdeconfig.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kmessagebox.h>

#include <tdesu/ssh.h>
#include <tdesu/client.h>
#include <tdesu/defaults.h>

#include "sshdlg.h"

static TDECmdLineOptions options[] =
{
    { "+host", I18N_NOOP("Specifies the remote host"), 0 },
    { "+command", I18N_NOOP("The command to run"), 0 },
    { "u <user>", I18N_NOOP("Specifies the target uid"), 0 },
    { "s <path>", I18N_NOOP("Specify remote stub location"), "tdesu_stub" },
    { "n", I18N_NOOP("Do not keep password"), 0 },
    { "q", I18N_NOOP("Stop the daemon (forgets all passwords)"), 0 },
    { "t", I18N_NOOP("Enable terminal output (no password keeping)"), 0 },
    TDECmdLineLastOption
};


int main(int argc, char *argv[])
{
    TDEAboutData aboutData("kdessh", I18N_NOOP("TDE ssh"),
	    VERSION, I18N_NOOP("Runs a program on a remote host"),
	    TDEAboutData::License_Artistic,
	    "Copyright (c) 2000 Geert Jansen");
    aboutData.addAuthor("Geert Jansen", I18N_NOOP("Maintainer"),
	    "jansen@kde.org", "http://www.stack.nl/~geertj/");

    TDECmdLineArgs::init(argc, argv, &aboutData);
    TDECmdLineArgs::addCmdLineOptions(options);

    TDEApplication app;
    TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();

    // Stop daemon and exit?
    if (args->isSet("q"))
    {
	KDEsuClient client;
	if (client.ping() == -1)
	{
	    kdError(1511) << "Daemon not running -- nothing to stop\n";
	    exit(1);
	}
	if (client.stopServer() != -1)
	{
	    kdDebug(1511) << "Daemon stopped\n";
	    exit(0);
	}
	kdError(1511) << "Could not stop daemon\n";
	exit(1);
    }

    if (args->count() < 2)
	TDECmdLineArgs::usage(i18n("No command or host specified."));

    // Check if ssh is available
    if (TDEStandardDirs::findExe(TQString::fromLatin1("ssh")).isEmpty())
    {
	kdError(1511) << "ssh not found\n";
	exit(1);
    }

    // Get remote userid
    TQCString user = args->getOption("u");
    if (user.isNull())
    {
	struct passwd *pw = getpwuid(getuid());
	if (pw == 0L)
	{
	    kdError(1511) << "You don't exist!\n";
	    exit(1);
	}
	user = pw->pw_name;
    }

    // Remote stub location
    TQCString stub = args->getOption("s");

    // Get remote host, command
    TQCString host = args->arg(0);
    TQCString command = args->arg(1);
    for (int i=2; i<args->count(); i++)
    {
	command += " ";
	command += args->arg(i);
    }

    // Check for daemon and start if necessary
    bool have_daemon = true;
    KDEsuClient client;
    if (!client.isServerSGID())
    {
	kdWarning(1511) << "Daemon not safe (not sgid), not using it.\n";
	have_daemon = false;
    }
    else if ((client.ping() == -1) && (client.startServer() == -1))
    {
	kdWarning(1511) << "Could not start daemon, reduced functionality.\n";
	have_daemon = false;
    }

    // Try to exec the command with tdesud?
    bool keep = !args->isSet("n") && have_daemon;
    bool terminal = args->isSet("t");
    if (keep && !terminal)
    {
	client.setHost(host);
	if (client.exec(command, user) != -1)
	    return 0;
    }

    // Set core dump size to 0 because we will have
    // root's password in memory.
    struct rlimit rlim;
    rlim.rlim_cur = rlim.rlim_max = 0;
    if (setrlimit(RLIMIT_CORE, &rlim))
    {
	kdError(1511) << "rlimit(): " << perror << "\n";
	exit(1);
    }


    // Read configuration
    TDEConfig *config = TDEGlobal::config();
    config->setGroup(TQString::fromLatin1("Passwords"));
    int timeout = config->readNumEntry(TQString::fromLatin1("Timeout"), defTimeout);

    SshProcess proc(host, user);
    proc.setStub(stub);
    int needpw = proc.checkNeedPassword();
    if (needpw < 0)
    {
	TQString msg = i18n("Ssh returned with an error!\n"
		"The error message is:\n\n");
	msg += proc.error();
	KMessageBox::error(0L, msg);
	exit(1);
    }

    TQCString password;
    if (needpw != 0)
    {
	KDEsshDialog *dlg = new KDEsshDialog(host, user, stub,
		proc.prompt(), keep && !terminal);
	dlg->addLine(i18n("Command"), command);
	int res = dlg->exec();
	if (res == KDEsshDialog::Rejected)
	    exit(0);
	keep = dlg->keep();
	password = dlg->password();
	delete dlg;
    } else
	keep = 0;

    // Make the dialog go away.
    app.processEvents();

    // Run command
    if (keep && have_daemon)
    {
	client.setHost(host);
	client.setPass(password, timeout);
	return client.exec(command, user);
    } else
    {
	proc.setCommand(command);
	proc.setTerminal(terminal);
	proc.setErase(true);
	return proc.exec(password);
    }
}

