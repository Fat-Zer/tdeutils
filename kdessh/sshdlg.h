/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module tdesu.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 */

#ifndef __SshDlg_h_Included__
#define __SshDlg_h_Included__

#include <kpassdlg.h>

class KDEsshDialog
    : public KPasswordDialog
{
    Q_OBJECT
  TQ_OBJECT

public:
    KDEsshDialog(TQCString host, TQCString user, TQCString stub, 
	    TQString prompt, bool enableKeep);
    ~KDEsshDialog();

protected:
    bool checkPassword(const char *password);
    
private:
    TQCString m_User, m_Host, m_Stub;
};
    

#endif // __SshDlg_h_Included__
