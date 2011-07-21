//---------------------------------------------------------------------------
// dmk_range.cpp
//
// range from seq for dmk3
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------


#include "a_base.h"
#include "a_str.h"
#include "a_rand.h"
#include "a_collect.h"
#include "dmk_source.h"
#include "dmk_tagdict.h"
#include "dmk_xmlutil.h"
#include "dmk_strings.h"
#include "dmk_types.h"
#include "dmk_random.h"

using std::string;
using std::vector;

namespace DMK {

//----------------------------------------------------------------------------

const char * const RANGE_TAG 	= "range";
const char * const FILL_ATTR 	= "fill";
const char * const WIDTH_ATTR	= "width";
const char * const CONT_ATTR	= "continue";



//----------------------------------------------------------------------------

class DSRange : public CompositeDataSource {

	public:

		DSRange( const FieldList & order,
					unsigned int min, unsigned int max, bool fill, bool cont );

		Row Get();
		int Size();
		void Reset();

		static DataSource * FromXML( const ALib::XMLElement * e );

	private:

		unsigned int mMin, mMax;
		bool mFill, mCont;
		Row mLast;

};

//----------------------------------------------------------------------------

static RegisterDS <DSRange> regrs1_( RANGE_TAG );

//----------------------------------------------------------------------------
// Specify min & max no. of values to produce, and whther to fill the range
//----------------------------------------------------------------------------

DSRange :: DSRange( const FieldList & order,
					unsigned int min, unsigned int max, bool fill, bool cont )
	: CompositeDataSource( order ),
	    mMin( min ), mMax( max ), mFill( fill ), mCont( cont ) {
}

//----------------------------------------------------------------------------
// Reset needs to clear last
//----------------------------------------------------------------------------


void DSRange :: Reset() {
	mLast = Row();
	CompositeDataSource::Reset();
}

//----------------------------------------------------------------------------
// Produce values. Note that final row may be ragged, which may cause
// problems for order, but we allow its use anyway
//----------------------------------------------------------------------------

Row DSRange :: Get() {

	Row r;
	int n = RNG::Random( mMin, mMax + 1 );

	if ( mCont && mLast.Size() ) {
		r.AppendRow( mLast );
		mLast = Row();
	}
	else {
		r.AppendRow( CompositeDataSource::Get() );
	}

	for ( int i = 1; i < n; i++ ) {

		Row r2 = CompositeDataSource::Get();

		if ( mFill || i == n - 1 ) {
			r.AppendRow( r2 );
		}

		if ( mCont && i == n - 1 ) {
			mLast = r2;
		}
	}
	return Order( r );
}

//----------------------------------------------------------------------------
// ranges don't support size
//----------------------------------------------------------------------------

int DSRange :: Size() {
	return 1;
}


//----------------------------------------------------------------------------
// Helper to get range width specifed as min, max pair. If macx is omitted
// dedfaults to min value.
//----------------------------------------------------------------------------

static void GetWidth( const ALib::XMLElement * e,
						unsigned int & min, unsigned int & max ) {

	ALib::CommaList cl( e->AttrValue( WIDTH_ATTR ));
	if ( cl.Size() < 1 || cl.Size() > 2 ) {
		XMLERR( e, "Invalid width" );
	}

	int imin = ALib::ToInteger( cl.At(0) );
	int imax = cl.Size() == 2 ? ALib::ToInteger( cl.At(1) ) : imin;

	if ( imin < 1 || imax < imin ) {
		XMLERR( e, "Invalid width" );
	}
	min  = (unsigned int) imin;
	max = (unsigned int) imax;
}

//----------------------------------------------------------------------------
// Create from XML
//----------------------------------------------------------------------------

DataSource * DSRange :: FromXML( const ALib::XMLElement * e ) {
	RequireChildren( e );
	RequireAttrs( e, AttrList( WIDTH_ATTR, 0 ));
	AllowAttrs( e, AttrList( ORDER_ATTRIB, WIDTH_ATTR, FILL_ATTR, CONT_ATTR, 0 ) );

	bool fill = GetBool( e, FILL_ATTR, NO_STR );
	bool cont = GetBool( e, CONT_ATTR, NO_STR );
	unsigned int min = 0, max = 0;
	GetWidth( e, min, max );

	std::auto_ptr <DSRange> rp( new DSRange( GetOrder( e ), min, max, fill, cont ) );
	rp->AddChildSources( e );

	return rp.release();
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

#ifdef DMK_TEST

#include "a_myth.h"
using namespace ALib;
using namespace DMK;

DEFSUITE( "Range" );

const char * const XML1 =
	"<range width='2,2' fill='yes'>\n"
	   "<int_seq  begin='1' end='10' />\n"
	"</range>\n";

DEFTEST( Simplest ) {
	Randomise();
	XMLPtr xml( XML1 );
	DSRange * p = (DSRange *) DSRange::FromXML( xml );
	Row r = p->Get();
	//std::cout << r << std::endl;
	FAILNE( r.Size(), 2 );
}


#endif

//----------------------------------------------------------------------------

// end

