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
  ${CMAKE_SOURCE_DIR}/kregexpeditor/KWidgetStreamer
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

add_definitions(
  -DQT_NO_CAST_ASCII
)


##### kmultiformlistbox (static) ################

tde_add_library( kmultiformlistbox STATIC_PIC AUTOMOC
  SOURCES ccp.cpp kmultiformlistboxentry.cpp
    kmultiformlistbox.cpp
    kmultiformlistbox-multivisible.cpp
    kmultiformlistboxfactory.cpp indexWindow.cpp
    kmultiformlistbox-windowed.cpp
    widgetwindow.cpp windowlistboxitem.cpp
)

