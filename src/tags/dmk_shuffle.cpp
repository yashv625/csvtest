//---------------------------------------------------------------------------
// dmk_shuffle.cpp
//
// shuffle values as for deck of cards
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

const char * const SHUFFLE_TAG 		= "shuffle";

//----------------------------------------------------------------------------

class DSShuffle : public CompositeDataSource {

	public:

		DSShuffle( const FieldList & order );
		static DataSource * FromXML( const ALib::XMLElement * e );

		int Size();
		void Discard();
		Row Get();

	private:

		void Populate();
		Rows mRows;
		int mEnd;
};

//----------------------------------------------------------------------------
// Register shuffle tag
//----------------------------------------------------------------------------

static RegisterDS <DSShuffle> regrs1_( SHUFFLE_TAG );

//----------------------------------------------------------------------------
// note does not make sense for shuffle to support random
//----------------------------------------------------------------------------

DSShuffle :: DSShuffle( const FieldList & order  )
	: CompositeDataSource( order ) , mEnd(0) {
}

//----------------------------------------------------------------------------
// We can support size only after population
//----------------------------------------------------------------------------

int DSShuffle :: Size() {
	Populate();
	return mRows.size();
}

//----------------------------------------------------------------------------
// Discard everything used for shuffling
//----------------------------------------------------------------------------

void DSShuffle :: Discard() {
	mRows.clear();
	CompositeDataSource::Discard();
}

//----------------------------------------------------------------------------
// get all the values to shuffle
//----------------------------------------------------------------------------

void DSShuffle :: Populate() {
	if ( mRows.size() ) {
		return;
	}
	// must not call our Size to avoid infinite recursion
	int sz = CompositeDataSource::Size();

	for (  int i = 0; i < sz ; i++ ) {
		mRows.push_back( CompositeDataSource::Get() );
	}

	if ( mRows.size() == 0 ) {
		throw Exception( "Empty result set" );
	}

	mEnd = 0;
}

//----------------------------------------------------------------------------
// Deal out shuffled row by picking random value from set of values. Then
// remove it from the set and reduce set size by one.
//----------------------------------------------------------------------------

Row DSShuffle :: Get() {
	Populate();
	if ( mEnd == 0 ) {
		mEnd = mRows.size();
	}
	int i = RNG::Random( 0, mEnd-- );
	Row r = mRows[i];
	mRows[i] = mRows[mEnd];
	return Order(r);
}


//----------------------------------------------------------------------------
// create from xml
//----------------------------------------------------------------------------

DataSource * DSShuffle :: FromXML( const ALib::XMLElement * e ) {
	RequireChildren( e );
	AllowAttrs( e, AttrList( ORDER_ATTRIB, 0 ) );
	std::auto_ptr <DSShuffle> s( new DSShuffle( GetOrder( e ) ));
	s->AddChildSources( e );
	return s.release();
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

#ifdef DMK_TEST

#include "a_myth.h"
using namespace ALib;
using namespace DMK;

DEFSUITE( "Shuffle" );

const char * const XML1 =
	"<shuffle>\n"
		"<rows values='1,2,3' />\n"
	"</shuffle>\n";

DEFTEST( Simple ) {
	XMLPtr xml( XML1 );
	DSShuffle * p = (DSShuffle *) DSShuffle::FromXML( xml );
	Row r = p->Get();
	FAILNE( r.Size(), 1 );
}


#endif

//----------------------------------------------------------------------------

// end

