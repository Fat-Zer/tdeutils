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

add_subdirectory( pixmaps )

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


##### kedit (tdeinit) ###########################

tde_add_tdeinit_executable( kedit AUTOMOC
  SOURCES kedit.cpp ktextfiledlg.cpp misc.ui
    color.ui prefs.kcfgc
  LINK tdeprint-shared
)


##### other data ################################

install( FILES keditui.rc
  DESTINATION ${DATA_INSTALL_DIR}/kedit
)

install( FILES KEdit.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR}
)

install( FILES kedit.kcfg
  DESTINATION ${KCFG_INSTALL_DIR}
)

