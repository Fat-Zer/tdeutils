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

add_subdirectory( controller )
add_subdirectory( codecs )

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}/controller
  ${CMAKE_CURRENT_BINARY_DIR}/codecs
)


##### khexeditcommon (shared) ###################

tde_add_library( khexeditcommon SHARED AUTOMOC
  SOURCES kcolumn.cpp kbordercolumn.cpp
    koffsetcolumn.cpp kbuffercolumn.cpp
    kvaluecolumn.cpp kcharcolumn.cpp
    kcolumnsview.cpp khexedit.cpp kbytesedit.cpp
    koffsetformat.cpp kdatabuffer.cpp
    kwrappingrobuffer.cpp kplainbuffer.cpp
    kfixedsizebuffer.cpp kbigbuffer.cpp
    kbuffercursor.cpp kbufferlayout.cpp
    kbufferranges.cpp kcursor.cpp kbufferdrag.cpp
    kwordbufferservice.cpp ksectionlist.cpp
    kcoordrangelist.cpp kbordercoltextexport.cpp
    koffsetcoltextexport.cpp
    kbuffercoltextexport.cpp
    kvaluecoltextexport.cpp
    kcharcoltextexport.cpp
  LINK tdecore-shared kcontroller-static
    khecodecs-static
  VERSION 0.0.0
  DESTINATION ${LIB_INSTALL_DIR}
)

