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
add_subdirectory( konfigurator )

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


##### tdewalletmanager (executable) ###############

tde_add_executable( tdewalletmanager AUTOMOC
  SOURCES walletwidget.ui tdewalletmanager.cpp
    tdewalletmanager.skel main.cpp tdewalletpopup.cpp
    tdewalleteditor.cpp kwmapeditor.cpp
    allyourbase.cpp kbetterthankdialogbase.ui
  LINK kio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


##### icons #####################################

tde_install_icons( tdewalletmanager)


##### other data ################################

install( FILES tdewalletmanager.rc tdewalleteditor.rc
  DESTINATION ${DATA_INSTALL_DIR}/tdewalletmanager
)

install( FILES tdewalletmanager.desktop
  tdewalletmanager-tdewalletd.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR}
)

