//---------------------------------------------------------------------------
// dmk_intseq.cpp
//
// Random & non-random integer sequences.
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

using std::string;
using std::vector;

namespace DMK {

//----------------------------------------------------------------------------

const char * const INTSEQ_TAG 		= "int_seq";
const char * const RANDINT_TAG 	= "rand_int";
const char * const MODE_ATTR 		= "mode";


//----------------------------------------------------------------------------
// Sequential integers
//----------------------------------------------------------------------------

class DSIntSeq : public DataSource, public SequenceType {

	public:

		DSIntSeq( const FieldList & order,	int begin, int end, int inc );

		Row Get();
		int Size();
		void Reset();

		static DataSource * FromXML( const ALib::XMLElement * e );

	private:

		int mBegin, mNow, mEnd, mInc;
};

//----------------------------------------------------------------------------
// Random integers. Now support uniform & triangular distributions.
//----------------------------------------------------------------------------

class DSRandInt : public DataSource {

	public:

		DSRandInt( const FieldList & order,	int begin, int end );
		DSRandInt( const FieldList & order,	int begin, int end, int mode );

		~DSRandInt();

		Row Get();
		int Size();
        void Reset() {} 	// does nothing

		static DataSource * FromXML( const ALib::XMLElement * e );

	private:

		Distribution * mDist;
};

//----------------------------------------------------------------------------
// Tag dictionary registration
//----------------------------------------------------------------------------

static RegisterDS <DSIntSeq> regrs1_( INTSEQ_TAG );
static RegisterDS <DSRandInt> regrs2_( RANDINT_TAG );

//----------------------------------------------------------------------------
// Non-random integer sequence
//----------------------------------------------------------------------------

DSIntSeq :: DSIntSeq( const FieldList & order, int begin, int end, int inc )
	: DataSource( order ),
		mBegin( begin ), mNow( begin ), mEnd( end), mInc( inc )  {
}

//----------------------------------------------------------------------------
// Return current value, increment value. Needs to take into account
// whether we are going up or down.
//----------------------------------------------------------------------------

Row DSIntSeq  :: Get() {
	string ns = ALib::Str( mNow );
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

	return Order( Row( ns ) );
}

//----------------------------------------------------------------------------
// reset to start of seq
//----------------------------------------------------------------------------

void DSIntSeq :: Reset() {
	mNow = mBegin;
}

//----------------------------------------------------------------------------
// Size is difference between begin & end - always at least 1
//----------------------------------------------------------------------------

int DSIntSeq :: Size() {
	return 1 + std::abs( mBegin - mEnd ) / std::abs( mInc );
}

//----------------------------------------------------------------------------
// Create from XML. We are quite strict about allowed values for begin,
// end and inc.
//----------------------------------------------------------------------------

DataSource * DSIntSeq :: FromXML( const ALib::XMLElement * e ) {

	ForbidChildren( e );
	AllowAttrs( e, AttrList( BEGIN_ATTRIB, END_ATTRIB,
								INC_ATTRIB, ORDER_ATTRIB, 0 ));
	RequireAttrs( e, AttrList( BEGIN_ATTRIB, END_ATTRIB, 0 ) );
	int begin = GetInt( e, BEGIN_ATTRIB );
	int end = GetInt( e, END_ATTRIB );
	int inc = GetInt( e, INC_ATTRIB, "1" );

	if ( inc == 0 ) {
		throw XMLError( "increment cannot be zero", e );
	}
	else if ( (begin < end && inc < 0 ) || (begin > end  && inc > 0 ) ) {
		throw XMLError( "invalid begin/end/inc combination", e );
	}

	return new DSIntSeq( GetOrder( e ), begin, end, inc );
}

//----------------------------------------------------------------------------
// Random integers
//----------------------------------------------------------------------------

DSRandInt :: DSRandInt( const FieldList & order, int begin, int end )
	: DataSource( order ), mDist( 0 ) {

	if ( begin == end ) {
		mDist = new UniformDist( 0, INT_MAX);
	}
	else {
		mDist = new UniformDist( begin, end );
	}
}

//----------------------------------------------------------------------------
// Random with mode to allow skewing of distribution.
//----------------------------------------------------------------------------

DSRandInt :: DSRandInt( const FieldList & order, int begin, int end, int mode )
	: DataSource( order ), 	mDist( new TriangleDist( begin, mode, end )  ) {
}

//----------------------------------------------------------------------------
// Junk distribution
//----------------------------------------------------------------------------

DSRandInt :: ~DSRandInt() {
	delete mDist;
}

//----------------------------------------------------------------------------
// Not a lot to do here any more...
//----------------------------------------------------------------------------

Row DSRandInt :: Get() {
	return Order( Row( ALib::Str( mDist->NextInt() ) ) );
}

//----------------------------------------------------------------------------
// Random numbers cannot provide size info.
//----------------------------------------------------------------------------

int DSRandInt :: Size() {
	return 1;
}

//----------------------------------------------------------------------------
// Create from XML.
// note if we have begin we must have end & vice versa
//----------------------------------------------------------------------------

DataSource * DSRandInt :: FromXML( const ALib::XMLElement * e ) {
	ForbidChildren( e );
	AllowAttrs( e, AttrList( BEGIN_ATTRIB, END_ATTRIB, MODE_ATTR,
								ORDER_ATTRIB, 0 ));
	int begin = 0, end = 0;
	if ( e->HasAttr( BEGIN_ATTRIB ) || e->HasAttr( END_ATTRIB ) ) {
		begin = GetInt( e, BEGIN_ATTRIB );
		end = GetInt( e, END_ATTRIB );
		if ( begin >= end ) {
			throw XMLError( ALib::SQuote( BEGIN_ATTRIB )
							+ " must be less than "
							+ ALib::SQuote( END_ATTRIB ),
							e );
		}
	}
	if ( e->HasAttr( MODE_ATTR ) ) {
		int mode = GetInt( e, MODE_ATTR );
		if ( mode < begin || mode > end ) {
			throw XMLError( "bad mode value", e );
		}
		return new DSRandInt( GetOrder( e ), begin, end, mode );
	}
	else {
		return new DSRandInt( GetOrder( e ), begin, end );
	}
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

#ifdef DMK_TEST

#include "a_myth.h"
using namespace ALib;
using namespace DMK;

DEFSUITE( "IntSeq" );


DEFTEST( FromXML1 ) {
	string xml = "<int_seq begin='1' end='10'/>";
	XMLPtr xp( xml );
	DSIntSeq * c = (DSIntSeq *) DSIntSeq::FromXML( xp );
	Row r = c->Get();
	FAILNE( r.Size(), 1 );
	FAILNE( r.At(0), "1" );
	r = c->Get();
	FAILNE( r.At(0), "2" );
}

DEFTEST( FromXML2 ) {
	string xml = "<random_int begin='1' end='10'/>";
	XMLPtr xp( xml );
	DSRandInt * c = (DSRandInt *) DSRandInt::FromXML( xp );
	Row r = c->Get();
	FAILNE( r.Size(), 1 );
}


#endif

//----------------------------------------------------------------------------

// end

