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

add_subdirectory( pics )

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


##### kdf_common (static) #######################

tde_add_library( kdf_common STATIC_PIC AUTOMOC
  SOURCES kdfwidget.cpp kdfconfig.cpp
    mntconfig.cpp distdelist.cpp disks.cpp
    listview.cpp optiondialog.cpp stdoption.cpp
  LINK tdeui-shared tdeio-shared
)


##### kdf (executable) ##########################

tde_add_executable( kdf AUTOMOC
  SOURCES kdf.cpp
  LINK kdf_common-static 
  DESTINATION ${BIN_INSTALL_DIR}
)


##### kwikdisk (executable) #####################

tde_add_executable( kwikdisk AUTOMOC
  SOURCES kwikdisk.cpp
  LINK kdf_common-static
  DESTINATION ${BIN_INSTALL_DIR}
)


##### kcm_kdf (kpart) ###########################

tde_add_kpart( kcm_kdf AUTOMOC
  SOURCES kcmdf.cpp
  LINK kdf_common-static
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### other data ################################

install( FILES kdfui.rc
  DESTINATION ${DATA_INSTALL_DIR}/kdf
)

install( FILES kdf.desktop kwikdisk.desktop
  kcmdf.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR}
)

