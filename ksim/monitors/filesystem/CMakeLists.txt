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
  ${CMAKE_SOURCE_DIR}/ksim/library
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${CMAKE_SOURCE_DIR}/ksim/library
)


##### ksim_filesystem (kpart) ###################

tde_add_kpart( ksim_filesystem AUTOMOC
  SOURCES ksimfsystem.cpp fsystemconfig.cpp
    fsystemiface.skel filesystemwidget.cpp
    filesystemstats.cpp
  LINK ksimcore-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### other data ################################

install( FILES DFree.desktop
  DESTINATION ${DATA_INSTALL_DIR}/ksim/monitors
)

