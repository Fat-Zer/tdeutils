
/*********************************************************************
 *                
 * Filename:      smapidev.h
 * Description:   Header for the SMAPI device driver access library
 * Author:        Bill Mair, Thomas Hood
 * Created:       13 July 1999
 *
 * Please report bugs to the author ASAP.
 * 
 *     Copyright (c) 1999 Bill Mair, All rights reserved
 *     
 *     This program is free software; you can redistribute it and/or 
 *     modify it under the terms of the GNU General Public License as 
 *     published by the Free Software Foundation; either version 2 of 
 *     the License, or (at your option) any later version.
 * 
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *     GNU General Public License for more details.
 * 
 *     To receive a copy of the GNU General Public License, please write
 *     to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *     Boston, MA 02110-1301 USA
 *     
 ********************************************************************/

#ifndef __SMAPIDEV_H__
#define __SMAPIDEV_H__

/****** defines ******/

/*** smapidev function error codes ***/
/*
 * These codes must not fall in the range 0x0-0xFF which is
 * reserved for SMAPI BIOS error codes
 */
#define ERR_SMAPIDEV_MIN                          ((int)0x1000)
#define ERR_SMAPIDEV_PARM_INVALID                 ((int)0x1050)
#define ERR_SMAPIDEV_STRUCTURE_SIZE_INVALID       ((int)0x1051)
#define ERR_SMAPIDEV_SMAPI_RESULT_NOT_UNDERSTOOD  ((int)0x1090)
#define ERR_SMAPIDEV_MAX                          ((int)0x10FF)

/****** typedefs ******/

/*** enum ***/

typedef enum _smapidev_stateplace {
	SMAPIDEV_STATEPLACE_CURR=0,
	SMAPIDEV_STATEPLACE_CMOS,
	SMAPIDEV_STATEPLACE_CMOS_AND_CURR
} smapidev_stateplace_t;

typedef enum _smapidev_ablestate {
	SMAPIDEV_ABLESTATE_DISABLED=0,
	SMAPIDEV_ABLESTATE_ENABLED,
	SMAPIDEV_ABLESTATE_AUTO
} smapidev_ablestate_t;

typedef enum _smapidev_displaycaptv {
	SMAPIDEV_DISPLAYCAPTV_NONE=0,
	SMAPIDEV_DISPLAYCAPTV_NONSIMULTANEOUS,
	SMAPIDEV_DISPLAYCAPTV_OTHER
} smapidev_displaycaptv_t;

/*
 * The following are the display modes that can be enabled or disabled
 */
typedef enum _smapidev_dispmode {
	SMAPIDEV_DISPMODE_INTERNAL=0,
	SMAPIDEV_DISPMODE_CRT,
	SMAPIDEV_DISPMODE_TV,
	SMAPIDEV_DISPMODE_CRT_DETECTION_IGNORE,
	SMAPIDEV_DISPMODE_DUAL,
	SMAPIDEV_DISPMODE_SELECT_TV
} smapidev_dispmode_t;

typedef enum _smapidev_fnkeymode {
	SMAPIDEV_FNKEY_NORMAL=0,
	SMAPIDEV_FNKEY_STICKY,
	SMAPIDEV_FNKEY_LOCKED
} smapidev_fnkeymode_t;

typedef enum _smapidev_ternality {
	SMAPIDEV_TERNALITY_IN=0,
	SMAPIDEV_TERNALITY_EX
} smapidev_ternality_t;

typedef enum _smapidev_powersrc {
	SMAPIDEV_POWERSRC_AC=0,
	SMAPIDEV_POWERSRC_BATTERY
} smapidev_powersrc_t;

typedef enum _smapidev_powermode {
	SMAPIDEV_POWERMODE_HIGH=0,
	SMAPIDEV_POWERMODE_AUTO,
	SMAPIDEV_POWERMODE_MANUAL,
	SMAPIDEV_POWERMODE_UNRECOGNIZED
} smapidev_powermode_t;


/*** struct ***/

typedef struct _smapidev_info
{
	char	szVersion[LEN_VERSION_MAX+1];
} smapidev_info_t;

typedef struct _smapidev_biosinfo
{
	size_t	sizeStruct;
	word	wSysId;
	word	wCountryCode;
	word	wSysBiosRevMajor;
	word	wSysBiosRevMinor;
	word	wSysMgmtBiosRevMajor;
	word	wSysMgmtBiosRevMinor;
	word	wSmapiBiosIfaceRevMajor;
	word	wSmapiBiosIfaceRevMinor;
	word	wVideoBiosRevMajor;
	word	wVideoBiosRevMinor;
} smapidev_biosinfo_t;

typedef struct _smapidev_cpuinfo
{
	size_t	sizeStruct;
	word	wManufacturer;
	word	wType;
	word	wStepping;
	word	wClock;
	word	wInternalClock;
} smapidev_cpuinfo_t;

typedef struct _smapidev_displayinfo
{
	size_t	sizeStruct;
	byte	bPanelType;
	byte	bPanelDim;
	byte	bCrtType;
	byte	bCrtFeatures;
} smapidev_displayinfo_t;

typedef struct _smapidev_dockinginfo
{
	size_t	sizeStruct;
	word	wID;
	flag_t	fDocked;
	flag_t	fKeyUnlocked;
	flag_t	fBusConnected;
} smapidev_dockinginfo_t;

typedef struct _smapidev_ultrabayinfo
{
	size_t	sizeStruct;
	byte	bType;  
	byte	bID;
} smapidev_ultrabayinfo_t;

typedef struct _smapidev_slaveinfo
{
	size_t	sizeStruct;
	flag_t	fAscii;
	char	szID[3];
	word	wIDMajor;
	word	wIDMinor;
} smapidev_slaveinfo_t;

typedef struct _smapidev_sensorinfo
{
	size_t	sizeStruct;
	flag_t	fLidClosed;
	flag_t	fKeyboardOpen;
	flag_t	fACAdapterAttached;
} smapidev_sensorinfo_t;

typedef struct _smapidev_screenrefreshinfo
{
	size_t	sizeStruct;
	flag_t	f43i;
	flag_t	f48i;
	flag_t	f56;
	flag_t	f60;
	flag_t	f70;
	flag_t	f72;
	flag_t	f75;
	flag_t	f85;
} smapidev_screenrefreshinfo_t;

typedef struct _smapidev_displaycap {
	size_t	sizeStruct;
	flag_t  fSupported;
	smapidev_displaycaptv_t tv;
} smapidev_displaycap_t;


/****** function declarations ******/

bcd8_t bcd8_of_byte( byte bThe );
byte byte_of_bcd8( bcd8_t bcd8The );
int ioctl_smapi( int intFiledesc, smapi_ioparm_t *pioparmThe );

/*** Get/Set####Info ***/

int smapidev_GetInfo( smapidev_info_t *pinfoThe );
int smapidev_GetBiosInfo(
	int intFiledesc,
	smapidev_biosinfo_t *pbiosinfoThe
);
int smapidev_GetCpuInfo(
	int intFiledesc,
	smapidev_cpuinfo_t *pcpuinfoThe
);
int smapidev_GetDisplayInfo(
	int intFiledesc,
	smapidev_displayinfo_t *pdisplayinfoThe
);
int smapidev_GetDockingInfo(
	int intFiledesc,
	smapidev_dockinginfo_t *pdockinginfoThe
);
int smapidev_GetUltrabayInfo(
	int intFiledesc,
	smapidev_ultrabayinfo_t *pultrabayinfoThe
);
int smapidev_GetSlaveControllerInfo(
	int intFiledesc,
	smapidev_slaveinfo_t *pslaveinfoThe
);
int smapidev_GetSensorInfo(
	int intFiledesc,
	smapidev_sensorinfo_t *psensorinfoThe
);
int smapidev_GetScreenRefreshInfo(
	int intFiledesc,
	word wMode,
	smapidev_screenrefreshinfo_t *pscreenrefreshinfoThe
);

/*** Get/Set####State ***/

int smapidev_GetDisplayCapability(
	int intFiledesc,
	smapidev_stateplace_t stateplace,
	smapidev_displaycap_t *pdisplaycap
);
int smapidev_GetDisplayState(
	int intFiledesc,
	smapidev_stateplace_t stateplace,
	smapidev_dispmode_t dispmode,
	smapidev_ablestate_t *pablestate
);
int smapidev_SetDisplayState(
	int intFiledesc,
	smapidev_stateplace_t stateplace,
	smapidev_dispmode_t dispmode,
	smapidev_ablestate_t ablestate
);

/*** PM ***/

int smapidev_GetPowerExpenditureMode(
	int intFiledesc,
	smapidev_powersrc_t powersrc,
	smapidev_powermode_t *ppowermode
);
int smapidev_SetPowerExpenditureMode(
	int intFiledesc,
	smapidev_powersrc_t powersrc,
	smapidev_powermode_t powermode
);

#endif
