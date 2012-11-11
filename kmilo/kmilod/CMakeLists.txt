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


##### kmilo (shared) ############################

tde_add_library( kmilo SHARED
  SOURCES monitor.cpp displayskin.cpp
  VERSION 1.0.0
  LINK tdecore-shared
  DESTINATION ${LIB_INSTALL_DIR}
)


##### kded_kmilod (kpart) #######################

tde_add_kpart( kded_kmilod AUTOMOC
  SOURCES kmilod.cpp kmilod.skel defaultskin.cpp
    defaultwidget.ui kmilointerface.cpp
  LINK kio-shared kmilo-shared tdeinit_kded-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### other data ################################

install( FILES kmilod.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/kded
)

install( FILES kmilopluginsvc.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR}/kmilo
)

