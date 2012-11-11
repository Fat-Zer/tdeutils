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

add_definitions(
  -D_LARGEFILE64_SOURCE
)


##### ark_common (static) #######################

tde_add_library( ark_common STATIC_PIC
  SOURCES settings.kcfgc archiveformatinfo.cpp
)


##### ark (tdeinit) #############################

tde_add_tdeinit_executable( ark AUTOMOC
  SOURCES main.cpp arkapp.cpp mainwindow.cpp
  LINK ark_common-static khtml-shared
)


##### arkpart (kpart) ###########################

tde_add_kpart( libarkpart AUTOMOC
  SOURCES ark_part.cpp arkfactory.cpp zip.cpp
    tar.cpp filelistview.cpp arch.cpp lha.cpp
    compressedfile.cpp zoo.cpp rar.cpp ar.cpp
    arkutils.cpp archiveformatdlg.cpp
    arkwidget.cpp searchbar.cpp addition.ui
    extraction.ui general.ui arkviewer.cpp
    sevenzip.cpp extractiondialog.cpp ace.cpp
    tarlistingthread.cpp arj.cpp
  LINK ark_common-static khtml-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### icons #####################################

tde_install_icons( ark )


##### other data ################################

install( FILES arkui.rc ark_part.rc
  ark_part_readonly.rc
  DESTINATION ${DATA_INSTALL_DIR}/ark
)

install( FILES ark.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR}
)

install( FILES ark_part.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
)

install( FILES ark.kcfg
  DESTINATION ${KCFG_INSTALL_DIR}
)

