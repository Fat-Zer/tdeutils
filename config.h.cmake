#cmakedefine PACKAGE "@PACKAGE@"
#cmakedefine VERSION "@VERSION@"

// ark

#cmakedefine HAVE_STRLCPY_PROTO
#if !defined(HAVE_STRLCPY_PROTO)
# ifdef __cplusplus
    extern "C" {
# endif
  unsigned long strlcpy(char*, const char*, unsigned long);
# ifdef __cplusplus
  }
#  endif
#endif

#cmakedefine HAVE_STRLCAT_PROTO
#if !defined(HAVE_STRLCAT_PROTO)
# ifdef __cplusplus
    extern "C" {
# endif
  unsigned long strlcat(char*, const char*, unsigned long);
# ifdef __cplusplus
  }
#  endif
#endif

// NOTE: some macros already defined in python.h so if they
//       won't be ifdefed they couse superkaramba to 
//       produce lots of warnings.


// kcalc

@SIZEOF_UNSIGNED_LONG_CODE@

#if !defined( HAVE_STDLIB_H )
#cmakedefine HAVE_STDLIB_H
#endif // HAVE_STDLIB_H

#if !defined( HAVE_LONG_DOUBLE )
#cmakedefine HAVE_LONG_DOUBLE
#endif // HAVE_LONG_DOUBLE

#cmakedefine HAVE_L_FUNCS
#cmakedefine HAVE_IEEEFP_H
#cmakedefine HAVE_FUNC_ISINF
#cmakedefine HAVE_FUNC_ROUND
#cmakedefine HAVE_FUNC_ROUNDL


// klaptopdaemon

#if !defined( HAVE_STDINT_H )
#cmakedefine HAVE_STDINT_H
#endif // HAVE_STDINT_H

#cmakedefine HAVE_XSCREENSAVER
#cmakedefine HAVE_DPMS
#cmakedefine HAVE_DPMSINFO_PROTO


// ksim

#if !defined( HAVE_SYS_STATVFS_H )
#cmakedefine HAVE_SYS_STATVFS_H
#endif // HAVE_SYS_STATVFS_H

#cmakedefine HAVE_SYS_STATFS_H
#cmakedefine HAVE_SYS_VFS_H
#cmakedefine HAVE_SYS_MOUNT_H
#cmakedefine HAVE_MNTENT_H
#cmakedefine HAVE_SYS_UCRED_H
#cmakedefine HAVE_SYS_MNTTAB_H
#cmakedefine HAVE_SYS_LOADAVG_H
#cmakedefine HAVE_SYSINFO_HIGH

#if !defined( HAVE_GETLOADAVG )
#cmakedefine HAVE_GETLOADAVG
#endif // HAVE_GETLOADAVG


// superkaramba

#cmakedefine HAVE_XMMS
#cmakedefine HAVE_KNEWSTUFF

#if !defined( HAVE_SYS_TYPES_H )
#cmakedefine HAVE_SYS_TYPES_H
#endif // HAVE_SYS_TYPES_H


