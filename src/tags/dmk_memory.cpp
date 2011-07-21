//---------------------------------------------------------------------------
// dmk_memory.cpp
//
// Tags which allow remembering & referring to result sets
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "dmk_modman.h"
#include "dmk_source.h"
#include "dmk_tagdict.h"
#include "dmk_xmlutil.h"
#include "dmk_strings.h"
#include "dmk_random.h"
#include "a_base.h"
#include "a_str.h"
#include "a_collect.h"

using std::string;
using std::vector;

namespace DMK {

//----------------------------------------------------------------------------

const char * const MEMORY_TAG 		= "remember";
const char * const REFER_TAG 		= "recall";
const char * const MODE_ATTRIB 	= "mode";

const char * const FIRST_MODE 		= "first";
const char * const LAST_MODE 		= "last";
const char * const ALL_MODE 		= "all";

//----------------------------------------------------------------------------
// Memory remembers things
//----------------------------------------------------------------------------

class DSMemory : public CompositeDataSource {

	friend class DSReference;

	public:

		DSMemory( const string & name, const string & mode,
					const FieldList & mFields );

		Row Get();
		int Size();

		static DataSource * FromXML( const ALib::XMLElement * e );

		static DSMemory * Find( const string & name );
		static void Add( const string & name, DSMemory * m );

	private:

		string mName, mMode;
		std::vector <Row> mRows;
		FieldList mFields;
		typedef std::map <string, DSMemory *> MapType;
		static MapType mMap;
};

//----------------------------------------------------------------------------
// Reference recalls things
//----------------------------------------------------------------------------

class DSReference : public DataSource {

	public:

		DSReference( const FieldList & order, const string & name,
							bool rand );

		Row Get();
		int Size();
		void Reset() {} 	// does NOT reset thing referred to

		static DataSource * FromXML( const ALib::XMLElement * e );

	private:

		void GetMem();

		string mName;
		int mPos;
		DSMemory * mMem;
		Generator * mGen;
		bool mRand;
};

//----------------------------------------------------------------------------
// static name to memory mep
//----------------------------------------------------------------------------

DSMemory::MapType DSMemory::mMap;

//----------------------------------------------------------------------------
// tag registration
//----------------------------------------------------------------------------

static RegisterDS <DSMemory> regrs1_( MEMORY_TAG );
static RegisterDS <DSReference> regrs2_( REFER_TAG );

//----------------------------------------------------------------------------
// note we do not support ordering for memory - too confusing!
// we do support "fields" which says which ones to remember
//----------------------------------------------------------------------------

DSMemory :: DSMemory( const string & name, const string & mode,
						const FieldList & fields )
	: mName( name ), mMode( mode ), mFields( fields ) {
}

//----------------------------------------------------------------------------
// Get row from child sources and remember it. Use field list to only
// save some specific fields.
//----------------------------------------------------------------------------

Row DSMemory :: Get() {

	Row r = CompositeDataSource::Get();

	if ( mFields.Size() ) {
		Row tmp;
		for ( unsigned int i = 0; i < r.Size(); i++ ) {
			if ( mFields.Contains( i ) ) {
				tmp.AppendValue( r.At(i) );
			}
		}
		r = tmp;
	}

	if ( mMode == ALL_MODE ) {
		mRows.push_back( r );
	}
	else if ( mMode == FIRST_MODE ) {
		if ( mRows.size() == 0 ) {
			mRows.push_back( r );
		}
	}
	else if ( mMode == LAST_MODE ){
		if ( mRows.size() == 0 ) {
			mRows.push_back( r );
		}
		else {
			mRows[0] = r;
		}
	}
	else {
		throw Exception( "Bad mode: " + mMode );
	}

	return r;

}

//----------------------------------------------------------------------------
// size is as normal
//----------------------------------------------------------------------------

int DSMemory :: Size() {
	return CompositeDataSource::Size();
}

//----------------------------------------------------------------------------
// look up name in map - return NULL if not there
//----------------------------------------------------------------------------

DSMemory * DSMemory :: Find( const string & name ) {
	MapType::iterator it = mMap.find( name );
	if ( it == mMap.end() ) {
		return 0;
	}
	else {
		return it->second;
	}
}

//----------------------------------------------------------------------------
// add to map - no need for dupe check here
//----------------------------------------------------------------------------

void DSMemory :: Add( const string & name, DSMemory * m ) {
	mMap.insert( std::make_pair( name, m ) );
}

//----------------------------------------------------------------------------
// create memory object and add it to the map
//----------------------------------------------------------------------------

DataSource * DSMemory :: FromXML( const ALib::XMLElement * e ) {

	RequireChildren( e );
	RequireAttrs( e, AttrList( NAME_ATTRIB, 0 ) );
	AllowAttrs( e, AttrList( NAME_ATTRIB, MODE_ATTRIB, FIELDS_ATTRIB, 0 ) );

	string name = e->AttrValue( NAME_ATTRIB );
	string mode = e->AttrValue( MODE_ATTRIB, ALL_MODE );

	if ( DSMemory::Find( name ) ) {
		throw XMLError( "duplicate memory name " + ALib::SQuote( name ), e );
	}

	if ( mode != ALL_MODE && mode != LAST_MODE && mode != FIRST_MODE ) {
		throw XMLError( "invalid mode " + ALib::SQuote( mode ), e );
	}

	FieldList fl( e->AttrValue( FIELDS_ATTRIB, "" ));

	std::auto_ptr <DSMemory> m( new DSMemory( name, mode, fl ) );
	m->AddChildSources( e );
	DSMemory::Add( name, m.get() );
	return m.release();
}

//----------------------------------------------------------------------------
// Name refers to memory or gen
//----------------------------------------------------------------------------

DSReference :: DSReference( const FieldList & order,
								const string & name, bool random )
		: DataSource( order ), mName( name ), mPos( 0 ),
					mMem( 0 ), mGen(0), mRand( random ) {
}

//----------------------------------------------------------------------------
// helper to get pointer to memory or generator using name
//----------------------------------------------------------------------------

void DSReference :: GetMem() {
	if ( mMem == 0 && mGen == 0 ) {
		mMem = DSMemory::Find( mName );
		if ( mMem == 0 ) {
			mGen = ModelManager::Instance()->FindGen( mName );
		}
	}
	if ( mMem == 0 && mGen == 0 ) {
		throw Exception( "Cannot find memory/generator named "
								+ ALib::SQuote( mName ) );
	}
}

//----------------------------------------------------------------------------
// get from memory or generator referred to by name
// note mPos is recalculated even if we are in random mode when
// it will get overwritten first thing next call - no big deal
//----------------------------------------------------------------------------

Row DSReference :: Get() {

	GetMem();

	if ( mRand ) {
		mPos = RNG::Random() % Size();
	}

	if ( mMem ) {
		Row r = mMem->mRows.at( mPos );
		mPos = (mPos + 1) % mMem->mRows.size();
		return Order( r );
	}
	else {
		if ( mGen->Size() == 0 ) {
			throw Exception( "Generator " + ALib::SQuote( mName )
								+ " has no data" );
		}
		Row r = mGen->RowAt( mPos );
		mPos = (mPos + 1) % mGen->Size();
		return Order( r );
	}
}

//----------------------------------------------------------------------------
// size comes from memory too
//----------------------------------------------------------------------------

int DSReference :: Size() {
	GetMem();
	return mMem ? mMem->mRows.size() : mGen->Size();
}

//----------------------------------------------------------------------------
// the memory object this reference refers to must already have been created
//----------------------------------------------------------------------------

DataSource * DSReference :: FromXML( const ALib::XMLElement * e ) {

	ForbidChildren( e );
	RequireAttrs( e, AttrList( NAME_ATTRIB, 0 ) );
	AllowAttrs( e, AttrList( ORDER_ATTRIB, NAME_ATTRIB, RANDOM_ATTRIB, 0 ) );

	string name = e->AttrValue( NAME_ATTRIB );
	bool rand = GetRandom( e );
	return new DSReference( GetOrder( e ), name, rand );
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

#ifdef DMK_TEST

#include "a_myth.h"
using namespace ALib;
using namespace DMK;

DEFSUITE( "Memory" );

DEFTEST( TestMemory ) {
	string xml =
		"<memory name='foo' mode='first' >\n"
			"<row values='one' />\n"
		"</memory>\n";
	XMLPtr xp( xml );
	DSMemory * m = (DSMemory *) DSMemory::FromXML( xp );
	Row r = m->Get();
	FAILNE( r.Size(), 1 );
	FAILNE( r.At(0), "one" );

}

DEFTEST( TestRef ) {
	string xml = "<reference name='foo' />";
	XMLPtr xp( xml );
	DSReference * m = (DSReference *) DSReference::FromXML( xp );
	Row r = m->Get();
	FAILNE( r.Size(), 1 );
	FAILNE( r.At(0), "one" );
}


#endif

//----------------------------------------------------------------------------

// end
