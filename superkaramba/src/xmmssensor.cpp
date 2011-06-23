/***************************************************************************
*   Copyright (C) 2003 by Hans Karlsson                                   *
*   karlsson.h@home.se                                                      *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/
#include "xmmssensor.h"

#ifdef HAVE_XMMS
#include <tqlibrary.h>

class XMMSSensor::XMMS
{
public:
    XMMS() : libxmms( 0 )
    {
        libxmms = new TQLibrary( "xmms.so.1" );
        if ( !libxmms->load() )
        {
            delete libxmms;
            libxmms = 0;
        }

        if ( libxmms != 0 )
        {
            // resolve functions
            *(void**) (&xmms_remote_is_running) =
                    libxmms->resolve( "xmms_remote_is_running" );

            *(void**) (&xmms_remote_is_playing) =
                    libxmms->resolve( "xmms_remote_is_playing" );

            *(void**) (&xmms_remote_get_playlist_title) =
                    libxmms->resolve( "xmms_remote_get_playlist_title" );

            *(void**) (&xmms_remote_get_playlist_time) =
                    libxmms->resolve( "xmms_remote_get_playlist_time" );

            *(void**) (&xmms_remote_get_playlist_pos) =
                    libxmms->resolve( "xmms_remote_get_playlist_pos" );

            *(void**) (&xmms_remote_get_output_time) =
                    libxmms->resolve( "xmms_remote_get_output_time" );
        }
    }

    bool isInitialized() const
    {
        return libxmms != 0 &&
               xmms_remote_is_running != 0 &&
               xmms_remote_is_playing != 0 &&
               xmms_remote_get_playlist_title != 0 &&
               xmms_remote_get_playlist_time  != 0 &&
               xmms_remote_get_playlist_pos   != 0 &&
               xmms_remote_get_output_time    != 0;
    }

    bool isRunning(int session)
    {
        if ( !isInitialized() ) return false;

        return (*xmms_remote_is_running)(session);
    }

    bool isPlaying(int session)
    {
        if ( !isInitialized() ) return false;

        return (*xmms_remote_is_playing)(session);
    }

    char* getPlaylistTitle(int session, int pos)
    {
        if ( !isInitialized() ) return "";

        return (*xmms_remote_get_playlist_title)(session, pos);
    }

    int getPlaylistTime(int session, int pos)
    {
        if ( !isInitialized() ) return 0;

        return (*xmms_remote_get_playlist_time)(session, pos);
    }

    int getPlaylistPos(int session)
    {
        if ( !isInitialized() ) return 0;

        return (*xmms_remote_get_playlist_pos)(session);
    }

    int getOutputTime(int session)
    {
        if ( !isInitialized() ) return 0;

        return (*xmms_remote_get_output_time)(session);
    }

private:
    TQLibrary* libxmms;

    bool (*xmms_remote_is_running)(int);
    bool (*xmms_remote_is_playing)(int);

    char* (*xmms_remote_get_playlist_title)(int, int);
    int   (*xmms_remote_get_playlist_time)(int, int);
    int   (*xmms_remote_get_playlist_pos)(int);
    int   (*xmms_remote_get_output_time)(int);
};

#else // No XMMS

class XMMSSensor::XMMS
{
public:
    XMMS() {}

    bool isInitialized() const { return false; }
};
#endif // HAVE_XMMS


XMMSSensor::XMMSSensor( int interval, const TQString &encoding )
    : Sensor( interval ), xmms( 0 )
{
     if( !encoding.isEmpty() )
    {
        codec = TQTextCodec::codecForName( encoding.ascii() );
        if ( codec == 0)
            codec = TQTextCodec::codecForLocale();
    }
    else
        codec = TQTextCodec::codecForLocale();

    xmms = new XMMS();

}
XMMSSensor::~XMMSSensor()
{
    delete xmms;
}

void XMMSSensor::update()
{
    TQString format;
    SensorParams *sp;
    Meter *meter;
    TQObjectListIt it( *objList );

#ifdef HAVE_XMMS

    int pos;
    TQString title;
    int songLength = 0;
    int currentTime = 0;
    bool isPlaying = false;
    bool isRunning = xmms->isRunning(0);

    if( isRunning )
    {
        isPlaying = xmms->isPlaying(0);
        pos = xmms->getPlaylistPos(0);
        qDebug("tqunicode start");
        title = codec->toUnicode( TQCString( xmms->getPlaylistTitle( 0, pos ) )  );
        qDebug("tqunicode end");
        if( title.isEmpty() )
            title = "XMMS";

        qDebug("Title: %s", title.ascii());
        songLength = xmms->getPlaylistTime( 0, pos );
        currentTime = xmms->getOutputTime( 0 );
    }
#endif // HAVE_XMMS

    while (it != 0)
    {
        sp = (SensorParams*)(*it);
        meter = sp->getMeter();

#ifdef HAVE_XMMS

        if( isRunning )
        {

            format = sp->getParam("FORMAT");


            if (format.length() == 0 )
            {
                format = "%title %time / %length";
            }

            if( format == "%ms" )
            {
                meter->setMax( songLength );
                meter->setValue( currentTime );
            }
            else

                if ( format == "%full" )
                {
                    meter->setValue( 1 );
                }
                else

                {


                    format.tqreplace( TQRegExp("%title", false), title );

                    format.tqreplace( TQRegExp("%length", false), TQTime( 0,0,0 ).
                                    addMSecs( songLength )
                                    .toString( "h:mm:ss" ) );

                    format.tqreplace( TQRegExp("%time", false), TQTime( 0,0,0 ).
                                    addMSecs( currentTime )
                                    .toString( "h:mm:ss" ) );

                    if( isPlaying  )
                    {
                        format.tqreplace( TQRegExp("%remain", false), TQTime( 0,0,0 ).
                                        addMSecs( songLength )
                                        .addMSecs(-currentTime )
                                        .toString( "h:mm:ss" ) );
                    }

                    else
                    {
                        format.tqreplace( TQRegExp("%remain", false), TQTime( 0,0,0 ).toString("h:mm:ss" ) );
                    }
                    meter->setValue(format);
                }
        }
        else
#endif // HAVE_XMMS

        {
            meter->setValue("");
        }
        ++it;

    }

}

void XMMSSensor::setMaxValue( SensorParams *sp)
{
    Meter *meter;
    meter = sp->getMeter();
    TQString f;
    f = sp->getParam("FORMAT");

    if ( f == "%full" )
        meter->setMax( 1 );

}

bool XMMSSensor::hasXMMS() const
{
    return xmms->isInitialized();
}

#include "xmmssensor.moc"
