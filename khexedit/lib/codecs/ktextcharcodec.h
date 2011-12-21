/***************************************************************************
                          ktextcharcodec.h  -  description
                             -------------------
    begin                : Sa Nov 27 2004
    copyright            : (C) 2004 by Friedrich W. H. Kossebau
    email                : Friedrich.W.H@Kossebau.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License version 2 as published by the Free Software Foundation.       *
 *                                                                         *
 ***************************************************************************/


#ifndef KHE_KTEXTCHARCODEC_H
#define KHE_KTEXTCHARCODEC_H


#include "kcharcodec.h"

class TQTextCodec;
class TQTextDecoder;
class TQTextEncoder;

namespace KHE
{

// used by all codecs with full char coping, i.e. there are no undefined chars
class KTextCharCodec : public KCharCodec
{
  public:
    static KTextCharCodec *createCodec( const TQString &CodeName );
    static KTextCharCodec *createCodec( KEncoding C );
    static KTextCharCodec *createLocalCodec();

    static const TQStringList &codecNames();

  protected:
    KTextCharCodec( TQTextCodec *C );
  public:
    virtual ~KTextCharCodec();

  public: // KCharCodec API
    virtual bool encode( char *D, const TQChar &C ) const;
    virtual KHEChar decode( char Byte ) const;
    virtual const TQString& name() const;


  protected:
    TQTextCodec *Codec;
    /** decodes the chars to unicode */
    TQTextDecoder *Decoder;
    /** encodes the chars from unicode */
    TQTextEncoder *Encoder;
    /** */
    mutable TQString Name;

    static TQStringList CodecNames;
};

}

#endif
