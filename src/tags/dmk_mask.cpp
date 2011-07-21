//----------------------------------------------------------------------------
// dmk_mask.cpp
//
// Masked output
//
// Copyright (C) 2009 Neil Butterworth
//----------------------------------------------------------------------------

#include "a_base.h"
#include "a_str.h"
#include "a_collect.h"
#include "a_chsrc.h"
#include "dmk_source.h"
#include "dmk_tagdict.h"
#include "dmk_xmlutil.h"
#include "dmk_strings.h"
#include "dmk_random.h"

//----------------------------------------------------------------------------

const char * const MASKED_TAG 		= "mask";
const char * const MASK_ATTRIB 	= "value";

using std::string;
using std::vector;

namespace DMK {

class DSMask : public DataSource {

	public:

		DSMask( const string & mask );

		Row Get();
		int Size();
		void Reset() {}		// does nothing

		static DataSource * FromXML( const ALib::XMLElement * e );

	private:

		std::string GenChars( unsigned int  i ) const;
		void Encode();

		std::string mMask;

		// encoded mask character
		struct EncodedMaskChar {
			unsigned int mMin, mMax;
			bool mOptional;
			std::string mCharset;
			EncodedMaskChar( unsigned int mn,
					  unsigned int mx,
					  bool opt,
					  const std::string & cset )
				: mMin( mn ), mMax( mx ), mOptional( opt ), mCharset( cset ) {}
		};

		std::vector <EncodedMaskChar> mChars;
};

//----------------------------------------------------------------------------

static RegisterDS <DSMask> regrs1_( MASKED_TAG );

//---------------------------------------------------------------------------
// Mask special characters
//---------------------------------------------------------------------------

const char RANGE_INTRO		= '[';	// begin range
const char RANGE_OUTRO		= ']';	// end range
const char RANGE_DASH		= '-';	// dash in range
const char REPEAT			= '*';	// repeat N times
const char REPEAT_OPT		= '?';	// repeat 0 - N times
const char REPEAT_DASH		= ':';	// repeat N1 - N2 times

//---------------------------------------------------------------------------
// Create from mask
//---------------------------------------------------------------------------

DSMask :: DSMask(  const string & mask )
	: DataSource( FieldList()  ), mMask( mask ) {
	Encode();
}

//---------------------------------------------------------------------------
// Get random string generated from mask
//---------------------------------------------------------------------------

Row DSMask :: Get() {
	string rv;
	for ( unsigned int i = 0; i < mChars.size(); i++ ) {
		rv += GenChars( i );
	}
	return Row( rv );
}

//---------------------------------------------------------------------------
// Masks are always randomised and so don't have size
//---------------------------------------------------------------------------

int DSMask  :: Size() {
	return 1;
}


//---------------------------------------------------------------------------
// Perform value generation. We need to randomise the character being
// generated and (optionally) the number of chracters.
//---------------------------------------------------------------------------

string DSMask :: GenChars( unsigned int  i ) const {
	string rv;
	EncodedMaskChar m = mChars[i];

	unsigned int count;
	if ( m.mMax == m.mMin ) {		// fixed count
		count = m.mMin;
	}
	else {							// randomised count
		count = m.mMin + RNG::Random( 0,  1 + m.mMax - m.mMin );
	}

	for ( unsigned int n = 0; n < count; n++ ) {
		if ( ! m.mOptional  || RNG::Random( 0, 2 ) ) {
			unsigned int r = RNG::Random( 0, m.mCharset.size() );
			rv += m.mCharset[r];	// single random char
		}
	}
	return rv;
}

//---------------------------------------------------------------------------
// Expand begin/end of range into full sequence of characters
//---------------------------------------------------------------------------

static string ExpandRange( char begin, char end ) {
	if ( begin >= end ) {
		throw Exception( "invalid range " );
	}
	string s;
	while( begin <= end ) {
		s += begin;
		begin++;
	}
	return s;
}

//---------------------------------------------------------------------------
// Generate range of characters from expression like 0-9. Expression is
// terminated by RANGE_OUTRO. Generated range is just a sequence of
// characters so 0-9 producws "0123456789".
//---------------------------------------------------------------------------

static string CreateRange( ALib::CharSource & src ) {
	string r;
	ALib::CharSource::Char c;
	while( src.Next( c ) ) {
		if ( c.Value() == RANGE_OUTRO && ! c.Escaped() ) {
			return r;
		}
		else if ( c.Value() != RANGE_DASH ) {
			char c1 = c.Value();
			if ( src.Peek( c ) && c.Value() == RANGE_DASH && ! c.Escaped() ) {
				src.Next( c );		// skip -
				if ( ! src.Next( c ) ) {
					break;	// unexpected end of input
				}
				r += ExpandRange( c1, c.Value() );
			}
			else {
				r += c1;
			}
		}
	}
	throw Exception( "Invalid range" );
}

//---------------------------------------------------------------------------
// Read integer from char source or throw.
// This should be method of CharSource?
//---------------------------------------------------------------------------

static unsigned int MustReadNumber( ALib::CharSource & src ) {
	string s;
	ALib::CharSource::Char c;
	while( src.Peek( c ) ) {
		if ( isdigit( c.Value() ) && ! c.Escaped() ) {
			s += c.Value();
			src.Next( c );
		}
		else {
			break;
		}
	}
	int n;
	if ( s == "" || (n = atoi( s.c_str() )) < 0 ) {
		throw Exception( "Invalid character count" );
	}
	return n;
}

//---------------------------------------------------------------------------
// Read a character count, which may be a single number or a pair of numbers
// (min & max) separated by a colon
//---------------------------------------------------------------------------

static void ReadNumber( ALib::CharSource & src, unsigned int & min,
											unsigned int & max ) {
	string s;
	ALib::CharSource::Char c;
	src.Next( c );		// skip '*';
	max = min = MustReadNumber( src );
	src.Peek( c );
	if ( c.Value() == REPEAT_DASH && ! c.Escaped() ) {
		src.Next( c );
		max = MustReadNumber( src );
	}
	if ( min > max || (min == 0 && max == 0) ) {
		throw Exception( "Invalid character counr" );
	}
}

//---------------------------------------------------------------------------
// Encode mask for later generation. Encoding contains expanded range to
// pick chars from and minimum/maximum values for number of chars to pick.
//---------------------------------------------------------------------------

void DSMask :: Encode() {
	ALib::CharSource src( mMask );
	ALib::CharSource::Char c;
	string cset;
	while( src.Next( c ) ) {
		if ( c.Value() == RANGE_INTRO && ! c.Escaped() ) {
			cset = CreateRange( src );
		}
		else {
			cset = c.Value();
		}
		unsigned int min = 1, max = 1;
		bool optional = false;
		if ( src.Peek( c )
				&& (c.Value() == REPEAT || c.Value() == REPEAT_OPT )
				&& ! c.Escaped() ) {
			optional = c.Value() == REPEAT_OPT;
			ReadNumber( src, min, max );
		}
		mChars.push_back( EncodedMaskChar( min, max, optional, cset ) );
	}
}

DataSource * DSMask :: FromXML( const ALib::XMLElement * e ) {

	ForbidChildren( e );
	RequireAttrs( e, MASK_ATTRIB );
	AllowAttrs( e, AttrList( MASK_ATTRIB, 0 ) );

	string mask = e->AttrValue( MASK_ATTRIB );
	return new DSMask( mask );
}


} // namespace
