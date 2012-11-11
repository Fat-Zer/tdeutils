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
  ${CMAKE_SOURCE_DIR}/kregexpeditor/KMultiFormListBox 
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

add_definitions(
  -DQT_NO_CAST_ASCII
)


##### kwidgetstreamer (static) ##################

tde_add_library( kwidgetstreamer STATIC_PIC AUTOMOC
  SOURCES kwidgetstreamer.cpp
)


##### other data ################################

install(
  FILES
    kwidgetstreamer.h
  DESTINATION ${INCLUDE_INSTALL_DIR}
)
