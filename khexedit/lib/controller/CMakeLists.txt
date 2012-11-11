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
)


##### kcontroller (static) ######################

tde_add_library( kcontroller STATIC_PIC AUTOMOC
  SOURCES kcontroller.cpp ktabcontroller.cpp
    knavigator.cpp keditor.cpp kvalueeditor.cpp
    kchareditor.cpp
)

