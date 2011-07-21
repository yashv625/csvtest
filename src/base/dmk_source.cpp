//---------------------------------------------------------------------------
// dmk_source.cpp
//
// Base class for data sources & simplest possible row-based data source
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_str.h"
#include "a_collect.h"
#include "dmk_source.h"
#include "dmk_tagdict.h"
#include "dmk_random.h"

using std::string;
using std::vector;

namespace DMK {

//----------------------------------------------------------------------------

DataSource :: DataSource( const FieldList & order )
	: mOrder( order ) {
}

// default discard action is to do nothing
void DataSource :: Discard() {
}

Row DataSource :: Last() const {
	return mLast;
}

Row DataSource :: SaveLast( const Row & row ) const {
	return mLast = row;
}

Row DataSource :: Order( const Row & row ) const {
	Row r = mOrder.OrderRow( row );
	return SaveLast( r );
}

//----------------------------------------------------------------------------

CompositeDataSource :: CompositeDataSource( const FieldList & order )
	: DataSource( order ) {
}

CompositeDataSource :: ~CompositeDataSource() {
	ALib::FreeClear( mSources );
}

void CompositeDataSource :: AddSource( DataSource * src ) {
	mSources.push_back( src );
}

unsigned int CompositeDataSource :: SourceCount() const {
	return mSources.size();
}

DataSource * CompositeDataSource :: SourceAt( unsigned int i ) const {
	return mSources.at( i );
}

Row CompositeDataSource :: Get() {
	Row r;
	for ( unsigned int i = 0; i < SourceCount(); i++ ) {
		r.AppendRow( SourceAt( i )->Get() );
	}
	return Order( r );
}

// size is the recursive maximum of all child sources
// calling size on a source may cause it to populate itself
int CompositeDataSource :: Size() {
	int sz = INT_MIN;
	for ( unsigned int i = 0; i < SourceCount(); i++ ) {
		sz = std::max( sz, SourceAt(i)->Size() );
	}
	return sz;
}

// reset all kids
void CompositeDataSource :: Reset() {
	for ( unsigned int i = 0; i < SourceCount(); i++ ) {
		SourceAt(i)->Reset();
	}
}

// discard informs a sourcev that it can discard any buffered
// rows (or other data) it may be hanging on to
void CompositeDataSource :: Discard() {
	for ( unsigned int i = 0; i < SourceCount(); i++ ) {
		SourceAt( i )->Discard();
	}
}

// add sources represented by child elements of 'e'
void CompositeDataSource :: AddChildSources( const ALib::XMLElement * e ) {
	for ( unsigned int i = 0; i < e->ChildCount(); i++ ) {
		const ALib::XMLElement * ce = e->ChildElement(i);
		if ( ce ) {
			std::auto_ptr <DataSource> ds(
				TagDictionary::Instance()->CreateDS( ce )
			);
			AddSource( ds.release() );
		}
	}
}

//----------------------------------------------------------------------------

// rm indicates if we want Get() to provide randomised rows
Intermediate :: Intermediate( const FieldList & order, bool rm  )
	: CompositeDataSource( order ), mRand( rm ), mPos( 0 ) {
}

// nothing yet
Intermediate :: ~Intermediate() {
}


// helper to only call Populate PVF if we actually need it
void Intermediate :: SafePopulate() {
	if ( mRows.size() == 0 ) {
		Populate();
	}
}

// get a row in random or sequential mode
Row Intermediate :: Get() {

	SafePopulate();

	if ( mRows.size() == 0 ) {
		throw Exception( "Empty result set" );
	}

	if ( ! mRand ) {
		Row r = mRows[mPos++];
		mPos %= mRows.size();
		return Order( r );
	}
	else {
		int i = RNG::Random() % mRows.size();
		return Order( mRows[i] );
	}
}

// need to populate in order to get size
int Intermediate :: Size() {
	SafePopulate();
	return mRows.size();
}

// results no longer needed
void Intermediate :: Discard()  {
	mRows.clear();
	CompositeDataSource::Discard();
}

// derived classes use this to add their rows
void Intermediate :: AddRow( const Row & row ) {
	mRows.push_back( row );
}

// pritected accessor needed for some derived functionality
const Rows & Intermediate :: ResultRows() const {
	return mRows;
}

//----------------------------------------------------------------------------

SourceList :: SourceList() {
}

SourceList ::~SourceList() {
	Clear();
}

void SourceList ::Add( DataSource * ds ) {
	mSources.push_back( ds );
}

void SourceList ::Clear() {
	ALib::FreeClear( mSources );
}

unsigned int SourceList ::Size() const {
	return mSources.size();
}

const DataSource * SourceList :: At( unsigned int i ) const {
	return mSources.at( i );
}

DataSource * SourceList :: At( unsigned int i ) {
	return mSources.at( i );
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

DEFSUITE( "Source" );

//----------------------------------------------------------------------------
// Simple derived class. alwayys returns number of beast
//----------------------------------------------------------------------------

class SourceBeast : public DMK::DataSource {

	public:

		SourceBeast() : DMK::DataSource(FieldList( "1,1,1" ) ) {}

		int Size() { return 1; }
		void Reset() {}

		Row Get() {
			Row r( "6" );
			return Order( r );
		}
};

//----------------------------------------------------------------------------

DEFTEST( Ctor ) {
	SourceBeast b;
	Row r = b.Get();
	FAILNE( r.Size(), 3 );
	FAILNE( r[1], "6" );
	TESTOUT( r );
}


DEFTEST( TestComposite ) {
	DataSource * rs1 = new SourceBeast;
	DataSource * rs2 = new SourceBeast;
	CompositeDataSource cs;
	cs.AddSource( rs1 );
	cs.AddSource( rs2 );

	Row r = cs.Get();
	FAILNE( r.Size(), 6 );
	FAILNE( r.At(1), "6" );
	FAILNE( r.At(5), "6" );
}


#endif

//----------------------------------------------------------------------------

// end

