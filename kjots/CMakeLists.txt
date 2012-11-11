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
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### kjots (executable) ########################

tde_add_executable( kjots AUTOMOC
  SOURCES main.cpp KJotsMain.cpp kjotsentry.cpp
    kjotsedit.cpp kjotsbookmarks.cpp
    confpagefont.ui confpagemisc.ui
    KJotsSettings.kcfgc
  LINK tdeprint-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


##### other data ################################

install( FILES kjotsui.rc
  DESTINATION ${DATA_INSTALL_DIR}/kjots
)

install( FILES Kjots.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR}
)

install( FILES kjots.kcfg
  DESTINATION ${KCFG_INSTALL_DIR}
)

