//---------------------------------------------------------------------------
// dmk_datafile.cpp
//
// Data source which gets its data from a file of CSV data.
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_str.h"
#include "a_collect.h"
#include "a_opsys.h"
#include "dmk_source.h"
#include "dmk_tagdict.h"
#include "dmk_xmlutil.h"
#include "dmk_strings.h"
#include "dmk_random.h"
#include <fstream>

using std::string;
using std::vector;

namespace DMK {

//----------------------------------------------------------------------------

const char * const DATAFILE_TAG 	= "datafile";

//----------------------------------------------------------------------------
// Datafile class provides access to a CSV data file
//----------------------------------------------------------------------------

class DSDataFile : public DataSource {

	public:

		DSDataFile( const std::string & filename,
					bool random,
					const FieldList & order );

		Row Get();
		int Size();
		void Discard();
		void Reset();

		static DataSource * FromXML( const ALib::XMLElement * e );

	private:

		void Populate();
		string mFilename;
		unsigned int mPos;
		bool mRandom;
		Strings mLines;

};

//----------------------------------------------------------------------------
// Register with tag dictionary
//----------------------------------------------------------------------------

static RegisterDS <DSDataFile> regrs1_( DATAFILE_TAG );

//----------------------------------------------------------------------------
// Create from filename, which is currently relative to the data directory
// in the distribution root.
//----------------------------------------------------------------------------

DSDataFile :: DSDataFile( const string & filename,
							bool random,
							const FieldList & order )
	: DataSource( order ), mFilename( filename ),
		mPos( 0 ), mRandom( random ) {
}

//----------------------------------------------------------------------------
// Read into memory if not already read then select record either at random
// or using current position.
//----------------------------------------------------------------------------

Row DSDataFile :: Get() {
	Populate();
	if ( mRandom ) {
		Row r( mLines[ RNG::Random() % mLines.size() ] );
		return Order( r );
	}
	else {
		Row r( mLines[ mPos++ ] );
		mPos %= mLines.size();
		return Order( r );
	}
}

//----------------------------------------------------------------------------
// Size is number of CSV records read from file
//----------------------------------------------------------------------------

int DSDataFile :: Size() {
	Populate();
	return mLines.size();
}

//----------------------------------------------------------------------------
// start again
//----------------------------------------------------------------------------

void DSDataFile :: Reset() {
	mPos = 0;
}

//----------------------------------------------------------------------------
// no longer need lines
//----------------------------------------------------------------------------

void DSDataFile :: Discard() {
	mLines.clear();
}

//----------------------------------------------------------------------------
// Populate if not already done so
//----------------------------------------------------------------------------

void DSDataFile :: Populate() {
	if ( mLines.size() ) {
		return;
	}

	// hard code data path for now
	string fpath;
	if ( ALib::Peek( mFilename, 0 ) == '.' ) {
		fpath = mFilename;
	}
	else {
		fpath = ALib::ExePath() + "/data/" + mFilename;
	}
	std::ifstream ifs( fpath.c_str() );
	if ( ! ifs.is_open() ) {
		throw Exception( "Cannot open file " + fpath + " for input" );
	}
	string line;
	while( std::getline( ifs, line ) ) {
		if ( ! ALib::IsEmpty( line ) ) {
			mLines.push_back( line );
		}
	}

	if ( mLines.size() == 0 ) {
		throw Exception( "File " + mFilename + " is empty" );
	}
}

//----------------------------------------------------------------------------
// Attribute "values" can contain comma-separated list of values. More
// values can also be specified as lines of text content.
//----------------------------------------------------------------------------

DataSource * DSDataFile :: FromXML( const ALib::XMLElement * e ) {

	ForbidChildren( e );
	RequireAttrs( e, FILE_ATTRIB );
	AllowAttrs( e, AttrList( FILE_ATTRIB, ORDER_ATTRIB, RANDOM_ATTRIB, 0 ));

	string f = e->AttrValue( FILE_ATTRIB );
	bool random = GetRandom( e );
	FieldList order = GetOrder( e );

	std::auto_ptr <DSDataFile> df( new DSDataFile( f, random, order ) );
	return df.release();
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------
// Testing
//----------------------------------------------------------------------------

#ifdef DMK_TEST

#include "a_myth.h"
using namespace ALib;
using namespace DMK;

DEFSUITE( "DSDataFile" );

DEFTEST( digits ) {

	string XML = "<datafile file='data/digits.dat' />";
	XMLPtr xml( XML );
	DSDataFile * df = (DSDataFile*) DSDataFile::FromXML( xml );
	Row r = df->Get();
	FAILNE( r.Size(), 2 );
	FAILNE( r.At(1), "zero" );
	r = df->Get();
	FAILNE( r.Size(), 2 );
	FAILNE( r.At(1), "one" );
}

#endif

//----------------------------------------------------------------------------

// end

