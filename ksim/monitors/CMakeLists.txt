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

add_subdirectory( filesystem )
add_subdirectory( net )
add_subdirectory( cpu )
add_subdirectory( disk )

if( WITH_I8K )
  add_subdirectory( i8k )
endif( WITH_I8K )

if( WITH_SNMP )
  add_subdirectory( snmp )
endif( WITH_SNMP )

if( WITH_SENSORS )
  add_subdirectory( lm_sensors )
endif( WITH_SENSORS )

