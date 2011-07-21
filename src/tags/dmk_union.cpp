//---------------------------------------------------------------------------
// dmk_union.cpp
//
// Produce union of all sources
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

using std::string;
using std::vector;

namespace DMK {

//----------------------------------------------------------------------------

const char * const UNION_TAG 		= "union";

//----------------------------------------------------------------------------

class DSUnion : public Intermediate {

	public:

		DSUnion( const FieldList & order, bool rand );
		static DataSource * FromXML( const ALib::XMLElement * e );
		void Populate();

	private:

		void AddToUnion( DataSource * s );
		bool AlreadyHave( const Row & r ) const;
};

//----------------------------------------------------------------------------

static RegisterDS <DSUnion> regrs1_( UNION_TAG );

//----------------------------------------------------------------------------

// ctor
DSUnion :: DSUnion( const FieldList & order, bool rand  )
	: Intermediate( order, rand )  {
}


// does this row already exist in union?
// TODO (neilb#1#): change to use map  as index
bool DSUnion :: AlreadyHave( const Row & r ) const {
	for ( unsigned int i = 0; i < ResultRows().size(); i++ ) {
		if ( r == ResultRows().at( i ) ) {
			return true;
		}
	}
	return false;
}

// add row to union only if not already there
// this is currently extremely inefficient!
void DSUnion :: AddToUnion( DataSource * s ) {
	int n = s->Size();
	while( n-- ) {
		Row r = s->Get();
		if ( ! AlreadyHave( r ) ) {
			AddRow( r );	// add to rows held by base
		}
	}
}

// build the union from the data sources
void DSUnion :: Populate() {
	for ( unsigned int i = 0; i < SourceCount(); i++ ) {
		if ( SourceAt( i )->Size() <= 0 ) {
			throw Exception( "source " + ALib::Str(i+1) + " has no size" );
		}
		AddToUnion( SourceAt(i) );
	}
}

// create from xml - must be at least 2 child sources
DataSource * DSUnion :: FromXML( const ALib::XMLElement * e ) {
	RequireChildren( e );
	AllowAttrs( e, AttrList( ORDER_ATTRIB, RANDOM_ATTRIB, 0 ) );
	if ( e->ChildCount() < 2 ) {
		XMLError( "require at least two two child sources", e );
	}
	bool rand = GetRandom( e, NO_STR);
	std::auto_ptr <DSUnion> u( new DSUnion( GetOrder( e ), rand ));
	u->AddChildSources( e );
	return u.release();
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

#ifdef DMK_TEST

#include "a_myth.h"
using namespace ALib;
using namespace DMK;

DEFSUITE( "Union" );

const char * const XML1 =
	"<union random='no'>\n"
		"<rows values='1,3,2' random='no'/>\n"
		"<rows values='2,4,3' random='no'/>\n"
	"</union>\n";

DEFTEST( Simple ) {
	XMLPtr xml( XML1 );
	DSUnion * p = (DSUnion *) DSUnion::FromXML( xml );
	FAILNE( p->Size(), 4 );
	Row r = p->Get();
	FAILNE( r.Size(), 1 );
	FAILNE( r.At(0), "1" );
	r = p->Get();
	FAILNE( r.At(0), "3" );
	r = p->Get();
	FAILNE( r.At(0), "2" );
	r = p->Get();
	FAILNE( r.At(0), "4" );


}


#endif

//----------------------------------------------------------------------------

// end

