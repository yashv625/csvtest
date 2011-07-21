//---------------------------------------------------------------------------
// dmk_pick.cpp
//
// pick from data sources alternate or randomly
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

using std::string;
using std::vector;

namespace DMK {

//----------------------------------------------------------------------------

const char * const PICK_TAG 		= "pick";
const char * const DIST_ATTR		= "distribute";

//----------------------------------------------------------------------------

class DSPick : public CompositeDataSource {

	public:

		DSPick( const FieldList & order, bool rand,
					const vector <int> & dist  );

		Row Get();
		int Size();

		static DataSource * FromXML( const ALib::XMLElement * e );

	private:

		bool mRand;
		int mPos;
		vector <int> mDistrib;
};

//----------------------------------------------------------------------------
// Tag registration
//----------------------------------------------------------------------------

static RegisterDS <DSPick> regrs1_( PICK_TAG );


//----------------------------------------------------------------------------
// Constructor specifies whther to randomise and what distribution (expressed
// as percents in the dist array) to use.
//----------------------------------------------------------------------------

DSPick :: DSPick( const FieldList & order, bool rand,
						const vector <int> & dist  )
	: CompositeDataSource( order ), mRand( rand ), mPos(0), mDistrib( dist )  {
}

//----------------------------------------------------------------------------
// pick randomly or alternately
//----------------------------------------------------------------------------

Row DSPick  :: Get() {
	if ( mRand ) {
		unsigned int i;
		if ( mDistrib.size() == 0 ) {
			i = RNG::Random() % SourceCount();
		}
		else {
			int rp = RNG::Random( 0, 100 );
			int t = 0;
			for ( i = 0; i < mDistrib.size(); i++ ) {
				t += mDistrib.at(i);
				if ( rp <= t ) {
					break;
				}
			}
		}
		Row r = SourceAt( i )->Get();
		return r;
	}
	else {
		Row r = SourceAt( mPos++ )->Get();
		mPos %= SourceCount();
		return r;
	}
}


//----------------------------------------------------------------------------
// pick does not support sizing
//----------------------------------------------------------------------------

int DSPick :: Size() {
	return 1;
}

//----------------------------------------------------------------------------
// Create from xml spec
//----------------------------------------------------------------------------

DataSource * DSPick :: FromXML( const ALib::XMLElement * e ) {
	AllowAttrs( e, AttrList( RANDOM_ATTRIB, ORDER_ATTRIB, DIST_ATTR, 0 ) );
	RequireChildren( e );
	bool rand = GetRandom( e );
	ALib::CommaList d =  e->AttrValue( DIST_ATTR, "" );
	if ( d.Size() && ! rand ) {
		throw XMLError( "Cannot have distribution", e );
	}
	vector <int> dv;
	int total = 0;
	for ( unsigned int i = 0; i < d.Size(); i++ ) {
		if ( ! ALib::IsInteger( d.At(i) ) ) {
			throw XMLError( d.At(i) + " not integer", e );
		}
		int n = ALib::ToInteger( d.At(i) );
		if ( n <= 0 )  {
			throw XMLError( "Distribution value cannot be zero or less", e );
		}
		dv.push_back( n );
		total += n;
	}
	if ( dv.size() != 0 && total != 100 ) {
		throw XMLError( "Values must total 100%", e );
	}

	std::auto_ptr <DSPick> pk( new DSPick( GetOrder( e ), rand, dv ) );
	pk->AddChildSources( e );
	if ( dv.size() && (dv.size() != pk->SourceCount()) ) {
		throw XMLError( "Vlues does not match source count" , e);
	}
	return pk.release();
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

#ifdef DMK_TEST

#include "a_myth.h"
using namespace ALib;
using namespace DMK;

DEFSUITE( "Pick" );

DEFTEST( Alternate ) {
	string xml =
		"<pick random='no'>\n"
			"<row values='foo' />\n"
			"<row values='bar' />\n"
		"</pick>\n";

	XMLPtr xp( xml );
	DSPick * c = (DSPick *) DSPick::FromXML( xp );
	Row r = c->Get();
	FAILNE( r.Size(), 1 );
	FAILNE( r.At(0), "foo" );
	r = c->Get();
	FAILNE( r.At(0), "bar" );
	r = c->Get();
	FAILNE( r.At(0), "foo" );
}

DEFTEST( TestRandom ) {
	string xml =
		"<pick random='yes'>\n"
			"<row values='foo' />\n"
			"<row values='bar' />\n"
		"</pick>\n";

	XMLPtr xp( xml );
	DSPick * c = (DSPick *) DSPick::FromXML( xp );
	Row r = c->Get();
}

#endif

//----------------------------------------------------------------------------

// end

