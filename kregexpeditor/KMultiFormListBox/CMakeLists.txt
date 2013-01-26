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


##### tdemultiformlistbox (static) ################

tde_add_library( tdemultiformlistbox STATIC_PIC AUTOMOC
  SOURCES ccp.cpp tdemultiformlistboxentry.cpp
    tdemultiformlistbox.cpp
    tdemultiformlistbox-multivisible.cpp
    tdemultiformlistboxfactory.cpp indexWindow.cpp
    tdemultiformlistbox-windowed.cpp
    widgetwindow.cpp windowlistboxitem.cpp
)


##### other data ################################

install(
  FILES
    tdemultiformlistbox.h
    tdemultiformlistboxentry.h
    tdemultiformlistboxfactory.h
  DESTINATION ${INCLUDE_INSTALL_DIR}
)
