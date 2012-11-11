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
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### kcm_kwallet (kpart) #######################

tde_add_kpart( kcm_kwallet AUTOMOC
  SOURCES walletconfigwidget.ui konfigurator.cpp
  LINK tdeui-shared kwalletclient-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### other data ################################

install( FILES kwalletconfig.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR}
)

