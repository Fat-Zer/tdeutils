/*
 * portable.cpp
 *
 * $Id$
 *
 * Copyright (c) 1999 Paul Campbell <paul@taniwha.com>
 *
 * Requires the TQt widget libraries, available at no cost at
 * http://www.troll.no/
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
//	this file contains the machine specific laptop power management stuff
//	to add support for your own OS this is should be the only place you need
//	to change - to add your own stuff insert above the line marked 
//	'INSERT HERE' :
//
//		#ifdef MY_OS"
//			.. copy of linux code or whatever you want to use as a base
//		# else
//
//	then tag an extra '#endif' at the end
//
// There is support for the following OSsen right now:
//
//	Linux 		(#if __linux__)
//	FreeBSD		(#elif __FreeBSD__)
//	NetBSD		(#elif __NetBSD_APM__)
//	generic nothing	(#else)
//
//
// The code here is written in a C rather than C++ like to encourage
// people more used to kernel types to do stuff here
//
//	If you have any problems, questions, whatever please get in touch
//
//		Paul Campbell
//		paul@taniwha.com
//		
//
//	Thanks to Cajus Pollmeier <C.Pollmeier@gmx.net>
//	and Robert Ellis Parrott <parrott@fas.harvard.edu>
//	who both provided ACPI support
//	and Volker Krause <volker.krause@rwth-aachen.de> who provided ACPI bug fixes
//

#include <config.h>
#include <klocale.h>
#include <kdebug.h>
#include <stdio.h>
#include "portable.h"

#ifdef __linux__

/*
** This is the Linux-specific laptop code.
*/
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <tqpushbutton.h>
#include <tqdir.h>
#include <tqfile.h>
#include <tqfileinfo.h>
#include <tqstringlist.h>
#include <tqstring.h>
#include <tqobject.h>
#include <tqregexp.h>
#include <tqiodevice.h>
#include <tqlayout.h>
#include <tqvgroupbox.h>
#include <tqvaluevector.h>

#include <kactivelabel.h>
#include <tdeconfig.h>
#include <kstandarddirs.h>
#include <kprocess.h>
#include <krichtextlabel.h>

// ibm specific stuff
extern "C"{
#include "thinkpad_common.h"
#include "smapi.h"
#include "smapidev.h"
}

//
//	here's the Linux specific laptop control panel stuff
//

typedef struct apm_info {
   unsigned int apm_flags;
   unsigned int ac_line_status;
   int          battery_percentage;
   int          battery_time;
} apm_info;
 
static int
apm_read(apm_info *ap)
{
   	FILE	*f = 0;
   	char	tmp2[10];
	int	tmp, s;
	unsigned int utmp;
	char 	version[256];

	f = fopen("/proc/apm", "r");
	if (f == NULL)
		return(1);
	s = fscanf(f, "%255s %d.%d %x %x %x %x %d%% %d %s\n",
		version,
		&tmp,
		&tmp,
		&ap->apm_flags,
	   	&ap->ac_line_status,
		&utmp,
		&utmp,
	   	&ap->battery_percentage,
	   	&ap->battery_time,
	   	tmp2);
        fclose(f);
	if (s < 9) 
		return(1);	
	if (version[0] == 'B')
		return(2);
   	if (ap->battery_percentage > 100) 
		ap->battery_percentage = -1;
	if (strcmp(tmp2, "sec") == 0)
		ap->battery_time /= 60;
	return(0);
}


// 0 => unknown
// 1 => have it
// -1 => don't have it
static int pmustate = 0;

static bool
have_pmu(void)
{
	if (pmustate != 0)
		return (pmustate == 1);
	if (!access("/proc/pmu", R_OK|X_OK)) {
		kdDebug() << "Found powermac PMU.  Using that." << endl;
		pmustate = 1;
		return true;
	}
	pmustate = -1;
	return false;
}

/* Only supports 1 battery right now - all batteries are merged into one stat */
static int
pmu_read(apm_info *ap)
{
	int bcnt = 0;
	memset(ap, 0, sizeof(apm_info));
	TQFile f("/proc/pmu/info");
	if (!f.exists() || !f.open(IO_ReadOnly))
		return 1;

	while (!f.atEnd()) {
		TQString l;
	       	f.readLine(l, 500);
		TQStringList ll = TQStringList::split(':', l, false);
		if (ll[0].stripWhiteSpace() == "AC Power") {
			ap->ac_line_status = ll[1].stripWhiteSpace().toInt();
			//kdDebug() << "line status " << ap->ac_line_status << endl;
		} else if (ll[0].stripWhiteSpace() == "Battery count") {
			bcnt = ll[1].stripWhiteSpace().toInt();
			//kdDebug() << "batteries: " << bcnt << endl;
		}
	}

	f.close();

	int charge = 0;
	int timerem = 0;
	int maxcharge = 0;
	for (int i = 0; i < bcnt; i++) {
		TQFile bf(TQString("/proc/pmu/battery_%1").arg(i));
		if (!bf.exists() || !bf.open(IO_ReadOnly))
			continue;

		while(!bf.atEnd()) {
			TQString l;
			bf.readLine(l, 500);
			TQStringList ll = TQStringList::split(':', l, false);
			if (ll[0].stripWhiteSpace() == "charge") {
				charge += ll[1].stripWhiteSpace().toInt();
				//kdDebug() << "charge: " << charge << endl;
			} else if (ll[0].stripWhiteSpace() == "max_charge") {
				maxcharge += ll[1].stripWhiteSpace().toInt();
				//kdDebug() << "max charge: " << maxcharge << endl;
			} else if (ll[0].stripWhiteSpace() == "time rem.") {
				timerem += ll[1].stripWhiteSpace().toInt();
				//kdDebug() << "time: " << timerem << endl;
			}
		}
		bf.close();
	}
	ap->battery_percentage = int(rint(100.0*float(float(charge)/float(maxcharge))));
	ap->battery_time = timerem;

	if (ap->ac_line_status > 0 || timerem == 0 ||
		(ap->ac_line_status == 0 && charge > 100 && timerem == 0))
		ap->battery_time = -1;

	return 0;
}


struct acpi_battery_info {
	int percentage;
	bool present;
	int cap;
	int cap_low;
	int remaining;
	int rate;
	TQString name;
	TQString state_file;
	TQString info_file;
};

static TQValueVector<acpi_battery_info> acpi_batteries;
static int acpi_last_known=0;
static int last_seed=1;	// increment this to force revaluation

static unsigned char acpi_ac_ok;

//
//	linux APCI doesn't return useful stuff like how much TIME is left yet
//		the 'rate' is not smoothed over time so it's faked out here
//		it's not pretty and it's a quick estimate
//
//		for the moment we prefer APM
//

static int
acpi_ac_status()
{
	DIR *dfd;
	struct dirent *dp;
	FILE *f = NULL;
	static char buff[NAME_MAX+50];
	static bool inited=0;
	static bool bad=0;
	
	if (inited) {
		if (bad)
			return(-1);
		f = fopen(buff, "r");
		goto readit;
	}
	inited = 1;

	dfd = opendir("/proc/acpi/ac_adapter/");
	if (dfd) {
		for (;;) {
			dp = readdir(dfd);
			if (dp == 0)
				break;
			if (strcmp(dp->d_name, ".") == 0 ||
		            strcmp(dp->d_name, "..") == 0)
				continue;
			strcpy(buff, "/proc/acpi/ac_adapter/");
			strcat(buff, dp->d_name);
			strcat(buff, "/status");
			f = fopen(buff, "r");
			if (!f) {
				strcpy(buff, "/proc/acpi/ac_adapter/");
				strcat(buff, dp->d_name);
				strcat(buff, "/state");
				f = fopen(buff, "r");
			}
			if (f)
				break;
		}
		closedir(dfd);
readit:
		if (f) {
			for (;;) {
				char buff2[1024];
				if (fgets(buff2, sizeof(buff), f) == NULL)
					break;
				if (strstr(buff2, "Status:") != NULL ||
			    	    strstr(buff2, "state:") != NULL) {
					if (strstr(buff2, "on-line") != NULL) {
						fclose(f);
						return(1);
					}
				}
			}
			fclose(f);
			return(0);
		}
	}
	bad=1;
	return(-1);
}

static void acpi_read_batteries() {
	TQString buff;
	TQFile *f;
	static int test_count = 0;
	bool skip = false;

	for(unsigned int i = 0; i < acpi_batteries.count(); ++i) {
		acpi_battery_info& bat = acpi_batteries[i];
		bool present = false;
		if ((test_count==0 || acpi_last_known != last_seed) && !bat.info_file.isNull()) {
			f = new TQFile(bat.info_file);
			if (f && f->exists() && f->open(IO_ReadOnly)) {
				while(f->readLine(buff,1024) > 0) {
					if (buff.contains("design capacity low:", false)) {
						TQRegExp rx("(\\d*)\\D*$");
						rx.search(buff);
						bat.cap_low = rx.cap(1).toInt();
						if (bat.cap_low < 0)
							bat.cap_low = 0;
						continue;
					}
					if (buff.contains("last full capacity:", false)) {
						TQRegExp rx("(\\d*)\\D*$");
						rx.search(buff);
						bat.cap = rx.cap(1).toInt();
						continue;
					}
				}
				f->close();
				bat.cap -= bat.cap_low;
			}
			delete f;
		}
		if (bat.cap <= 0) {
			TDEConfig* config = new TDEConfig("kcmlaptoprc", true /*readonly*/, false /*useKDEGlobals*/);
			config->setGroup("AcpiBattery");
			bat.cap = config->readNumEntry(bat.name,0);
			delete config;
		}
		if (!bat.state_file.isNull()) {
			f = new TQFile(bat.state_file);
			if (f && f->exists() && f->open(IO_ReadOnly)) {
				while(f->readLine(buff,1024) > 0) {
					if (buff.contains("present rate:", false)) {
						TQRegExp rx("(\\d*)\\D*$");
						rx.search(buff);
						bat.rate = rx.cap(1).toInt();
						if (bat.rate < 0)
				    		bat.rate = 0;
						present = true;
						continue;
					}
					if (buff.contains("remaining capacity:", false)) {
						TQRegExp rx("(\\d*)\\D*$");
						rx.search(buff);
						bat.remaining = rx.cap(1).toInt();
						bat.remaining -= bat.cap_low;
						if (bat.remaining < 0)
							bat.remaining = 0;
						present = true;
						continue;
					}
				}
				f->close();
			}
			delete f;
		}
		if(present && !bat.present) // recheck capacity if a battery was put in
			skip = true;
		bat.present = present;
		if (bat.present) {
			if (bat.remaining > bat.cap) {	// happens e.g. if the system doesn't provide a capacity value
				bat.cap = bat.remaining;
				TDEConfig* config = new TDEConfig("kcmlaptoprc", false /*readonly*/, false /*useKDEGlobals*/);
				config->setGroup("AcpiBattery");
				config->writeEntry(bat.name, bat.cap);
				config->sync();
				delete config;
				skip = true;
			}
			if (bat.cap == 0)
				bat.percentage = 0;
			else
				bat.percentage = bat.remaining*100/bat.cap;
		}
		else 
			bat.percentage = 0;
	}

	if (!skip) {
		acpi_last_known = last_seed;
		test_count++;
	} 
	else
		test_count = 0;
	if (test_count > 1000)		// every 1000 or so times recheck the battery capacity
		test_count = 0;

}

static int
acpi_read(apm_info *ap)
{
	int rate, part, total; 
	int ret = 1;
	bool present = 0;

	part = 0;
	total = 0;
	rate = 0;

	acpi_read_batteries();

	for(unsigned int i = 0; i < acpi_batteries.count(); ++i) {
		acpi_battery_info& bat = acpi_batteries[i];
		present |= bat.present;
		if(bat.present) {
			total += bat.cap;
			part += bat.remaining;
			rate += bat.rate;
		}
		ret = 0;
	}

	// some broken ACPI implementations don't return a rate
	// compute a 'fake' rate by diffing remaining values
	if (rate == 0)
	{
	    static int last_remaining = 0;
	    static time_t last_time = 0;
	    if (last_remaining != 0
		&& last_time != 0)
	    {
		int diff_time = time(0) - last_time;
		if (diff_time > 0)
		    rate = (last_remaining - part) * 3600 / diff_time;
	    }
	    last_remaining = part;
	    last_time = time(0);
	    if (rate < 0)
		rate = 0;
	}

	static int nrates = 0;
	static int saved_rate[8];
	static unsigned char ignore_next = 2;	// ignore the first couple
	
	ap->ac_line_status = 0;
	//
	//	ACPI (unlike nice systems like some APM implementations) doesn't
	//	tell us how much battery TIME we have left - here we
	//	do a weighted average of the discharge rate (in mW) and
	//	figure out how long we have left by dividing it into the 
	//	remaining capacity
	//
	//	because some ACPI implementations return bogus 
	//	rates when charging we can't estimate the battery life
	//	so we only collect discharge rate data when we're actually
	//	discharging
	//
	if (acpi_ac_status() == 1) {
		ap->ac_line_status |= 1;
		ignore_next = 2;
	} else {
		// after switching from power to unpowered we often get
		// a bad reading from ACPI resulting in bizarre
		// readings
		if (ignore_next == 0) {
			if (nrates < 8)				// smooth the power flow
				nrates++;			// simple box filter
			for (int i = 8-1; i > 0; i--) 
				saved_rate[i] = saved_rate[i-1];
			saved_rate[0] = rate;
		} else {
			ignore_next--;
		}
	}
	//
	//	if we haven't got any discharge rate data yet don't return a
	//	battery time - probably happens when you start up with the
	//	ac adaptor plugged in
	//
	if (nrates == 0) {
		ap->battery_time = -1;
	} else {
		rate = 0;
		for (int i = 0; i < nrates; i++)
			rate += saved_rate[i];
		rate = (rate+2*saved_rate[0])/(nrates+2);		// weight it slighly
		ap->battery_time = (rate==0 ? -1 : 60*part/rate);
	}
	ap->battery_percentage = (total==0?0:100*part/total);
	if (!present) {
		ap->battery_percentage = -1;
		ap->battery_time = -1;
	}
	ap->apm_flags = 0;
	return(ret);
}

static int apm_no_time;
static apm_info apmx = {0,0,0,0};
static int
has_apm()
{
        static int init = 0;
        static int val;
        if (init)
                return(val);
        init = 1;
        val = 1;
        apm_no_time=0;
        if (apm_read(&apmx) || (apmx.apm_flags&0x20)) {
                val = 0;
                apm_no_time = 1;
        } else {
                apm_no_time = apmx.battery_time < 0;
        }

	if (val == 0) {
		val = have_pmu();
		if (val && pmu_read(&apmx)) {
			val = 0;
		}
	}
        return(val);
}

// Power to the powermacs!!
static int
has_pmu()
{
	static int init = 0;
	static int val;
	if (init)
		return val;
	init = 1;
	val = 1;
	if (!TQDir("/proc/pmu").exists()) {
		val = 0;
	}
	return val;
}

static bool software_suspend_is_preferred = false;	// user prefers to use SS for hibernate
static bool acpi_helper_ok(bool type);
static bool
has_software_suspend(int type)
{
	static int known=0;
	static bool present=0;		// functionality seems to be here	
	static bool available=0;	// helper can work
	if (known != last_seed) {
		known = last_seed;
		available = 0;
		present = (((::access("/proc/sys/kernel/swsusp", F_OK) == 0) ||
                            (::access("/proc/software_suspend", F_OK) == 0) ||
                            (::access("/proc/suspend2", F_OK) == 0)) &&
		           ::access("/usr/sbin/hibernate", F_OK) == 0);
		if (present) {
			if (::getuid() == 0) {	// running as root
				available = ::access("/usr/sbin/hibernate", X_OK) == 0 && acpi_helper_ok(1);
			} else {
				available = acpi_helper_ok(0);
			}
		}
	}
	switch (type) {
	case 0: return(present);
	case 1: return(present&available&software_suspend_is_preferred);
	case 2: return(present&available);
	default:return(0);
	}
}

bool
laptop_portable::has_software_suspend(int type)
{
	return(::has_software_suspend(type));
}

void
laptop_portable::software_suspend_set_mask(bool hibernate)
{
	software_suspend_is_preferred = hibernate;
}


static int x_acpi_init = 0;
static bool
has_acpi()
{
	static bool val;
	static bool checked = 0;

	if (!checked) {
		val = ::access("/proc/acpi", F_OK) == 0;
		checked = 1;
	}
	return(val);
}

static int
has_acpi_power()
{
	static int val;

	if (x_acpi_init)
		return(val);
	x_acpi_init = 1;
	val = 0;

	acpi_batteries.clear();
	
	if (acpi_ac_status() >= 0)
		acpi_ac_ok = 1;

	TQDir battdir("/proc/acpi/battery");
	battdir.setFilter(TQDir::Dirs);
	if(!battdir.isReadable())
		return(val = 0);
	for(uint i = 0; !battdir[i].isNull(); ++i) {
		if(battdir[i] == "." || battdir[i] == "..")
			continue;
		bool ok = 0;
		acpi_battery_info bat = {0,0,0,0,0,0,0,0,0};
		TQString base = battdir.path() + "/" + battdir[i] + "/";
		TQString name(base + "state");
		TQFileInfo f(name);
		if(f.isReadable()) {
			bat.state_file = name;
			ok = true;
		} else {
			name = base + "status";
			f.setFile(name);
			if(f.isReadable()) {
				bat.state_file = name;
				ok = true;
			}
		}
		name = base + "info";
		f.setFile(name);
		if(f.isReadable()) {
			bat.info_file = name;
			ok = true;
		} 
		if (ok) {
			bat.name = battdir[i];
			acpi_batteries.append(bat);
			val = 1;
		}
	}
	if (!acpi_ac_ok)
		val = 0;
	return(val);
}

static unsigned long acpi_sleep_enabled = 0x00;	// acpi sleep functions enabled mask


static bool
has_acpi_sleep(int state) 
{
	static int known=0;
	static unsigned long mask=0;
	if (known != last_seed) {
		known = last_seed;
		mask = 0;

		TQFile p("/sys/power/state");
		TQFile f("/proc/acpi/sleep");

		if (p.exists() && p.open(IO_ReadOnly)) {
			TQString l;
			TQTextStream t(&p);
			l = t.readLine();
			TQStringList ll = TQStringList::split(' ',l,false);
			for (TQValueListIterator<TQString> i = ll.begin(); i!=ll.end(); i++) {
				TQString s = *i;
				
				if (s.compare("standby")==0)
				mask |= (1<<1);
				else if (s.compare("mem")==0)
				mask |= (1<<3);
				else if (s.compare("disk")==0)
				mask |= (1<<4);
			}
			p.close();
		}
		else if (f.exists() && f.open(IO_ReadOnly)) {
			TQString l;
			TQTextStream t(&f);
			l = t.readLine();
			TQStringList ll = TQStringList::split(' ',l,false);
			for (TQValueListIterator<TQString> i = ll.begin(); i!=ll.end(); i++) {
				TQString s = *i;
				if (s[0] == 'S') {
					int c = s[1].digitValue();
					if (c >= 0 && c <= 9)
						mask |= 1<<c;
				}
			}
			f.close();
		}
	}
	return((mask&acpi_sleep_enabled&(1<<state)) != 0);
}

static bool acpi_performance_enabled = 0;
static bool acpi_throttle_enabled = 0;
void
laptop_portable::acpi_set_mask(bool standby, bool suspend, bool hibernate, bool perf, bool throttle)
{
	acpi_sleep_enabled = 
			(standby ? (1<<1)|(1<<2) : 0) |
			(suspend ? 1<<3 : 0) |
			(hibernate ? 1<<4 : 0);
	acpi_performance_enabled = perf;
	acpi_throttle_enabled = throttle;
	last_seed++;
}

static void
invoke_acpi_helper(const char *param, const char *param2, const char *param3)
{
	TDEProcess proc;
	proc << TDEStandardDirs::findExe("klaptop_acpi_helper");
	proc << param;
	if (param2) 
		proc << param2;
	if (param3) 
		proc << param3;
	proc.start(TDEProcess::Block);	// helper runs fast and we want to see the result
}

static unsigned long apm_sleep_enabled = 0x0c;	// apm sleep functions enabled mask
static bool
has_apm_sleep(int state) 
{
	return((apm_sleep_enabled&(1<<state)) != 0);
}

void
laptop_portable::apm_set_mask(bool standby, bool suspend)
{
	apm_sleep_enabled = 
			(standby ? 1<<2 : 0) |
			(suspend ? 1<<3 : 0);
	last_seed++;
}

static bool apm_sleep_access_ok();
static bool acpi_sleep_access_ok();
 
static int
apm_has_time()
{
        return(!apm_no_time);
}

//
//	something changed maybe we need to check out environment again
//
void
laptop_portable::power_management_restart()
{
	last_seed++;
	x_acpi_init = 0;
}

int laptop_portable::has_apm(int type)
{
	switch (type) {
	case 0:
		return(::has_apm());
	case 1:
		return(::has_apm() && ::apm_sleep_access_ok());
	default:
		return(0);
	}
}

//
//	returns 1 if we support acpi
//
int laptop_portable::has_acpi(int type)
{
	switch (type) {
	case 0:
		return(::has_acpi_power());
	case 1:
		return(::has_acpi() && ::acpi_sleep_access_ok());
	case 3:
		return(::has_acpi() &&::acpi_sleep_access_ok() && (::has_acpi_sleep(1)||::has_acpi_sleep(2)));
	case 4:
		return(::has_acpi() &&::acpi_sleep_access_ok() && ::has_acpi_sleep(3));
	case 5:
		return(::has_acpi() &&::acpi_sleep_access_ok() && ::has_acpi_sleep(4));
	default:
		return(0);
	}
}

//
// The following code was derived from tpctl for IBM thinkpad support
// 		(Thomas Hood, Bill Mair - tpctl.sourceforge.net)
//
//	we check for the existence of /dev/thinkpad/thinkpad or /dev/thinkpad 
//
static const char *ibm_device;
static int ibm_fd = -1;

static bool has_ibm()
{
	static int known=0;
	static bool result;
	if (known == last_seed)
		return(result);
	known = last_seed;
	result = 0;
	if (ibm_fd >= 0) {
		result = 1;
		return(1);
	}
	ibm_device = "/dev/thinkpad/thinkpad";
	if ((ibm_fd = open(ibm_device, O_RDWR)) < 0) {
		ibm_device = "/dev/thinkpad";
		if ((ibm_fd = open(ibm_device, O_RDWR)) < 0) 
			return(0);
	}
	result = 1;
	return(1);
}
//
//	returns 1 if we support power management
//
int laptop_portable::has_power_management()
{
	if (::has_apm())
		return 1;
	if (::has_pmu())
		return 1;
	if (::has_acpi_power())
		return 1;
	if (::has_ibm())
		return 1;
	// INSERT HERE
	return 0;
}
//
//	returns 1 if the BIOS returns the time left in the battery rather than a % of full
//
int laptop_portable::has_battery_time()
{
	if (::has_acpi_power())
		return 1;
	if (::apm_has_time())
		return 1;
	// INSERT HERE
	return 0;
}
//
//	returns 1 if we can perform a change-to-suspend-mode operation for the user
//	(has_power_management() has already returned 1)
//
int laptop_portable::has_suspend()
{
	if (::has_acpi()) 
		return ::acpi_sleep_access_ok() && ::has_acpi_sleep(3);
	if (::has_pmu()) 
		return 1;
	if (::has_ibm()) 
		return 1;
	if (::has_apm()) 
		return ::apm_sleep_access_ok() && ::has_apm_sleep(3);
	// INSERT HERE
	return 0;
}
//
//	returns 1 if we can perform a change-to-standby-mode operation for the user
//	(has_power_management() has already returned 1)
//
int laptop_portable::has_standby()
{
	if (::has_pmu()) 
		return 0;
	if (::has_acpi()) 
		return ::acpi_sleep_access_ok() && (::has_acpi_sleep(1)||::has_acpi_sleep(2));
	if (::has_ibm()) 
		return 1;
	if (::has_apm()) 
		return ::apm_sleep_access_ok() && ::has_apm_sleep(2);
	// INSERT HERE
	return 0;
}
//
//	returns 1 if we can perform a change-to-hibernate-mode for a user
//      (has_power_management() has already returned 1)  [hibernate is the save-to-disk mode
//	not supported by linux APM]
//
int laptop_portable::has_hibernation()
{
	if (::has_pmu()) 
		return 0;
	if (::has_software_suspend(1))	// must be before acpi
		return 1;
	if (::has_acpi()) 
		return ::acpi_sleep_access_ok() && ::has_acpi_sleep(4);
	if (::has_ibm()) 
		return 1;
	// INSERT HERE
	return 0;
}

//
//	explain to the user what they need to do if has_power_management() returned 0
//	to get any software they lack
//

KActiveLabel *laptop_portable::no_power_management_explanation(TQWidget *parent)
{
	if (access("/proc/acpi", F_OK) == 0) {	// probably has default kernel ACPI installed
		KActiveLabel* explain = new KActiveLabel(i18n("Your computer seems to have a partial ACPI installation. ACPI was probably enabled, but some of the sub-options were not - you need to enable at least 'AC Adaptor' and 'Control Method Battery' and then rebuild your kernel."), parent);
		return(explain);
	}

	KActiveLabel* explain = new KActiveLabel(i18n("Your computer doesn't have the Linux APM (Advanced Power Management) or ACPI software installed, or doesn't have the APM kernel drivers installed - check out the <a href=\"http://www.linuxdoc.org/HOWTO/Laptop-HOWTO.html\">Linux Laptop-HOWTO</a> document for information on how to install APM."), parent);
 
	return(explain);
}

//
//	explain to the user what they need to do to get suspend/resume to work from user mode
//
TQLabel *laptop_portable::how_to_do_suspend_resume(TQWidget *parent)
{
	if (::has_apm()) {
                // TODO: remove linefeed from string, can't do it right now coz we have a string freeze
 		TQLabel* note = new KRichTextLabel(i18n("\nIf you make /usr/bin/apm setuid then you will also "
						"be able to choose 'suspend' and 'standby' in the above "
						"dialog - check out the help button below to find out "
						"how to do this").replace("\n", TQString()), parent);
		return(note);
	}
	if (::has_acpi()) {
                // TODO: remove linefeed from string, can't do it right now coz we have a string freeze
 		TQLabel* note = new KRichTextLabel(i18n("\nYou may need to enable ACPI suspend/resume in the ACPI panel").replace("\n", TQString()), parent);
		return(note);
	}
        // TODO: remove linefeed from string, can't do it right now coz we have a string freeze
 	TQLabel* note = new KRichTextLabel(i18n("\nYour system does not support suspend/standby").replace("\n", TQString()), parent);
	return(note);
}

static char tmp0[256], tmp1[256];
static int present=0;
static 
void get_pcmcia_info()
{
      FILE *f = fopen("/var/lib/pcmcia/stab", "r");
      if (!f) f = fopen("/var/run/stab", "r");
      if (f) {
	int c;
	char *cp;

	present = 1;
	cp = tmp0;
	for (;;) {
		c = fgetc(f);
		if (c == EOF || c == '\n')
			break;
		if (c == ':') {
			while ((c = fgetc(f)) == ' ')
				;
			if (c == EOF)
				break;
			for (;;) {
				*cp++ = c;
				c = fgetc(f);
				if (c == EOF || c == '\n')
					break;
			}
			break;
		}
	}
	*cp = 0;

	if (c != EOF) {
		cp = tmp1;
		for (;;) {
			c = fgetc(f);
			if (c == EOF)
				break;
			if (c == ':') {
				while ((c = fgetc(f)) == ' ')
					;
				if (c == EOF)
					break;
				for (;;) {
					*cp++ = c;
					c = fgetc(f);
					if (c == EOF || c == '\n')
						break;
				}
				break;
			}
		}
	*cp = 0;
	}

	fclose(f);
      } else {
	present = 0;
      }
}

//
//	pcmcia support - this will be replaced by better - pcmcia support being worked on by
//	others
//
TQLabel *laptop_portable::pcmcia_info(int x, TQWidget *parent)
{
	if (x == 0) 
		get_pcmcia_info();
	if (!present) {
		if (x == 1)
      			return(new TQLabel(i18n("No PCMCIA controller detected"), parent));
      		return(new TQLabel(i18n(""), parent));
	} else {
		switch (x) {
        	case 0: return(new TQLabel(i18n("Card 0:"), parent));
        	case 1: return(new TQLabel(tmp0, parent));
		case 2: return(new TQLabel(i18n("Card 1:"), parent));
        	default:return(new TQLabel(tmp1, parent));  
		}
	}
}

//
//	puts us into standby mode
//
void laptop_portable::invoke_standby()
{
	last_seed++;	// make it look for battery removal/return
	if (::has_acpi()) {
		if (::has_acpi_sleep(1)) {	// people seem to argue as to which is 'standby' the S1 people are winning
			::invoke_acpi_helper("--standby", 0, 0);
		} else {
			::invoke_acpi_helper("--standby2", 0, 0);
		}
		return;
	} 
	if (::has_ibm()) {
		smapi_ioparm_t ioparmMy;
	 	sync();
		ioparmMy.in.bFunc = (byte) 0x70;
		ioparmMy.in.bSubFunc = (byte) 0;
		ioparmMy.in.wParm1 = (word) 0;
		ioparmMy.in.wParm2 = (word) 0;
		ioparmMy.in.wParm3 = (word) 0;
		ioparmMy.in.dwParm4 = (dword) 0;
		ioparmMy.in.dwParm5 = (dword) 0;
		(void)ioctl_smapi( ibm_fd, &ioparmMy );
		return;
	}
	// add other machine specific standbys here
	TDEProcess proc;
	proc << "/usr/bin/apm";
	proc << "--standby";
	proc.start(TDEProcess::Block);	// helper runs fast and we want to see the result
}

//
//	puts us into suspend mode
//
void laptop_portable::invoke_suspend()
{
	last_seed++;	// make it look for battery removal/return

	if (::has_pmu()) {
		TDEProcess proc;
		proc << "/usr/bin/apm";
		proc << "-f";
		proc.start(TDEProcess::Block);	// helper runs fast and we want to see the result
		return;
	}

	if (::has_acpi()) {
		::invoke_acpi_helper("--suspend", 0, 0);
		return;
	}
	if (::has_ibm()) {
		smapi_ioparm_t ioparmMy;
	 	sync();
		ioparmMy.in.bFunc = (byte) 0x70;
		ioparmMy.in.bSubFunc = (byte) 1;
		ioparmMy.in.wParm1 = (word) 0;
		ioparmMy.in.wParm2 = (word) 0;
		ioparmMy.in.wParm3 = (word) 0;
		ioparmMy.in.dwParm4 = (dword) 0;
		ioparmMy.in.dwParm5 = (dword) 0;
		(void)ioctl_smapi( ibm_fd, &ioparmMy );
		return;
	}
	// add other machine specific suspends here
	TDEProcess proc;
	proc << "/usr/bin/apm";
	proc << "--suspend";
	proc.start(TDEProcess::Block);	// helper runs fast and we want to see the result
}

//
//	puts us into hibernate mode
//
void laptop_portable::invoke_hibernation()
{
	last_seed++;	// make it look for battery removal/return
	if (::has_software_suspend(1)) {	// must be before acpi
		::invoke_acpi_helper("--software-suspend", 0, 0);
		return;
	}
	if (::has_acpi()) {
		::invoke_acpi_helper("--hibernate", 0, 0);
		return;
	}
	if (::has_ibm()) {
		smapi_ioparm_t ioparmMy;
	 	sync();
		ioparmMy.in.bFunc = (byte) 0x70;
		ioparmMy.in.bSubFunc = (byte) 2;
		ioparmMy.in.wParm1 = (word) 0;
		ioparmMy.in.wParm2 = (word) 0;
		ioparmMy.in.wParm3 = (word) 0;
		ioparmMy.in.dwParm4 = (dword) 0;
		ioparmMy.in.dwParm5 = (dword) 0;
		(void)ioctl_smapi( ibm_fd, &ioparmMy );
		return;
	}
	// add other machine specific hibernates here
}

void
laptop_portable::extra_config(TQWidget *wp, TDEConfig *, TQVBoxLayout *top_layout)
{
	if (laptop_portable::has_apm(1) || laptop_portable::has_acpi(1))
		return;
	if (laptop_portable::has_apm(0)) {
      		TQLabel* explain = new KRichTextLabel( i18n("Your system has APM installed but may not be able to use all "
					           "of its features without further setup - look in the 'APM Config' "
						   "tab for information about setting up APM for suspend and resume"), wp);
		top_layout->addWidget(explain, 0);
	}
	if (laptop_portable::has_acpi(0)) {
      		TQLabel* explain = new KRichTextLabel( i18n("Your system has ACPI installed but may not be able to use all "
					           "of its features without further setup - look in the 'ACPI Config' "
						   "tab for information about setting up ACPI for suspend and resume"), wp);
      		top_layout->addWidget(explain, 0);
	}
}

//
//	return current battery state
//
struct power_result laptop_portable::poll_battery_state()
{
	struct power_result p;

	apm_info x = {0,0,0,-1};
	if (have_pmu()) {
		pmu_read(&x);
	} else
	if ((::has_acpi_power() ? ::acpi_read(&x) : ::apm_read(&x)) || (x.apm_flags&0x20)) {
		// INSERT HERE
                p.powered = 0;
                p.percentage=0;
                p.time = -1;
		return p;
        }                                 
	p.powered = x.ac_line_status&1;
	p.percentage = x.battery_percentage;
	p.time = x.battery_time;
	return(p);
}

void 
laptop_portable::get_battery_status(int &num_batteries, TQStringList &names, TQStringList &state, TQStringList &values)
{
	struct power_result r;

	if (!has_power_management()) {
		num_batteries = 0;
		names.clear();
		state.clear();
		values.clear();
		return;
	}

	if (::has_acpi_power()) {
		names.clear();
		state.clear();
		values.clear();
		acpi_read_batteries();
		num_batteries = acpi_batteries.count();
		for(unsigned int i = 0; i < acpi_batteries.count(); ++i) {
			acpi_battery_info& bat = acpi_batteries[i];
			names.append(bat.name);
			values.append(TQString("%1").arg(bat.percentage));
			state.append(bat.present ? "yes" : "no");
		}
		return;
	}

	// INSERT HERE
	
	num_batteries = 1;
	r = poll_battery_state();
	names.append("BAT1");
	state.append("yes");
	TQString s;
	s.setNum(r.percentage);
	values.append(s);
}


//
//	returns the current system load average, -1 if none
//	
static TQFile lav_file;
static bool lav_inited=0;
static bool lav_openok=0;

static bool has_lav()
{
	if (!lav_inited) {
		lav_inited =1;
		lav_file.setName("/proc/loadavg");
		lav_openok = lav_file.exists() && lav_file.open( IO_ReadOnly );
		if (lav_openok)
			lav_file.close();
	}
	return(lav_openok);
}

bool laptop_portable::has_lav()
{
	return ::has_lav();
}

float laptop_portable::get_load_average()
{
	if (!::has_lav())
		return(-1);
	lav_file.open( IO_ReadOnly );
	TQString l;
       	lav_file.readLine(l, 500);
	lav_file.close();
	TQStringList ll = TQStringList::split(' ', l, false);
	l = ll[0];
	bool ok;
	float f = l.toFloat(&ok);
	if (!ok)
		f = -1;
	return(f);
}


int laptop_portable::has_cpufreq() {
	struct stat sb;
	int rc;
	rc = stat("/proc/cpufreq", &sb);
	if (rc == 0) {
		rc = stat("/proc/cpuinfo", &sb);
		if (rc == 0)
			return 1;
	}
	// INSERT HERE
	return 0;
}


TQString laptop_portable::cpu_frequency() {
	TQString rc;
	TQFile cf("/proc/cpufreq");
	bool haveProfile = false;
	
	if (cf.exists() && cf.open(IO_ReadOnly)) {
		while (!cf.atEnd()) {
			TQString l;
		       	cf.readLine(l, 500);
			if (l.left(3) == "CPU") {
				TQStringList ll = TQStringList::split(' ', l, false);
				rc = ll.last();
				haveProfile = true;
				break;
			}
		}

	}
	if (haveProfile) {
		TQFile ci("/proc/cpuinfo");
		if (ci.exists() && ci.open(IO_ReadOnly)) {
			while (!ci.atEnd()) {
				TQString l;
	       			ci.readLine(l, 500);
				TQStringList ll =
					TQStringList::split(':',l,false);
				if (ll.count() != 2)
					continue;
				if (ll.first().stripWhiteSpace() 
						== "cpu MHz") {
					rc = i18n("%1 MHz (%2)").arg(ll.last().stripWhiteSpace()).arg(rc);
					break;
				} else if (ll.first().stripWhiteSpace()
						== "clock") {
					rc = TQString("%1 (%2)").arg(ll.last().stripWhiteSpace()).arg(rc);
					break;
				}
			}
		}
	}

	return rc;
}

static int sonyFd = -1;
static int has_toshiba_brightness = 0;

static bool
acpi_helper_ok(bool type)
{
	static int known[2]={0,0};
	static bool known_res[2];

	if (known[type] == last_seed)
		return(known_res[type]);
	known[type] = last_seed;
	known_res[type] = 0;
	struct stat sb;
	TQString str = TDEStandardDirs::findExe("klaptop_acpi_helper");
	if (str.isNull() || str.isEmpty())
		return(0);
	
	if (stat(str.latin1(), &sb) < 0)
		return(0);
	if (!S_ISREG(sb.st_mode))
		return(0);
	if (!type && getuid() != 0 && (sb.st_uid != 0 || !(sb.st_mode&S_ISUID))) 
		return(0);	
	if (!(sb.st_mode&0x111))
		return(0);
	known_res[type] = 1;
	return(1);
}

static bool 
acpi_sleep_access_ok()
{
	static int known=0;
	static int known_res=1;

	if (known != last_seed) {
		known = last_seed;
	    	known_res = (::access("/proc/acpi/sleep", R_OK|W_OK)==0 && ::acpi_helper_ok(1)) ||
 	                    (::access("/proc/acpi/sleep", R_OK)==0 && ::acpi_helper_ok(0)) ||
	    	            (::access("/sys/power/state", R_OK|W_OK)==0 && ::acpi_helper_ok(1)) ||
 	                    (::access("/sys/power/state", R_OK)==0 && ::acpi_helper_ok(0));
	}
	return(known_res);
}

static bool
apm_helper_ok(bool type)
{
	static int known[2]={0,0};
	static bool known_res[2];

	if (known[type] == last_seed)
		return(known_res[type]);
	known[type] = last_seed;
	known_res[type] = 0;
	struct stat sb;
	TQString str = "/usr/bin/apm";
	if (str.isNull() || str.isEmpty())
		return(0);
	if (stat(str.latin1(), &sb) < 0)
		return(0);
	if (!S_ISREG(sb.st_mode))
		return(0);
	if (!type && getuid() != 0 && (sb.st_uid != 0 || !(sb.st_mode&S_ISUID))) 
		return(0);	
	if (!(sb.st_mode&0x111))
		return(0);
	known_res[type] = 1;
	return(1);
}

static bool 
apm_sleep_access_ok()
{
	static int known=0;
	static int known_res=1;

	if (known != last_seed) {
		known = last_seed;
	    	known_res = (::access("/proc/apm", R_OK|W_OK)==0 && ::apm_helper_ok(1)) ||
 	                    (::access("/proc/apm", R_OK)==0 && ::apm_helper_ok(0));
	}
	return(known_res);
}

int
laptop_portable::has_brightness()
{
	static int known=0;
	static int known_res=1;

	if (known == last_seed)
		return(known_res);
	known = last_seed;
  	if (sonyFd == -1 && ::access("/dev/sonypi", R_OK) == 0) 
		sonyFd = ::open("/dev/sonypi", O_RDONLY|O_NONBLOCK);
	if (sonyFd >= 0)
		return(1);
	if (::has_acpi() &&
 	    (((::access("/proc/acpi/TOSHIBA1/lcd", R_OK|W_OK)==0 || ::access("/proc/acpi/toshiba/lcd", R_OK|W_OK)==0) && ::acpi_helper_ok(1)) ||
 	     ((::access("/proc/acpi/TOSHIBA1/lcd", R_OK)==0 || ::access("/proc/acpi/toshiba/lcd", R_OK)==0) && ::acpi_helper_ok(0)))) {
		has_toshiba_brightness = 1;
		return(1);
	}
	// check for new devices here
	// INSERT HERE
	known_res = 0;
	return 0;
}

#define SONYPI_IOCSBRT  _IOW('v', 0, unsigned char)

//
// There are two sorts of brightness calls - ones where we expect the user to raise the brightness themselves
// 	by doing something (for these we don't allow the panel to go completely dark), and those where the
// 	laptop software is going to remember the brightness and set it back later (like closing the lid
//	where we can go all the way black)
//
// NOTE: some panel interfaces (sony) can't actually go all the way blank anyway so we ignore the blank tag
//
void 
laptop_portable::set_brightness(bool blank, int val)	// val = 0-255 255 brightest, 0 means dimmest (if !blank it must be still visible), 
{
	if (sonyFd >= 0) {
		unsigned char v;
		if (val < 0)
			val = 0;
		if (val > 255)
			val = 255;
		v = val;
		(void)::ioctl(sonyFd, SONYPI_IOCSBRT, &v);
		return;
	}
	if (has_toshiba_brightness) {
		char tmp[20];

		if (val < 0)
			val = 0;
		if (val > 255)
			val = 255;
		val >>= 5;
		if (val == 0 && !blank)
			val = 1;
		snprintf(tmp, sizeof(tmp), "%d", val&7);
		::invoke_acpi_helper("--toshibalcd", tmp, 0);
		return;
	}
	// INSERT HERE
}

int 
laptop_portable::get_brightness() // return a val 0-255, or -1 if you can't
{
	if (sonyFd >= 0) 
		return(-1);	// sonypi doesn't seem to return this reliably

	// INSERT HERE
	return(-1);	// means can't extract it
}

#define MAP_SIZE 20
static int acpi_performance_map[MAP_SIZE];	// hidden acpi state map
static bool acpi_performance_enable[MAP_SIZE];
static TQStringList performance_list;
static TQString acpi_performance_cpu;
static int acpi_throttle_map[MAP_SIZE];
static bool acpi_throttle_enable[MAP_SIZE];
static TQStringList throttle_list;
static TQString acpi_throttle_cpu;

static bool
get_acpi_list(char p, int *map, const char *dev, TQStringList &list, int &index, TQString &cpu, bool get_enable, bool *enable_list)
{
	DIR *dfd;
	struct dirent *dp;
	unsigned int i = 0;
	bool result = 0;

	index = 0;
	list.clear();
	dfd = opendir("/proc/acpi/processor/");
	if (dfd) {
		for (dp = readdir(dfd);dp ;dp = readdir(dfd)) {
			if (strcmp(dp->d_name, ".") == 0 ||
    		  	    strcmp(dp->d_name, "..") == 0)
				continue;
			TQString name("/proc/acpi/processor/");
			name += dp->d_name;
			name += dev;
 	    		if (!(::access(name.latin1(), R_OK|W_OK)==0 && ::acpi_helper_ok(1)) &&
 	                    !(::access(name.latin1(), R_OK)==0 && ::acpi_helper_ok(0))) 
				continue;
			TQFile f(name);
			if (f.exists() && f.open(IO_ReadOnly)) {
				while (!f.atEnd() && i < MAP_SIZE) {
					TQString l;
      					f.readLine(l, 500);
					TQStringList ll = TQStringList::split(':',l,false);
					TQString tag = ll[0].stripWhiteSpace();
					bool is_this = tag[0] == '*';
					if (is_this) {
						if (tag[1] != p)
							continue;
						tag.remove(0, 2);
					} else {
						if (tag[0] != p)
							continue;
						tag.remove(0, 1);
					}
					bool ok;
					int val = tag.toInt(&ok);
					if (ok) {
						if (!result)
							cpu = dp->d_name;
						if (is_this)
							index = i;
						map[i] = val;
						list.append(ll[1].stripWhiteSpace());
						result = 1;
						enable_list[i] = 1;
						i++;
					}
				}
				f.close();

				//
				// get the limit info if asked for
				//
				if (get_enable) {
					name = TQString("/proc/acpi/processor/")+dp->d_name+"/limit";
					f.setName(name);
					if (f.exists() && f.open(IO_ReadOnly)) {
						while (!f.atEnd() && i < MAP_SIZE) {
							TQString l;
							f.readLine(l, 500);
							if (l.contains("active limit", false)) {
								TQRegExp rx(TQString("%1(\\d+)").arg(p));
								if (rx.search(l) >= 0) {
									bool ok;
									int min = rx.cap(1).toInt(&ok);
									if (ok) {
										for (int j = 0; j < min; j++)
											enable_list[j] = 0;

									}
								}
								break;
							}
						}
						f.close();
					}
				}
				break;
			}
		}
		closedir(dfd);
	}
	return(result);
}

#define CPUFREQ_NONE 0
#define CPUFREQ_24 1
#define CPUFREQ_25 2
#define CPUFREQ_SYSFS 3

static TQString cpufreq_cpu = "";
static TQString cpufreq_minmax_frequency[2];

// get CPUFreq scaling policies via the sysfs interface
static int get_cpufreq_sysfs_state(TQStringList &states, int &current, const TQString cpu) {
	TQString cur, buffer;

	// read current scaling policy
	TQFile f("/sys/devices/system/cpu/" + cpu + "/cpufreq/scaling_governor");
	if(!f.exists() || !f.open(IO_ReadOnly) || f.atEnd())
		return CPUFREQ_NONE;
	f.readLine(buffer, 256);
	cur = buffer.stripWhiteSpace();
	f.close();

	// read available scaling policies
	states.clear();
	f.setName("/sys/devices/system/cpu/" + cpu + "/cpufreq/scaling_available_governors");
	if(!f.exists() || !f.open(IO_ReadOnly))
		return CPUFREQ_NONE;
	int count = 0;
	if(!f.atEnd()) {
		f.readLine(buffer, 1024);
		TQStringList l = TQStringList::split(' ', buffer.stripWhiteSpace(), false);
		for(unsigned int i = 0; i < l.size(); ++i, ++count) {
			states.append(l[i].stripWhiteSpace());
			if(states[i] == cur)
				current = count;
		}
	}
	f.close();

	return CPUFREQ_SYSFS;
}

// get CPUFreq scaling policies via the 2.5 /proc interface
// sample output of cat /proc/cpufreq:
//           minimum CPU frequency  -  maximum CPU frequency  -  policy
// CPU  0       700000 kHz ( 70 %)  -    1000000 kHz (100 %)  -  powersave
static int get_cpufreq_25_state(TQStringList &states, int &current) {
	current = -1;
	states.clear();

	TQFile f("/proc/cpufreq");
	if (f.exists() && f.open(IO_ReadOnly)) {
		while (!f.atEnd()) {
			TQString l;
			f.readLine(l, 1024);
			TQRegExp rx("CPU.*\\d+.*(\\d+).*-.*(\\d+).*-\\W*(\\w*)");
			if (rx.search(l) >= 0) {
				cpufreq_minmax_frequency[0] = rx.cap(1);
				cpufreq_minmax_frequency[1] = rx.cap(2);
				if(rx.cap(3) == "performance")
					current = 0;
				else
					current = 1;
				break;
			}
		}
	}
	f.close();

	if(current < 0)
		return CPUFREQ_NONE;

	// we don't know all available scaling governors, so use only a minimal set
	states.append("performance");
	states.append("powersave");
	return CPUFREQ_25;
}

// get CPUFreq scaling policies via the 2.4 /proc interface
// The old interface doesn't support policies yet, we only get the min and max frequency,
// so we use these as performance states.
static int get_cpufreq_24_state(TQStringList &states, int &current, const TQString cpu) {
	TQString buffer, cur;
	states.clear();

	// current frequency
	TQFile f("/proc/sys/cpu/" + cpu + "/speed");
	if(!f.exists() || !f.open(IO_ReadOnly) || f.atEnd())
		return CPUFREQ_NONE;
	f.readLine(buffer, 16);
	f.close();
	cur = buffer.stripWhiteSpace();

	// read maximal and minimal frequency
	const char* files[] = { "max", "min" };
	for(int i = 0; i <= 1; ++i) {
		f.setName("/proc/sys/cpu/" + cpu + "/speed-" + files[i]);
		if(!f.exists() || !f.open(IO_ReadOnly) || f.atEnd())
			return CPUFREQ_NONE;
		f.readLine(buffer, 16);
		f.close();
		cpufreq_minmax_frequency[i] = buffer;
		unsigned int val = buffer.toUInt() / 1000;
		states.append(i18n("%1 MHz").arg(val));
		if(buffer.stripWhiteSpace() == cur)
			current = i;
	}
	return CPUFREQ_24;
}

// check for CPUFreq support and get a list of all available scaling policies
// this method doesn't support multiple CPU's (neither does get_acpi_list() above), 
// but this shouldn't be a problem on notebooks...
static int get_cpufreq_state(bool force, TQStringList &states, int &current) {
	static int known = -1;

	// check wether we already know which interface to use
	if(!force && known >= 0) {
		switch(known) {
			case CPUFREQ_SYSFS:
				return get_cpufreq_sysfs_state(states, current, cpufreq_cpu);
				break;
			case CPUFREQ_25:
				return get_cpufreq_25_state(states, current);
				break;
			case CPUFREQ_24:
				return get_cpufreq_24_state(states, current, cpufreq_cpu);
				break;
			default:
				return CPUFREQ_NONE;
		}
	}

	// look for the CPUFreq sysfs interface first
	TQDir dir("/sys/devices/system/cpu");
	dir.setFilter(TQDir::Dirs);
	if(dir.isReadable()) {
		for(unsigned int i = 0; !dir[i].isNull(); ++i) {
			if(dir[i] == "." || dir[i] == "..")
				continue;
			cpufreq_cpu = dir[i];
			if(get_cpufreq_sysfs_state(states, current, cpufreq_cpu))
				return (known = CPUFREQ_SYSFS);
		}
	}

	// try the /proc interface from the 2.5 kernel series next
	if(get_cpufreq_25_state(states, current))
		return (known = CPUFREQ_25);

	// last chance: the /proc interface from the 2.4 kernel series
	dir.setPath("/proc/sys/cpu");
	dir.setFilter(TQDir::Dirs);
	if(dir.isReadable()) {
		for(unsigned int i = 0; !dir[i].isNull(); ++i) {
			if(dir[i] == "." || dir[i] == "..")
				continue;
			cpufreq_cpu = dir[i];
			if(get_cpufreq_24_state(states, current, cpufreq_cpu))
				return (known = CPUFREQ_24);
		}
	}

	// no CPUFreq support found
	return (known = CPUFREQ_NONE);
}

bool
laptop_portable::get_system_performance(bool force, int &current, TQStringList &s, bool *&active)	 // do something to help get system profiles from places like ACPI
{
	if(!acpi_performance_enabled)
		return false;
	static int known=0;
	static int index=0;
	static bool result = 0;
	if (known != last_seed || force) {
		known = last_seed;
		performance_list.clear();
		result = 0;
		current = 0;
		if (::has_acpi()) {
			active = acpi_performance_enable;
			result = get_acpi_list('P', acpi_performance_map, "/performance", performance_list, index, acpi_performance_cpu, force, acpi_performance_enable);
		}
		// CPUFreq support
		if (!result && get_cpufreq_state(force, performance_list, index)) {
			for(unsigned int i = 0; i < performance_list.size(); ++i)
				acpi_performance_enable[i] = true;
			active = acpi_performance_enable;
			result = true;
		}
		if (!result) {
			// INSERT HERE
		}
	}
	current = index;
	s = performance_list;
	return(result);
}

bool
laptop_portable::get_system_throttling(bool force, int &current, TQStringList &s, bool *&active)   // do something to help get system throttling data from places like ACPI
{
	static int known=0;
	static int index=0;
	static bool result = 0;
	if (known != last_seed || force) {
		known = last_seed;
		throttle_list.clear();
		result = 0;
		current = 0;
		if (::has_acpi() && acpi_throttle_enabled) {
			active = acpi_throttle_enable;
			result = get_acpi_list('T', acpi_throttle_map, "/throttling", throttle_list, index, acpi_throttle_cpu, force, acpi_throttle_enable);
		}
		if (!result) {
			// INSERT HERE
		}
	}
	current = index;
	s = throttle_list;
	return(result);
}

void
laptop_portable::set_system_performance(TQString val)	// val = string given by get_system_performance above
{
	if(!acpi_performance_enabled)
		return;
	int current, result;
	if((result = get_acpi_list('P', acpi_performance_map, "/performance", performance_list, current, acpi_performance_cpu, false, acpi_performance_enable))) {
		char tmp[20];
		int ind = performance_list.findIndex(val);
		if (ind < 0 || ind >= MAP_SIZE || current == ind)
			return;
		snprintf(tmp, sizeof(tmp), "%d", acpi_performance_map[ind]);
		tmp[sizeof(tmp)-1]=0;
		::invoke_acpi_helper("--performance", acpi_performance_cpu.latin1(), tmp);
		return;
	}
	// CPUFreq support
	if((result = get_cpufreq_state(false, performance_list, current))) {
		if(performance_list[current] == val)
			return;
		TQString tmp;
		switch(result) {
			case CPUFREQ_SYSFS:
				invoke_acpi_helper("--cpufreq-sysfs", cpufreq_cpu.latin1(), val.latin1());
				return;
			case CPUFREQ_25:
				tmp = cpufreq_minmax_frequency[0] + ":" + cpufreq_minmax_frequency[1] + ":" + val;
				invoke_acpi_helper("--cpufreq-25", tmp.latin1(), 0);
				return;
			case CPUFREQ_24:
				int target = performance_list.findIndex(val);
				invoke_acpi_helper("--cpufreq-24", cpufreq_cpu.latin1(), cpufreq_minmax_frequency[target].latin1());
				return;
		}
	}
	// INSERT HERE
	return;
}

void
laptop_portable::set_system_throttling(TQString val)	// val = string given by get_system_throttle above
{
	if (::has_acpi()) {
		char tmp[20];
		int ind = throttle_list.findIndex(val);
		if (ind < 0 || ind >= MAP_SIZE)
			return;
		snprintf(tmp, sizeof(tmp), "%d", acpi_throttle_map[ind]);
		tmp[sizeof(tmp)-1]=0;
		::invoke_acpi_helper("--throttling", acpi_throttle_cpu.latin1(), tmp);
		return;
	}
	// INSERT HERE
	return;
}

static TQString acpi_power_name, acpi_lid_name;	// names of paths to ACPI lid states

static bool
acpi_check_button(const char *prefix, TQString &result)
{
	DIR *dfd;
	struct dirent *dp;
	bool v=0;

	dfd = opendir(prefix);
	if (dfd) {
		for (dp = readdir(dfd);dp ;dp = readdir(dfd)) {
			if (strcmp(dp->d_name, ".") == 0 ||
    		  	    strcmp(dp->d_name, "..") == 0)
				continue;
			TQString name(prefix);
			name += "/";
			name += dp->d_name;
			name += "/state";
 	                if (::access(name.latin1(), R_OK)!=0)
				continue;
			TQFile f(name);
			if (f.exists() && f.open(IO_ReadOnly)) {
				while (!f.atEnd()) {
					TQString l;
      					f.readLine(l, 500);
					if (l.contains("state:")) {
						result = name;
						v = 1;
						break;
					}
				}
				f.close();
				if (v)
					break;
			}
		}
		closedir(dfd);
	}
	return(v);
}

bool
laptop_portable::has_button(LaptopButton l)	// true if we have support for a particular button
{
	static int known[2]={0,0};
	static bool result[2] = {0,0};
	int type = (l==LidButton?0:1);
	if (known[type] != last_seed) { 
		result[type] = 0;
		known[type] = last_seed;
		if (::has_acpi()) {
			switch (l) {
			case LidButton:
				result[type] = ::acpi_check_button("/proc/acpi/button/lid", acpi_lid_name);
				break;
			case PowerButton:
				result[type] = ::acpi_check_button("/proc/acpi/button/power", acpi_power_name);
				break;
			default:
				break;
			}
		}
		if (!result[type] && ::has_ibm() && l == LidButton) 
			result[type] = 1;
		if (!result[type]) {
		// INSERT HERE
		}
	}
	return(result[type]);
}

bool
laptop_portable::get_button(LaptopButton l)	// true if a button is pressed
{
	if (::has_acpi()) {
		TQString name;
		switch (l) {
		case LidButton:
			name = acpi_lid_name;
			break;
		case PowerButton:
			name = acpi_power_name;
			break;
		default:
			break;
		}
		if (!name.isEmpty()) {
			TQFile f(name);
			if (f.exists() && f.open(IO_ReadOnly)) {
				while (!f.atEnd()) {
					TQString l;
					f.readLine(l, 500);
					TQStringList ll = TQStringList::split(':',l,false);
					if (ll[0].stripWhiteSpace() == "state") {
						if (ll[1].stripWhiteSpace() == "open") {
							f.close();
							return(0);
						}
						if (ll[1].stripWhiteSpace() == "closed") {
							f.close();
							return(1);
						}
						break;
					}
				}
				f.close();
			}

		}
	}
	if (::has_ibm() && l == LidButton) {
		smapidev_sensorinfo_t t;
		if (smapidev_GetSensorInfo(ibm_fd, &t) == 0) 
			return(t.fLidClosed != 0);
	}
	// INSERT HERE
	return(0);
}

#elif defined(__FreeBSD__) && HAVE_MACHINE_APM_BIOS_H

/*
** This is the FreeBSD-specific code.
*/

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <machine/apm_bios.h>
#include <sys/stat.h>
#include <tqpushbutton.h>
#include <tqobject.h>
#include <kactivelabel.h>
#include <kprocess.h>

#define APMDEV "/dev/apm"

// FreeBSD support by yours truly.  Yay.
// Actually this code was "adapted" from apm(8) from
// FreeBSD's collection of tools.  The original apm program
// was pieced together by Tatsumi Hosokawa <hosokawa@jp.FreeBSD.org> in 1994

//
//	returns 1 if we support power management
//

#include <iostream>
using namespace std;

//
//	something changed maybe we need to check out environment again
//
void
laptop_portable::power_management_restart()
{
	// INSERT HERE
}

int
laptop_portable::has_power_management()
{
	int ret, fd = ::open(APMDEV, O_RDWR);

	if (fd == -1) {
	  return 0;
	}

	struct apm_info info;
	ret=ioctl(fd, APMIO_GETINFO, &info);
	::close(fd);

	if (ret == -1) {
	  return 0;
	}

	return info.ai_status;
}
//
//	returns 1 if the BIOS returns the time left in the battery rather than a % of full
//
int laptop_portable::has_battery_time()
{
	int ret, fd = ::open(APMDEV, O_RDWR);

	if (fd == -1)
	  return 0;

	struct apm_info info;
	ret=ioctl(fd, APMIO_GETINFO, &info);
	::close(fd);

	if (ret == -1)
	  return 0;

	return (info.ai_batt_time != 0xffff);
}

//
//	returns 1 if we can perform a change-to-suspend-mode operation for the user
//	(has_power_management() has already returned 1)
//
int laptop_portable::has_suspend()
{
	int ret, fd = ::open(APMDEV, O_RDWR);

	if (fd == -1)
	  return 0;

	struct apm_info info;
	ret=ioctl(fd, APMIO_GETINFO, &info);
	::close(fd);

	if (ret == -1)
	  return 0;

	return (info.ai_capabilities & 0x02);
}
//
//	returns 1 if we can perform a change-to-standby-mode operation for the user
//	(has_power_management() has already returned 1)
//
int laptop_portable::has_standby()
{
	int ret, fd = ::open(APMDEV, O_RDWR);

	if (fd == -1)
	  return 0;

	struct apm_info info;
	ret=ioctl(fd, APMIO_GETINFO, &info);
	::close(fd);

	if (ret == -1)
	  return 0;

	return (info.ai_capabilities & 0x01);
}
//
//	returns 1 if we can perform a change-to-hibernate-mode for a user
//      (has_power_management() has already returned 1)  [hibernate is the save-to-disk mode
//	not supported by linux - different laptops have their own - the first here is for 
//	a ThinkPad]
//
int laptop_portable::has_hibernation()
{
	if (::access(PATH_TPCTL, X_OK)==0)
		return(1);
	return(0);
}

//
//	explain to the user what they need to do if has_power_management() returned 0
//	to get any software they lack
//
KActiveLabel *laptop_portable::no_power_management_explanation(TQWidget *parent)
{
  int fd;
  KActiveLabel *explain;

  fd = ::open(APMDEV, O_RDWR);
  if (fd == -1) {
    switch (errno) {
    case ENOENT:
      explain = new KActiveLabel(i18n("There is no /dev/apm file on this system. Please review the FreeBSD handbook on how to create a device node for the APM device driver (man 4 apm)."), parent);
      break;
    case EACCES:
      explain = new KActiveLabel(i18n("Your system has the proper device node for APM support, however you cannot access it. If you are logged in as root right now, you have a problem, otherwise contact your local sysadmin and ask for read/write access to /dev/apm."), parent);
      break;
    case ENXIO:
      explain = new KActiveLabel(i18n("Your kernel lacks support for Advanced Power Management."), parent);
      break;
      break;
    default:
      explain = new KActiveLabel(i18n("There was a generic error while opening /dev/apm."), parent);
      break;
    }
  } else {
    close(fd);
    explain = new KActiveLabel(i18n("APM has most likely been disabled."), parent);
  }
  
  return(explain);
}

//
//	explain to the user what they need to do to get suspend/resume to work from user mode
//
TQLabel *laptop_portable::how_to_do_suspend_resume(TQWidget *parent)
{
 	TQLabel* note = new TQLabel(" ", parent);
	return(note);
}


//
//	pcmcia support - this will be replaced by better - pcmcia support being worked on by
//	others
//
TQLabel *laptop_portable::pcmcia_info(int x, TQWidget *parent)
{
      	if (x == 0)
		return(new TQLabel(i18n("No PCMCIA controller detected"), parent));
      	return(new TQLabel(i18n(""), parent));
}
//
//	puts us into standby mode
//
void laptop_portable::invoke_standby()
{
  	int fd = ::open(APMDEV, O_RDWR);

	if (fd == -1)
	  return;

	ioctl(fd, APMIO_STANDBY, NULL);
	::close(fd);

	return;
}

//
//	puts us into suspend mode
//
void laptop_portable::invoke_suspend()
{
  	int fd = ::open(APMDEV, O_RDWR);

	if (fd == -1)
	  return;

	ioctl(fd, APMIO_SUSPEND, NULL);
	::close(fd);

	return;
}
//
//	puts us into hibernate mode
//
void laptop_portable::invoke_hibernation()
{
	TDEProcess thisProc;

	if (::access(PATH_TPCTL, X_OK)==0) {
		thisProc << PATH_TPCTL;
		thisProc << "---hibernate";
		thisProc.start(TDEProcess::Block);
		return;
	}
}

//
//ACPI specific - chances are you don't need to implement this one
//
void
laptop_portable::acpi_set_mask(bool, bool, bool, bool, bool )
{
	// INSERT HERE
}

int laptop_portable::has_acpi(int)
{
	// INSERT HERE
	return (0);
}

int laptop_portable::has_apm(int type)
{
	if (type == 1)		// implement me .... this is the hook that pops up the panel for making /usr/sbin/apm setuid
		return(0);	//			or in this case you could make an acpi_helper type app freebsd
	return (1);
}

void
laptop_portable::apm_set_mask(bool , bool )
{
}


//
//	adds extra widgets to the battery panel
//
void
laptop_portable::extra_config(TQWidget * /*parent*/, TDEConfig * /*config*/, TQVBoxLayout * /*layout*/)
{
	// INSERT HERE
}



//
//	return current battery state
//
struct power_result laptop_portable::poll_battery_state()
{
	struct power_result p;
       	int ret;

       	int fd = ::open(APMDEV, O_RDWR);

       	if (fd == -1)
         	goto bad;

       	struct apm_info info;
       	ret=ioctl(fd, APMIO_GETINFO, &info);
       	::close(fd);
	
       	if (ret == -1)
         	goto bad;

       	p.powered = info.ai_acline;
       	p.percentage = (info.ai_batt_life==255 ? 100 : info.ai_batt_life);
       	p.time = (info.ai_batt_time != 0xffff ? info.ai_batt_time/60 : -1);
       	return(p);

bad:
       	p.powered = 1; 
	p.percentage = 100;
	p.time = 0;
	return(p);
}

//
//	return the system load 
//	

bool laptop_portable::has_lav() { return 0; }

float laptop_portable::get_load_average()
{
	// INSERT HERE
	return(-1);
}

int laptop_portable::has_cpufreq() {
	// INSERT HERE
	return 0;
}

TQString laptop_portable::cpu_frequency() {
	// INSERT HERE
	return "";
}

int
laptop_portable::has_brightness()
{
	// INSERT HERE
	return 0;
}

void 
laptop_portable::set_brightness(bool /*blank*/, int /*val*/)	// val = 0-255 255 brightest, 0 means dimmest (if !blank it must be still visible), 
{
	// INSERT HERE
}

int 
laptop_portable::get_brightness()
{
	// INSERT HERE
	return(-1);	// means can't extract it
}

bool
laptop_portable::get_system_performance(bool, int &current, TQStringList &s, bool *&)	 // do something to help get system profiles from places like ACPI
{
	// INSERT HERE
	current = 0;
	s.clear();
	return(0);	// if no profiles are available
}

bool
laptop_portable::get_system_throttling(bool, int &current, TQStringList &s, bool *&)   // do something to help get system throttling data from places like ACPI
{
	// INSERT HERE
	current = 0;
	s.clear();
	return(0);
}

void
laptop_portable::set_system_performance(TQString)
{
	// INSERT HERE
}

void
laptop_portable::set_system_throttling(TQString)
{
	// INSERT HERE
}

bool
laptop_portable::has_button(LaptopButton)	// true if we have support for a particular button
{
	// INSERT HERE
	return(0);
}

bool
laptop_portable::get_button(LaptopButton)	// true if a button is pressed
{
	// INSERT HERE
	return(0);
}

void
laptop_portable::get_battery_status(int &num_batteries, TQStringList &names, TQStringList &state, TQStringList &values) // get multiple battery status
{
	struct power_result r;

	if (!has_power_management()) {
		num_batteries = 0;
		names.clear();
		state.clear();
		values.clear();
		return;
	}

	// INSERT HERE
	
	num_batteries = 1;
	r = poll_battery_state();
	names.append("BAT1");
	state.append("yes");
	TQString s;
	s.setNum(r.percentage);
	values.append(s);
}

bool
laptop_portable::has_software_suspend(int /*type*/)
{
	return false; // (::has_software_suspend(type));
}

void
laptop_portable::software_suspend_set_mask(bool /*hibernate*/)
{
	// software_suspend_is_preferred = hibernate;
}


#elif  defined(__NetBSD_APM__)

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <machine/apmvar.h>
#include <iostream.h>

//
// klaptopdeamon interface to NetBSD 1.5 apm.
// Scott Presnell, srp@zgi.com, srp@tworoads.net
// Fri Jun 29 17:21:25 PDT 2001
// Tested on Dell I4K running NetBSD 1.5R
//
#define APMDEV "/dev/apm"

//
//     Check for apm in kernel by talking to /dev/apm
//     (opening read only is allowed by any process).
//     returns 1 if we support power management
//
int
laptop_portable::has_power_management()
{
       int ret, fd = ::open(APMDEV, O_RDONLY);

       if (fd == -1) {
         return 0;
       }

       struct apm_power_info info;
       ret=ioctl(fd, APM_IOC_GETPOWER, &info);
       ::close(fd);

       if (ret == -1) {
         return 0;
       }

       return 1;
}

void
laptop_portable::power_management_restart()
{
	// INSERT HERE
}

//
//     returns 1 if the BIOS returns the time left in the battery rather than a % of full
//
int laptop_portable::has_battery_time()
{
       int ret, fd = ::open(APMDEV, O_RDONLY);

       if (fd == -1)
         return 0;

       struct apm_power_info info;
       ret=ioctl(fd, APM_IOC_GETPOWER, &info);
       ::close(fd);

       if (ret == -1)
         return 0;

       return (info.minutes_left != 0xffff);
}

//
//     returns 1 if we can perform a change-to-suspend-mode operation for the user
//     (ust check to see if we have the binary)
//     (has_power_management() has already returned 1)
//
int laptop_portable::has_suspend()
{

       struct stat s;
        if (stat("/usr/sbin/apm", &s))
               return(0);
       return(1);
}

//
//     returns 1 if we can perform a change-to-standby-mode operation for the user
//     (just check to see if we have the binary)
//     (has_power_management() has already returned 1)
//
int laptop_portable::has_standby()
{

       struct stat s;
        if (stat("/usr/sbin/apm", &s))
               return(0);
       return(1);
}

//
//     returns 1 if we can perform a change-to-hibernate-mode for a user
//      (has_power_management() has already returned 1)  [hibernate is the save-to-disk mode
//     not supported by linux - different laptops have their own - the first here is for 
//     a ThinkPad]
//     No support in NetBSD at this time.
//
int laptop_portable::has_hibernation()
{
	// INSERT HERE
       return(0);
}

//
//     explain to the user what they need to do if has_power_management() returned 0
//     to get any software they lack
//
KActiveLabel *laptop_portable::no_power_management_explanation(TQWidget *parent)
{
  int fd;
  KActiveLabel *explain;

  fd = ::open(APMDEV, O_RDONLY);
  if (fd == -1) {
    switch (errno) {
    case ENOENT:
      explain = new KActiveLabel(i18n("There is no /dev/apm file on this system. Please review the NetBSD documentation on how to create a device node for the APM device driver (man 4 apm)."), parent);
      break;
    case EACCES:
      explain = new KActiveLabel(i18n("Your system has the proper device node for APM support, however you cannot access it. If you have APM compiled into the kernel this should not happen."), parent);
      break;
    case ENXIO:
      explain = new KActiveLabel(i18n("Your kernel lacks support for Advanced Power Management."), parent);
      break;
      break;
    default:
      explain = new KActiveLabel(i18n("There was a generic error while opening /dev/apm."), parent);
      break;
    }
  } else {
    close(fd);
    explain = new KActiveLabel(i18n("APM has most likely been disabled."), parent);
  }
  
  return(explain);
}

//
//     explain to the user what they need to do to get suspend/resume to work from user mode
//
TQLabel *laptop_portable::how_to_do_suspend_resume(TQWidget *parent)
{
	// INSERT HERE
       TQLabel* note = new TQLabel(" ", parent);
       return(note);
}

//
//     pcmcia support - this will be replaced by better - pcmcia support being worked on by
//     others
//
TQLabel *laptop_portable::pcmcia_info(int x, TQWidget *parent)
{
	// INSERT HERE
       if (x == 0)
               return(new TQLabel(i18n("No PCMCIA controller detected"), parent));
       return(new TQLabel(i18n(""), parent));
}

//
//     puts us into standby mode
//     Use apm rather than ioctls in case they are running apmd
//     (as they should be).
//
void laptop_portable::invoke_standby()
{
	TDEProcess proc;
	proc << "/usr/sbin/apm";
	proc << "-S";
	proc.start(TDEProcess::Block);	// helper runs fast and we want to see the result
}

//
//     puts us into suspend mode
//     Use apm rather than ioctls in case they are running apmd
//     (as they should be).
//
void laptop_portable::invoke_suspend()
{
	TDEProcess proc;
	proc << "/usr/sbin/apm";
	proc << "-z";
	proc.start(TDEProcess::Block);	// helper runs fast and we want to see the result
}

//
//     puts us into hibernate mode
//     No hibernate mode for NetBSD.
//
void laptop_portable::invoke_hibernation()
{
	// INSERT HERE
       return;
}

//
//ACPI specific - chances are you don't need to implement this one
//
void
laptop_portable::acpi_set_mask(bool, bool, bool, bool, bool )
{
	// INSERT HERE
}

int laptop_portable::has_acpi(int)
{
	// INSERT HERE
	return (0);
}

int laptop_portable::has_apm(int type)
{
	if (type == 1)		// implement me .... this is the hook that pops up the panel for making /usr/sbin/apm setuid
		return(0);
	return (1);
}

void
laptop_portable::apm_set_mask(bool , bool )
{
}


//
//     return current battery state
//
struct power_result laptop_portable::poll_battery_state()
{
       struct power_result p;
               int ret;

               int fd = ::open(APMDEV, O_RDONLY);

               if (fd == -1)
               goto bad;

               struct apm_power_info info;
               ret=ioctl(fd, APM_IOC_GETPOWER, &info);
               ::close(fd);
       
               if (ret == -1)
               goto bad;

               p.powered = (info.ac_state == APM_AC_ON);
               p.percentage = (info.battery_life==255 ? 100 : info.battery_life);
               p.time = (info.minutes_left != 0xffff ? info.minutes_left : -1);
               return(p);

bad:
               p.powered = 1; 
       p.percentage = 100;
       p.time = 0;
       return(p);
}

//
//	return the system load 
//	

bool laptop_portable::has_lav() { return 0; }

float laptop_portable::get_load_average()
{
	// INSERT HERE
	return(-1);
}


int laptop_portable::has_cpufreq() {
	// INSERT HERE
	return 0;
}

TQString laptop_portable::cpu_frequency() {
	// INSERT HERE
	return "";
}

int
laptop_portable::has_brightness()
{
	// INSERT HERE
	return 0;
}

void 
laptop_portable::set_brightness(bool blank, int val)	// val = 0-255 255 brightest, 0 means dimmest (if !blank it must be still visible), 
{
	// INSERT HERE
}

int 
laptop_portable::get_brightness()
{
	// INSERT HERE
	return(-1);	// means can't extract it
}

bool
laptop_portable::get_system_throttling(bool, int &current, TQStringList &s)   // do something to help get system throttling data from places like ACPI
{
	// INSERT HERE
	current = 0;
	s.clear();
	return(0);
}

void
laptop_portable::set_system_performance(TQString)
{
	// INSERT HERE
}

void
laptop_portable::set_system_throttling(TQString)
{
	// INSERT HERE
}

bool
laptop_portable::has_button(LaptopButton)	// true if we have support for a particular button
{
	// INSERT HERE
	return(0);
}

bool
laptop_portable::get_button(LaptopButton)	// true if a button is pressed
{
	// INSERT HERE
	return(0);
}

void
laptop_portable::get_battery_status(int &num_batteries, TQStringList &names, TQStringList &state, TQStringList &values) // get multiple battery status
{
	struct power_result r;

	if (!has_power_management()) {
		num_batteries = 0;
		names.clear();
		state.clear();
		values.clear();
		return;
	}

	// INSERT HERE
	
	num_batteries = 1;
	r = poll_battery_state();
	names.append("BAT1");
	state.append("yes");
	TQString s;
	s.setNum(r.percentage);
	values.append(s);
}

#else

/*
** This is utterly generic code.
*/

//
//	something changed maybe we need to check out environment again
//
void
laptop_portable::power_management_restart()
{
	// INSERT HERE
}

//
//	returns 1 if we support power management
//
int
laptop_portable::has_power_management()
{
	// INSERT HERE
	return(0);
}

//
//	returns 1 if the BIOS returns the time left in the battery rather than a % of full
//
int laptop_portable::has_battery_time()
{
	// INSERT HERE
	return (0);
}

//
//	returns 1 if we can perform a change-to-suspend-mode operation for the user
//	(has_power_management() has already returned 1)
//
int laptop_portable::has_suspend()
{
	// INSERT HERE
	return(0);
}
//
//	returns 1 if we can perform a change-to-standby-mode operation for the user
//	(has_power_management() has already returned 1)
//
int laptop_portable::has_standby()
{
	// INSERT HERE
	return(0);
}
//
//	returns 1 if we can perform a change-to-hibernate-mode for a user
//      (has_power_management() has already returned 1)  [hibernate is the save-to-disk mode
//	not supported by linux]
//
int laptop_portable::has_hibernation()
{
	// INSERT HERE
	return(0);
}

//
//	explain to the user what they need to do if has_power_management() returned 0
//	to get any software they lack
//
KActiveLabel *laptop_portable::no_power_management_explanation(TQWidget *parent)
{
	KActiveLabel* explain = new KActiveLabel(i18n("Your computer or operating system is not supported by the current version of the\nKDE laptop control panels. If you want help porting these panels to work with it\nplease contact paul@taniwha.com."), parent);
	// INSERT HERE
	return(explain);
}

//
//	explain to the user what they need to do to get suspend/resume to work from user mode
//
TQLabel *laptop_portable::how_to_do_suspend_resume(TQWidget *parent)
{
 	TQLabel* note = new TQLabel(" ", parent);
	// INSERT HERE
	return(note);
}


//
//	pcmcia support - this will be replaced by better - pcmcia support being worked on by
//	others
//
TQLabel *laptop_portable::pcmcia_info(int x, TQWidget *parent)
{
	// INSERT HERE
      	if (x == 0)
		return(new TQLabel(i18n("No PCMCIA controller detected"), parent));
      	return(new TQLabel(i18n(""), parent));
}
//
//	puts us into standby mode
//
void laptop_portable::invoke_standby()
{
	// INSERT HERE
}

//
//	puts us into suspend mode
//
void laptop_portable::invoke_suspend()
{
	// INSERT HERE
}
//
//	puts us into hibernate mode
//
void laptop_portable::invoke_hibernation()
{
	// INSERT HERE
}

//
//ACPI specific - chances are you don't need to implement this one
//
void
laptop_portable::acpi_set_mask(bool, bool, bool, bool, bool )
{
	// INSERT HERE
}

int laptop_portable::has_acpi(int)
{
	// INSERT HERE
	return (0);
}

void
laptop_portable::apm_set_mask(bool, bool)
{
	// INSERT HERE
}

int laptop_portable::has_apm(int)
{
	// INSERT HERE
	return (0);
}


//
//	adds extra widgets to the battery panel
//
void
laptop_portable::extra_config(TQWidget *parent, TDEConfig *config, TQVBoxLayout *layout)
{
	// INSERT HERE
}

//
//	return current battery state
//
struct power_result laptop_portable::poll_battery_state()
{
	struct power_result p;
	// INSERT HERE
	p.powered = 0;
	p.percentage = 0;
	p.time = 0;
	return(p);
}

//
//	return the system load 
//	

bool laptop_portable::has_lav() { return 0; }

float laptop_portable::get_load_average()
{
	// INSERT HERE
	return(-1);
}

int laptop_portable::has_cpufreq() {
	// INSERT HERE
	return 0;
}

TQString laptop_portable::cpu_frequency() {
	// INSERT HERE
	return "";
}

int
laptop_portable::has_brightness()
{
	// INSERT HERE
	return 0;
}

void 
laptop_portable::set_brightness(bool blank, int val)	// val = 0-255 255 brightest, 0 means dimmest (if !blank it must be still visible), 
{
	// INSERT HERE
}

int 
laptop_portable::get_brightness()
{
	// INSERT HERE
	return(-1);	// means can't extract it
}

bool
laptop_portable::get_system_performance(bool, int &current, TQStringList &s, bool *&)	 // do something to help get system profiles from places like ACPI
{	
	// INSERT HERE
	current = 0;
	s.clear();
	return(0);	// if no profiles are available
}

bool
laptop_portable::get_system_throttling(bool, int &current, TQStringList &s, bool *&)   // do something to help get system throttling data from places like ACPI
{
	// INSERT HERE
	current = 0;
	s.clear();
	return(0);
}

void
laptop_portable::set_system_performance(TQString)
{
	// INSERT HERE
}

void
laptop_portable::set_system_throttling(TQString)
{
	// INSERT HERE
}

bool
laptop_portable::has_button(LaptopButton)	// true if we have support for a particular button
{
	// INSERT HERE
	return(0);
}

bool
laptop_portable::get_button(LaptopButton)	// true if a button is pressed
{
	// INSERT HERE
	return(0);
}

void
laptop_portable::get_battery_status(int &num_batteries, TQStringList &names, TQStringList &state, TQStringList &values) // get multiple battery status
{
	struct power_result r;

	if (!has_power_management()) {
		num_batteries = 0;
		names.clear();
		state.clear();
		values.clear();
		return;
	}

	// INSERT HERE
	
	num_batteries = 1;
	r = poll_battery_state();
	names.append("BAT1");
	state.append("yes");
	TQString s;
	s.setNum(r.percentage);
	values.append(s);
}

bool
laptop_portable::has_software_suspend(int type)
{
	return false; // (::has_software_suspend(type));
}

void
laptop_portable::software_suspend_set_mask(bool hibernate)
{
	// software_suspend_is_preferred = hibernate;
}
#endif
