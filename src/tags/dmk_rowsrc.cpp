//---------------------------------------------------------------------------
// dmk_rowsrc.cpp
//
// Data sources which produce single and multiople rows.
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_str.h"
#include "a_collect.h"
#include "dmk_source.h"
#include "dmk_tagdict.h"
#include "dmk_xmlutil.h"
#include "dmk_random.h"
#include "dmk_strings.h"

using std::string;
using std::vector;

namespace DMK {

//----------------------------------------------------------------------------

const char * const ROW_TAG 		= "row";
const char * const VALUES_ATTRIB 	= "values";
const char * const ROWS_TAG 		= "rows";
const char * const FREQ_ATTR 		= "freq";

//----------------------------------------------------------------------------
// Single row as comma separated list. Each field  is a field in the output
//----------------------------------------------------------------------------

class DSRow : public DataSource {

	public:

		DSRow( const FieldList & order  );

		Row Get();
		int Size();
		void Reset() {}		// do nothing
		static DataSource * FromXML( const ALib::XMLElement * e );

	private:

		Row mRow;
};

//----------------------------------------------------------------------------
// Multiple rows as comma separated list - each field is a row in output.
// Alternatively, multiple rows specified as cdata.
//----------------------------------------------------------------------------

class DSRows : public DataSource {

	public:

		DSRows( const FieldList & order );

		Row Get();
		int Size();
		void Reset();

		static DataSource * FromXML( const ALib::XMLElement * e );

	private:

		Row FreqGet();

		bool mRandom;
		int mPos;
		Rows mRows;
		std::vector <int> mFreqs;
};

//----------------------------------------------------------------------------
// Register sources
//----------------------------------------------------------------------------

static RegisterDS <DSRow> regrs1_( ROW_TAG );
static RegisterDS <DSRows> regrs2_( ROWS_TAG );

//----------------------------------------------------------------------------
// Single row source
//----------------------------------------------------------------------------

DSRow :: DSRow( const FieldList & order )
	: DataSource( order ) {
}

//----------------------------------------------------------------------------
// Get single row
//----------------------------------------------------------------------------

Row DSRow :: Get() {
	return Order( mRow );
}

//----------------------------------------------------------------------------
// Size is always 1 - single row
//----------------------------------------------------------------------------

int DSRow :: Size() {
	return 1;
}

//----------------------------------------------------------------------------
// Attribute "values" can contain comma-separated list of values. More
// values can also be specified as lines of text content.
//----------------------------------------------------------------------------

DataSource * DSRow :: FromXML( const ALib::XMLElement * e ) {

	AllowChildTags( e, "" );
	AllowAttrs( e, AttrList( ORDER_ATTRIB, VALUES_ATTRIB, 0 ) );
	ALib::CommaList cl = ListFromAttrib( e, VALUES_ATTRIB );
	for ( unsigned int i = 0; i < e->ChildCount(); i++ ) {
		if ( const ALib::XMLText * t = e->ChildText( i ) ) {
			cl.Append( ALib::Trim( t->Text() ) );
		}
	}

	std::auto_ptr <DSRow> r( new DSRow( GetOrder( e ) ) );
	r->mRow.AppendStrings( cl.Items() );
	if ( r->mRow.Size() == 0 ) {
		r->mRow.AppendValue( "" );
	}
	return r.release();
}

//----------------------------------------------------------------------------
// Multiple row source
//----------------------------------------------------------------------------

DSRows :: DSRows( const FieldList & order )
		: DataSource( order ), mRandom( false ), mPos( 0 )  {
}

//----------------------------------------------------------------------------
// Get row. If frequency field is specified, use that.
//----------------------------------------------------------------------------

Row DSRows :: Get() {

	if ( mFreqs.size()  && mRandom ) {
		return FreqGet();
	}

	int i;
	if ( mRandom ) {
		i = RNG::Random() % mRows.size();
	}
	else {
		i = mPos++;
		mPos %= mRows.size();
	}
	return Order( mRows[i] );
}

//----------------------------------------------------------------------------
// Frequency was specified. Use that field to select row. This is only used
// if we are in nrandom mode.
//----------------------------------------------------------------------------

Row DSRows :: FreqGet() {
	int n = RNG::Random( 0, 100 ), sum = 0;
	for ( unsigned int i = 0; i < mFreqs.size(); i++ ) {
		sum += mFreqs[i];
		if ( n < sum ) {
			return Order( mRows[i] );
		}
	}
	throw Exception( "freq problem" );
}

//----------------------------------------------------------------------------
// Start again - for non-random access
//----------------------------------------------------------------------------

void DSRows :: Reset() {
	mPos = 0;
}

//----------------------------------------------------------------------------
// Size is same whether in random mode or not
//----------------------------------------------------------------------------

int DSRows :: Size() {
	return mRows.size();
}

//----------------------------------------------------------------------------
// Create from xml
// ??? needs refactoring ???
//----------------------------------------------------------------------------

DataSource * DSRows :: FromXML( const ALib::XMLElement * e ) {

	AllowChildTags( e, "" );
	AllowAttrs( e, AttrList( ORDER_ATTRIB, VALUES_ATTRIB,
								RANDOM_ATTRIB, FREQ_ATTR, 0 ) );

	ALib::CommaList vl = e->AttrValue( VALUES_ATTRIB, "" );
	int freq = GetInt( e, FREQ_ATTR, "0" ) - 1;
	std::auto_ptr <DSRows> rp( new DSRows( GetOrder( e ) ) );
	rp->mRandom = GetRandom( e );;

	// rows from value attribute
	for ( unsigned int i = 0; i < vl.Size() ; i++ ) {
		rp->mRows.push_back( Row( vl.At(i) ) );
	}

	// rows as cdata
	for ( unsigned int i = 0; i < e->ChildCount() ; i++ ) {
		const ALib::XMLText * t = e->ChildText( i );
		if ( t ) {
			rp->mRows.push_back( Row( ALib::Trim( t->Text() ) ) );
		}
	}

	// if no rows specified, generates empty row
	if ( rp->mRows.size() == 0 ) {
		rp->mRows.push_back( Row("") );
	}

	// if frequency column number specified, save frequencies and
	// remove frequency column from row
	if ( freq >= 0 ) {
		int sum = 0;
		for ( unsigned int i = 0; i < rp->mRows.size();i++ ) {
			Row r = rp->mRows.at( i );
			if ( r.Size() <= freq || ! ALib::IsInteger( r.At(freq) ) ) {
				XMLERR( e, "No valid frequency value in row " << r );
			}
			int fv = ALib::ToInteger( r.At( freq ) );
			if ( fv < 0 || fv > 100 ) {
				XMLERR( e, "Frequency value must be in range 0 to 100" << r );
			}
			rp->mRows.at(i).Erase( freq );
			rp->mFreqs.push_back( fv );
			sum += fv;
		}
		if ( sum != 100 ) {
			XMLERR( e, "Frequencies must sum to 100%" );
		}
	}

	return rp.release();
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

#ifdef DMK_TEST

#include "a_myth.h"
using namespace ALib;
using namespace DMK;

DEFSUITE( "Rows" );

DEFTEST( FromXML1 ) {
	string xml = "<row values='foo,bar' random='no' />";
	XMLPtr xp( xml );
	DSRow * dsr = (DSRow *) DSRow::FromXML( xp );
	Row r = dsr->Get();
	FAILNE( r.Size(), 2 );
	FAILNE( r.At(0), "foo" );
	FAILNE( r.At(1), "bar" );
}

DEFTEST( FromXML2 ) {
	string xml = 		"<row values='foo,bar'  random='no' >\n"
							"alpha\n"
							"beta\n"
						"</row>";
	XMLPtr xp( xml );
	DSRow * dsr = (DSRow *) DSRow::FromXML( xp );
	Row r = dsr->Get();
	FAILNE( r.Size(), 4 );
	FAILNE( r.At(0), "foo" );
	FAILNE( r.At(1), "bar" );
	FAILNE( r.At(2), "alpha" );
	FAILNE( r.At(3), "beta" );
}

DEFTEST( FromXML3 ) {
	string xml = "<rows values='foo,bar'  random='no' />";
	XMLPtr xp( xml );
	DSRows * dsr = (DSRows *) DSRows::FromXML( xp );
	Row r = dsr->Get();
	FAILNE( r.Size(), 1 );
	FAILNE( r.At(0), "foo" );
	r = dsr->Get();
	FAILNE( r.At(0), "bar" );
}


#endif

//----------------------------------------------------------------------------

// end

