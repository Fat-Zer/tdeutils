
/*********************************************************************
 *                
 * Filename:      thinkpad_common.h
 * Description:   stuff required by the "thinkpad" drivers and
 *                by programs accessing them
 * Author:        Thomas Hood
 * Created:       19 July 1999 
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

#ifndef __THINKPAD_COMMON_H__
#define __THINKPAD_COMMON_H__

#include <config.h>
#include <sys/types.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

/* All module (etc.) names should be no longer than this: */
#define LEN_NAME_MAX 80

/* All version strings should be no longer than this: */
#define LEN_VERSION_MAX 30

/****** macros ******/

#ifdef DEBUG_IOPARM
#define DEBUG_PRINT_OUTPARMS( ioparmThe ) { \
	printf( "bRc:      0x%x\n", ioparmThe.out.bRc ); \
	printf( "bSubRc:   0x%x\n", ioparmThe.out.bSubRc ); \
	printf( "wParm1:   0x%x\n", ioparmThe.out.wParm1 ); \
	printf( "wParm2:   0x%x\n", ioparmThe.out.wParm2 ); \
	printf( "wParm3:   0x%x\n", ioparmThe.out.wParm3 ); \
	printf( "dwParm4:  0x%lx\n", (unsigned long) ioparmThe.out.dwParm4 ); \
	printf( "dwParm5:  0x%lx\n", (unsigned long) ioparmThe.out.dwParm5 ); \
}
#define DEBUG_PRINT_INPARMS( ioparmThe ) { \
	printf( "bFunc:    0x%x\n", ioparmThe.in.bFunc ); \
	printf( "bSubFunc: 0x%x\n", ioparmThe.in.bSubFunc ); \
	printf( "wParm1:   0x%x\n", ioparmThe.in.wParm1 ); \
	printf( "wParm2:   0x%x\n", ioparmThe.in.wParm2 ); \
	printf( "wParm3:   0x%x\n", ioparmThe.in.wParm3 ); \
	printf( "dwParm4:  0x%lx\n", (unsigned long) ioparmThe.in.dwParm4 ); \
	printf( "dwParm5:  0x%lx\n", (unsigned long) ioparmThe.in.dwParm5 ); \
}
#else
#define DEBUG_PRINT_OUTPARMS( ioparmThe )
#define DEBUG_PRINT_INPARMS( ioparmThe )
#endif


/****** types ******/
typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;
typedef char flag_t;
typedef byte bcd8_t;

/*** ioctl commands ***/

/*#include "thinkpad.h" */
#include "smapi.h"
/*#include "superio.h" */
/*#include "rtcmosram.h" */
/*#include "thinkpadpm.h" */

#define MAGIC_THINKPAD_IOCTL  ('(')
#define IOCTL_THINKPAD_GETVER    _IOR (MAGIC_THINKPAD_IOCTL,0x1,thinkpad_ioparm_t)
#define IOCTL_THINKPAD_ENABLE    _IOWR(MAGIC_THINKPAD_IOCTL,0x2,thinkpad_ioparm_t)
#define IOCTL_THINKPAD_DISABLE   _IOWR(MAGIC_THINKPAD_IOCTL,0x3,thinkpad_ioparm_t)
#define IOCTL_SMAPI_REQUEST      _IOWR(MAGIC_THINKPAD_IOCTL,0x10,smapi_ioparm_t)
#define IOCTL_SUPERIO_REQUEST    _IOWR(MAGIC_THINKPAD_IOCTL,0x11,superio_ioparm_t)
#define IOCTL_RTCMOSRAM_REQUEST  _IOWR(MAGIC_THINKPAD_IOCTL,0x12,rtcmosram_ioparm_t)
#define IOCTL_THINKPADPM_REQUEST _IOWR(MAGIC_THINKPAD_IOCTL,0x20,thinkpadpm_ioparm_t)

/****** return values ******/

/* 0 as a return value always means OK */

/*
 * We use the following standard UNIX errno's, defined in <asm/errno.h>
 * EFAULT       memory error
 * EBUSY        device is already open
 * EINVAL       function code not recognized
 * ENOTTY       ioctl code not recognized
 * EACCESS      inadequate permission to perform action
 *
 * We use the following standard errnos under names more descriptive
 * of our circumstances.  Remember that the ioctl handler in the 
 * driver returns the negatives of these, but they turn up positive
 * in user space in errno after an ioctl() call.  Our convention
 * for ioctl_blah() wrapper functions is to return the errno as a
 * negative number.
 */
#define ETHINKPAD_PROGRAMMING      (1024)
#define ETHINKPAD_HW_NOT_FOUND     (1025)
#define ETHINKPAD_MODULE_DISABLED  (1026)
#define ETHINKPAD_MODULE_NOT_FOUND (1027)
#define ETHINKPAD_SUBDRIVER        (1028)

/*
 * Subdriver error codes are returned in the parameter block
 * and errno is set to THINKPAD_SUBDRIVER
 *
 */

#endif /* __THINKPAD_COMMON_H__ */
