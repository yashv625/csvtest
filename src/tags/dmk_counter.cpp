//---------------------------------------------------------------------------
// dmk_counter.cpp
//
// Data source which outputs a numeric count.
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

const char * const COUNTER_TAG 		= "counter";

//----------------------------------------------------------------------------
// Counts each time used. Unlike int-seq, doesn't have  size.
//----------------------------------------------------------------------------

class DSCounter : public DataSource {

	public:

		DSCounter( const FieldList & order,	int begin, int inc );

		Row Get();
		int Size();
		void Reset();

		static DataSource * FromXML( const ALib::XMLElement * e );

	private:

		int mBegin, mValue, mInc;
};

//----------------------------------------------------------------------------
// Register counter tag
//----------------------------------------------------------------------------

static RegisterDS <DSCounter> regrs1_( COUNTER_TAG );


//----------------------------------------------------------------------------
// Construct from begin value and increment - both default to 1.
//----------------------------------------------------------------------------

DSCounter :: DSCounter( const FieldList & order, int begin, int inc )
	: DataSource( order ), mBegin( begin ), mValue( begin ), mInc( inc )  {
}

//----------------------------------------------------------------------------
// Return current value & increment
//----------------------------------------------------------------------------

Row DSCounter  :: Get() {
	string ns = ALib::Str( mValue );
	mValue += mInc;
	return Order( Row( ns ) );
}


//----------------------------------------------------------------------------
// counters do not support sizing
int DSCounter :: Size() {
	return 1;
}

//----------------------------------------------------------------------------
// start again
//----------------------------------------------------------------------------

void DSCounter :: Reset() {
	mValue = mBegin;
}

//----------------------------------------------------------------------------
// Counters begin & inc values are both optional.
//----------------------------------------------------------------------------

DataSource * DSCounter :: FromXML( const ALib::XMLElement * e ) {

	AllowAttrs( e, AttrList( BEGIN_ATTRIB, INC_ATTRIB, ORDER_ATTRIB, 0 ) );
	AllowChildTags( e, "" );
	int b = GetInt( e, BEGIN_ATTRIB, "1" );
	int i = GetInt( e, INC_ATTRIB, "1" );
	return new DSCounter( GetOrder( e ), b, i );
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

#ifdef DMK_TEST

#include "a_myth.h"
using namespace ALib;
using namespace DMK;

DEFSUITE( "DSCounter" );

DEFTEST( FromXML1 ) {
	string xml = "<counter begin='100' />";
	XMLPtr xp( xml );
	DSCounter * c = (DSCounter *) DSCounter::FromXML( xp );
	Row r = c->Get();
	FAILNE( r.Size(), 1 );
	FAILNE( r.At(0), "100" );
	r = c->Get();
	FAILNE( r.At(0), "101" );
}


#endif

//----------------------------------------------------------------------------

// end

