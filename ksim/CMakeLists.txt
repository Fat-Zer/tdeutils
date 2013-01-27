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

add_subdirectory( themes )
add_subdirectory( library )
add_subdirectory( monitors )
add_subdirectory( pics )

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/library
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}/library
)


##### systeminfo (static) #######################

tde_add_library( systeminfo STATIC_PIC AUTOMOC
  SOURCES systeminfo.cpp
)


##### ksim_panelextension (kpart) ###############

tde_add_kpart( ksim_panelextension AUTOMOC
  SOURCES ksim.cpp ksimview.cpp ksimview.skel
    ksimsysinfo.cpp ksimsysinfo.skel
    ksimframe.cpp ksimpref.cpp monitorprefs.cpp
    generalprefs.cpp themeprefs.cpp
  LINK tdeio-shared ksimcore-shared systeminfo-static
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### other data ################################

install( FILES ksim.desktop
  DESTINATION
    ${DATA_INSTALL_DIR}/kicker/extensions
)

install( FILES ksim_panelextensionrc
  DESTINATION ${CONFIG_INSTALL_DIR}
)

