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

add_subdirectory( icons )

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


##### kgpg (executable) #########################

tde_add_executable( kgpg AUTOMOC
  SOURCES kgpgoptions.cpp listkeys.cpp
    popuppublic.cpp kgpgview.cpp kgpg.cpp
    main.cpp keygener.cpp kgpginterface.cpp
    keyservers.cpp keyserver.ui searchres.ui
    kgpgeditor.cpp detailedconsole.cpp
    kgpglibrary.cpp kgpgwizard.ui
    keyproperties.ui conf_encryption.ui
    conf_decryption.ui conf_gpg.ui dcopiface.skel
    keyinfowidget.cpp groupedit.ui
    kgpgrevokewidget.ui newkey.ui adduid.ui
    keyexport.ui sourceselect.ui
    kgpgsettings.kcfgc conf_servers.ui
    conf_ui2.ui conf_misc.ui
  LINK kabc-shared kutils-shared tdeprint-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


##### icons #####################################

tde_install_icons( kgpg )


##### other data ################################

install( FILES kgpg_anim.gif kgpg_docked.png
    kgpg_docked.gif kgpg_blank.png kgpg_fill.png
  DESTINATION ${DATA_INSTALL_DIR}/kgpg/pics
)

install( FILES kgpg.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR}
)

install( FILES kgpg.desktop
  DESTINATION ${AUTOSTART_INSTALL_DIR}
)

install( FILES kgpg.rc listkeys.rc tips
  DESTINATION ${DATA_INSTALL_DIR}/kgpg
)

install( FILES encryptfile.desktop
  encryptfolder.desktop
  DESTINATION
    ${DATA_INSTALL_DIR}/konqueror/servicemenus
)

install( FILES kgpg.kcfg
  DESTINATION ${KCFG_INSTALL_DIR}
)

