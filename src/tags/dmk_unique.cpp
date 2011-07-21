//---------------------------------------------------------------------------
// dmk_unique.cpp
//
// Unique filter for DMK
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
#include <set>

using std::string;
using std::vector;

namespace DMK {

//----------------------------------------------------------------------------

const char * const UNIQUE_TAG 			= "unique";
const char * const RETRY_ATTRIB 		= "retry";
const int UNIQUE_RETRY 				= 100;		// retry count

//----------------------------------------------------------------------------

class DSUnique : public CompositeDataSource {

	public:

		// do comparisons of two fields taking field list into account
		struct CmpRows {
			CmpRows( const FieldList & fl ) : mFields( fl ) {}
			bool operator()( const Row & r1, const Row & r2 ) const {
				bool n = Cmp( r1, r2, mFields ) < 0;
				return n;
			}
			FieldList mFields;
		};

		DSUnique( const FieldList & order, const FieldList & cmpf, int retry );

		int Size();
		Row Get();

		void Discard();

		static DataSource * FromXML( const ALib::XMLElement * e );

	private:

		typedef std::set <Row,CmpRows> SetType;
		SetType mUniqueRows;
		vector <Row> mRows;
		int mPos, mRetry;
};

//----------------------------------------------------------------------------

static RegisterDS <DSUnique> regrs1_( UNIQUE_TAG );

DSUnique :: DSUnique( const FieldList & order,
						const FieldList & cmpfields,
						int retry )
	: CompositeDataSource( order ),
		mUniqueRows( cmpfields ), mPos( -1 ), mRetry( retry + 1) {
}

// if we receive size message read rows from kids discarding dupes
// and use those rows to fufill future requests
int DSUnique :: Size() {

	if ( mPos >= 0 ) {			// already have size
		return mRows.size();
	}

	int n = CompositeDataSource::Size();
	while( n-- ) {
		Row r = CompositeDataSource::Get();
		if ( mUniqueRows.find( r ) == mUniqueRows.end() ) {
			mUniqueRows.insert( r );
			mRows.push_back( r );
		}
	}
	//ALib::Dump( std::cout, mRows );
	mPos = 0;
	if ( mRows.size() == 0 ) {
		throw Exception( "Empty result set" );
	}
	return mRows.size();
}

// if we ghave already fetched the rows because we recieved a
// size message, return one of those rows. otherwise try to
// get a unique row anf fail noisily if that seems not possible
Row DSUnique :: Get() {
	if ( mPos >= 0 ) {
		Row r = mRows[ mPos++];
		mPos %= mRows.size();
		return Order( r );
	}
	else {
		int n = mRetry;
		while( n-- ) {
			Row r = CompositeDataSource::Get();
			if ( mUniqueRows.find( r ) == mUniqueRows.end() ) {
				mUniqueRows.insert( r );
				return Order( r );
			}
		}
		throw Exception( "cannot find enough unique values" );
	}
}

// discard needs to chuck away saved rows
void DSUnique :: Discard() {
	mRows.clear();
}

// unique supports the following  attributes:
//	order	- usual stuff
//	fields	- list of fields to consider for uniqueness
//	retry	- number of time sto retry getting unique row
DataSource * DSUnique :: FromXML( const ALib::XMLElement * e ) {

	RequireChildren( e );
	AllowAttrs( e, AttrList( ORDER_ATTRIB,FIELDS_ATTRIB, RETRY_ATTRIB, 0 ) );

	string fl = e->AttrValue( FIELDS_ATTRIB, "" );
	FieldList cmpf( fl );
	int retry = GetInt( e, RETRY_ATTRIB, ALib::Str( UNIQUE_RETRY) );
	if ( retry < 0 ) {
		throw XMLError( ALib::SQuote( RETRY_ATTRIB ) + " cannot be negative", e );
	}

	std::auto_ptr <DSUnique> c( new DSUnique( GetOrder( e ), cmpf , retry ));

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

DEFSUITE( "Unique" );

const char * const XML1 =
	"<unique fields='1'>\n"
		"<row values='1,2' />\n"
		"<row values='three' />\n"
	"</unique>\n";

DEFTEST( Simple ) {
	XMLPtr xml( XML1 );
	DSUnique * p = (DSUnique *) DSUnique::FromXML( xml );
	Row r = p->Get();
	FAILNE( r.Size(), 3 );
	FAILNE( r.At(1), "2" );
}


#endif

//----------------------------------------------------------------------------

// end

