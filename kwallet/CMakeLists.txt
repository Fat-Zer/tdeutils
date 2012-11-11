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


##### kwalletmanager (executable) ###############

tde_add_executable( kwalletmanager AUTOMOC
  SOURCES walletwidget.ui kwalletmanager.cpp
    kwalletmanager.skel main.cpp kwalletpopup.cpp
    kwalleteditor.cpp kwmapeditor.cpp
    allyourbase.cpp kbetterthankdialogbase.ui
  LINK kio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


##### icons #####################################

tde_install_icons( kwalletmanager)


##### other data ################################

install( FILES kwalletmanager.rc kwalleteditor.rc
  DESTINATION ${DATA_INSTALL_DIR}/kwalletmanager
)

install( FILES kwalletmanager.desktop
  kwalletmanager-kwalletd.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR}
)

