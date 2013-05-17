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

cmake_minimum_required( VERSION 2.8 )


##### general package setup #####################

project( tdeutils )

set( PACKAGE tdeutils )
set( VERSION R14 )


##### include essential cmake modules ###########

include( FindPkgConfig )
include( CheckIncludeFile )
include( CheckLibraryExists )
include( CheckSymbolExists )
include( CheckFunctionExists )
include( CheckCSourceCompiles )
include( CheckCXXSourceCompiles )
include( CheckTypeSize )


##### include our cmake modules #################

set( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" )
include( TDEMacros )


##### setup install paths #######################

include( TDESetupPaths )
tde_setup_paths( )


##### optional stuff ############################

option( WITH_DPMS "Enable Display Power Management Signaling support" OFF )
option( WITH_XSCREENSAVER "Enable xscreensaver support" OFF )
option( WITH_ASUS "Enable asus laptop support" OFF )
option( WITH_POWERBOOK "Enable powerbook laptop support" OFF )
option( WITH_POWERBOOK2 "Enable powerbook2 laptop support" OFF )
option( WITH_VAIO "Enable vaio laptop support" OFF )
option( WITH_THINKPAD "Enable thinkpad laptop support" OFF )
option( WITH_I8K "Enable dell laptop support" OFF )
option( WITH_SNMP "Enable SNMP support" OFF )
option( WITH_SENSORS "Enable lm_sensors support" OFF )
option( WITH_XMMS "Enable xmms support" OFF )
option( WITH_KNEWSTUFF "Enable knewstuff support" OFF )

# option( WITH_NV "Enable nv support" OFF )


##### options comments ##########################

# WITH_DPMS         affects klaptopdaemon
# WITH_DPMS         description enables the klaptopdaemon to take into account
#                               the state of DPMS X extention to determine is
#                               there any user activity.
# WITH_DPMS         requires libXext to build
# WITH_XSCREENSAVER affects klaptopdaemon
# WITH_XSCREENSAVER description this makes klaptopdaemon not to register user
#                               events when screensaver is disabled.
# WITH_ASUS         affects kmilo 
# WITH_POWERBOOK    affects kmilo
# WITH_POWERBOOK    requires pbbuttonsd
# WITH_POWERBOOK2   affects kmilo
# WITH_POWERBOOK2   requires pbbuttonsd
# WITH_VAIO         affects kmilo
# WITH_THINKPAD     affects kmilo
# WITH_I8K          affects kmilo ksim
# WITH_SNMP         affects ksim
# WITH_SENSORS      affects ksim
# WITH_XMMS         affects superkaramba
# WITH_XMMS         description enables supercaramba applets to gain 
#                               information about now playing track etc.
# WITH_KNEWSTUFF    affects superkaramba
# WITH_KNEWSTUFF    description enebles support for downloadable content
#                               from kdelooks.
# FIXME: test if WITH_KNEWSTUFF works in trinity or not

# NOTE: In addition to affects DPMS and XScreenSaver backend options for 
#       klaptopdaemon there is some XIdle code but it wasn't compleatly 
#       implemented and disabled permanently.

# NOTE: WITH_<laptop> options controls what kamilo modules will be build

# NOTE: WITH_{SENSORS,SNMP,I8K} options for ksim determin which ksim modules
#       will be build.

# NOTE: WITH_XMMS doesn't requires xmms2 palayer to be compiled.


##### user requested modules ####################

option( BUILD_ALL "Build all" OFF )
option( BUILD_DOC "Build documentation" ${BUILD_ALL} )
option( BUILD_ARK "Build ark" ${BUILD_ALL} )
option( BUILD_CHARSELECTAPPLET "Build charselectapplet" ${BUILD_ALL} )
option( BUILD_KCALC "Build kcalc" ${BUILD_ALL} )
option( BUILD_KCHARSELECT "Build kcharselect" ${BUILD_ALL} )
option( BUILD_TDELIRC "Build tdelirc" ${BUILD_ALL} )
option( BUILD_TDESSH "Build tdessh" ${BUILD_ALL} )
option( BUILD_KDF "Build kdf" ${BUILD_ALL} )
option( BUILD_KEDIT "Build kedit" ${BUILD_ALL} )
option( BUILD_KFLOPPY "Build kfloppy" ${BUILD_ALL} )
option( BUILD_KGPG "Build kgpg" ${BUILD_ALL} )
option( BUILD_KHEXEDIT "Build khexedit" ${BUILD_ALL} )
option( BUILD_KJOTS "Build kjots" ${BUILD_ALL} )
option( BUILD_KLAPTOPDAEMON "Build klaptopdaemon" ${BUILD_ALL} )
option( BUILD_KMILO "Build kmilo" ${BUILD_ALL} )
option( BUILD_KREGEXPEDITOR "Build kregexpeditor" ${BUILD_ALL} )
option( BUILD_KSIM "Build ksim" ${BUILD_ALL} )
option( BUILD_KTIMER "Build ktimer" ${BUILD_ALL} )
option( BUILD_KWALLET "Build tdewallet" ${BUILD_ALL} )

option( BUILD_SUPERKARAMBA "Build superkaramba" ${BUILD_ALL} )

##### add apidox targets ############

add_custom_target(apidox
  COMMAND "./generate_apidox" "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" "${HTML_INSTALL_DIR}" "/usr/share/qt3/doc/html"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/cmake/")

add_custom_target(install-apidox
  COMMAND "./install_apidox" "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" "${HTML_INSTALL_DIR}" "${CMAKE_INSTALL_PREFIX}"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/cmake/")

##### configure checks ##########################

include( ConfigureChecks.cmake )


###### global compiler settings #################

add_definitions(
  -DHAVE_CONFIG_H
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TQT_CXX_FLAGS}" )
set( CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined" )
set( CMAKE_MODULE_LINKER_FLAGS "-Wl,--no-undefined" )


##### tdeutils directories ######################

tde_conditional_add_subdirectory( BUILD_ARK ark )
tde_conditional_add_subdirectory( BUILD_DOC doc )
tde_conditional_add_subdirectory( BUILD_CHARSELECTAPPLET charselectapplet )
tde_conditional_add_subdirectory( BUILD_KCALC kcalc )
tde_conditional_add_subdirectory( BUILD_KCHARSELECT kcharselect )
tde_conditional_add_subdirectory( BUILD_TDELIRC tdelirc )
tde_conditional_add_subdirectory( BUILD_TDESSH tdessh )
tde_conditional_add_subdirectory( BUILD_KDF kdf )
tde_conditional_add_subdirectory( BUILD_KEDIT kedit )
tde_conditional_add_subdirectory( BUILD_KFLOPPY kfloppy )
tde_conditional_add_subdirectory( BUILD_KGPG kgpg )
tde_conditional_add_subdirectory( BUILD_KHEXEDIT khexedit )
tde_conditional_add_subdirectory( BUILD_KJOTS kjots )
tde_conditional_add_subdirectory( BUILD_KLAPTOPDAEMON klaptopdaemon )
tde_conditional_add_subdirectory( BUILD_KMILO kmilo )
tde_conditional_add_subdirectory( BUILD_KREGEXPEDITOR kregexpeditor )
tde_conditional_add_subdirectory( BUILD_KSIM ksim )
tde_conditional_add_subdirectory( BUILD_KTIMER ktimer )
tde_conditional_add_subdirectory( BUILD_KWALLET tdewallet )
tde_conditional_add_subdirectory( BUILD_SUPERKARAMBA superkaramba )


##### write configure files #####################

configure_file( config.h.cmake config.h @ONLY )
