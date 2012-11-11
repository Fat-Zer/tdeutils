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
  -DQT_NO_CAST_ASCII
)


##### kdessh (executable) #######################

tde_add_executable( kdessh AUTOMOC
  SOURCES kdessh.cpp sshdlg.cpp
  LINK tdeui-shared tdesu-shared
  DESTINATION ${BIN_INSTALL_DIR}
)

