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

# FIXME: add optional testing support
# add_subdirectory( tests )

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

add_definitions(
  -D_GNU_SOURCE
  -D_ISOC99_SOURCE
)


##### knumber (static) ##########################

tde_add_library( knumber STATIC_PIC
  SOURCES knumber.cpp knumber_priv.cpp
  LINK ${GMP_LIBRARY}
)

