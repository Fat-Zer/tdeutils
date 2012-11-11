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

add_subdirectory( kcmthinkpad )

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
  ${CMAKE_BINARY_DIR}/kmilo/kmilod
)


##### kmilo_thinkpad (kpart) ####################

tde_add_kpart( kmilo_thinkpad AUTOMOC
  SOURCES thinkpad.cpp
  LINK kio-shared kmilo-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### other data ################################

install( FILES kmilo_thinkpad.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/kmilo
)

