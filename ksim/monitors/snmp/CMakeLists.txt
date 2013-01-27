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


##### ksim_snmp (kpart) #########################

tde_add_kpart( ksim_snmp AUTOMOC
  SOURCES plugin.cpp view.cpp configpage.cpp
    configwidget.ui snmp.cpp hostdialogbase.ui
    hostdialog.cpp value.cpp monitor.cpp
    monitordialogbase.ui monitordialog.cpp
    labelmonitor.cpp chartmonitor.cpp
    identifier.cpp monitorconfig.cpp
    hostconfig.cpp session.cpp probedialog.cpp
    snmplib.cpp pdu.cpp proberesultdialogbase.ui
    proberesultdialog.cpp browsedialogbase.ui
    browsedialog.cpp walker.cpp
  LINK tdeio-shared ksimcore-shared
    ${NETSNMP_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### other data ################################

install( FILES Snmp.desktop
  DESTINATION ${DATA_INSTALL_DIR}/ksim/monitors
)

