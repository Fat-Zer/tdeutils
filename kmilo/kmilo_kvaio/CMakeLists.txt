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

add_subdirectory( kcmkvaio )

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

##### kvaiodriverinterface (shared) #############

# this object is required by kcmvaio in subdirectory
tde_add_library( kvaiodriverinterface STATIC_PIC AUTOMOC
  SOURCES kvaiodriverinterface.cpp
)


##### kmilo_kvaio (kpart) #######################

tde_add_kpart( kmilo_kvaio AUTOMOC
  SOURCES kmilo_kvaio.cpp kvaio.cpp
  LINK tdeio-shared kmilo-shared 
    kvaiodriverinterface-static
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### other data ################################

install( FILES kmilo_kvaio.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/kmilo
)

