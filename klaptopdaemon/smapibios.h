
/*********************************************************************
 *                
 * Filename:      smapibios.h
 * Description:   header file for the IBM SMAPI BIOS
 * Author:        Thomas Hood
 * Created:       14 July 1999 
 *
 * Please report bugs to the author ASAP.
 * 
 *     Copyright (c) 1999 J.D. Thomas Hood, All rights reserved
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

#ifndef __SMAPIBIOS_H__
#define __SMAPIBIOS_H__

/* Is included by smapi.h */

/*** SMAPI BIOS error codes ***/
#define ERR_SMB_MIN                   ((byte)0x01)
#define ERR_SMB_FUNC_NOT_AVAIL        ((byte)0x53)
#define ERR_SMB_FUNC_NOT_SUPPORTED    ((byte)0x86)
#define ERR_SMB_SYSTEM_ERROR          ((byte)0x90)
#define ERR_SMB_SYSTEM_INVALID        ((byte)0x91)
#define ERR_SMB_SYSTEM_BUSY           ((byte)0x92)
#define ERR_SMB_DEVICE_ERROR          ((byte)0xA0)
#define ERR_SMB_DEVICE_BUSY           ((byte)0xA1)
#define ERR_SMB_DEVICE_NOT_ATTACHED   ((byte)0xA2)
#define ERR_SMB_DEVICE_DISABLED       ((byte)0xA3)
#define ERR_SMB_PARM_INVALID          ((byte)0x81)
#define ERR_SMB_PARM_OUT_OF_RANGE     ((byte)0xA4)
#define ERR_SMB_PARM_NOT_ACCEPTED     ((byte)0xA5)
#define ERR_SMB_MAX                   ((byte)0xFF)

/* The following structure definitions come from the ThinkPad 560Z Technical Reference */

/*** SMAPI BIOS header ***/
typedef struct _smb_header {
	byte	bSig[4];        /* signature */
	byte 	bVerMajor;      /* major version */
	byte	bVerMinor;      /* minor version */
	byte    bLen;           /* length */
	byte    bChksum;        /* checksum */
	word   wInfo;          /* information word */
	word	wRsv1;          /* reserve 1 */
	word   wR_offset;      /* real mode offset */
	word   wR_segment;     /* real mode segment */
	word	wRsv2;          /* reserve 2 */
	word	wP16_offset;    /* 16-bit protect mode offset */
	dword	dwP16_base;     /* 16-bit protect mode base address */
	dword	dwP32_offset;   /* 32-bit protect mode offset */
	dword	dwP32_base;     /* 32-bit protect mode base address */
} smb_header_t; 

/*** SMAPI BIOS call input parameters ***/
typedef struct _smb_inparm {
	byte	bFunc;
	byte	bSubFunc;
	word	wParm1;
	word	wParm2;
	word	wParm3;
	dword	dwParm4;
	dword	dwParm5;
} smb_inparm_t;


/*** SMAPI BIOS call output parameters ***/
typedef struct _smb_outparm {
	byte	bRc;
	byte	bSubRc;
	word	wParm1;
	word	wParm2;
	word	wParm3;
	dword	dwParm4;
	dword	dwParm5;
} smb_outparm_t;

#endif
