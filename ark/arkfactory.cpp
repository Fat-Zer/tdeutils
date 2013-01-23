/*
 ark -- archiver for the KDE project

 Copyright (C) 2003: Georg Robbers <georg.robbers@urz.uni-hd.de>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <kaboutdata.h>
#include <kinstance.h>

#include "ark_part.h"
#include "arkfactory.h"

TDEInstance* ArkFactory::s_instance = 0L;
TDEAboutData* ArkFactory::s_about = 0L;
int ArkFactory::instanceNumber = 0;

K_EXPORT_COMPONENT_FACTORY( libarkpart, ArkFactory )

ArkFactory::~ArkFactory()
{
    delete s_instance;
    delete s_about;
    s_instance = 0L;
}

KParts::Part * ArkFactory::createPartObject( TQWidget *parentWidget,
                  const char *widgetName, TQObject *parent,
                  const char *name, const char *classname,
                  const TQStringList &args )
{
    bool readWrite = false; // for e.g. Browser/View or KParts::ReadOnlyPart
    if ( TQCString( classname ) == "KParts::ReadWritePart" 
         || TQCString( classname ) == "ArkPart" )
    {
            readWrite = true;
    }
    ArkPart* obj = new ArkPart( parentWidget, widgetName, parent, name,
                                args, readWrite );
        //kdDebug( 1601 ) << "classname is: " << TQCString( classname ) << endl;
        return obj;
}

TDEInstance* ArkFactory::instance()
{
    instanceNumber++;
    if( !s_instance )
    {
        s_about = ArkPart::createAboutData();
        s_instance = new TDEInstance( s_about );
    }
    return s_instance;
}

