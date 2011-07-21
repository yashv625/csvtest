//----------------------------------------------------------------------------
// dmk_timeseq.cpp
//
// time sequence and random times
//
// Copyright (C) 2009 Neil Butterworth
//----------------------------------------------------------------------------

#include "a_base.h"
#include "a_str.h"
#include "a_collect.h"
#include "dmk_source.h"
#include "dmk_tagdict.h"
#include "dmk_xmlutil.h"
#include "dmk_strings.h"
#include "dmk_types.h"
#include "dmk_random.h"

#include <sstream>
#include <iomanip>

using std::string;
using std::vector;

namespace DMK {

//----------------------------------------------------------------------------

const char * const TIMESEQ_TAG 		= "time_seq";
const char * const RANDTIME_TAG 		= "rand_time";
const char * const MODE_ATTR 			= "mode";

//----------------------------------------------------------------------------
// Representation of a time in 24 hr clock
//----------------------------------------------------------------------------

class TimeRep {

	public:

		TimeRep();
		TimeRep( const std::string & s );
		std::string Str() const;
		TimeRep &  Inc( int secs );
		int AsInt() const;

	private:

		int GetComponent( const std::string & c, int max );
		int mSecs;

};

//----------------------------------------------------------------------------
// Time Sequence
//----------------------------------------------------------------------------

class TimeSeq : public DataSource, public SequenceType {

	public:

		TimeSeq( const FieldList & order,	const TimeRep & begin,
							const TimeRep & end, int inc );

		Row Get();
		int Size();
		void Reset();

		static DataSource * FromXML( const ALib::XMLElement * e );

	private:

		TimeRep mBegin, mEnd, mNow;
		int mInc;
};

//----------------------------------------------------------------------------
// Random times
//----------------------------------------------------------------------------

class RandTime : public DataSource {

	public:

		RandTime( const FieldList & order,	const TimeRep & begin,
						const TimeRep &  end );
		RandTime( const FieldList & order,	const TimeRep &  begin,
						const TimeRep &  end, const TimeRep &  mode );

		~RandTime();

		Row Get();
		int Size();
        void Reset() {} 	// does nothing

		static DataSource * FromXML( const ALib::XMLElement * e );

	private:

		TimeRep mBegin;
		Distribution * mDist;
};

//----------------------------------------------------------------------------
// Register classes
//----------------------------------------------------------------------------

static RegisterDS <TimeSeq> regrs1_( TIMESEQ_TAG );
static RegisterDS <RandTime> regrs2_( RANDTIME_TAG );

//----------------------------------------------------------------------------
// Default start at midnight
//----------------------------------------------------------------------------

TimeRep :: TimeRep() : mSecs( 0 ) {
}

//----------------------------------------------------------------------------
// Construct from string in form hh:mm:ss
//----------------------------------------------------------------------------

TimeRep :: TimeRep( const string & s )  : mSecs( 0 ) {
	vector <string> tmp;
	ALib::Split( s, ':', tmp );
	if ( tmp.size() != 3 ) {
		throw Exception( "Invalid time format: " + s );
	}
	int hrs = GetComponent( tmp[0], 24 );
	int mins = GetComponent( tmp[1], 60 );
	int secs = GetComponent( tmp[2], 60 );
	mSecs = hrs * 60 * 60 + mins * 60 + secs;
}

//----------------------------------------------------------------------------
// Helper to get hr/min/sec component with range check
//----------------------------------------------------------------------------

int TimeRep :: GetComponent( const string & c, int max ) {
	if ( ALib::IsInteger( c ) ) {
		int n = ALib::ToInteger( c );
		if ( n >= 0 && n < max ) {
			return n;
		}
	}
	throw Exception( "Invalid time component: " + c );
}

//----------------------------------------------------------------------------
// Convert to hh:mm:ss format for output
//----------------------------------------------------------------------------

string TimeRep :: Str() const {
	int hrs = mSecs / ( 60 * 60 );
	int mins = (mSecs - (hrs * 60 * 60)) / 60;
	int secs = mSecs % 60;
	std::ostringstream os;
	os << std::setw(2) << std::setfill( '0' ) << hrs << ":";
	os << std::setw(2) << std::setfill( '0' ) << mins << ":";
	os << std::setw(2) << std::setfill( '0' ) << secs;
	return os.str();
}

//----------------------------------------------------------------------------
// Increment by number of seconds
//----------------------------------------------------------------------------

TimeRep &  TimeRep :: Inc( int secs ) {
	mSecs += secs;
	if ( mSecs < 0 ) {
		mSecs += 24 * 60 * 60;
	}
	return * this;
}

//----------------------------------------------------------------------------
// Get integer rep
//----------------------------------------------------------------------------

int TimeRep :: AsInt() const {
	return mSecs;
}

//----------------------------------------------------------------------------
// Construct from begin/end or range & increment
//----------------------------------------------------------------------------

TimeSeq :: TimeSeq( const FieldList & order, const TimeRep & begin,
							const TimeRep & end, int inc )
		: mBegin( begin ), mEnd( end ), mNow( begin ), mInc( inc ) {
}


//----------------------------------------------------------------------------
// Get next in range, wrapping at end
//----------------------------------------------------------------------------

Row TimeSeq :: Get() {
	string s = mNow.Str();
	mNow.Inc( mInc );
	if ( mNow.AsInt() > mEnd.AsInt() ) {
		mNow = mBegin;
	}
	return Order( Row( s ) );
}

//----------------------------------------------------------------------------
// size always at least 1
//----------------------------------------------------------------------------

int TimeSeq :: Size() {
	return 1 + (mEnd.AsInt() - mBegin.AsInt()) / mInc;
}

//----------------------------------------------------------------------------
// Reset to start of range
//----------------------------------------------------------------------------

void TimeSeq :: Reset() {
	mNow = mBegin;
}

//----------------------------------------------------------------------------
// Construct from XML
//----------------------------------------------------------------------------

DataSource * TimeSeq :: FromXML( const ALib::XMLElement * e ) {
	ForbidChildren( e );
	AllowAttrs( e, AttrList( BEGIN_ATTRIB, END_ATTRIB,
								INC_ATTRIB, ORDER_ATTRIB, 0 ));
	RequireAttrs( e, AttrList( BEGIN_ATTRIB, END_ATTRIB, 0 ) );

	TimeRep begin, end;

	try {
		begin = TimeRep(  e->AttrValue( BEGIN_ATTRIB ) );
		end = TimeRep( e->AttrValue( END_ATTRIB ) );
		if ( begin.AsInt() >= end.AsInt() ) {
			throw Exception( "begin must be before end" );
		}
	}
	catch ( const Exception & ex ) {
		XMLERR( e, ex.what() );
	}
	int inc = GetInt( e, INC_ATTRIB, "1" );
	if ( inc <= 0  || inc >= 24 * 60 * 60 ) {
		XMLERR( e, "Invalid increment: "  << inc );
	}

	return new TimeSeq( GetOrder( e ), begin, end, inc );

}

//----------------------------------------------------------------------------
// random time without mode
//----------------------------------------------------------------------------

RandTime :: RandTime( const FieldList & order,	const TimeRep & begin,
								const TimeRep &  end )
	: DataSource( order ), mBegin( begin ), mDist( 0 ) {

	if ( begin.AsInt() == end.AsInt() ) {
		mDist = new UniformDist( 0, 24 * 60 * 60 - 1  );
	}
	else {
		mDist = new UniformDist( 0, end.AsInt() - begin.AsInt() );
	}
}

//----------------------------------------------------------------------------
// Random time with mode
//----------------------------------------------------------------------------

RandTime :: RandTime( const FieldList & order,	const TimeRep &  begin,
				const TimeRep &  end, const TimeRep &  mode )
	: DataSource( order ), mBegin( begin ), mDist( 0 ) {

	if ( begin.AsInt() == end.AsInt() ) {
		mDist = new TriangleDist( 0, mode.AsInt(), 24 * 60 * 60 - 1  );
	}
	else {
		mDist = new TriangleDist( begin.AsInt(), mode.AsInt(), end.AsInt() );
	}
}

//----------------------------------------------------------------------------
// Junk distribution
//----------------------------------------------------------------------------

RandTime :: ~RandTime() {
	delete mDist;
}

//----------------------------------------------------------------------------
// Get random int & transform into time rep
//----------------------------------------------------------------------------

Row RandTime :: Get() {
	int n = mDist->NextInt();
	TimeRep t = mBegin;
	t.Inc( n );
	return Order( Row( t.Str() ) );
}

//----------------------------------------------------------------------------
// Size always 1
//----------------------------------------------------------------------------

int RandTime :: Size() {
	return 1;
}

//----------------------------------------------------------------------------
// Create from XML
//----------------------------------------------------------------------------

DataSource * RandTime :: FromXML( const ALib::XMLElement * e ) {
	ForbidChildren( e );
	AllowAttrs( e, AttrList( BEGIN_ATTRIB, END_ATTRIB, MODE_ATTR,
								ORDER_ATTRIB, 0 ));
	bool havemode = false;
	TimeRep begin, end, mode;
	try {
		if ( e->HasAttr( BEGIN_ATTRIB ) || e->HasAttr( END_ATTRIB ) ) {
			begin = TimeRep( e->AttrValue( BEGIN_ATTRIB ) );
			end = TimeRep( e->AttrValue( END_ATTRIB ) );
			if ( begin.AsInt() >= end.AsInt() ) {
				throw Exception( "begin must be before end" );
			}
		}

		if ( e->HasAttr( MODE_ATTR ) ) {
			havemode = true;
			mode = TimeRep( e->AttrValue( MODE_ATTR ) );
		}
	}
	catch( const Exception & ex ) {
		XMLERR( e, ex.what() );
	}

//	std::cout << begin.Str() <<" " << end.Str() << std::endl;

	if ( havemode ) {
		return new RandTime( GetOrder( e ), begin, end, mode );
	}
	else {
		return new RandTime( GetOrder( e ), begin, end );
	}
}

//----------------------------------------------------------------------------

}	// namespace
