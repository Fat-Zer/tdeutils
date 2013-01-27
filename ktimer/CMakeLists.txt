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


##### ktimer (executable) #######################

tde_add_executable( ktimer AUTOMOC
  SOURCES main.cpp ktimer.cpp prefwidget.ui
  LINK tdeio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


##### icons #####################################

tde_install_icons( ktimer)


##### other data ################################

install( FILES ktimer.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR}
)

