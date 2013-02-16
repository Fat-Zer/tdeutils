/*
 *   khexedit - Versatile hex editor
 *   Copyright (C) 1999  Espen Sand, espensa@online.no
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <kdialog.h>
#include <tdelocale.h>

#include "hextoolwidget.h"
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <tqcheckbox.h>
#include <tqcombobox.h>


CHexToolWidget::CHexToolWidget( TQWidget *parent, const char *name )
  : TQFrame( parent, name )
{
  setFrameStyle( TQFrame::Panel | TQFrame::Raised );
  setLineWidth( 1 );

  TQString text;
  mUtilBox = new TQGridLayout( this, 5, 4, KDialog::marginHint(), KDialog::spacingHint() );
  mUtilBox->setColStretch( 3, 10 );

  TQString msg1[4] =
  {
    i18n("Signed 8 bit:"), i18n("Unsigned 8 bit:"),
    i18n("Signed 16 bit:"), i18n("Unsigned 16 bit:")
  };

  TQString msg2[4] =
  {
    i18n("Signed 32 bit:"), i18n("Unsigned 32 bit:"),
    i18n("32 bit float:"), i18n("64 bit float:")
  };

  TQString msg3[4] =
  {
    i18n("Hexadecimal:"), i18n("Octal:"),
    i18n("Binary:"), i18n("Text:")
  };

  TQGridLayout *ValuesBox = new TQGridLayout( this, 4, 6, 0, KDialog::spacingHint() );
  ValuesBox->setColStretch( 2, 10 );
  ValuesBox->setColStretch( 5, 10 );

  for( int i=0; i<4; i++ )
  {
    TQLabel *Label = new TQLabel( msg1[i], this );
    Label->setAlignment( AlignRight|AlignVCenter );
    ValuesBox->addWidget( Label, i, 0 );

    mText1[i] = new TQLineEdit( this );
    mText1[i]->setReadOnly( true );
    mText1[i]->setAlignment( AlignRight );
    ValuesBox->addWidget( mText1[i], i, 1 );

    Label = new TQLabel( msg2[i], this );
    Label->setAlignment( AlignRight|AlignVCenter );
    ValuesBox->addWidget( Label, i, 3 );

    mText2[i] = new TQLineEdit( this );
    mText2[i]->setReadOnly( true );
    mText2[i]->setAlignment( AlignRight );
    ValuesBox->addWidget( mText2[i], i, 4 );

    Label = new TQLabel( msg3[i], this );
    Label->setAlignment( AlignRight|AlignVCenter );
    mUtilBox->addWidget( Label, i, 1 );

    mText3[i] = new TQLineEdit( this );
    mText3[i]->setReadOnly( true );
    mText3[i]->setAlignment( AlignRight );
    mUtilBox->addWidget( mText3[i], i, 2 );
  }

  TQBoxLayout * SettingsBox = new TQHBoxLayout( this, 0, KDialog::spacingHint() );

  text = i18n("Show little endian decoding");
  mCheckIntelFormat = new TQCheckBox( text, this );
  mCheckIntelFormat->setMinimumSize( mCheckIntelFormat->sizeHint() );
  connect( mCheckIntelFormat, TQT_SIGNAL(clicked()), this, TQT_SLOT(intelFormat()) );
  SettingsBox->addWidget( mCheckIntelFormat, 0, AlignVCenter );
  mCheckIntelFormat->setChecked( // default value to please endian system users
#ifdef WORDS_BIGENDIAN
    false   // Big Endian: SUN, Motorola machines (amongst others)
#else
    true    // Little Endian: Intel, Alpha
#endif
  );
  // TODO: make this a pulldown box, adding PDP endianess

  text = i18n("Show unsigned as hexadecimal");
  mCheckHexadecimal = new TQCheckBox( text, this );
  mCheckHexadecimal->setMinimumSize( mCheckHexadecimal->sizeHint() );
  connect( mCheckHexadecimal, TQT_SIGNAL(clicked()), this, TQT_SLOT(unsignedFormat()) );
  SettingsBox->addWidget( mCheckHexadecimal, 0, AlignVCenter );

  mUtilBox->addMultiCellLayout( ValuesBox,   0, 3, 0, 0, AlignLeft|AlignVCenter );
  mUtilBox->addMultiCellLayout( SettingsBox, 4, 4, 0, 0, AlignLeft|AlignVCenter );

  //
  // Variable bitwidth. Based on Craig Graham's work.
  //
  TQLabel *bitLabel = new TQLabel( i18n("Stream length:"), this );
  bitLabel->setAlignment( AlignRight|AlignVCenter );
  mUtilBox->addWidget( bitLabel, 4, 1 );

  mBitCombo = new TQComboBox( false, this );
  text = i18n("Fixed 8 Bit" );
  mBitCombo->insertItem( text );
  for( int i=0; i<16; i++ )
  {
    text.sprintf("%u ", i+1 );
    text += i==0 ? i18n("Bit Window") : i18n("Bits Window");
    mBitCombo->insertItem( text );
  }
  mBitCombo->setMinimumSize( mBitCombo->sizeHint() );
  connect( mBitCombo, TQT_SIGNAL(activated(int)), TQT_SLOT(bitWidthChanged(int)));
  mUtilBox->addWidget( mBitCombo, 4, 2 );

	/* load font metrics */
	fontChanged();

  mUtilBox->activate();

  connect( kapp, TQT_SIGNAL( tdedisplayFontChanged() ),
	   TQT_SLOT( fontChanged() ) );

  mCursorState.valid = false;
  mViewHexCaps = true;

  setMinimumSize( sizeHint() );
  show();
}


CHexToolWidget::~CHexToolWidget( void )
{
}


void CHexToolWidget::writeConfiguration( TDEConfig &config )
{
  config.setGroup("Conversion" );
  config.writeEntry("LittleEndian",  mCheckIntelFormat->isChecked() );
  config.writeEntry("UnsignedAsHex", mCheckHexadecimal->isChecked() );
  config.writeEntry("StreamWindow", mBitCombo->currentItem() );
}

void CHexToolWidget::readConfiguration( TDEConfig &config )
{
  config.setGroup("Conversion" );
  bool s1  = config.readBoolEntry( "LittleEndian", true );
  bool s2  = config.readBoolEntry( "UnsignedAsHex", false );
  int  val = config.readNumEntry( "StreamWindow", 0 );

  mCheckIntelFormat->setChecked( s1 );
  mCheckHexadecimal->setChecked( s2 );
  mBitCombo->setCurrentItem( val );
}

//++cg[6/7/1999]: handler for change signal from bit width combo
void CHexToolWidget::bitWidthChanged( int /*i*/ )
{
  cursorChanged( mCursorState );
}


//
// Variable bitwidth. Based on Craig Graham's work.
//
// ++cg[6/7/1999]: Read n bit's from a bitstream (allows N length bit
// values to cross byte boundarys).
//
unsigned long CHexToolWidget::bitValue( SCursorState &state, int n )
{
  static const unsigned char bitmask[9] =
  {
    0, 1<<7, 3<<6, 7<<5, 15<<4, 31<<3, 63<<2, 127<<1, 255
  };

  unsigned long rtn = 0;
  unsigned char *byte = state.data;
  int bit = 7 - state.cell;

  while( n )
  {
    //
    // c hold's current byte, shifted to put remaining bits in
    // high bits of byte
    //
    unsigned char c = *byte << bit;

    //
    // if there are n bits or more remaining in this byte, we
    // swallow n bits, otherwise we swallow as many
    // bits as we can (8-bit)
    //
    int this_time = ((8-bit)>=n)?n:(8-bit);

    //
    // mask to get only the bit's we're swallowing
    //
    c &= bitmask[this_time];

    //
    // shift down to get bit's in low part of byte
    //
    c >>= 8-this_time;

    //
    // shift up previous results to make room and OR in the extracted bits.
    //
    rtn = (rtn<<this_time)|c;

    n   -= this_time;  // update the count of remaining bits
    bit += this_time;  // tell the stream we swallowed some swallowed bits

    //
    // if we've swallowed 8 bits, we zero the bit count and move on to
    // the next byte
    //
    if( bit==8 )
    {
      bit=0;
      byte++;
    }
  }

  return( rtn );
}


void CHexToolWidget::cursorChanged( SCursorState &state )
{
  if( state.valid == true )
  {
    TQString buf;
    // change by Kossebau[03.11.2003]:
    // checking for system endianess, using the compiler for the byte interpretation and cutting bloaded code
    // TODO: add PDP endianess
    void *P8Bit, *P16Bit, *P32Bit, *P64Bit;
    // ensure strict alignment for double as needed on some architectures (e.g. PA-RISC)
    typedef union { unsigned char b[8]; double d; } aligned_t;
    aligned_t Data;
    if(
#ifdef WORDS_BIGENDIAN
        !  // Assume Big Endian. This is the case for SUN machines (amongst others)
#else
           // Assume Little Endian. This is the case for the Intel architecture.
#endif
         mCheckIntelFormat->isChecked() )
    {
      // take it as it is
      memcpy( Data.b, state.data, 8 );
      P8Bit = P16Bit = P32Bit = P64Bit = Data.b;
    }
    else
    {
      // reverse order
      for( int i=0,j=7; i<8; ++i,--j )
        Data.b[i] = state.data[j];

      P8Bit =  &Data.b[7];
      P16Bit = &Data.b[6];
      P32Bit = &Data.b[4];
      P64Bit = Data.b;
    }

    bool NoHex = !mCheckHexadecimal->isChecked();

    // unsigned 8 bit
    buf.sprintf( NoHex?"%u":mViewHexCaps?"0x%02X":"0x%02x", *(unsigned char*)P8Bit );
    mText1[1]->setText( buf );
    // signed int 8 bit
    buf.sprintf( "%d", *(signed char*)P8Bit );
    mText1[0]->setText( buf );

    // unsigned int 16 bit
    buf.sprintf( NoHex?"%u":mViewHexCaps?"0x%04X":"0x%04x", *(unsigned short*)P16Bit );
    mText1[3]->setText( buf );
    // signed int 16 bit
    buf.sprintf( "%d", *(short*)P16Bit );
    mText1[2]->setText( buf );

    // unsigned int 32 bit
    buf.sprintf( NoHex?"%u":mViewHexCaps?"0x%08X":"0x%08x", *(unsigned int*)P32Bit );
    mText2[1]->setText( buf );
    // signed int 32 bit
    buf.sprintf( "%d", *(int*)P32Bit );
    mText2[0]->setText( buf );

    // float 32 bit
    buf.sprintf( "%E", *(float*)P32Bit );
    mText2[2]->setText( buf );
    // float 64 bit
    buf.sprintf( "%E", *(double*)P64Bit );
    mText2[3]->setText( buf );

    int numBits = mBitCombo->currentItem();
    if( numBits == 0 )
    {
      //
      // This is the original stuff
      //
      unsigned char data = (unsigned char)state.data[0];
      buf.sprintf( mViewHexCaps?"%02X":"%02x", data );
      mText3[0]->setText( buf );
      buf.sprintf( "%03o", data );
      mText3[1]->setText( buf );

      char bitBuf[32];
      for( int i = 0; i < 8; i++ )
        bitBuf[7-i] = (data&(1<<i)) ? '1' : '0';

      bitBuf[8] = 0;
      mText3[2]->setText( TQString(bitBuf) );
    }
    else
    {
      //
      // Variable bitwidth. Based on Craig Graham's work.
      //
      unsigned long data = bitValue( state, numBits );
      buf.sprintf( mViewHexCaps?"%02lX %02lX":"%02lx %02lx", (data>>8)&0xFF, data&0xFF );
      mText3[0]->setText( buf );
      buf.sprintf( "%03lo %03lo", (data>>8)&0xFF, data&0xFF );
      mText3[1]->setText( buf );
      char bitBuf[32];
      for( int i = 0; i<numBits; i++ )
        bitBuf[numBits-i-1] = (data&(1L<<i)) ? '1' : '0';
      bitBuf[numBits] = 0;
      mText3[2]->setText( TQString(bitBuf) );
    }

    // Fix by Sergey A. Sukiyazov
    unsigned char data[2] = { 0, 0 };
    data[0] = state.charValid == false ? '.' :
      (char)((unsigned char)state.data[0]&0xff );
    buf = TQString::fromLocal8Bit( (const char *)data );

    mText3[3]->setText( buf );
  }
  else
  {
    TQString str;
    for( int i=0; i<4; i++)
    {
      mText1[i]->setText( str );
      mText2[i]->setText( str );
      mText3[i]->setText( str );
    }
  }

  mCursorState = state;
}


void CHexToolWidget::fontChanged( void )
{
  TQFontMetrics fm( mText1[0]->font() );
  int W1 = fm.width( "XXXXXXXX" );
  int W2 = fm.width( "XXXXXXXXXXXX" );
  int W3 = fm.width( "888888888888888888" );
	for( int i=0; i<4; i++ )
  {
		mText1[i]->setFixedWidth( W1 );
		mText2[i]->setFixedWidth( W2 );
		mText3[i]->setFixedWidth( W3 );
  }
}


void CHexToolWidget::intelFormat( void )
{
  cursorChanged( mCursorState );
}


void CHexToolWidget::unsignedFormat( void )
{
  cursorChanged( mCursorState );
}


void CHexToolWidget::resizeEvent( TQResizeEvent */*e*/ )
{
}



void CHexToolWidget::closeEvent( TQCloseEvent *e )
{
  e->accept();
  emit closed();
}

#include "hextoolwidget.moc"
