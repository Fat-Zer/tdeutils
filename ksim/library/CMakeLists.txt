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

project( libksimcore )

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


##### ksimcore (shared) #########################

tde_add_library( ksimcore SHARED AUTOMOC
  SOURCES common.cpp themeloader.cpp chart.cpp
    label.cpp led.cpp progress.cpp
    pluginglobal.cpp pluginloader.cpp
    pluginmodule.cpp ksimconfig.cpp
  VERSION 1.0.0
  LINK tdeui-shared
  DESTINATION ${LIB_INSTALL_DIR}
)


##### headers ###################################

install( FILES chart.h common.h ksimconfig.h 
    label.h led.h pluginglobal.h pluginloader.h
    pluginmodule.h progress.h themeloader.h 
    themetypes.h 
  DESTINATION ${INCLUDE_INSTALL_DIR}/ksim )


##### install cmake export file #################
#
#  It's not raeally neccessary but I belive it
# can be usefull.
tde_install_export()

