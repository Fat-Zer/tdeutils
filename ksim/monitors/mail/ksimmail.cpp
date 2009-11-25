/*
   Copyright (c) 2002 Malte Starostik <malte@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
 
   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// $Id$

#include <qbitmap.h>
#include <qlayout.h>
#include <qtimer.h>

#include <kaboutapplication.h>
#include <kdebug.h>
#include <klocale.h>

#include <label.h>
#include <themeloader.h>
#include <themetypes.h>

#include "ksimmail.moc"

KSIM_INIT_PLUGIN( MailPlugin );

MailPlugin::MailPlugin( const char* name )
	: KSim::PluginObject( name )
{
  setConfigFileName(instanceName());
}

MailPlugin::~MailPlugin()
{
}

KSim::PluginView* MailPlugin::createView( const char* name )
{
	return new MailView( this, name );
}

KSim::PluginPage* MailPlugin::createConfigPage( const char* name )
{
	return new MailConfig( this, name );
}

void MailPlugin::showAbout()
{
	KAboutData about( instanceName(),
	                  I18N_NOOP( "KSim Mail Plugin" ), "0.1",
	                  I18N_NOOP( "A mail monitor plugin for KSim" ),
	                  KAboutData::License_GPL, "(c) 2002 Malte Starostik" );
	about.addAuthor( "Malte Starostik", I18N_NOOP( "Author" ), "malte@kde.org" );

	KAboutApplication( &about ).exec();
}

MailView::MailView( KSim::PluginObject* parent, const char* name )
	: KSim::PluginView( parent, name )
{
	QVBoxLayout* layout = new QVBoxLayout( this );

	MailLabel* label = new MailLabel( this );
	layout->addWidget( label, 0, AlignHCenter );
}

MailView::~MailView()
{
}

void MailView::reparseConfig()
{
}

void MailView::updateDisplay()
{
}

MailLabel::MailLabel( QWidget* parent )
	: KSim::Label( KSim::Types::Mail, parent )
{
//	label->setPixmap( KSim::ThemeLoader::self().current().krellMail() );
	configureObject( false );
	QTimer* timer = new QTimer( this );
	connect( timer, SIGNAL( timeout() ), SLOT( animation() ) );
	timer->start( 100 );
}

MailLabel::~MailLabel()
{
}

void MailLabel::configureObject( bool repaint )
{
	m_envelope.load( themeLoader().current().mailPixmap() );
	m_frames = themeLoader().current().mailFrames();
	m_delay = themeLoader().current().mailDelay();

	if ( !m_frames ) m_frames = 18;
	if ( !m_delay ) m_delay = 1;

	setPixmap( frame( m_envelope, 1 ) );

	KSim::Label::configureObject( repaint );
}

void MailLabel::paintEvent( QPaintEvent* e )
{
	KSim::Label::paintEvent( e );
}

void MailLabel::animation()
{
	static int f = 1;
	setPixmap( frame( m_envelope, f ) );
	if ( f++ >= m_frames ) f = 1;
}

QPixmap MailLabel::frame( const QPixmap& source, int number ) const
{
	QPixmap result( source.width(), source.height() / m_frames );
	bitBlt( &result, 0, 0, &source, 0, number * source.height() / m_frames );
	if ( source.mask() )
	{
		QBitmap mask( result.size() );
		bitBlt( &mask, 0, 0, source.mask(), 0, number * source.height() / m_frames );
		result.setMask( mask );
	}
	return result;
}

MailConfig::MailConfig( KSim::PluginObject* parent, const char* name )
	: KSim::PluginPage( parent, name )
{
}

MailConfig::~MailConfig()
{
}

void MailConfig::saveConfig()
{
}

void MailConfig::readConfig()
{
}

// vim: ts=4 sw=4 noet
