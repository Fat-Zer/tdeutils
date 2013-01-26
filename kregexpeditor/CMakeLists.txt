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

add_subdirectory( KWidgetStreamer )
add_subdirectory( KMultiFormListBox )
add_subdirectory( picts )
add_subdirectory( predefined )

# FIXME: add optional test compilation
# add_subdirectory( test-without-dl )

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/KWidgetStreamer
  ${CMAKE_CURRENT_SOURCE_DIR}/KMultiFormListBox 
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}/KWidgetStreamer
)

add_definitions(
  -DQT_NO_CAST_ASCII
)


##### kregexpeditor (executable) ################

tde_add_executable( kregexpeditor AUTOMOC
  SOURCES main.cpp
  LINK kio-shared
  DESTINATION ${BIN_INSTALL_DIR}
  LINK kregexpeditorcommon-shared
)



##### kregexpeditorcommon (shared) ##############

tde_add_library( kregexpeditorcommon SHARED AUTOMOC
  SOURCES kregexpeditorgui.cpp editorwindow.cpp
    regexpwidget.cpp drag.cpp concwidget.cpp
    zerowidgets.cpp dragaccepter.cpp
    repeatwidget.cpp characterswidget.cpp
    altnwidget.cpp limitedcharlineedit.cpp
    textregexp.cpp textrangeregexp.cpp
    concregexp.cpp altnregexp.cpp
    positionregexp.cpp dotregexp.cpp
    repeatregexp.cpp textwidget.cpp
    selectablelineedit.cpp
    multicontainerwidget.cpp
    scrollededitorwindow.cpp compoundwidget.cpp
    singlecontainerwidget.cpp widgetfactory.cpp
    dcbutton.cpp gen_qregexplexer.cpp
    gen_qregexpparser.cc charselector.cpp
    lookaheadregexp.cpp lookaheadwidget.cpp
    regexp.cpp infopage.cpp
    kregexpeditorprivate.cpp compoundregexp.cpp
    auxbuttons.cpp myfontmetrics.cpp
    regexpbuttons.cpp userdefinedregexps.cpp
    verifier.cpp verifybuttons.cpp errormap.cpp
    qtregexphighlighter.cpp regexpconverter.cpp
    qtregexpconverter.cpp
    emacsregexpconverter.cpp
    regexphighlighter.cpp util.cpp
  VERSION 1.0.0
  LINK kio-shared tdemultiformlistbox-static
    kwidgetstreamer-static
  DESTINATION ${LIB_INSTALL_DIR}
)



##### kregexpeditorgui (kpart) ##################

tde_add_kpart( libkregexpeditorgui AUTOMOC
  SOURCES kregexpeditorfactory.cpp
  DESTINATION ${PLUGIN_INSTALL_DIR}
  LINK kregexpeditorcommon-shared
)


##### icons #####################################

tde_install_icons( )


##### other data ################################

install( FILES kregexpeditor.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR}
)

install( FILES kregexpeditorgui.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
)

