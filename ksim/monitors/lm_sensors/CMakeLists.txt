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
  ${CMAKE_SOURCE_DIR}/ksim/library
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${CMAKE_SOURCE_DIR}/ksim/library
)


##### ksim_sensors (kpart) ######################

#FIXME: make it optional but it requires some source code fixes
#      so it will be a separate patch
# if( WITH_NV )
   set(NVCTRL_SOURCES NVCtrl.c)
# endif( WITH_NV )

tde_add_kpart( ksim_sensors AUTOMOC
  SOURCES ksimsensors.cpp sensorsconfig.cpp
    sensorbase.cpp ksimsensorsiface.skel
    ${NVCTRL_SOURCES}
  LINK ksimcore-shared ${XEXT_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### other data ################################

install( FILES Lm_sensors.desktop
  DESTINATION ${DATA_INSTALL_DIR}/ksim/monitors
)

