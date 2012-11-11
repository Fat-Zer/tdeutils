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

add_subdirectory( host )
add_subdirectory( mail )

##### other data ################################

install( FILES decal_net_leds.png
  DESTINATION
    ${DATA_INSTALL_DIR}/ksim/themes/ksim/net
)

install( FILES bg_grid.png bg_panel.png 
    krell_meter.png krell_slider.png bg_chart.png 
    bg_meter.png gkrellmrc gkrellmrc_ksim 
    krell_panel.png frame_top.png 
    frame_bottom.png frame_left.png 
    frame_right.png
  DESTINATION
    ${DATA_INSTALL_DIR}/ksim/themes/ksim
)

