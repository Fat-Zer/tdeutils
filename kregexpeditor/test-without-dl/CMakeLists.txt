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
  ${CMAKE_BINARY_DIR}/kregexpeditor 
)


##### regexptest (test) #########################

tde_add_executable( regexptest AUTOMOC
  SOURCES main.cpp
  DESTINATION ${BIN_INSTALL_DIR}
  LINK tdeui-shared kregexpeditorcommon-shared
)

add_test( NAME regexptest-test
  COMMAND regexptest)

