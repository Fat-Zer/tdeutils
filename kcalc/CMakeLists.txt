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

add_subdirectory( knumber )

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/knumber
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}/knumber
)

add_definitions(
  -D_ISOC99_SOURCE
)


##### kcalc (tdeinit) ###########################

tde_add_tdeinit_executable( kcalc AUTOMOC
  SOURCES kcalc.cpp kcalc_button.cpp
    kcalc_const_button.cpp kcalc_const_menu.cpp
    kcalc_core.cpp kcalcdisplay.cpp dlabel.cpp
    stats.cpp colors.ui general.ui constants.ui
    kcalc_settings.kcfgc
  LINK tdeui-shared knumber-static
)


##### icons #####################################

tde_install_icons( kcalc)


##### other data ################################

install( FILES kcalcui.rc
  DESTINATION ${DATA_INSTALL_DIR}/kcalc
)

install( FILES kcalc.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR}
)

install( FILES kcalcrc.upd
  DESTINATION ${KCONF_UPDATE_INSTALL_DIR}
)

install( FILES kcalc.kcfg
  DESTINATION ${KCFG_INSTALL_DIR}
)

