/*
 * acpi_helper.cpp - acpi helper 
 *
 * Copyright (c) 2002 Paul Campbell <paul@taniwha.com>
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

//
//	README!!
//
//	This file contains code that is intended to be run setuid root
//	(only if the end user enables it themselves, it's not set that
//	way as part of a standard KDE build).
//
//	Because of this this code should be simple and easily visually
//	inspected for security holes and/or bugs - if you feel the need
//	to change this file please get someone else to review your work
//	(I'll happily do it for you - mail me at paul@taniwha.com, please
//	review mine!)
//
//	I recommend the following practices here - both for safety and
//	transparency:
//
//		- check all array references (snprintf/strncpy etc)
//
//		- avoid malloc/new calls and pointers  too if possible
//

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

#define MAX_TOSHIBA_STRING 64

/* Write a value to /proc/acpi/sleep, where value may be between
   1 and 4 (whatever they mean). Does not return; calls exit(). */

void write_to_proc_sleep(int value)
{
	char tmp[256];
	int fd;

	/* Sanity check value */
	if ((value<1) || (value>4))
		exit(1);

	/* Convert value to string */
	snprintf(tmp,sizeof(tmp),"%d",value);
	tmp[sizeof(tmp)-1]=0;

	/* Broken imitation of typing sync <enter> sync <enter>
	   on the command line before shutting down the machine;
	   part of the lore of UNIX machines. */
	sync();
	sync();
	fd = open("/proc/acpi/sleep", O_RDWR);
	if (fd < 0)
		exit(1);
	write(fd, tmp, 1);
	close(fd);
	setuid(getuid());	// drop all priority asap
	exit(0);
}

/* Write string to new acpi power interface */
void write_to_power(const char * str)
{
	int fd;
	/* Broken imitation of typing sync <enter> sync <enter>
	   on the command line before shutting down the machine;
	   part of the lore of UNIX machines. */
	sync();
	sync();
	fd = open("/sys/power/state", O_RDWR);
	if (fd < 0)
		exit(1);
	write(fd, str, strlen(str));
	close(fd);
	setuid(getuid());	// drop all priority asap
	exit(0);
}

/* Run the program @param path, if it exists and seems safe to do so.
   Returns only if the program does not exist; if the program exists
   and is unsafe, exit; if the program exists and is safe, run it
   and never return. */
void run_program(const char *path)
{
	struct stat sb;
	int err;

	if (!path) exit(1); /* Bad pointer */
	if (path[0] != '/') exit(1); /* Not an absolute path */

	if ((err = stat(path, &sb)) != 0 || sb.st_mode&S_IWOTH) {
		if (err != 0) {
			fprintf(stderr, "Can't find %s\n", path);
			return;
		} else {
			fprintf(stderr, "%s is writeable by anyone - we don't trust it\n", path);
		}
		exit(1);
	}
	::setuid(::geteuid());					// otherwise bash will throw it away
	::execl(path, NULL);	// this is not KDE environment code 
	exit(0);
}

int
main(int argc, char **argv)
{
	int fd;
	int i;
	int toshibalcd_val = 0;
        bool useSysPower=false;

	fd = open("/sys/power/state", O_RDWR);
	if (fd >= 0)
		useSysPower=true;
        close(fd);

	::close(0);	// we're setuid - this is just in case
	for (i = 1; i < argc; i++)
	if (strcmp(argv[i], "--suspend") == 0 || strcmp(argv[i], "-suspend") == 0) {
		/* Returns only if suspend does not exist. */
		run_program("/usr/sbin/suspend");
		if (useSysPower)
			write_to_power("mem");
		else
			write_to_proc_sleep(3);
		exit(0);
	} else
	if (strcmp(argv[i], "--standby") == 0 || strcmp(argv[i], "-standby") == 0) {
		if (useSysPower)
			write_to_power("standby");
		else
			write_to_proc_sleep(1);
		exit(0);
	} else
	if (strcmp(argv[i], "--standby2") == 0 || strcmp(argv[i], "-standby2") == 0) {
		write_to_proc_sleep(2);
		exit(0);
	} else
	if (strcmp(argv[i], "--hibernate") == 0 || strcmp(argv[i], "-hibernate") == 0) {
		run_program("/usr/sbin/hibernate");
		if (useSysPower)
			write_to_power("disk");
		else
			write_to_proc_sleep(4);
		exit(0);
	} else
	if (strcmp(argv[i], "--software-suspend") == 0 || strcmp(argv[i], "-software-suspend") == 0) {
		run_program("/usr/sbin/hibernate");
		exit(0);
	} else
	if (strcmp(argv[i], "--throttling") == 0 || strcmp(argv[i], "-throttling") == 0) {
		int val;
		char tmp[256];

		i++;
		if (i >= argc) 
			break;
		if (strlen(argv[i]) > 50 || strchr(argv[i], '.'))
			break;
		snprintf(tmp, sizeof(tmp), "/proc/acpi/processor/%s/throttling", argv[i]);
		tmp[sizeof(tmp)-1] = 0;
		i++;
		if (i >= argc) 
			break;
		val= atoi(argv[i]);
		if (val < 0)
			break;
		sync();
		sync();
		fd = open(tmp, O_RDWR);
		if (fd < 0)
			exit(1);
		snprintf(tmp, sizeof(tmp), "%d", val);
		write(fd, tmp, strlen(tmp));
		close(fd);
        	setuid(getuid());	// drop all priority asap
		exit(0);
	} else
	if (strcmp(argv[i], "--performance") == 0 || strcmp(argv[i], "-performance") == 0) {
		int val;
		char tmp[256];

		i++;
		if (i >= argc) 
			break;
		if (strlen(argv[i]) > 50 || strchr(argv[i], '.'))
			break;
		snprintf(tmp, sizeof(tmp), "/proc/acpi/processor/%s/performance", argv[i]);
		tmp[sizeof(tmp)-1] = 0;
		i++;
		if (i >= argc) 
			break;
		val= atoi(argv[i]);
		if (val < 0)
			break;
		sync();
		sync();
		fd = open(tmp, O_RDWR);
		if (fd < 0)
			exit(1);
		snprintf(tmp, sizeof(tmp), "%d", val);
		write(fd, tmp, strlen(tmp));
		close(fd);
        	setuid(getuid());	// drop all priority asap
		exit(0);
	} else
	if (strcmp(argv[i], "--toshibalcd") == 0 || strcmp(argv[i], "-toshibalcd") == 0) {
		
		i++;
		if (i >= argc) 
			break;
		toshibalcd_val= atoi(argv[i]);
		if (toshibalcd_val < 0)
			toshibalcd_val = 0;
		if (toshibalcd_val > 7)
			toshibalcd_val = 7;
		fd = open("/proc/acpi/TOSHIBA1/lcd", O_RDWR);
		if (fd >= 0 ) {
		    char c;
		    
		    c = '0'+toshibalcd_val;
		    write(fd, &c, 1);
		    close(fd);
		} else {
		    fd = open("/proc/acpi/toshiba/lcd", O_RDWR);
		    if (fd >= 0) {
			char str[MAX_TOSHIBA_STRING];
			
			snprintf(str,sizeof(str),"brightness : %d",toshibalcd_val);
			str[sizeof(str)-1]=0;
			write(fd,str,strlen(str));	
			close(fd);
		    }
		}
        	setuid(getuid());	// drop all priority asap
		exit(0);
	} else
	// CPUFreq support
	if (strncmp(argv[i], "--cpufreq", 9) == 0 || strncmp(argv[i], "-cpufreq", 8) == 0) {
		if ((i+1) >= argc)
			break;
		if (strlen(argv[i+1]) > 50 || strchr(argv[i+1], '.'))
			break;
		int val;
		char tmp[256];
		// CPUFreq support for the interface of the 2.4 kernell (/proc/sys/cpu/N/)
		if (strcmp(argv[i], "--cpufreq-24") == 0 || strcmp(argv[i], "-cpufreq-24") == 0) {
			++i;
			snprintf(tmp, sizeof(tmp), "/proc/sys/cpu/%s/speed", argv[i]);
			tmp[sizeof(tmp)-1] = 0;
			++i;
			if (i >= argc)
				break;
			val = atoi(argv[i]);
			if (val < 0)
				break;
			fd = open(tmp, O_WRONLY);
			if (fd < 0)
				exit(1);
			snprintf(tmp, sizeof(tmp), "%d", val);
			write(fd, tmp, strlen(tmp));
		} else
		// CPUFreq support for the interface of the 2.5 kernel (/proc/cpufreq)
		if (strcmp(argv[i], "--cpufreq-25") == 0 || strcmp(argv[i], "-cpufreq-25") == 0) {
			++i;
			snprintf(tmp, sizeof(tmp), "%s", argv[i]);
			tmp[sizeof(tmp)-1] = 0;
			fd = open("/proc/cpufreq", O_WRONLY);
			if (fd < 0)
				exit(1);
			write(fd, tmp, strlen(tmp));
		} else
		// CPUFreq support fot the sysfs interface of the 2.5 kernel (/sys/devices/sys/cpuN/cpufreq/)
		if (strcmp(argv[i], "--cpufreq-sysfs") == 0 || strcmp(argv[i], "-cpufreq-sysfs") == 0) {
			++i;
			snprintf(tmp, sizeof(tmp), "/sys/devices/system/cpu/%s/cpufreq/scaling_governor", argv[i]);
			tmp[sizeof(tmp)-1] = 0;
			++i;
			if (i >= argc)
				break;
			fd = open(tmp, O_WRONLY);
			if (fd < 0)
				exit(1);
			if (strlen(argv[i]) > 50)
				break;
			snprintf(tmp, sizeof(tmp), "%s", argv[i]);
			tmp[sizeof(tmp)-1] = 0;
			write(fd, tmp, strlen(tmp));
		} else {
			break;
		}
		close(fd);
		setuid(getuid()); // drop all priority asap
		exit(0);
	} else {
usage:
        	setuid(getuid());	// drop all priority asap
		fprintf(stderr, "Usage: %s [--suspend] [--standby] [--hibernate][--software-suspend][--toshibalcd N][--performance CPU N][--throttling CPU N][--cpufreq-[24|25|sysfs]]\n", argv[0]);
		exit(1);
	}
	goto usage;
    
}

