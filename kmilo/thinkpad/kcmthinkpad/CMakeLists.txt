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
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

add_definitions(
  -DKDE_BINDIR="${BIN_INSTALL_DIR}"
)

##### kcm_thinkpad (kpart) ######################

tde_add_kpart( kcm_thinkpad AUTOMOC
  SOURCES kcmthinkpad_general.ui main.cpp
  LINK tdeio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### other data ################################

install( FILES thinkpad.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR}
)

