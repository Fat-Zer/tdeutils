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

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/kmilo/kmilo-vaio
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${CMAKE_BINARY_DIR}/kmilo/kmilo-vaio
)


##### kcm_kvaio (kpart) #########################

tde_add_kpart( kcm_kvaio AUTOMOC
  SOURCES kcmkvaio_general.ui main.cpp
  LINK tdeui-shared kvaiodriverinterface-static
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### other data ################################

install( FILES kvaio.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR}
)

