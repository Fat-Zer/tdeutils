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

#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H

class System
{
  public:
    /**
     * converts bytes to megabytes
     */
    static int bytesToMegs( unsigned long );

    /**
     * @return a pointer to the instance
     */
    static System & self();
    /**
     * Deletes the instance and cleans up after itself
     */
    static void cleanup();

    /**
     * @return the total amount of seconds
     * the pc has been running
     */
    long uptime() const { return m_uptime; }
    /**
     * @return the 3 load averages
     */
    const double * loadAverages() const { return m_loads; }
    /**
     * this function does the same as the
     * above but returns only the @ref load
     * average
     */
    double loadAverage( int ) const;
    /**
     * @return the total ram the system has
     */
    unsigned long totalRam() const { return m_totalram; }
    /**
     * @return the total used ram
     */
    unsigned long usedRam() const { return m_usedram; }
    /**
     * @return the total free ram the system has
     */
    unsigned long freeRam() const { return m_freeram; }
    /**
     * @return the amount of shared
     * memory in the system
     */
    unsigned long sharedRam() const { return m_sharedram; }
    /**
     * @return the amount of buffered
     * memory in the system
     */
    unsigned long bufferRam() const { return m_bufferram; }
    /**
     * @return the amount of buffered
     * memory in the system
     */
    unsigned long cacheRam() const { return m_cacheram; }
    /**
     * @return the total amount of
     * high memory
     */
    unsigned long totalHigh() const { return m_totalhigh; }
    /**
     * @return the total amount of
     * high free memory
     */
    unsigned long freeHigh() const { return m_freehigh; }
    /**
     * @return the total amount of
     * swap the system has
     */
    unsigned long totalSwap() const { return m_totalswap; }
    /**
     * @return the total amount of
     * swap used
     */
    unsigned long usedSwap() const { return m_usedswap; }
    /**
     * @return the total amount of
     * free swap the system has
     */
    unsigned long freeSwap() const { return m_freeswap; }
    /**
     * @return the number of
     * procs running
     */
    unsigned short procs() const { return m_procs; }

  private:
    System();
    System( const System & );
    System & operator=( const System & );
    ~System();

    void updateData();

    long m_uptime; // seconds since boot
    double m_loads[3]; // 1, 5, and 15 minute load averages
    unsigned long m_totalram; // Total usable main memory size
    unsigned long m_usedram; // Used memory size
    unsigned long m_freeram; // Available memory size
    unsigned long m_sharedram; // Amount of shared memory
    unsigned long m_bufferram; // Memory used by buffers
    unsigned long m_cacheram; // Amount of cached ram
    unsigned long m_totalhigh; // Total high memory size
    unsigned long m_freehigh; // Available high memory size
    unsigned long m_totalswap; // Total swap space size
    unsigned long m_usedswap; // Used swap space size
    unsigned long m_freeswap; // Swap space still available
    unsigned short m_procs; // Number of current processes
    static System * m_self;
};
#endif
