(C) 2003, 2004: Mirko Boehm <mirko@kde.org>
Licensed under the terms of the LGPL. 

This file explains the bad hack of copying the sonypi.h header file into this
directory: 
-----------------------------------------------------------------------------

Along with glibc, an outdated header snapshot is installed. The event
definitions in include/linux/sonypi.h in newer kernels differ too much from
it. Since the code shipped here depends on the proper event definitions, I
decided to ship a copy of a more up to date include file here. 

The solution is not optimal, but it works, and I try to keep the files in this
directory up to date. 

If somebody knows a better solution, please let me know at mirko@kde.org.


