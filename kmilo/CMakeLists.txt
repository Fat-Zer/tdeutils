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

add_subdirectory( kmilod )
add_subdirectory( generic )

if( WITH_ASUS )
  add_subdirectory( asus  )
endif( WITH_ASUS )

if( WITH_POWERBOOK AND WITH_POWERBOOK2 )
  tde_message_fatal( "WITH_POWERBOOK and WITH_POWERBOOK2 cannot be setted concurently" )
elseif( WITH_POWERBOOK )
  add_subdirectory( powerbook )
elseif( WITH_POWERBOOK2 )
  add_subdirectory( powerbook2 )
endif( )

if( WITH_VAIO )
  add_subdirectory( kmilo_kvaio )
endif( WITH_VAIO )

if( WITH_THINKPAD )
  add_subdirectory( thinkpad )
endif( WITH_THINKPAD )

if( WITH_I8K )
  add_subdirectory( delli8k )
endif( WITH_I8K )

