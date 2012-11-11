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
  ${CMAKE_SOURCE_DIR}/khexedit/lib
)


##### khexedit2part (kpart) #####################

tde_add_kpart( libkhexedit2part AUTOMOC
  SOURCES khepart.cpp khebrowserextension.cpp
    khepartfactory.cpp
  LINK khexeditcommon-shared kparts-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### other data ################################

install( FILES khexedit2part.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
)

install( FILES khexedit2partui.rc
  DESTINATION ${DATA_INSTALL_DIR}/khexedit2part
)

