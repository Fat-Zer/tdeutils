
/*********************************************************************
 *                
 * Filename:      smapi.h
 * Description:   header file for the smapi driver
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

#ifndef __SMAPI_H__
#define __SMAPI_H__

#include "smapibios.h"


/****** typedefs ******/

typedef smb_inparm_t smapi_inparm_t;
typedef smb_outparm_t smapi_outparm_t;

typedef union _smapi_ioparm_t {
	smapi_inparm_t in;
	smapi_outparm_t out;
} smapi_ioparm_t;


/****** declarations ******/

int smapi_do(
	unsigned long ulongIoctlArg,
	flag_t fCallerHasWritePerm
);


#endif
