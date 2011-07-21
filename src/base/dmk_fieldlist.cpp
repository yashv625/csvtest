//---------------------------------------------------------------------------
// dmk_fieldlist.cpp
//
// list of field indexes for DMK
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_str.h"
#include "dmk_fieldlist.h"
#include <iostream>
#include <algorithm>

using std::string;
using std::vector;

namespace DMK {

//----------------------------------------------------------------------------
// validate integer field index, returning zero-based index
//----------------------------------------------------------------------------

static int ValidateField( const std::string & f ) {
	if ( ! ALib::IsInteger( f ) ) {
		throw Exception( "Field index must be integer, not "
								+ ALib::SQuote( f ) );
	}
	int n = ALib::ToInteger( f );
	if ( n <= 0 ) {
		throw Exception( "Field index must be greater than zero" );
	}
	return n - 1;
}

//----------------------------------------------------------------------------
// Create list from comma-separated list of 1-based field indexes. If no
// list s provided, "all fields" is implied.
//----------------------------------------------------------------------------

FieldList :: FieldList( const string & s ) {
	ALib::CommaList cl( s );
	for ( unsigned int i = 0; i < cl.Size(); i++ ) {
		mFields.push_back( ValidateField( cl.At( i ) ));	// to zero-based
	}
}

//----------------------------------------------------------------------------
// Clear index
//----------------------------------------------------------------------------

FieldList :: ~FieldList() {
	Clear();
}

void FieldList :: Clear() {
	mFields.clear();
}

//----------------------------------------------------------------------------
// simple accessors
//----------------------------------------------------------------------------

unsigned int FieldList :: Size() const {
	return mFields.size();
}

unsigned int FieldList :: At( unsigned int i ) const {
	return mFields.at( i );
}

//----------------------------------------------------------------------------
// Does list contain zero-based index?
//----------------------------------------------------------------------------

bool FieldList :: Contains( unsigned int idx ) const {
	return std::find( mFields.begin(), mFields.end(), idx ) != mFields.end();
}

//----------------------------------------------------------------------------
// Given a row, re-order it using the contents of the field list. If a field
// does not xist, use the emptyval value.
//----------------------------------------------------------------------------

Row FieldList :: OrderRow( const Row & r, const string & emptyval  ) const {

	if ( Size() == 0 ) {
		return r;
	}
	else {
		Row tmp;
		for ( unsigned int i = 0; i < mFields.size(); i++ ) {
			unsigned int idx = mFields[i];
			if ( idx <= r.Size() ) {
				tmp.AppendValue( r.At( idx ) );
			}
			else {
				tmp.AppendValue( emptyval );
			}
		}
		return tmp;
	}
}

//----------------------------------------------------------------------------
// Field pairs are a comma separted list of pairs of field indexes used
// for joins, lookups etc. Example: "1:1,2:4"
//----------------------------------------------------------------------------

FieldPairs :: FieldPairs( const string & fp ) {
	vector <string> pairs;
	if ( ALib::Split( fp, ',', pairs ) == 0 ) {
		return;
	}
	for ( unsigned int i = 0; i < pairs.size(); i++ ) {
		vector <string> pair;
		if ( ALib::Split( pairs[i], ':', pair ) != 2 ) {
			throw Exception( "Invalid pair " + ALib::SQuote( pairs[i] ) );
		}
		mLeft.push_back( ValidateField( pair[0] ) );
		mRight.push_back( ValidateField( pair[1] ) );
	}
}

//----------------------------------------------------------------------------
// both vectors are same size
//----------------------------------------------------------------------------

unsigned int FieldPairs :: Size() const {
	return mLeft.size();
}


unsigned int FieldPairs :: AtLeft( unsigned int i ) const {
	return mLeft.at( i );
}

unsigned int FieldPairs :: AtRight( unsigned int i ) const {
	return mRight.at( i );
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

DEFSUITE( "Fields" );

DEFTEST( FieldListCtor ) {
	FieldList fl;
	FAILNE( fl.Size(), 0 );
	fl = FieldList( "2,1,3" );
	FAILNE( fl.Size(), 3 );
	FAILNE( fl.At(1), 0 );
}

DEFTEST( FieldListOrder ) {
	FieldList f( "3,1,2,666" );
	Row r1( "one,two,three" );
	Row r2 = f.OrderRow( r1 );
	FAILNE(  r2.Size(), 4 );
	FAILNE( r2[0], "three" );
	FAILNE( r2[1], "one" );
	FAILNE( r2[2], "two" );
	FAILNE( r2[3], "" );

	FieldList f2;
	r2 = f2.OrderRow( r1 );
	FAILNE( r2[0], "one" );
	FAILNE( r2[1], "two" );
	FAILNE( r2[2], "three" );
}

DEFTEST( FieldPairsTest ) {
	FieldPairs fp( "1:1,3:4" );
	FAILNE( fp.Size(), 2 );
	FAILNE( fp.AtLeft( 0 ), 0 );
	FAILNE( fp.AtRight( 0 ), 0 );
	FAILNE( fp.AtLeft( 1 ), 2 );
	FAILNE( fp.AtRight( 1 ), 3 );

}
#endif

//----------------------------------------------------------------------------


// end

