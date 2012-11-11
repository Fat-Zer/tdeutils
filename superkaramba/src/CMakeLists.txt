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
  ${PYTHON_INCLUDE_DIRS}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### superkaramba (executable) #################

tde_add_executable( superkaramba AUTOMOC
  SOURCES main.cpp karamba.cpp meter.cpp bar.cpp
    sensor.cpp datesensor.cpp textlabel.cpp
    memsensor.cpp uptimesensor.cpp cpusensor.cpp
    networksensor.cpp imagelabel.cpp graph.cpp
    xmmssensor.cpp programsensor.cpp
    disksensor.cpp sensorparams.cpp
    sensorsensor.cpp textfilesensor.cpp
    clickarea.cpp noatunsensor.cpp
    karambarootpixmap.cpp clickmap.cpp
    rsssensor.cpp textfield.cpp taskmanager.cpp
    showdesktop.cpp richtextlabel.cpp
    karambasessionmanaged.cpp systemtray.cpp
    bar_python.cpp meter_python.cpp
    textlabel_python.cpp richtextlabel_python.cpp
    imagelabel_python.cpp config_python.cpp
    misc_python.cpp systray_python.cpp
    task_python.cpp widget_python.cpp
    menu_python.cpp karambalistboxitem.cpp
    graph_python.cpp dcopinterface.skel
    dcopinterface.stub karambainterface.cpp
    karambaapp.cpp karamba_python.cpp
    lineparser.cpp themefile.cpp themesdlg.cpp
    themes_layout.ui themewidget_layout.ui
    themewidget.cpp kwidgetlistbox.cpp
    sknewstuff.h sknewstuff.cpp
    superkarambasettings.kcfgc themelocale.cpp
    input.cpp sklineedit.cpp input_python.cpp
    svcgrp_python.cpp
  LINK kio-shared ${PYTHON_LIBRARIES} 
    ${KNEWSTUFF_LIBRARIES}
  DESTINATION ${BIN_INSTALL_DIR}
)


##### other data ################################

install( FILES superkarambaui.rc
  DESTINATION ${DATA_INSTALL_DIR}/superkaramba
)

install( FILES superkaramba.desktop
  DESTINATION ${APPS_INSTALL_DIR}/Utilities
)

