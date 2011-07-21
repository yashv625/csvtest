//---------------------------------------------------------------------------
// dmk_masked.cpp
//
// Output random strings based on charactrer masks.
// Available masks to be expanded considerably in later versions.
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_str.h"
#include "a_collect.h"
#include "dmk_source.h"
#include "dmk_tagdict.h"
#include "dmk_xmlutil.h"
#include "dmk_strings.h"
#include "dmk_random.h"
#include <cstring>

using std::string;
using std::vector;

namespace DMK {

//----------------------------------------------------------------------------

const char * const MASKED_TAG 		= "masked";
const char * const MASK_ATTRIB 		= "mask";

//----------------------------------------------------------------------------

class DSMasked : public DataSource {

	public:

		DSMasked( const FieldList & order, const string & mask );

		Row Get();
		int Size();
		void Reset() {}		// does nothing

		static DataSource * FromXML( const ALib::XMLElement * e );

	private:

		string mMask;
};

//static RegisterDS <DSMasked> regrs1_( MASKED_TAG );

DSMasked :: DSMasked( const FieldList & order, const string & mask )
	: DataSource( order ), mMask( mask ) {
}

// helper to produce random character from sequence of chars
static char RandomChar( const char * s ) {
	int n = std::strlen( s );
	return s[ RNG::Random() % n ];
}

// All mask decoding done from here
Row DSMasked :: Get() {
	string r;
	for ( unsigned int i = 0; i < mMask.size() ; i++ ) {
		char c = mMask[i];
		if ( c == '\\' && i < mMask.size() - 1 ) {
			r += mMask[++i];
		}
		else if ( c == 'A' ) {
			r += RandomChar( "ABCDEFGHIJKLMNOPQRSTUVWXYZ" );
		}
		else if ( c == 'a' ) {
			r += RandomChar( "abcdefghijklmnopqrstuvwxyz" );
		}
		else if ( c == '0' ) {
			r += RandomChar( "0123456789" );
		}
		else if ( c == '9' ) {
			r += RandomChar( "123456789" );
		}
		else {
			r += c;
		}
	}
	return Order( Row( r ) );
}

// Masks don't have size
int DSMasked :: Size() {
	return DMK_NOSIZE;
}

// Masks don't need to be validated as all characters do something
DataSource * DSMasked :: FromXML( const ALib::XMLElement * e ) {

	ForbidChildren( e );
	RequireAttrs( e, MASK_ATTRIB );
	AllowAttrs( e, AttrList( MASK_ATTRIB, ORDER_ATTRIB, 0 ) );

	string mask = e->AttrValue( MASK_ATTRIB );
	return new DSMasked( GetOrder( e ), mask );
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

#ifdef DMK_TEST

#include "a_myth.h"
using namespace ALib;
using namespace DMK;

DEFSUITE( "Masked" );

DEFTEST( FromXML1 ) {
	string xml = "<masked mask='AAA-99'/>";
	XMLPtr xp( xml );
	DSMasked * m = (DSMasked *) DSMasked::FromXML( xp );
	Row r = m->Get();
	FAILNE( r.Size(), 1 );
	FAILNE( r.At(0).size(), 6 );
}

#endif

//----------------------------------------------------------------------------

// end

