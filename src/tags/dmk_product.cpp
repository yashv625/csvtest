//---------------------------------------------------------------------------
// dmk_product.cpp
//
// Produce cartesian product of two sources
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

const char * const PRODUCT_TAG 		= "product";

//----------------------------------------------------------------------------

class DSProduct : public Intermediate {

	public:

		DSProduct( const FieldList & order, bool rand );
		static DataSource * FromXML( const ALib::XMLElement * e );

		void Populate();


};

//----------------------------------------------------------------------------
// Register tag
//----------------------------------------------------------------------------

static RegisterDS <DSProduct> regrs1_( PRODUCT_TAG );

//----------------------------------------------------------------------------
// Note random defaults to "no" for product
//----------------------------------------------------------------------------

DSProduct :: DSProduct( const FieldList & order, bool rand  )
	: Intermediate( order, rand )  {
}


//----------------------------------------------------------------------------
// build the product - we know there are two child sources
//----------------------------------------------------------------------------

void DSProduct :: Populate() {

	// get sizes & force creation of child data
	int sz0 = SourceAt(0)->Size();
	int sz1 = SourceAt(1)->Size();

	if ( sz0 <= 0 || sz1 <= 0 ) {
		throw Exception( "no size info" );
	}

	// get all rows from source #1
	Rows tmp1;
	for (  int i = 0; i < sz1 ; i++ ) {
		tmp1.push_back( SourceAt(1)->Get() );
	}

	// build product of sources #1 and #2
	for ( int i = 0; i < sz0 ; i++ ) {
		Row r = SourceAt(0)->Get();
		for ( int i = 0; i < sz1 ; i++ ) {
			Row r2(r);
			r2.AppendRow( tmp1[i] );
			AddRow( r2 );
		}
	}

	// no longer need the child data
	SourceAt(0)->Discard();
	SourceAt(1)->Discard();
}

//----------------------------------------------------------------------------
// create from xml - need to check that there are only two child sources
//----------------------------------------------------------------------------

DataSource * DSProduct :: FromXML( const ALib::XMLElement * e ) {
	RequireChildren( e );
	AllowAttrs( e, AttrList( ORDER_ATTRIB, RANDOM_ATTRIB, 0 ) );
	if ( e->ChildCount() != 2 ) {
		XMLError( "require exactly two child sources", e );
	}
	bool rand = GetRandom( e, NO_STR );
	std::auto_ptr <DSProduct> c( new DSProduct( GetOrder( e ), rand ));
	c->AddChildSources( e );
	return c.release();
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

#ifdef DMK_TEST

#include "a_myth.h"
using namespace ALib;
using namespace DMK;

DEFSUITE( "Product" );

const char * const XML1 =
	"<product random='no' >\n"
		"<rows values='1,2,3' random='no'/>\n"
		"<rows values='a,b' random='no'/>\n"
	"</product>\n";

DEFTEST( Simple ) {
	XMLPtr xml( XML1 );
	DSProduct * p = (DSProduct *) DSProduct::FromXML( xml );
	Row r = p->Get();
	FAILNE( r.Size(), 2 );
	FAILNE( r.At(0), "1" );
	FAILNE( r.At(1), "a" );
	r = p->Get();
	FAILNE( r.Size(), 2 );
	FAILNE( r.At(0), "1" );
	FAILNE( r.At(1), "b" );
	r = p->Get();
	FAILNE( r.Size(), 2 );
	FAILNE( r.At(0), "2" );
	FAILNE( r.At(1), "a" );
}


#endif

//----------------------------------------------------------------------------

// end

