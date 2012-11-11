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


##### kcharselect_panelapplet (kpart) ###########

tde_add_kpart( kcharselect_panelapplet AUTOMOC
  SOURCES charselectapplet.cpp
  LINK tdeui-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### other data ################################

install( FILES kcharselectapplet.desktop
  DESTINATION ${DATA_INSTALL_DIR}/kicker/applets
)

