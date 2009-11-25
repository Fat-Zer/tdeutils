/*  System Information
 *
 *  Copyright (C) 2001  Robbie Ward <linuxphreak@gmx.co.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "systeminfo.h"

#ifdef __linux__
#include <sys/sysinfo.h>
#include <linux/kernel.h>
#endif

#ifdef HAVE_SYS_LOADAVG_H
#include <sys/loadavg.h>
#endif

#ifdef __FreeBSD__
#include <osreldate.h>
#include <kvm.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/conf.h>
#if __FreeBSD_version < 400000
#include <sys/rlist.h>
#endif
#include <sys/vmmeter.h>
#include <sys/sysctl.h>
#include <vm/vm_param.h>
#include <iomanip>
#include <iostream>
#endif

// Some global vars
// expand as needed for other platforms
int System::bytesToMegs( unsigned long value )
{
#ifdef __linux__
  return ((((value / 1024) >> 10) | 11) + 1);
#endif

#ifdef __FreeBSD__
  return (((value) + (1 << 19)) >> 20);
#endif
}

System * System::m_self = 0; // initialize pointer
System & System::self()
{
  if ( !m_self )
  {
    m_self = new System;
    atexit( System::cleanup );
  }

  m_self->updateData();
  return *m_self;
}

void System::cleanup()
{
  if ( !m_self )
    return;

  delete m_self;
  m_self = 0;
}

double System::loadAverage( int load ) const
{
  int value = --load;
  return loadAverages()[ value > 2 ? 2 : value ];
}

System::System()
{
  m_loads[ 0 ] = 0;
  m_loads[ 1 ] = 0;
  m_loads[ 2 ] = 0;

  m_uptime = 0;
  m_totalram = 0;
  m_usedram = 0;
  m_freeram = 0;
  m_sharedram = 0;
  m_bufferram = 0;
  m_cacheram = 0;
  m_totalhigh = 0;
  m_freehigh = 0;
  m_totalswap = 0;
  m_usedswap = 0;
  m_freeswap = 0;
  m_procs = 0;

  m_self = this;
}

System::~System()
{
}

void System::updateData()
{
#ifdef __linux__
  struct sysinfo system;
  if (sysinfo(&system) < 0) {
    fprintf(stderr, "Error calling sysinfo()\n");
    return;
  }

  m_uptime = system.uptime;
  m_totalram = system.totalram;
  m_sharedram = system.sharedram;
  m_bufferram = system.bufferram;
  m_usedram = m_totalram - m_freeram;

#ifdef HAVE_SYSINFO_HIGH
  m_totalhigh = system.totalhigh;
  m_freehigh = system.freehigh;
#else
#ifdef __GNUC__
#warning "TODO: find alternative way to get total high and freehigh"
#endif
#endif

  m_totalswap = system.totalswap;
  m_freeswap = system.freeswap;
  m_procs = system.procs;
  m_cacheram = 0;

  FILE *file = fopen("/proc/meminfo", "r");
  char buffer[70];
  if (file) {
    while (fgets(buffer, sizeof(buffer), file)) {
      sscanf(buffer, "Mem: %*d %*d %*d %*d %*d %lu", &m_cacheram);
      if (m_cacheram != 0)
        break;
    }

    fclose(file);
  }

  m_freeram = system.freeram;
#endif

#ifdef __FreeBSD__
  struct timeval bootTime;
  size_t size = sizeof(bootTime);

  if (sysctlbyname("kern.boottime",
        &bootTime, &size, NULL, 0) != -1 && bootTime.tv_sec != 0) {
    m_uptime = time(0) - bootTime.tv_sec + 30;
  }

  char buf[80];
  char *used;
  char *total;
  FILE *pipe;
  static int psize = 0, pshift = 0;
  static int name2oid[2] = { 0, 3 };
  struct vmtotal vmt;
  size_t vmtLength = sizeof(vmt);
  static char name[] = "vfs.bufspace";
  static int oidBufspace[CTL_MAXNAME + 2];
  static size_t oidBufspaceLength = sizeof(oidBufspace);
  size_t bufspaceLength = sizeof(int);
  static int initialized = 0;
  unsigned long memXUsed, memTotal;
  size_t memLength = sizeof(memTotal);

  if (pshift == 0) {
    psize = getpagesize();
    while (psize > 1) {
      ++pshift;
      psize >>= 1;
    }
  }

  unsigned long xused1, xused2;
  size_t xused1Length = sizeof(xused1);
  size_t xused2Length = sizeof(xused2);
  if (sysctlbyname("vm.stats.vm.v_active_count", &xused1, &xused1Length, NULL, 0) == 0
     && sysctlbyname("vm.stats.vm.v_inactive_count", &xused2, &xused2Length, NULL, 0) == 0) {
    memXUsed = (xused1 + xused2) << pshift;
  }

  unsigned long cacheRam;
  size_t cacheSize = sizeof(cacheRam);
  if (sysctlbyname("vm.stats.vm.v_active_count", &cacheRam, &cacheSize, NULL, 0) == 0) {
    m_cacheram = cacheRam << pshift;
  }

  if (!m_totalram && sysctlbyname("hw.physmem", &memTotal, &memLength, NULL, 0) == 0)
    m_totalram = memTotal;

  if (sysctlbyname("vm.meter", &vmt, &vmtLength, NULL, 0) == 0)
    m_sharedram = vmt.t_rmshr << pshift;

  if (!initialized) {
    if (sysctl(name2oid, 2, oidBufspace, &oidBufspaceLength,
          name, strlen(name)) < 0)
      return;

    oidBufspaceLength /= sizeof(int);
    ++initialized;
  }

  if (sysctl(oidBufspace, oidBufspaceLength,
         &m_bufferram, &bufspaceLength, 0, 0) < 0)
    return;

  m_usedram = memXUsed - m_bufferram - m_cacheram;
  m_freeram = memTotal - m_usedram;

  //TODO: get the total and free high mem values
  m_totalhigh = 0;
  m_freehigh = 0;

  if ((pipe = popen("/usr/sbin/swapinfo -k", "r")) == 0) {
    fprintf(stderr, "Error reading swap info\n");
    m_freeswap = m_totalswap = 1;
    return;
  }

  fgets(buf, sizeof(buf), pipe);
  fgets(buf, sizeof(buf), pipe);
  fgets(buf, sizeof(buf), pipe);
  fgets(buf, sizeof(buf), pipe);
  pclose(pipe);

  strtok(buf, " ");
  total = strtok(NULL, " ");
  used = strtok(NULL, " ");
  unsigned long usedSwap = atol(used) * 1024;

  m_totalswap = atol(total) * 1024;
  m_freeswap = m_totalswap - usedSwap;

  //TODO: get amount of procs
  m_procs = 0;
#endif

#ifdef HAVE_GETLOADAVG
  double load[3];
  if (getloadavg(load, 3) != -1) {
    m_loads[0] = load[0];
    m_loads[1] = load[1];
    m_loads[2] = load[2];
  }
#else
#warning "TODO: Find an alternative method for getloadavg"
#endif

  m_usedswap = m_totalswap - m_freeswap;
}
