//---------------------------------------------------------------------------
// dmk_row.cpp
//
// A row is the basic DMK data structure, passed around between data sources
// and filters. As it is copied a lot, we make it reference counted.
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_csv.h"
#include "dmk_row.h"
#include <iostream>
#include "dmk_fieldlist.h"

using std::string;
using std::vector;

namespace DMK {

//----------------------------------------------------------------------------

//#define DMK_ROW_DEBUG			// turns debug output on/off

//----------------------------------------------------------------------------
// This class provides the actual reference counted representation for rows.
// DMK is resolutely single-threaded so there are no synch issues here.
//----------------------------------------------------------------------------

class RowRep {

	public:

		RowRep() : mRefCount(1) {
#ifdef DMK_ROW_DEBUG
			std::cout << "Rep ctor count: " << ++mInstCount << std::endl;
#endif
		}

		~RowRep() {
#ifdef DMK_ROW_DEBUG
			std::cout << "Rep dtor count: " << --mInstCount << std::endl;
#endif
		}

		unsigned int RefCount() const {
			return mRefCount;
		}

		unsigned int IncRefCount() {
			return ++mRefCount;
		}

		unsigned int DecRefCount() {
			return --mRefCount;
		}

		vector<string> & Values()  {
			return mValues;
		}

		static RowRep * Copy( RowRep * p ) {
			p->mRefCount--;
			RowRep * tmp = new RowRep;
			tmp->mValues = p->mValues;
			return tmp;
		}

	private:

		unsigned int mRefCount;
		vector <string> mValues;
		static int mInstCount;
};

//----------------------------------------------------------------------------

// Count of all current instances of row & rep - used for debug only.
int RowRep::mInstCount = 0;
int Row::mInstCount = 0;

//----------------------------------------------------------------------------
// Default ctor creates row with no columns.
//----------------------------------------------------------------------------

Row :: Row() : mRep ( 0 ) {
#ifdef DMK_ROW_DEBUG
	std::cout << "Row ctor count: " << ++mInstCount << std::endl;
#endif
	mRep = new RowRep;
}

//----------------------------------------------------------------------------
// Destroy the rep only if we are its last user
//----------------------------------------------------------------------------

Row :: ~Row() {
#ifdef DMK_ROW_DEBUG
	std::cout << "Row dtor count: " << --mInstCount << std::endl;
#endif
	if ( mRep->DecRefCount() == 0 ) {
		delete mRep;
	}
}

//----------------------------------------------------------------------------
// construct from csv string
//----------------------------------------------------------------------------

Row :: Row( const string & csv ) : mRep ( 0 ){
	mRep = new RowRep;
	ALib::CSVLineParser lp;
	lp.Parse( csv, mRep->Values() );
#ifdef DMK_ROW_DEBUG
	std::cout << "Row ctor count: " << ++mInstCount << std::endl;
#endif
}

//----------------------------------------------------------------------------
// ctor - Copy a row by incrementing ref count
//----------------------------------------------------------------------------

Row :: Row( const Row & row ) : mRep( row.mRep ) {
	mRep->IncRefCount();
#ifdef DMK_ROW_DEBUG
	std::cout << "Row ctor count: " << ++mInstCount << std::endl;
#endif
}

//----------------------------------------------------------------------------
// Assign one row to another, checking for self-assignment (in which case
// we do nothing)
//----------------------------------------------------------------------------

Row & Row :: operator = ( const Row & row ) {
	if ( row.mRep  != this->mRep ) {
		if ( mRep->DecRefCount() == 0 ) {
			delete mRep;
		}
		mRep = row.mRep;
		mRep->IncRefCount();
	}
	return *this;
}

//----------------------------------------------------------------------------
// Access row as vector. Note rows are immutable via this interface.
//----------------------------------------------------------------------------

unsigned int Row :: Size() const {
	return mRep->Values().size();
}

//----------------------------------------------------------------------------
// Is row empty?
//----------------------------------------------------------------------------

bool Row :: IsEmpty() const {
	return mRep->Values().size() == 0;
}

//----------------------------------------------------------------------------
// Do all access via at() to get range checking.
//----------------------------------------------------------------------------

const string & Row :: operator[] ( unsigned int  i ) const {
	return At( i );
}

const string & Row :: At( unsigned int  i ) const {
	return mRep->Values().at( i );
}

//----------------------------------------------------------------------------
// Erase specified column - zero based.
//----------------------------------------------------------------------------

void Row :: Erase( unsigned int col ) {
	if ( col >= mRep->Values().size() ) {
		throw Exception( "Invalid column in Row::Erase" );
	}
	if ( mRep->RefCount() > 1 ) {
		mRep = RowRep::Copy( mRep );
	}
	mRep->Values().erase( mRep->Values().begin() + col );
}

//----------------------------------------------------------------------------
// Append a single field
//----------------------------------------------------------------------------

Row & Row :: AppendValue( const string & val ) {
	if ( mRep->RefCount() > 1 ) {
		mRep = RowRep::Copy( mRep );
	}
	mRep->Values().push_back( val );
	return *this;
}

//----------------------------------------------------------------------------
// Append from string containing CSV values, ach value becomes a new field
//----------------------------------------------------------------------------

Row & Row :: AppendCSV( const string & csv ) {
	Row r( csv );
	AppendRow( r );
	return *this;
}

//----------------------------------------------------------------------------
// Append from vector of strings - each string is new field
//----------------------------------------------------------------------------

Row & Row :: AppendStrings( const Strings & s ) {
	if ( mRep->RefCount() > 1 ) {
		mRep = RowRep::Copy( mRep );
	}
	for ( unsigned int i = 0; i < s.size(); i++ ) {
		mRep->Values().push_back( s[i] );
	}
	return *this;
}

//----------------------------------------------------------------------------
// Append one row to another. Possibly we are appending to ourself, which is
// an OK thing to do, though infrequent in DMK code, so no need to be very
//  efficient!
//----------------------------------------------------------------------------

Row & Row :: AppendRow( const Row & row ) {

	if ( mRep == row.mRep ) {					// self append
		vector <string> tmp( mRep->Values() );	// must be from copy
		for ( unsigned int i = 0; i < tmp.size(); i++ ) {
			AppendValue( tmp[i] );
		}
	}
	else {										// normal append
		for ( unsigned int i = 0; i < row.Size(); i++ ) {
			AppendValue( row[i] );
		}
	}
	return *this;
}

//----------------------------------------------------------------------------
// Return contents as CSV string
//----------------------------------------------------------------------------

string Row :: AsCSV() const {
	string s;
	for ( unsigned int i = 0; i < Size(); i++ ) {
		if ( s.size() > 0 ) {
			s += ',';
		}
		s += ALib::CSVQuote( At( i ) );
	}
	return s;
}

//----------------------------------------------------------------------------
// Debug output stuff
//----------------------------------------------------------------------------

std::ostream & operator << ( std::ostream & os, const Row & row ) {
	for ( unsigned int i = 0; i < row.Size(); i++ ) {
		os << "[" << row[i] << "]";
	}
	return os;
}

//----------------------------------------------------------------------------
// Comparisons
//----------------------------------------------------------------------------

bool operator < ( const Row & r1, const Row & r2 ) {
	return Cmp( r1, r2, FieldList() ) < 0;
}

bool operator == ( const Row & r1, const Row & r2 ) {
	return Cmp( r1, r2, FieldList() ) == 0;
}

bool operator != ( const Row & r1, const Row & r2 ) {
	return ! (r1 == r2);
}

//----------------------------------------------------------------------------
// Cmp returns as for strcmp. if the field list is non-empty only
// consider the fields it contains.
//----------------------------------------------------------------------------

int Cmp( const Row & r1, const Row & r2, const FieldList & fl ) {
	if ( r1.Size() <= r2.Size()  ) {
		for ( unsigned int i = 0; i < r1.Size() ; i++ ) {
			if ( fl.Size() == 0 || fl.Contains( i ) ) {
				if ( r1.At( i ) < r2.At(i) ) {
					return -1;
				}
				else if ( r1.At( i ) > r2.At(i) ) {
					return 1;
				}
			}
		}
		return r1.Size() < r2.Size() ? -1 : 0;
	}
	else {
		for ( unsigned int i = 0; i < r2.Size() ; i++ ) {
			if ( fl.Size() == 0 || fl.Contains( i ) ) {
				if ( r1.At( i ) < r2.At(i) ) {
					return -1;
				}
				else if ( r1.At( i ) > r2.At(i) ) {
					return 1;
				}
			}
		}
		return 1;
	}
}

//----------------------------------------------------------------------------
// Do nothing row source ctor & dtor
//----------------------------------------------------------------------------

RowSource :: RowSource() {
}

RowSource :: ~RowSource() {
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

DEFSUITE( "Row" );

// can create & copy? - may well segfault if ref counting is buggy.
DEFTEST( Ctor ) {
	Row r1;
	FAILNE( r1.Size(), 0 );
	Row r2 = r1;
	FAILNE( r2.Size(), 0 );
	r1 = r2;
	FAILNE( r2.Size(), 0 );
	r2 = Row( "stuff" );
	FAILNE( r2.Size(), 1 );
	r2 = Row( "one,two,three" );
	FAILNE( r2.Size(), 3 );
}

// Check basic access to values
DEFTEST( Values ) {
	Row r1( "one,two,three" );
	FAILNE(  r1.Size(), 3 );
	FAILNE(  r1[1], "two" );
}

// Test append variants
DEFTEST( Append ) {
	Row r1( "one,two,three" );
	r1.AppendValue( "four" );
	FAILNE( r1.Size(), 4 );
	FAILNE( r1[3], "four" );
	r1.AppendCSV( "five,six" );
	FAILNE(  r1.Size(), 6 );
	FAILNE( r1[5], "six" );
	Row r2( "seven,eight" );
	r1.AppendRow( r2 );
	FAILNE( r1.Size(), 8 );
	FAILNE( r1[7], "eight" );
	// self append - this is not a stupid to do
	r1.AppendRow( r1 );
}

// test comparisons
DEFTEST( Compare ) {
	Row r1( "one,two,three" );
	Row r2( "one,two,three" );
	FAILNE( Cmp(r1, r2, FieldList() ), 0 );
	Row r3( "one,two" );
	FAILNE( Cmp( r1, r3, FieldList() ), 1 );
	FAILNE( Cmp( r3, r1, FieldList() ), -1 );
	Row r4( "one,two,four" );
	FAILNE( Cmp(r1, r4, FieldList("1,2") ), 0 );
}

#endif

//----------------------------------------------------------------------------

// end

