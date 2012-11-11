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
add_subdirectory( lib )
add_subdirectory( parts )

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


##### khexedit (executable) #####################

tde_add_executable( khexedit AUTOMOC
  SOURCES main.cc toplevel.cc hexmanagerwidget.cc
    hexeditorwidget.cc hextoolwidget.cc
    hexviewwidget.cc hexbuffer.cc conversion.cc
    dialog.cc hexvalidator.cc hexclipboard.cc
    optiondialog.cc printdialogpage.cc
    statusbarprogress.cc hexprinter.cc
    hexerror.cc draglabel.cc stringdialog.cc
    bitswapwidget.cc chartabledialog.cc
    hexdrag.cc exportdialog.cc fileinfodialog.cc
    converterdialog.cc listview.cc searchbar.cc
  LINK kio-shared tdeprint-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


##### icons #####################################

tde_install_icons( khexedit)


##### other data ################################

install( FILES khexeditui.rc
  DESTINATION ${DATA_INSTALL_DIR}/khexedit
)

install( FILES khexedit.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR}
)

