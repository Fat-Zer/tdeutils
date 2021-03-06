/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module tdesu.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 */

#include <tdelocale.h>
#include <tdemessagebox.h>

#include <tdesu/ssh.h>
#include "sshdlg.h"


TDEsshDialog::TDEsshDialog(TQCString host, TQCString user, TQCString stub,
	TQString prompt, bool enableKeep)
    : KPasswordDialog(Password, enableKeep, 0)
{
    m_Host = host;
    m_User = user;
    m_Stub = stub;

    setCaption(TQString::fromLatin1("%1@%2").arg(TQString(m_User)).arg(TQString(m_Host)));

    // Make the prompt a little more polite :-)
    if (prompt.lower().left(6) == TQString::fromLatin1("enter "))
	prompt.remove(0, 6);
    int pos = prompt.find(':');
    if (pos != -1)
	prompt.remove(pos, 10);
    prompt += '.';
    prompt.prepend(i18n("The action you requested needs authentication. "
	    "Please enter "));
    setPrompt(prompt);
}


TDEsshDialog::~TDEsshDialog()
{
}


bool TDEsshDialog::checkPassword(const char *password)
{
    SshProcess proc(m_Host, m_User);
    proc.setStub(m_Stub);

    int ret = proc.checkInstall(password);
    switch (ret)
    {
    case -1:
	KMessageBox::error(this, i18n("Conversation with ssh failed.\n"));
	done(Rejected);
	return false;

    case 0:
	return true;

    case SshProcess::SshNotFound:
	KMessageBox::sorry(this,
		i18n("The programs 'ssh' or 'tdesu_stub' cannot be found.\n"
		"Make sure your PATH is set correctly."));
	done(Rejected);
	return false;

    case SshProcess::SshIncorrectPassword:
	KMessageBox::sorry(this, i18n("Incorrect password. Please try again."));
	return false;

    default:
        KMessageBox::error(this, i18n("Internal error: Illegal return from "
                "SshProcess::checkInstall()"));
        done(Rejected);
    }
    return true;
}


#include "sshdlg.moc"
