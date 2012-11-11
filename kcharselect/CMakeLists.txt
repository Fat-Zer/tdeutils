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


##### kcharselect (executable) ##################

tde_add_executable( kcharselect AUTOMOC
  SOURCES kcharselectdia.cc main.cc
  LINK tdeui-shared
  DESTINATION ${BIN_INSTALL_DIR} 
)


##### icons #####################################

tde_install_icons( kcharselect)


##### other data ################################

install( FILES kcharselectui.rc
  DESTINATION ${DATA_INSTALL_DIR}/kcharselect
)

install( FILES KCharSelect.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR}
)

install( FILES kcharselect.upd
  DESTINATION ${KCONF_UPDATE_INSTALL_DIR}
)

