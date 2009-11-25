#if defined(__linux__) || defined(__FreeBSD__)
/*********************************************************************
 *                
 * Filename:      smapidev.c
 * Description:   IBM SMAPI (System Management API) interface functions
 * Author:        Bill Mair, Thomas Hood
 * Created:       19 July 1999
 *
 * Please report bugs to the author ASAP.
 * 
 *     Copyright (c) 1999 J.D. Thomas Hood and Bill Mair,
 *         All rights reserved
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

#include <fcntl.h>
#include <stdlib.h>
#ifdef __linux__
#include <linux/unistd.h>
#else
#include <unistd.h>
#endif
#include <getopt.h>
#include <sys/ioctl.h>
#include <errno.h>
#ifdef __linux__
#include <linux/types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>

#ifndef __linux__
typedef uint8_t __u8 ;
typedef uint16_t __u16 ;
typedef uint32_t __u32 ;
#endif

#include "thinkpad_common.h"
/*#include "thinkpad.h" */
#include "smapi.h"
#include "smapidev.h"

/****** defines ******/

#define SZ_SMAPIDEV_VERSION "2.0"

#define SZ_SMAPIDEV_NAME "smapidev"

/*
 * The structures may need to be extended if more
 * functionality is added to the SMAPI by IBM.
 *
 * To cover this, future releases can check the size
 * defined in sizeStruct, and reduce the amount of
 * information put into the structure accordingly.
 */

#define SIZE_BIOSINFO_V1           ((size_t)24)
#define SIZE_CPUINFO_V1            ((size_t)16)
#define SIZE_DISPLAYINFO_V1        ((size_t) 8)
#define SIZE_DOCKINGINFO_V1        ((size_t)12)
#define SIZE_ULTRABAYINFO_V1       ((size_t) 8)
#define SIZE_SLAVEINFO_V1          ((size_t)12)
#define SIZE_SENSORINFO_V1         ((size_t) 8)
#define SIZE_SCREENREFRESHINFO_V1  ((size_t)12)
#define SIZE_DISPLAYCAP_V1         ((size_t)12)


/****** variables ******/

char szSmapidevName[] = SZ_SMAPIDEV_NAME;


/****** utility functions ******/

/*
 * This function returns the binary value of a two-digit bcd number
 * If the bcd8 value is 0xFF, as it may be if a location has never been
 * initialized in the ThinkPad CMOS RAM, then 0xFF is returned as the
 * binary equivalent.
 */
byte byte_of_bcd8( bcd8_t bcd8The )
{
	byte bTens, bUnits;

	/* catch uninitialized values: simply return them */
	if ( bcd8The == 0xFF ) return 0xFF;

	bUnits = (byte)bcd8The & 0xF;
	bTens = (byte)(bcd8The & 0xF0) >> 4;

	if ( bUnits > 9 || bTens > 9 ) {
		printf( "%s: Warning: value 0x%x which is supposed to be in BCD format is not; not converting.\n", szSmapidevName, bcd8The );
		return (byte)bcd8The;
	}

	return bUnits + (bTens * 10);
}


bcd8_t bcd8_of_byte( byte bThe )
{
	byte bTens, bUnits;

	if ( bThe > 99 ) {
		printf( "%s: the value %d being converted to BCD format will be limited to 99.\n", szSmapidevName, bThe );
		bThe = 99;
	}

	bTens = bThe / (byte)10;
	bUnits = bThe - (bTens * (byte)10);

	return (bTens << 4) | bUnits;
}

/*
 * This function returns the SMAPI BIOS error code if there is one,
 * otherwise the ioctl errno as a negative number
 */
int ioctl_smapi( int intFiledesc, smapi_ioparm_t *pioparmThe )
{
	int intRtn;

	intRtn = ioctl( intFiledesc, IOCTL_SMAPI_REQUEST, pioparmThe );
	if ( intRtn && errno == ETHINKPAD_SUBDRIVER ) return pioparmThe->out.bRc;
	if ( intRtn ) return -errno;
	return 0;
}


/****** functions ******/

int smapidev_GetInfo( smapidev_info_t *pinfoThe )
{

	strncpy( pinfoThe->szVersion, SZ_SMAPIDEV_VERSION, LEN_VERSION_MAX );

	/*** Make sure that the returned string is terminated ***/
	pinfoThe->szVersion[LEN_VERSION_MAX] = '\0';

	return 0;
}


/*** smapi-module-access functions ***/


/*
 * The Technical Reference fails to mention that the returned 
 * BIOS revision values are in BCD format
 */
int smapidev_GetBiosInfo(
	int intFiledesc,
	smapidev_biosinfo_t *pbiosinfoThe
) {
	bcd8_t bcd8SysHigh, bcd8SysLow;
	bcd8_t bcd8SysMgmtHigh, bcd8SysMgmtLow;
	bcd8_t bcd8SMAPIIfaceHigh, bcd8SMAPIIfaceLow;
	bcd8_t bcd8VideoHigh, bcd8VideoLow;
	smapi_ioparm_t ioparmThe;
	int intRtn;

	/* Check structure size */
	if( pbiosinfoThe->sizeStruct != SIZE_BIOSINFO_V1 ) {
#		if DEBUG_STRUCT_SIZES
		printf( "Declared size %d does not match expected size %d in GetBiosInfo\n" , pbiosinfoThe->sizeStruct, SIZE_BIOSINFO_V1 );
#		endif
		return ERR_SMAPIDEV_STRUCTURE_SIZE_INVALID;
	}

	memset( &ioparmThe, 0, sizeof( ioparmThe ) );

	ioparmThe.in.bFunc = (byte) 0;
	ioparmThe.in.bSubFunc = (byte) 0;

	intRtn = ioctl_smapi( intFiledesc, &ioparmThe );
	if ( intRtn ) return intRtn;

	pbiosinfoThe->wSysId               = ioparmThe.out.wParm1;
	pbiosinfoThe->wCountryCode         = ioparmThe.out.wParm2;

	bcd8SysHigh =     (bcd8_t)( ioparmThe.out.wParm3 >> 8          );
	bcd8SysLow =      (bcd8_t)( ioparmThe.out.wParm3        & 0xFF );
	pbiosinfoThe->wSysBiosRevMajor     = (word) byte_of_bcd8( bcd8SysHigh );
	pbiosinfoThe->wSysBiosRevMinor     = (word) byte_of_bcd8( bcd8SysLow );

	bcd8SysMgmtHigh = (bcd8_t)( (ioparmThe.out.dwParm4 >> 8) & 0xFF );
	bcd8SysMgmtLow =  (bcd8_t)(  ioparmThe.out.dwParm4       & 0xFF );
	pbiosinfoThe->wSysMgmtBiosRevMajor = (word) byte_of_bcd8( bcd8SysMgmtHigh );
	pbiosinfoThe->wSysMgmtBiosRevMinor = (word) byte_of_bcd8( bcd8SysMgmtLow );

	bcd8SMAPIIfaceHigh =   (bcd8_t)( (ioparmThe.out.dwParm5 >> 8) & 0xFF );
	bcd8SMAPIIfaceLow =    (bcd8_t)(  ioparmThe.out.dwParm5       & 0xFF );
	pbiosinfoThe->wSmapiBiosIfaceRevMajor = (word) byte_of_bcd8( bcd8SMAPIIfaceHigh );
	pbiosinfoThe->wSmapiBiosIfaceRevMinor = (word) byte_of_bcd8( bcd8SMAPIIfaceLow );

	/*** Video BIOS info ***/
	ioparmThe.in.bFunc = (byte) 0;
	ioparmThe.in.bSubFunc = (byte) 8;

	intRtn = ioctl_smapi( intFiledesc, &ioparmThe );
	if ( intRtn ) return intRtn;

	bcd8VideoHigh =   (bcd8_t)( (ioparmThe.out.wParm1 >> 8)        );
	bcd8VideoLow =    (bcd8_t)(  ioparmThe.out.wParm1       & 0xFF );

	pbiosinfoThe->wVideoBiosRevMajor = (word) byte_of_bcd8( bcd8VideoHigh );
	pbiosinfoThe->wVideoBiosRevMinor = (word) byte_of_bcd8( bcd8VideoLow );

	return 0;
}


int smapidev_GetCpuInfo(
	int intFiledesc,
	smapidev_cpuinfo_t *pcpuinfoThe
) {
	smapi_ioparm_t ioparmThe;
	int intRtn;

	/* Check structure size */
	if( pcpuinfoThe->sizeStruct != SIZE_CPUINFO_V1 ) {
#		if DEBUG_STRUCT_SIZES
		printf( "Declared size %d does not match expected size %d in GetCpuInfo\n" , pcpuinfoThe->sizeStruct, SIZE_CPUINFO_V1 );
#		endif
		return ERR_SMAPIDEV_STRUCTURE_SIZE_INVALID;
	}

	memset( &ioparmThe, 0, sizeof( ioparmThe ) );
	ioparmThe.in.bFunc = (byte) 0;
	ioparmThe.in.bSubFunc = (byte) 1;

	intRtn = ioctl_smapi( intFiledesc, &ioparmThe );
	if ( intRtn ) return intRtn;

	pcpuinfoThe->wManufacturer = (word)( 0xFF &  ioparmThe.out.wParm1 );
	pcpuinfoThe->wType         = (word)( 0xFF & (ioparmThe.out.wParm2 >> 8) );
	pcpuinfoThe->wStepping     = (word)( 0xFF &  ioparmThe.out.wParm2 );

	pcpuinfoThe->wClock        = (word)( 0xFF & (ioparmThe.out.wParm3 >> 8) );

	if ( pcpuinfoThe->wClock == 0xfe )
		pcpuinfoThe->wClock = (word) ioparmThe.out.dwParm4;

	pcpuinfoThe->wInternalClock = (word)( 0xFF & ioparmThe.out.wParm3 );
	if ( pcpuinfoThe->wInternalClock == 0xfe )
		pcpuinfoThe->wInternalClock = (word) ioparmThe.out.dwParm5;

	return 0;
}


int smapidev_GetDisplayInfo(
	int intFiledesc,
	smapidev_displayinfo_t *pdisplayinfoThe
) {
	smapi_ioparm_t ioparmThe;
	int intRtn;

	/* Check structure size */
	if( pdisplayinfoThe->sizeStruct != SIZE_DISPLAYINFO_V1 ) {
#		if DEBUG_STRUCT_SIZES
		printf( "Declared size %d does not match expected size %d in GetDisplayInfo\n" , pdisplayinfoThe->sizeStruct, SIZE_DISPLAYINFO_V1 );
#		endif
		return ERR_SMAPIDEV_STRUCTURE_SIZE_INVALID;
	}

	memset( &ioparmThe, 0, sizeof( ioparmThe ) );
	ioparmThe.in.bFunc    = (byte) 0;
	ioparmThe.in.bSubFunc = (byte) 2;
	ioparmThe.in.wParm1   = (word) 0x300;

	intRtn = ioctl_smapi( intFiledesc, &ioparmThe );
	if ( intRtn ) return intRtn;

	pdisplayinfoThe->bPanelType   = (byte)(ioparmThe.out.wParm1 >> 8);
	pdisplayinfoThe->bPanelDim    = (byte)(ioparmThe.out.wParm1 & 0xFF);
	pdisplayinfoThe->bCrtType     = (byte)(ioparmThe.out.wParm2 >> 8);
	pdisplayinfoThe->bCrtFeatures = (byte)(ioparmThe.out.wParm2 & 0xFF);

	return 0;
}


int smapidev_GetDockingInfo(
	int intFiledesc,
	smapidev_dockinginfo_t *pdockinginfoThe
) {
	smapi_ioparm_t ioparmThe;
	int intRtn;

	/* Check structure size */
	if( pdockinginfoThe->sizeStruct != SIZE_DOCKINGINFO_V1 ) {
#		if DEBUG_STRUCT_SIZES
		printf( "Declared size %d does not match expected size %d in GetDockingInfo\n" , pdockinginfoThe->sizeStruct, SIZE_DOCKINGINFO_V1 );
#		endif
		return ERR_SMAPIDEV_STRUCTURE_SIZE_INVALID;
	}

	memset( &ioparmThe, 0, sizeof( ioparmThe ) );
	ioparmThe.in.bFunc = (byte) 0;
	ioparmThe.in.bSubFunc = (byte) 3;

	intRtn = ioctl_smapi( intFiledesc, &ioparmThe );
	if ( intRtn ) return intRtn;

	pdockinginfoThe->wID           =           ioparmThe.out.wParm1;  
  	pdockinginfoThe->fDocked       = (flag_t)( ioparmThe.out.bSubRc       & 1);
	pdockinginfoThe->fKeyUnlocked  = (flag_t)((ioparmThe.out.bSubRc >> 6) & 1);
	pdockinginfoThe->fBusConnected = (flag_t)((ioparmThe.out.bSubRc >> 7) & 1);

	return 0;
}


int smapidev_GetUltrabayInfo(
	int intFiledesc,
	smapidev_ultrabayinfo_t *pultrabayinfoThe
) {
	smapi_ioparm_t ioparmThe;
	int intRtn;

	/* Check structure size */
	if( pultrabayinfoThe->sizeStruct != SIZE_ULTRABAYINFO_V1 ) {
#		if DEBUG_STRUCT_SIZES
		printf( "Declared size %d does not match expected size %d in GetUltrabayInfo\n" , pultrabayinfoThe->sizeStruct, SIZE_ULTRABAYINFO_V1 );
#		endif
		return ERR_SMAPIDEV_STRUCTURE_SIZE_INVALID;
	}

	memset( &ioparmThe, 0, sizeof( ioparmThe ) );
	ioparmThe.in.bFunc = (byte) 0;
	ioparmThe.in.bSubFunc = (byte) 4;

	intRtn = ioctl_smapi( intFiledesc, &ioparmThe );
	if ( intRtn ) return intRtn;

	pultrabayinfoThe->bType = (byte)(ioparmThe.out.wParm2 >> 8);
	pultrabayinfoThe->bID   = (byte)(ioparmThe.out.wParm2 & 0xFF);

	return 0;
}


/*
 * The ThinkPad 560Z Technical Reference describes function 0:6 as
 * "Get Power Management Module Information" while the ThinkPad 600
 * describes it as "Get Slave Micro Control Unit Information"
 */
int smapidev_GetSlaveControllerInfo(
	int intFiledesc,
	smapidev_slaveinfo_t *pslaveinfoThe
) {
	smapi_ioparm_t ioparmThe;
	bcd8_t bcd8High = 0, bcd8Low = 0;
	flag_t fInvalidID;
	int intRtn;

	/* Check structure size */
	if( pslaveinfoThe->sizeStruct != SIZE_SLAVEINFO_V1 ) {
#		if DEBUG_STRUCT_SIZES
		printf( "Declared size %d does not match expected size %d in GetSlaveControllerInfo\n" , pslaveinfoThe->sizeStruct, SIZE_SLAVEINFO_V1 );
#		endif
		return ERR_SMAPIDEV_STRUCTURE_SIZE_INVALID;
	}

	memset( &ioparmThe, 0, sizeof( ioparmThe ) );
	ioparmThe.in.bFunc = (byte) 0;
	ioparmThe.in.bSubFunc = (byte) 6;

	intRtn = ioctl_smapi( intFiledesc, &ioparmThe );
	if ( intRtn ) return intRtn;

	if ( ioparmThe.out.wParm2 == 0xFFFF ) {
		fInvalidID = 1;
	} else {
		fInvalidID = 0;
		bcd8High = (bcd8_t)( ioparmThe.out.wParm2 >> 8 );
		bcd8Low  = (bcd8_t)( ioparmThe.out.wParm2 & 0xFF );
	}

	pslaveinfoThe->fAscii  = (ioparmThe.out.bSubRc == 0);
	if ( fInvalidID ) {
		pslaveinfoThe->wIDMajor = 0xFFFF;
		pslaveinfoThe->wIDMinor = 0xFFFF;
	} else {
		pslaveinfoThe->wIDMajor = byte_of_bcd8( bcd8High );
		pslaveinfoThe->wIDMinor = byte_of_bcd8( bcd8Low );
	}
	pslaveinfoThe->szID[0] = (char)(0xFF&(ioparmThe.out.wParm2>>8));
	pslaveinfoThe->szID[1] = (char)(0xFF&ioparmThe.out.wParm2);
	pslaveinfoThe->szID[2] = '\0'; /* Add zero termination */

	return 0;
}


int smapidev_GetSensorInfo(
	int intFiledesc,
	smapidev_sensorinfo_t *psensorinfoThe
) {
	smapi_ioparm_t ioparmThe;
	int intRtn;

	/* Check structure size */
	if( psensorinfoThe->sizeStruct != SIZE_SENSORINFO_V1 ) {
#		if DEBUG_STRUCT_SIZES
		printf( "Declared size %d does not match expected size %d in GetSensorInfo\n" , psensorinfoThe->sizeStruct, SIZE_SENSORINFO_V1 );
#		endif
		return ERR_SMAPIDEV_STRUCTURE_SIZE_INVALID;
	}

	memset( &ioparmThe, 0, sizeof( ioparmThe ) );
	ioparmThe.in.bFunc = (byte) 0;
	ioparmThe.in.bSubFunc = (byte) 7;

	intRtn = ioctl_smapi( intFiledesc, &ioparmThe );
	if ( intRtn ) return intRtn;

	psensorinfoThe->fLidClosed         = (ioparmThe.out.wParm2 >>  8) & 1;
	psensorinfoThe->fKeyboardOpen      = (ioparmThe.out.wParm2 >>  9) & 1;
	psensorinfoThe->fACAdapterAttached = (ioparmThe.out.wParm2 >> 10) & 1;

	return 0;
}

int smapidev_GetScreenRefreshInfo(
	int intFiledesc,
	word wMode,
	smapidev_screenrefreshinfo_t *pscreenrefreshinfoThe
) {
	smapi_ioparm_t ioparmThe;
	int intRtn;

	/* Check structure size */
	if( pscreenrefreshinfoThe->sizeStruct != SIZE_SCREENREFRESHINFO_V1 ) {
#		if DEBUG_STRUCT_SIZES
		printf( "Declared size %d does not match expected size %d in GetScreenRefreshInfo\n" , pscreenrefreshinfoThe->sizeStruct, SIZE_SCREENREFRESHINFO_V1 );
#		endif
		return ERR_SMAPIDEV_STRUCTURE_SIZE_INVALID;
	}

	memset( &ioparmThe, 0, sizeof( ioparmThe ) );
	ioparmThe.in.bFunc = (byte) 0;
	ioparmThe.in.bSubFunc = (byte) 9;
	ioparmThe.in.wParm1 = wMode;

	intRtn = ioctl_smapi( intFiledesc, &ioparmThe );
	if ( intRtn ) return intRtn;

	pscreenrefreshinfoThe->f43i = (ioparmThe.out.wParm2 >> 3) & 1;
	pscreenrefreshinfoThe->f48i = (ioparmThe.out.wParm2 >> 7) & 1;
	pscreenrefreshinfoThe->f56  = (ioparmThe.out.wParm2 >> 4) & 1;
	pscreenrefreshinfoThe->f60  =  ioparmThe.out.wParm2 & 1;
	pscreenrefreshinfoThe->f70  = (ioparmThe.out.wParm2 >> 5) & 1;
	pscreenrefreshinfoThe->f72  = (ioparmThe.out.wParm2 >> 1) & 1;
	pscreenrefreshinfoThe->f75  = (ioparmThe.out.wParm2 >> 2) & 1;
	pscreenrefreshinfoThe->f85  = (ioparmThe.out.wParm2 >> 6) & 1;

	return 0;
}


int smapidev_GetDisplayCapability(
	int intFiledesc,
	smapidev_stateplace_t stateplace,
	smapidev_displaycap_t *pdisplaycapThe
) {
	smapi_ioparm_t ioparmThe;
	int intRtn;

	/* Check structure size */
	if( pdisplaycapThe->sizeStruct != SIZE_DISPLAYCAP_V1 ) {
#		if DEBUG_STRUCT_SIZES
		printf( "Declared size %d does not match expected size %d in GetDisplayCapability\n" , pdisplaycapThe->sizeStruct, SIZE_DISPLAYCAP_V1 );
#		endif
		return ERR_SMAPIDEV_STRUCTURE_SIZE_INVALID;
	}

	memset( &ioparmThe, 0, sizeof( ioparmThe ) );
	ioparmThe.in.bFunc = (byte) 0x10;
	ioparmThe.in.bSubFunc = (byte) 0;
	switch ( stateplace ) {
		case SMAPIDEV_STATEPLACE_CMOS: ioparmThe.in.wParm1 = (word) 1; break;
		case SMAPIDEV_STATEPLACE_CURR: ioparmThe.in.wParm1 = (word) 0; break;
		case SMAPIDEV_STATEPLACE_CMOS_AND_CURR:
		default: return ERR_SMAPIDEV_PARM_INVALID;
	}

	intRtn = ioctl_smapi( intFiledesc, &ioparmThe );
	if ( intRtn ) return intRtn;

	pdisplaycapThe->fSupported = (flag_t)(ioparmThe.out.wParm2 & 1);
	switch ( ioparmThe.out.wParm2 & 0xFF ) {
		case 0:
			pdisplaycapThe->tv = SMAPIDEV_DISPLAYCAPTV_NONE;
		break;
		case 1:
			pdisplaycapThe->tv = SMAPIDEV_DISPLAYCAPTV_NONSIMULTANEOUS;
		break;
		default:
			pdisplaycapThe->tv = SMAPIDEV_DISPLAYCAPTV_OTHER;
			return ERR_SMAPIDEV_SMAPI_RESULT_NOT_UNDERSTOOD;
	}

	return 0;
}


int smapidev_GetDisplayState(
	int intFiledesc,
	smapidev_stateplace_t stateplace,
	smapidev_dispmode_t dispmodeThe,
	smapidev_ablestate_t *pablestateThe
) {
	smapi_ioparm_t ioparmThe;
	int intRtn;

	memset( &ioparmThe, 0, sizeof( ioparmThe ) );
	ioparmThe.in.bFunc = (byte) 0x10;
	ioparmThe.in.bSubFunc = (byte) 0;
	switch ( stateplace ) {
		case SMAPIDEV_STATEPLACE_CMOS: ioparmThe.in.wParm1 = (word) 1; break;
		case SMAPIDEV_STATEPLACE_CURR: ioparmThe.in.wParm1 = (word) 0; break;
		case SMAPIDEV_STATEPLACE_CMOS_AND_CURR:
		default: return ERR_SMAPIDEV_PARM_INVALID;
	}

	intRtn = ioctl_smapi( intFiledesc, &ioparmThe );
	if ( intRtn ) return intRtn;

	switch( dispmodeThe )
	{
		case SMAPIDEV_DISPMODE_INTERNAL :
			*pablestateThe = ( ioparmThe.out.wParm2 & 0x100 ) ? SMAPIDEV_ABLESTATE_ENABLED : SMAPIDEV_ABLESTATE_DISABLED ;
		break;

		case SMAPIDEV_DISPMODE_CRT :
			*pablestateThe = ( ioparmThe.out.wParm2 & 0x200 ) ? SMAPIDEV_ABLESTATE_ENABLED : SMAPIDEV_ABLESTATE_DISABLED ;
		break;

		case SMAPIDEV_DISPMODE_TV :
			*pablestateThe = ( ioparmThe.out.wParm2 & 0x400 ) ? SMAPIDEV_ABLESTATE_ENABLED : SMAPIDEV_ABLESTATE_DISABLED ;
		break;

		case SMAPIDEV_DISPMODE_CRT_DETECTION_IGNORE :
			*pablestateThe = ( ioparmThe.out.wParm2 & 0x4000 ) ? SMAPIDEV_ABLESTATE_ENABLED : SMAPIDEV_ABLESTATE_DISABLED ;
		break;

		case SMAPIDEV_DISPMODE_DUAL :
			*pablestateThe = ( ioparmThe.out.wParm2 & 0x8000 ) ? SMAPIDEV_ABLESTATE_ENABLED : SMAPIDEV_ABLESTATE_DISABLED ;
		break;

		case SMAPIDEV_DISPMODE_SELECT_TV :
			*pablestateThe = ( ioparmThe.out.dwParm4 & 1 ) ? SMAPIDEV_ABLESTATE_ENABLED : SMAPIDEV_ABLESTATE_DISABLED ;
		break;

		default:
			return ERR_SMAPIDEV_PARM_INVALID;
	}

	return 0;
}


int smapidev_SetDisplayState(
	int intFiledesc,
	smapidev_stateplace_t stateplace,
	smapidev_dispmode_t dispmodeThe,
	smapidev_ablestate_t ablestateThe
) {
	smapi_ioparm_t ioparmGet;
	smapi_ioparm_t ioparmSet;
	int intRtn;

	/* We can only update CMOS and current state together */
	if ( stateplace != SMAPIDEV_STATEPLACE_CMOS_AND_CURR ) 
		return ERR_SMAPIDEV_PARM_INVALID;
	
	/* No SMAPIDEV_STATE_AUTO or other invalid values are allowed here */
	if( ablestateThe != SMAPIDEV_ABLESTATE_DISABLED && ablestateThe != SMAPIDEV_ABLESTATE_ENABLED )
	{
		return ERR_SMAPIDEV_PARM_INVALID;
	}

	/* Get the current CMOS state */
	memset( &ioparmGet, 0, sizeof( ioparmGet ) );
	ioparmGet.in.bFunc     = (byte)  0x10;
	ioparmGet.in.bSubFunc  = (byte)  0;
	ioparmGet.in.wParm1    = (word) 1; 

	intRtn = ioctl_smapi( intFiledesc, &ioparmGet );
	if ( intRtn ) return intRtn;

	memset( &ioparmSet, 0, sizeof( ioparmSet ) );
	ioparmSet.in.bFunc     = (byte) 0x10;
	ioparmSet.in.bSubFunc  = (byte) 1;
	ioparmSet.in.wParm1    = ioparmGet.out.wParm2 & 0xC700;
	ioparmSet.in.dwParm4   = ioparmGet.out.dwParm4 & 0x0001;

	switch( dispmodeThe )
	{

		case SMAPIDEV_DISPMODE_INTERNAL :
			if( ablestateThe == SMAPIDEV_ABLESTATE_ENABLED )
				ioparmSet.in.wParm1 |=  0x100;
			else
				ioparmSet.in.wParm1 &= ~0x100;
		break;

		case SMAPIDEV_DISPMODE_CRT :
			if( ablestateThe == SMAPIDEV_ABLESTATE_ENABLED )
				ioparmSet.in.wParm1 |=  0x200;
			else
				ioparmSet.in.wParm1 &= ~0x200;
		break;

		case SMAPIDEV_DISPMODE_TV :
			if( ablestateThe == SMAPIDEV_ABLESTATE_ENABLED )
				ioparmSet.in.wParm1 |=  0x400;
			else
				ioparmSet.in.wParm1 &= ~0x400;
		break;

		case SMAPIDEV_DISPMODE_CRT_DETECTION_IGNORE :
			if( ablestateThe == SMAPIDEV_ABLESTATE_ENABLED )
				ioparmSet.in.wParm1 |=  0x4000;
			else
				ioparmSet.in.wParm1 &= ~0x4000;
		break;

		case SMAPIDEV_DISPMODE_DUAL :
			if( ablestateThe == SMAPIDEV_ABLESTATE_ENABLED )
				ioparmSet.in.wParm1 |=  0x8000;
			else
				ioparmSet.in.wParm1 &= ~0x8000;
		break;

		case SMAPIDEV_DISPMODE_SELECT_TV :
			if( ablestateThe == SMAPIDEV_ABLESTATE_ENABLED )
				ioparmSet.in.dwParm4 |=  0x1;
			else
				ioparmSet.in.dwParm4 &= ~0x1;
		break;

		default:
			return ERR_SMAPIDEV_PARM_INVALID;
	}

	return ioctl_smapi( intFiledesc, &ioparmSet );
}


int smapidev_GetPowerExpenditureMode(
	int intFiledesc,
	smapidev_powersrc_t powersrcThe,
	smapidev_powermode_t *ppowermodeThe
) {
	smapi_ioparm_t ioparmThe;
	int intRtn;
	byte bModeAC, bModeBat, bModeSelected;

	memset( &ioparmThe, 0, sizeof( ioparmThe ) );
	ioparmThe.in.bFunc = (byte) 0x22;
	ioparmThe.in.bSubFunc = (byte) 0;

	intRtn = ioctl_smapi( intFiledesc, &ioparmThe );
	if ( intRtn ) return intRtn;

	bModeAC = (byte)(ioparmThe.out.wParm2 & 0xFF);
	bModeBat = (byte)(ioparmThe.out.wParm2 >> 8);
	bModeSelected = (powersrcThe == SMAPIDEV_POWERSRC_AC) ? bModeAC : bModeBat;

	switch ( bModeSelected ) {
		case 0: *ppowermodeThe = SMAPIDEV_POWERMODE_HIGH; break;
		case 1: *ppowermodeThe = SMAPIDEV_POWERMODE_AUTO; break;
		case 2: *ppowermodeThe = SMAPIDEV_POWERMODE_MANUAL; break;
		default:
		case 3: *ppowermodeThe = SMAPIDEV_POWERMODE_UNRECOGNIZED; break;
	}

	return 0;
}



int smapidev_SetPowerExpenditureMode(
	int intFiledesc,
	smapidev_powersrc_t powersrcThe,
	smapidev_powermode_t powermodeThe
) {
	smapi_ioparm_t ioparmThe;
	int intRtn;
	byte bMode;

	bMode =
		(powermodeThe == SMAPIDEV_POWERMODE_HIGH) ? 0 :
		(powermodeThe == SMAPIDEV_POWERMODE_AUTO) ? 1 :
		2
	;

	memset( &ioparmThe, 0, sizeof( ioparmThe ) );
	ioparmThe.in.bFunc = (byte) 0x22;
	ioparmThe.in.bSubFunc = (byte) 0;

	intRtn = ioctl_smapi( intFiledesc, &ioparmThe );
	if ( intRtn ) return intRtn;

	ioparmThe.in.bFunc = (byte) 0x22;
	ioparmThe.in.bSubFunc = (byte) 1;
	ioparmThe.in.wParm1 = ioparmThe.out.wParm2;
	if ( powersrcThe == SMAPIDEV_POWERSRC_AC ) {
		ioparmThe.in.wParm1 &= 0xff00;
		ioparmThe.in.wParm1 |= bMode;
	} else { /* powersrcThe == SMAPIDEV_POWERSRC_BATTERY */
		ioparmThe.in.wParm1 &= 0x00ff;
		ioparmThe.in.wParm1 |= ((word)bMode) << 8;
	}

	intRtn = ioctl_smapi( intFiledesc, &ioparmThe );
	if ( intRtn ) return intRtn;

	return 0;
}
#else
int smapi_dummy(){}
#endif
