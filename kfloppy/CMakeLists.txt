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


##### kfloppy (executable) ######################

tde_add_executable( kfloppy AUTOMOC
  SOURCES main.cpp format.cpp floppy.cpp
  LINK tdeui-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


##### icons #####################################

tde_install_icons( kfloppy)


##### other data ################################

install( FILES KFloppy.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR}
)

install( FILES floppy_format.desktop
  DESTINATION
    ${DATA_INSTALL_DIR}/konqueror/servicemenus
)

