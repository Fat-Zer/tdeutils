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

add_subdirectory( icons )

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/tdelirc/tdelirc
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${CMAKE_BINARY_DIR}/tdelirc/tdelirc
)


##### irkick_common (static) ####################

tde_add_library( irkick_common STATIC_PIC AUTOMOC
  SOURCES irkick.cpp irkick.skel
    irkick.stub klircclient.cpp
  LINK tdeui-shared tdelirc-static
)


##### irkick (tdeinit) ##########################

tde_add_tdeinit_executable( irkick AUTOMOC
  SOURCES main.cpp
  LINK irkick_common-static
)


##### icons #####################################

tde_install_icons( )


##### other data ################################

install( FILES irkick.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR}
)

install( FILES irkick.desktop
  DESTINATION ${AUTOSTART_INSTALL_DIR}
)

