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
  ${CMAKE_BINARY_DIR}/tdelirc/irkick
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/tdelirc/irkick
  ${CMAKE_SOURCE_DIR}/tdelirc/tdelirc
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${CMAKE_BINARY_DIR}/tdelirc/tdelirc
)


##### kcm_kcmlirc (kpart) #######################

tde_add_kpart( kcm_kcmlirc AUTOMOC
  SOURCES selectprofile.ui newmode.ui
    kcmlircbase.ui kcmlirc.cpp addactionbase.ui
    addaction.cpp kcmlirc.skel editactionbase.ui
    editmodebase.ui editmode.cpp editaction.cpp
    modeslist.cpp
  LINK tdelirc-static tdeui-shared tdeio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### other data ################################

install( FILES kcmlirc.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR}
)

