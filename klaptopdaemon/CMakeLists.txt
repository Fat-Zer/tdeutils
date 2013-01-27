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

add_subdirectory( pics )
add_subdirectory( applnk )

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### klaptop_acpi_helper (executable) ##########

tde_add_executable( klaptop_acpi_helper
  SOURCES acpi_helper.cpp
  LINK tdeio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


##### klaptop_check (executable) ################

tde_add_executable( klaptop_check
  SOURCES laptop_check.cpp
  LINK tdeio-shared kcmlaptop-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


##### kcmlaptop (shared) ########################

tde_add_library( kcmlaptop SHARED AUTOMOC
  SOURCES portable.cpp smapidev.c
    daemon_state.cpp wake_laptop.cpp
    krichtextlabel.cpp
  LINK tdeio-shared
  VERSION 0.0.0
  DESTINATION ${LIB_INSTALL_DIR}
)


##### kcm_laptop (kpart) ########################

tde_add_kpart( kcm_laptop AUTOMOC
  SOURCES battery.cpp main.cpp pcmcia.cpp
    power.cpp warning.cpp acpi.cpp sony.cpp
    profile.cpp buttons.cpp apm.cpp 
    ${CMAKE_CURRENT_BINARY_DIR}/crcresult.h
  LINK tdeio-shared kcmlaptop-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
# crcresult.h header is included here cause it is
# generated during build time. Otherwice the 
# dependency on it won't be handeled correctly.

##### kded_klaptopdaemon (kpart) ################

tde_add_kpart( kded_klaptopdaemon AUTOMOC
  SOURCES laptop_daemon.cpp kpcmcia.cpp 
    xautolock.cc kpcmciainfo.cpp daemondock.cpp
    xautolock_diy.c xautolock_engine.c
    laptop_daemon.skel
  LINK tdeio-shared kcmlaptop-shared 
    tdeinit_kded-shared ${XTEST_LIBRARIES} 
    ${XSCREENSAVER_LIBRARIES} ${XEXT_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### crcresult.h (header) ######################
# FIXME: that should potentialy break the cross 
#        compilation, If it is possible in
#        generall.

add_executable( makecrc makecrc.cpp )
target_link_libraries( makecrc z )

get_target_property( MAKECRC_EXE makecrc LOCATION)

add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/crcresult.h
  COMMAND 
    ${MAKECRC_EXE} >${CMAKE_CURRENT_BINARY_DIR}/crcresult.h
  DEPENDS makecrc
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

