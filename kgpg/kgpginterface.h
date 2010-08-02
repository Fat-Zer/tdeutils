/***************************************************************************
                          kgpginterface.h  -  description
                             -------------------
    begin                : Sat Jun 29 2002
    copyright          : (C) 2002 by Jean-Baptiste Mardelle
    email                : bj@altern.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KGPGINTERFACE_H
#define KGPGINTERFACE_H


#include <tqobject.h>
#include <kdialogbase.h>
#include <kurl.h>
#include <tqdatetime.h>

class TQLabel;
class KProcIO;
class KProcess;
class KLed;

/**
 * Encrypt a file using gpg.
 */

class KgpgInterface : public QObject
{

        Q_OBJECT

public:
        /**
         * Initialize the class
         */
        KgpgInterface();

	        /*
         * Destructor for the class.
         */
        ~KgpgInterface();

public slots:

        /**Encrypt file function
         * @param userIDs the recipients key id's.
         * @param srcUrl Kurl of the file to encrypt.
         * @param destUrl Kurl for the encrypted file.
         * @param Options String with the wanted gpg options. ex: "--armor"
         * @param symetrical bool whether the encryption should be symmetrical.
         */
        void KgpgEncryptFile(TQStringList encryptKeys,KURL srcUrl,KURL destUrl,TQStringList Options=TQString::null,bool symetrical=false);

        /**Encrypt file function
         * @param userIDs the key user identification.
         * @param srcUrl Kurl of the file to decrypt.
         * @param destUrl Kurl for the decrypted file.
         * @param chances int number of trials left for decryption (used only as an info displayed in the password dialog)
         */
        void KgpgDecryptFile(KURL srcUrl,KURL destUrl,TQStringList Options=TQStringList());

        /**Sign file function
         * @param keyID TQString the signing key ID.
         * @param srcUrl Kurl of the file to sign.
         * @param Options String with the wanted gpg options. ex: "--armor"
         */
        void KgpgSignFile(TQString keyID,KURL srcUrl,TQStringList Options=TQStringList());

        /**Verify file function
         * @param sigUrl Kurl of the signature file.
         * @param srcUrl Kurl of the file to be verified. If empty, gpg will try to find it using the signature file name (by removing the .sig extensio)
         */
        void KgpgVerifyFile(KURL sigUrl,KURL srcUrl=KURL()) ;

	void KgpgVerifyText(TQString text);
	void slotverifyread(KProcIO *p);
	void slotverifyresult(KProcess*);
	
	
        /**Import key function
         * @param url Kurl the url of the key file. Allows public & secret key import.
         */
        void importKeyURL(KURL url);
        /**Import key function
         * @param keystr TQString containing th key. Allows public & secret key import.
        */
        void importKey(TQString keystr);

        /**Key signature function
         * @param keyID TQString the ID of the key to be signed
         * @param signKeyID TQString the ID of the signing key
         * @param signKeyMail TQString the name of the signing key (only used to prompt user for passphrase)
         * @param local bool should the signature be local
         */
        void KgpgSignKey(TQString keyID,TQString signKeyID,TQString signKeyMail=TQString::null,bool local=false,int checking=0);

        /**Key signature deletion function
         * @param keyID TQString the ID of the key
         * @param signKeyID TQString the ID of the signature key
         */
        void KgpgDelSignature(TQString keyID,TQString signKeyID);

        /**Encrypt text function
         * @param text TQString text to be encrypted.
         * @param userIDs the recipients key id's.
         * @param Options String with the wanted gpg options. ex: "--armor"
         * returns the encrypted text or empty string if encyption failed
         */
        void KgpgEncryptText(TQString text,TQStringList userIDs, TQStringList Options=TQString::null);

        /**Decrypt text function
        * @param text TQString text to be decrypted.
        * @param userID TQString the name of the decryption key (only used to prompt user for passphrase)
        */
	//static TQString KgpgDecryptText(TQString text,TQString userID);
	void KgpgDecryptText(TQString text,TQStringList Options=TQString::null);
	void txtdecryptfin(KProcess *);

	/**Extract list of photographic user id's
        * @param keyID the recipients key id's.
        */
	void KgpgGetPhotoList(TQString keyID);

	void getOutput(KProcess *, char *data, int );
	void getCmdOutput(KProcess *p, char *data, int );

	TQString getKey(TQStringList IDs, bool attributes);

        void KgpgKeyExpire(TQString keyID,TQDate date,bool unlimited);
        void KgpgTrustExpire(TQString keyID,int keyTrust);
	void KgpgChangePass(TQString keyID);

	void KgpgRevokeKey(TQString keyID,TQString revokeUrl,int reason,TQString description);
	void revokeover(KProcess *);
	void revokeprocess(KProcIO *p);
	void KgpgDeletePhoto(TQString keyID,TQString uid);
	void KgpgAddPhoto(TQString keyID,TQString imagePath);

	void KgpgAddUid(TQString keyID,TQString name,TQString email,TQString comment);
	
        void KgpgDecryptFileToText(KURL srcUrl,TQStringList Options);
	void KgpgSignText(TQString text,TQString userIDs, TQStringList Options);

        static TQString getGpgSetting(TQString name,TQString configFile);
	static TQString getGpgMultiSetting(TQString name,TQString configFile);
        static void setGpgSetting(TQString name,TQString ID,TQString url);
	static void setGpgMultiSetting(TQString name,TQStringList values,TQString url);
        static bool getGpgBoolSetting(TQString name,TQString configFile);
	static void setGpgBoolSetting(TQString name,bool enable,TQString url);
        static TQStringList getGpgGroupNames(TQString configFile);
	static TQStringList getGpgGroupSetting(TQString name,TQString configFile);
	static void setGpgGroupSetting(TQString name,TQStringList values, TQString configFile);
	static void delGpgGroup(TQString name, TQString configFile);
	static TQString checkForUtf8(TQString txt);
	static TQString checkForUtf8bis(TQString txt);
	static int getGpgVersion();



private slots:

        void openSignConsole();
        /**
              * Checks output of the signature process
              */
        void signover(KProcess *);
        /**
                * Read output of the signature process
                */
        void sigprocess(KProcIO *p);

        /**
         * Checks if the encrypted file was saved.
         */
        void encryptfin(KProcess *);

        /**
                * Checks if the decrypted file was saved.
                */
        void decryptfin(KProcess *);

        /**
                * Checks if the signing was successful.
                */
        void signfin(KProcess *p);

        /**
                * Checks the number of uid's for a key-> if greater than one, key signature will switch to konsole mode
                */
        int checkuid(TQString KeyID);

        /**
                * Reads output of the delete signature process
                */
        void delsigprocess(KProcIO *p);
        /**
                * Checks output of the delete signature process
                */
        void delsignover(KProcess *p);
        /**
                * Checks output of the import process
                */
        void importURLover(KProcess *p);
        void importover(KProcess *);
        /**
                 * Read output of the import process
                 */
        void importprocess(KProcIO *p);
        /**
                 * Reads output of the current process + allow overwriting of a file
                 */
        void readprocess(KProcIO *p);
        /**
                * Reads output of the current encryption process + allow overwriting of a file
                */
        void readencprocess(KProcIO *p);
        /**
                * Reads output of the current signing process + allow overwriting of a file
                */
        void readsignprocess(KProcIO *p);
        /**
                * Reads output of the current decryption process + allow overwriting of a file
                */
        void readdecprocess(KProcIO *p);
        /**
                 * Checks output of the verify process
                 */
        void verifyfin(KProcess *p);

        void expprocess(KProcIO *p);
        void expover(KProcess*);
        void trustprocess(KProcIO *p);
	void passprocess(KProcIO *p);
        void trustover(KProcess *);
	void passover(KProcess *);

        void txtreadencprocess(KProcIO *p);

        void txtencryptfin(KProcess *);

	void delphotoover(KProcess *);
	void delphotoprocess(KProcIO *p);
	void addphotoover(KProcess *);
	void addphotoprocess(KProcIO *p);
	
	void adduidover(KProcess *);
	void adduidprocess(KProcIO *p);
		
	void slotReadKey(KProcIO *p);
	void photoreadover(KProcess *);
	void photoreadprocess(KProcIO *p);
	bool isPhotoId(int uid);
	void updateIDs(TQString txtString);
	
	void txtsignprocess(KProcIO *p);
	void txtsignfin(KProcess *);

        //void txtdecryptfin(KProcess *);


signals:

	void missingSignature(TQString);
	void verifyOver(TQString,TQString);

	/**
               *  emitted when a txt decryption failed. returns log output
               */
	void txtdecryptionfailed(TQString);
	/**
               *  emitted when a txt encryption starts.
               */
	void txtencryptionstarted();
	
	/**
               *  emitted when a txt decryption finished. returns decrypted text
               */
	void txtdecryptionfinished(TQString);
        /**
               *  emitted when a txt encryption finished. returns encrypted text
               */
        void txtencryptionfinished(TQString);
        /**
               *  emitted when an error occurred
               */
        void errormessage(TQString);
        /**
                *  true if encryption successful, false on error.
                */
        void encryptionfinished(KURL);
        /**
                *  true if key signature deletion successful, false on error.
                */
        void delsigfinished(bool);

        /**
                * Signature process result: 0=successful, 1=error, 2=bad passphrase
                */
        void signatureFinished(int);
        /**
                *  emitted when user cancels process
                */
        void processaborted(bool);
        /**
                *  emitted when the process starts
                */
        void processstarted(TQString);
        /**
                *  true if decryption successful, false on error.
                */
        void decryptionfinished();
        /**
                * emitted if bad passphrase was giver
                */
        void badpassphrase(bool);
        /**
                *  true if import successful, false on error.
                */
        void importfinished(TQStringList);
        /**
                *  true if verify successful, false on error.
                */
        void verifyfinished();
        /**
                *  emmitted if signature key is missing & user want to import it from keyserver
                */
        void verifyquerykey(TQString ID);
        /**
                *  true if signature successful, false on error.
                */
        void signfinished();
	void delPhotoFinished();
	void delPhotoError(TQString);
	
	void addPhotoFinished();
	void addPhotoError(TQString);
	void refreshOrphaned();
	
	void addUidFinished();
	void addUidError(TQString);
	
        void trustfinished();
	void revokecertificate(TQString);
	void revokeurl(TQString);
        void expirationFinished(int);
	void signalPhotoList(TQStringList);
	void passwordChanged();
	
	void txtSignOver(TQString);


private:
        /**
        * @internal structure for communication
        */
        TQString message,tempKeyFile,userIDs,output,keyString,txtToEncrypt,log;
        TQCString passphrase;
        bool deleteSuccess,konsLocal,anonymous,decfinished,decok,badmdc,revokeSuccess,addSuccess,delSuccess;
	bool signmiss;
	TQString signID;
        int signSuccess,expSuccess,trustValue,konsChecked;
        int step,signb,sigsearch,expirationDelay;
        TQString konsSignKey, konsKeyID,errMessage;
	int revokeReason,photoCount;
	TQString revokeDescription,certificateUrl,photoUrl;
	TQStringList photoList;
	TQString uidName, uidEmail, uidComment;
        KURL sourceFile;
	TQString decryptUrl;

	TQString gpgOutput;
	
        /**
         * @internal structure for the file information
         */
        KURL file;

};

class  Md5Widget :public KDialogBase
{
        Q_OBJECT
public:
        Md5Widget(TQWidget *parent=0, const char *name=0,KURL url=KURL());
        ~Md5Widget();
public slots:
        void slotApply();
private:
        TQString mdSum;
        KLed *KLed1;
        TQLabel *TextLabel1_2;
};

#endif // KGPGINTERFACE_HKGPGINTERFACE_H

