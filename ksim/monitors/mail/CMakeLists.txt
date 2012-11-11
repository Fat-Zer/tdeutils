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


##### ksim_mail (kpart) #########################

tde_add_kpart( ksim_mail AUTOMOC
  SOURCES ksimmail.cpp
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### other data ################################

install( FILES mail.desktop
  DESTINATION ${DATA_INSTALL_DIR}/ksim/monitors
)

