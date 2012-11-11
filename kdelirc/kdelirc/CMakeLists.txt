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


##### kdelirc (static) ##########################

tde_add_library( kdelirc STATIC_PIC
  SOURCES iraction.cpp arguments.cpp
    iractions.cpp prototype.cpp modes.cpp
    mode.cpp profileserver.cpp remoteserver.cpp
)

