//---------------------------------------------------------------------------
// dmk_dates.cpp
//
// date sequences etc. for dmk
// ??? lots of problems with this ???
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_str.h"
#include "a_collect.h"
#include "a_date.h"
#include "dmk_source.h"
#include "dmk_tagdict.h"
#include "dmk_xmlutil.h"
#include "dmk_strings.h"
#include "dmk_random.h"

using std::string;
using std::vector;

namespace DMK {

//----------------------------------------------------------------------------

const char * const DATESEQ_TAG 	= "date_seq";
const char * const DATERAND_TAG 	= "random_date";
const char * const DATERAND2_TAG 	= "rand_date";


const char * const INCTYPE_ATTRIB 	= "inc_type";
const char * const MODE_ATTRIB 	= "mode";

const char * const DAY_INCTYPE 	= "days";
const char * const WEEK_INCTYPE 	= "weeks";
const char * const MONTH_INCTYPE 	= "months";
const char * const YEAR_INCTYPE 	= "years";

//----------------------------------------------------------------------------
// Serquential dates, incremented by day
//----------------------------------------------------------------------------

class DSDateSeq : public DataSource {

	public:

		DSDateSeq( const FieldList & order,
					const ALib::Date & begin,
					const ALib::Date & end,
					unsigned int inc,
					const string & inctype );

		Row Get();
		int Size();
		void Reset();

		static DataSource * FromXML( const ALib::XMLElement * e );

	private:

		ALib::Date mBegin, mNow, mEnd;
		unsigned int mInc;
		string mIncType;
};

//----------------------------------------------------------------------------
// Random dates
//----------------------------------------------------------------------------

class DSRandomDate : public DataSource {

	public:

		DSRandomDate( const FieldList & order,
					const ALib::Date & begin,
					const ALib::Date & end );

		DSRandomDate( const FieldList & order,
					const ALib::Date & begin,
					const ALib::Date & end,
					 const ALib::Date & mode );

		~DSRandomDate();

		Row Get();
		int Size();
		void Reset() {} 	// does nothing

		static DataSource * FromXML( const ALib::XMLElement * e );

	private:

		ALib::Date mBegin;
		Distribution * mDist;
};

//----------------------------------------------------------------------------
// Register tags
//----------------------------------------------------------------------------

static RegisterDS <DSDateSeq> regrs1_( DATESEQ_TAG );
static RegisterDS <DSRandomDate> regrs2_( DATERAND_TAG );
static RegisterDS <DSRandomDate> regrs3_( DATERAND2_TAG  );

//----------------------------------------------------------------------------
// Note increment type not currently correctly supported
//----------------------------------------------------------------------------

DSDateSeq :: DSDateSeq( const FieldList & order,
							const ALib::Date & begin,
							const ALib::Date & end,
							unsigned int inc,
							const string & inctype )

	: DataSource( order ), mBegin( begin ), mNow( begin), mEnd( end ),
		mInc( inc ), mIncType( inctype ) {
}


//----------------------------------------------------------------------------
// ???Need to fix increment???
//----------------------------------------------------------------------------

Row DSDateSeq :: Get() {
	if ( mNow > mEnd && mBegin != mEnd ) {
		mNow = mBegin;
	}
	Row r( mNow.Str() );
	if ( mIncType == DAY_INCTYPE ) {
		mNow += mInc;
	}
	else if ( mIncType == WEEK_INCTYPE ) {
		mNow += 7 * mInc;
	}
	else if ( mIncType == MONTH_INCTYPE ) {
		// ??? this is not correct ???
		mNow += 28 * mInc;
	}
	else if ( mIncType == YEAR_INCTYPE ) {
		mNow = ALib::Date( mNow.Year() + mInc, mNow.Month(), mNow.Day() );
	}
	return Order( r );
}

//----------------------------------------------------------------------------
// need to take inctype into account for size
//----------------------------------------------------------------------------

int DSDateSeq :: Size() {
	if ( mBegin == mEnd ) {
		return 1;
	}
	else {
		// ??? not correct ??
		int diff = 1 + ALib::Date::Diff( mEnd, mBegin ) / mInc;
		return diff;
	}
}

void DSDateSeq :: Reset() {
	mNow = mBegin;
}

ALib::Date GetDate( const ALib::XMLElement * e, const string & name,
							const string & def ) {
	string ds = e->AttrValue( name, def );
	if ( ALib::Date::Validate( ds ) != ALib::Date::DATEOK ) {
		throw XMLError( "invalid date " + ALib::SQuote( ds ), e );
	}
	return ds;
}

//----------------------------------------------------------------------------
// Create from xml
//----------------------------------------------------------------------------

DataSource * DSDateSeq :: FromXML( const ALib::XMLElement * e ) {

	ForbidChildren( e );
	RequireAttrs( e, AttrList( BEGIN_ATTRIB, 0 ) );
	AllowAttrs( e, AttrList( BEGIN_ATTRIB, END_ATTRIB,
								INC_ATTRIB, INCTYPE_ATTRIB, 0 ) );

	ALib::Date begin = GetDate( e, BEGIN_ATTRIB, "" );
	ALib::Date end = GetDate( e, END_ATTRIB, begin.Str() );
	if ( end < begin ) {
		throw XMLError( "Invalid begin/end values", e );
	}
	int inc = GetInt( e, INC_ATTRIB, "1" );
	if ( inc < 1 ) {
		throw XMLError( ALib::SQuote( INC_ATTRIB )
							+ " must be greater than zero", e );
	}
	string it = e->AttrValue( INCTYPE_ATTRIB, DAY_INCTYPE );
	if ( it != DAY_INCTYPE && it != WEEK_INCTYPE
				&& it != MONTH_INCTYPE && it != YEAR_INCTYPE ) {
		throw XMLError( ALib::SQuote( it ) + " not valid increment type", e );
	}
	return new DSDateSeq( GetOrder( e ), begin, end, inc, it );
}

//----------------------------------------------------------------------------
// Random dates don't have increment problems
//----------------------------------------------------------------------------

DSRandomDate :: DSRandomDate( const FieldList & order,
								const ALib::Date & begin,
								const ALib::Date & end )
	: DataSource( order ), mBegin( begin ), mDist( 0 ) {

	int iend = ALib::Date::Diff( end, begin );
	 mDist = new UniformDist( 0,  iend );
}


//----------------------------------------------------------------------------
// This version allows a mode to skew the data
//----------------------------------------------------------------------------

DSRandomDate :: DSRandomDate( const FieldList & order,
								const ALib::Date & begin,
								const ALib::Date & end,
								const ALib::Date & mode )
	: DataSource( order ), mBegin( begin ), mDist( 0 ) {

	int iend = ALib::Date::Diff( end, begin );
	int imode = ALib::Date::Diff( mode, begin );
	 mDist = new TriangleDist( 0, imode, iend );
}

//----------------------------------------------------------------------------
// Junk distribution
//----------------------------------------------------------------------------

DSRandomDate :: ~DSRandomDate() {
	delete mDist;
}

//----------------------------------------------------------------------------
// Next random date
//----------------------------------------------------------------------------

Row DSRandomDate :: Get() {
	ALib::Date dt = ALib::Date::Add( mBegin, mDist->NextInt() );
	return Order( Row( dt.Str() ) );
}

//----------------------------------------------------------------------------
// Size of random objects always 1
//----------------------------------------------------------------------------

int DSRandomDate :: Size() {
	return 1;
}

//----------------------------------------------------------------------------
// Produce random date in range. If no range specified, produce random date
// in the current year.
//----------------------------------------------------------------------------

DataSource * DSRandomDate :: FromXML( const ALib::XMLElement * e ) {

	ForbidChildren( e );
	ALib::Date begin, end, mode;
	bool havemode = false;
	if ( e->AttrCount() == 0 ) {
		ALib::Date now = ALib::Date::Today();
		begin = ALib::Date( now.Year(), 1, 1 );
		end = ALib::Date( now.Year() + 1, 1, 1 );
	}
	else {
		RequireAttrs( e, AttrList( BEGIN_ATTRIB, END_ATTRIB,0 ) );
		AllowAttrs( e, AttrList( BEGIN_ATTRIB, END_ATTRIB, MODE_ATTRIB, 0 ) );
		begin = GetDate( e, BEGIN_ATTRIB, "" );
		end = GetDate( e, END_ATTRIB, "" );
		if ( e->HasAttr( MODE_ATTRIB) ) {
			mode = GetDate( e,MODE_ATTRIB, "" );
			havemode = true;
		}
	}
	if ( end < begin ) {
		throw XMLError( "Invalid begin/end values", e );
	}
	if ( havemode && ( mode < begin || mode > end ) ) {
		throw XMLError( "Invalid modal value", e );
	}
	return havemode ? new DSRandomDate( GetOrder( e ), begin, end, mode )
					 : new DSRandomDate( GetOrder( e ), begin, end );
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

#ifdef DMK_TEST

#include "a_myth.h"
using namespace ALib;
using namespace DMK;

DEFSUITE( "DateSeq" );

DEFTEST( FromXML1 ) {
	string xml = "<date_seq begin='2000-01-01'/>";
	XMLPtr xp( xml );
	DSDateSeq * m = (DSDateSeq *) DSDateSeq::FromXML( xp );
	Row r = m->Get();
	FAILNE( r.Size(), 1 );
	FAILNE( r.At(0), "2000-01-01");
	r = m->Get();
	FAILNE( r.Size(), 1 );
	FAILNE( r.At(0), "2000-01-02");
}

#endif

//----------------------------------------------------------------------------

// end

