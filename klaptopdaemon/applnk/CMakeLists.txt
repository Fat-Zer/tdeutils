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

install( FILES laptop.desktop pcmcia.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR}
)

install( FILES klaptopdaemon.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/kded
)

