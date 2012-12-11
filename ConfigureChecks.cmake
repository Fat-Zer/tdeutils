#################################################
#
#  (C) 2012 Golubev Alexander
#  fatzer2 (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

##### check for gcc visibility support #########
# FIXME
# This should check for [T]Qt3 visibility support

if( WITH_GCC_VISIBILITY )
  if( NOT UNIX )
    tde_message_fatal(FATAL_ERROR "\ngcc visibility support was requested, but your system is not *NIX" )
  endif( NOT UNIX )
  set( __KDE_HAVE_GCC_VISIBILITY 1 )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
endif( WITH_GCC_VISIBILITY )

tde_setup_architecture_flags( )


##### ark #######################################

if( BUILD_ARK )
  check_symbol_exists( strlcpy string.h HAVE_STRLCPY_PROTO )
  check_symbol_exists( strlcat string.h HAVE_STRLCAT_PROTO )
endif( BUILD_ARK )


##### kcalc #####################################

if( BUILD_KCALC )
  check_type_size( "unsigned long" SIZEOF_UNSIGNED_LONG )
  check_include_file( "stdlib.h" HAVE_STDLIB_H )
  check_include_file( "ieeefp.h" HAVE_IEEEFP_H )

# NOTE: this check could be insufficiant for build on some
#       buggy, very old or unusual system configurations
  check_type_size( "long double" SIZEOF_LONG_DOUBLE)
  if( SIZEOF_LONG_DOUBLE )
    set( HAVE_LONG_DOUBLE 1 )
  endif( SIZEOF_LONG_DOUBLE )

  set( CMAKE_REQUIRED_LIBRARIES m )
  check_symbol_exists( "fabsl"  "math.h" HAVE_L_FUNCS)
  check_symbol_exists( "isinf"  "math.h" HAVE_FUNC_ISINF)
  check_symbol_exists( "round"  "math.h" HAVE_FUNC_ROUND)
  check_symbol_exists( "roundl" "math.h" HAVE_FUNC_ROUNDL)
  
# also libgmp is requred
  set( CMAKE_REQUIRED_LIBRARIES gmp )
# most functions are defined as macros so we can't use check_library exists
  check_symbol_exists( "gmp_asprintf" "gmp.h" HAVE_GMPLIB  )
  set( CMAKE_REQUIRED_LIBRARIES )

  if( HAVE_GMPLIB )
    set( GMP_LIBRARY gmp CACHE INTERNAL "" )
  else( )
    tde_message_fatal( "libgmp is required, but was not found on your system" )
  endif( )
endif ( BUILD_KCALC )


##### klaptopdaemon #############################

if( BUILD_KLAPTOPDAEMON )
# stdint.h
  check_include_file( "stdint.h" HAVE_STDINT_H )
  if( NOT HAVE_STDINT_H )
    tde_message_fatal( "stdint.h header is required, but was not found on your system" )
  endif( NOT HAVE_STDINT_H )

# xtest
  pkg_search_module( XTEST xtst )
   if( XTEST_FOUND )
     set( HAVE_XTEST 1 )
   else( XTEST_FOUND )
     tde_message_fatal( "libXtst is requested, but was not found on your system" )
   endif( XTEST_FOUND )

  if( WITH_XSCREENSAVER )
    pkg_search_module( XSCREENSAVER xscrnsaver )
     if( XSCREENSAVER_FOUND )
       set( HAVE_XSCREENSAVER 1 )
     else( XSCREENSAVER_FOUND )
       tde_message_fatal( "xscreensaver is requested, but was not found on your system" )
     endif( )
  endif( WITH_XSCREENSAVER )
endif( BUILD_KLAPTOPDAEMON )


##### klaptopdaemon and ksim ####################

if( ( BUILD_KLAPTOPDAEMON AND WITH_DPMS ) OR ( BUILD_KSIM AND WITH_SENSORS ) )
  pkg_search_module( XEXT xext )

  if( XEXT_FOUND )
    if( WITH_DPMS )
      set( CMAKE_REQUIRED_LIBRARIES ${XEXT_LIBRARIES} )
      check_symbol_exists( DPMSInfo 
          "X11/Xlib.h;X11/extensions/dpms.h" 
          HAVE_DPMSINFO_PROTO )
      set( CMAKE_REQUIRED_LIBRARIES )
      set( HAVE_DPMS 1 )
    endif( WITH_DPMS )
  else( XEXT_FOUND )
    tde_message_fatal( "libXext is requested, but was not found on your system" )
  endif( XEXT_FOUND )
endif( )


##### kmilo #####################################

if( BUILD_KMILO )
# FIXME: If anybody will ever compile trinity for POWERPC and especialy for 
#        POWERBOOK he or she should test workability of those modules
  if( WITH_POWERBOOK OR WITH_POWERBOOK2 )
    check_library_exists( "pbb" "init_libpbb" "" HAVE_PBBIPC_LIBRARY )
    check_include_file( "pbbipc.h" HAVE_PBBIPC_H )
    check_include_file( "pbb.h" HAVE_PBB_H )
    if( HAVE_PBBIPC_LIBRARY AND ( ( WITH_POWERBOOK AND HAVE_PBBIPC_H ) OR 
                                  ( WITH_POWERBOOK2 AND HAVE_PBB_H ) ) )
      set( PBB_LIBRARY pbb CACHE INTERNAL "" )
    else()
      tde_message_fatal( "suitable pbbuttonosd is required, but was not found on your system" )
    endif()
  endif( WITH_POWERBOOK  OR WITH_POWERBOOK2 )
endif( BUILD_KMILO )


##### ksim ######################################

if ( BUILD_KSIM )
  check_include_file( "sys/statvfs.h" HAVE_SYS_STATVFS_H )
  check_include_file( "sys/statfs.h"  HAVE_SYS_STATFS_H )
  check_include_file( "sys/vfs.h"     HAVE_SYS_VFS_H )
  check_include_file( "sys/mount.h"   HAVE_SYS_MOUNT_H )
  check_include_file( "mntent.h"      HAVE_MNTENT_H )
  check_include_file( "sys/ucred.h"   HAVE_SYS_UCRED_H )
  check_include_file( "sys/mnttab.h"  HAVE_SYS_MNTTAB_H )

  check_include_file( "sys/loadavg.h"  HAVE_SYS_LOADAVG_H )
  check_function_exists( getloadavg    HAVE_GETLOADAVG )

  check_c_source_compiles(
    "#include <linux/kernel.h>
     int main() { struct sysinfo system; 
     int totalhigh = system.totalhigh; 
     int freehigh = system.freehigh; return 0; }"
    HAVE_SYSINFO_HIGH )

  if( WITH_SNMP )
    check_include_file( "net-snmp/net-snmp-config.h" HAVE_NET_SNMP_NET_SNMP_CONFIG_H )
    check_library_exists( netsnmp init_snmp "" HAVE_NETSNMP )
    if( HAVE_NET_SNMP_NET_SNMP_CONFIG_H AND HAVE_NETSNMP )
     set( NETSNMP_LIBRARIES netsnmp )
    else ()
      tde_message_fatal( "netsnmp is required, but was not found on your system" )
    endif ()
  endif( WITH_SNMP )
endif ( BUILD_KSIM )


##### superkaramba ##############################

if ( BUILD_SUPERKARAMBA )
  check_include_file( "sys/types.h" HAVE_SYS_TYPES_H )

  find_package( PythonLibs )
  if( NOT PYTHONLIBS_FOUND )
    tde_message_fatal( "python is required, but was not found on your system" )
  endif( NOT PYTHONLIBS_FOUND )

  if( WITH_KNEWSTUFF )
    set( HAVE_KNEWSTUFF 1 )
    set( KNEWSTUFF_LIBRARIES knewstuff-shared )
  endif( WITH_KNEWSTUFF )

  if( WITH_XMMS )
    set( HAVE_XMMS 1 )
  endif( WITH_XMMS )
endif ( BUILD_SUPERKARAMBA )


# required stuff
find_package( TQt )
find_package( TDE )
