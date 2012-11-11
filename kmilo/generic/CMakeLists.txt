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
  ${CMAKE_SOURCE_DIR}/kmilo/kmilod
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${CMAKE_SOURCE_DIR}/kmilo/kmilod
)


##### kmilo_generic (kpart) #####################

tde_add_kpart( kmilo_generic AUTOMOC
  SOURCES generic_monitor.cpp
  LINK kio-shared kmilo-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### other data ################################

install( FILES kmilo_generic.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/kmilo
)

