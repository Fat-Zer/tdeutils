/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
void Misc::settingsWrapMode( int mode )
{
  bool state = ((mode == 2) ? true : false);
  kcfg_WrapColumn->setEnabled( state );
  wrapLabel->setEnabled( state );
}
