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
  ${CMAKE_SOURCE_DIR}/khexedit/lib
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${CMAKE_BINARY_DIR}/khexedit/lib
)


##### kbyteseditwidget (kpart) ##################

tde_add_kpart( libkbyteseditwidget AUTOMOC
  SOURCES kbyteseditwidget.cpp
  DESTINATION ${PLUGIN_INSTALL_DIR}
  LINK khexeditcommon-shared kparts-shared
)


##### other data ################################

install( FILES kbyteseditwidget.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
)

