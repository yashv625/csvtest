//---------------------------------------------------------------------------
// dmk_realseqcpp
//
// Random & non-random real number sequences
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
#include "dmk_types.h"

#include <cmath>
#include <sstream>
#include <iomanip>
#include <float.h>

using std::string;
using std::vector;

namespace DMK {

//----------------------------------------------------------------------------

const char * const REALSEQ_TAG 	= "real_seq";
const char * const RANDREAL_TAG 	= "rand_real";
const char * const MODE_ATTR 		= "mode";
const char * const PREC_ATTR 		= "places";

//----------------------------------------------------------------------------
// Sequential reals
//----------------------------------------------------------------------------

class DSRealSeq : public DataSource, public SequenceType {

	public:

		DSRealSeq( const FieldList & order,	double  begin,
					double end,  double inc, int prec);

		Row Get();
		int Size();
		void Reset();

		static DataSource * FromXML( const ALib::XMLElement * e );

	private:

		double mBegin, mNow, mEnd, mInc;
		int mPrec;
};

//----------------------------------------------------------------------------
// Non-sequential random real numbers
//----------------------------------------------------------------------------

class DSRandReal : public DataSource {

	public:

		DSRandReal( const FieldList & order, double begin,
								double end, int prec );
		DSRandReal( const FieldList & order, double begin,
								double end, double mode, int prec );

		~DSRandReal();

		Row Get();
		int Size();
        void Reset() {} 	// does nothing

		static DataSource * FromXML( const ALib::XMLElement * e );

	private:

		Distribution * mDist;
		int mPrec;
};

//----------------------------------------------------------------------------
// Tag dictionary registration
//----------------------------------------------------------------------------

static RegisterDS <DSRealSeq> regrs1_( REALSEQ_TAG );
static RegisterDS <DSRandReal> regrs2_( RANDREAL_TAG );

//----------------------------------------------------------------------------
// Helper to print number with prec digiyts after decimal point
//----------------------------------------------------------------------------

static string WriteReal( double d, int prec ) {
	std::ostringstream os;
	if ( prec != 0 ) {
		os << std::showpoint << std::fixed << std::setprecision( prec ) << d;
	}
	else {
		os << std::noshowpoint << std::fixed << std::setprecision( 0 ) << d;
	}
	return os.str();
}

//----------------------------------------------------------------------------
// Non-random real sequence
//----------------------------------------------------------------------------

DSRealSeq :: DSRealSeq( const FieldList & order,
						double begin, double end, double inc, int prec )
	: DataSource( order ),
		mBegin( begin ), mNow( begin ), mEnd( end), mInc( inc ), mPrec( prec )  {
}

//----------------------------------------------------------------------------
// Return current value, increment value. Needs to take into account
// whether we are going up or down.
//----------------------------------------------------------------------------

Row DSRealSeq  :: Get() {

	string s = WriteReal( mNow, mPrec );

	if ( mInc > 0 ) {
		if ( mNow + mInc > mEnd ) {
			mNow = mBegin;
		}
		else {
			mNow += mInc;
		}
	}
	else {
		if ( mNow + mInc < mEnd ) {
			mNow = mBegin;
		}
		else {
			mNow += mInc;
		}
	}

	return Order( Row( s ) );
}

//----------------------------------------------------------------------------
// reset to start of seq
//----------------------------------------------------------------------------

void DSRealSeq :: Reset() {
	mNow = mBegin;
}

//----------------------------------------------------------------------------
// Size is difference between begin & end - always at least 1
//----------------------------------------------------------------------------

int DSRealSeq :: Size() {
	return 1 + std::fabs( mBegin - mEnd ) / std::fabs( mInc );
}

//----------------------------------------------------------------------------
// Build from XML
//----------------------------------------------------------------------------

DataSource * DSRealSeq :: FromXML( const ALib::XMLElement * e ) {

	ForbidChildren( e );
	AllowAttrs( e, AttrList( BEGIN_ATTRIB, END_ATTRIB,
								INC_ATTRIB, ORDER_ATTRIB, PREC_ATTR, 0 ));
	RequireAttrs( e, AttrList( BEGIN_ATTRIB, END_ATTRIB,0 ) );
	double begin = GetReal( e, BEGIN_ATTRIB );
	double end = GetReal( e, END_ATTRIB  );
	double inc = GetReal( e, INC_ATTRIB, "1.0" );
	int prec = GetInt( e, PREC_ATTR, "2" );
	if ( prec < 0 || prec > 10 ) {
		XMLERR( e, "Invalid number of decimal places: " << prec );
	}
	if ( inc == 0.0 ) {
		XMLERR( e, "Increment cannot be zero" );
	}
	else if ( (begin < end && inc < 0 ) || (begin > end  && inc > 0 ) ) {
		XMLERR( e, "invalid begin/end/inc combination" );
	}

	return new DSRealSeq( GetOrder( e ), begin, end, inc, prec );
}

//----------------------------------------------------------------------------
// Random reals. may use uniform or triangular distribution.
//----------------------------------------------------------------------------

DSRandReal :: DSRandReal( const FieldList & order, double begin,
									double end, int prec )
	: DataSource( order ), mDist( 0 ), mPrec( prec ) {

	if ( begin == end ) {
		mDist = new UniformDist( 0, DBL_MAX);
	}
	else {
		mDist = new UniformDist( begin, end );
	}
}

DSRandReal :: DSRandReal( const FieldList & order,
								double begin,
								double end, double mode, int prec )
	: DataSource( order ),
		mDist( new TriangleDist( begin, mode, end )  ), mPrec( prec ) {
}

//----------------------------------------------------------------------------
// Junk distribution
//----------------------------------------------------------------------------

DSRandReal :: ~DSRandReal() {
	delete mDist;
}

//----------------------------------------------------------------------------
// Get next value from distribution
//----------------------------------------------------------------------------

Row DSRandReal :: Get() {
	return Order( Row( WriteReal( mDist->NextReal(), mPrec ) ));
}

//----------------------------------------------------------------------------
// Random numbers cannot provide size info.
//----------------------------------------------------------------------------

int DSRandReal :: Size() {
	return 1;
}

//----------------------------------------------------------------------------
// Build from XML
//----------------------------------------------------------------------------

DataSource * DSRandReal :: FromXML( const ALib::XMLElement * e ) {
	ForbidChildren( e );
	AllowAttrs( e, AttrList( BEGIN_ATTRIB, END_ATTRIB, MODE_ATTR,
								ORDER_ATTRIB, PREC_ATTR, 0 ));
	double begin = 0, end = 0;
	int prec = GetInt( e, PREC_ATTR, "2" );
	if ( prec < 0 || prec > 10 ) {
		XMLERR( e, "Invalid number of decimal places: " << prec );
	}
	if ( e->HasAttr( BEGIN_ATTRIB ) || e->HasAttr( END_ATTRIB ) ) {
		begin = GetReal( e, BEGIN_ATTRIB );
		end = GetReal( e, END_ATTRIB );
		if ( begin >= end ) {
			XMLERR( e, "begin must be less than end" );
		}
	}
	if ( e->HasAttr( MODE_ATTR ) ) {
		double mode = GetReal( e, MODE_ATTR );
		if ( mode < begin || mode > end ) {
			XMLERR( e, "Bad mode value: " << mode );
		}
		return new DSRandReal( GetOrder( e ), begin, end, mode, prec);
	}
	else {
		return new DSRandReal( GetOrder( e ), begin, end, prec );
	}
}


//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

#ifdef DMK_TEST

#include "a_myth.h"
using namespace ALib;
using namespace DMK;

DEFSUITE( "RealSeq" );


DEFTEST( FromXML1 ) {
	string xml = "<real_seq begin='1' end='10' places='1'/>";
	XMLPtr xp( xml );
	DSRealSeq * c = (DSRealSeq *) DSRealSeq::FromXML( xp );
	Row r = c->Get();
	FAILNE( r.Size(), 1 );
	FAILNE( r.At(0), "1.0" );
	r = c->Get();
	FAILNE( r.At(0), "2.0" );
}

DEFTEST( FromXML2 ) {
	string xml = "<rand_real begin='1' end='10'/>";
	XMLPtr xp( xml );
	DSRandReal * c = (DSRandReal *) DSRandReal::FromXML( xp );
	Row r = c->Get();
	FAILNE( r.Size(), 1 );
}


#endif

//----------------------------------------------------------------------------

// end

