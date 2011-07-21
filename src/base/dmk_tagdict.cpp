//---------------------------------------------------------------------------
// dmk_tagdict.cpp
//
// Tag dictionary is a singleton that allows lookup of creation functions
// using XML tag names.
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "dmk_base.h"
#include "dmk_tagdict.h"
#include <assert.h>

using std::string;
using std::vector;

namespace DMK {

//----------------------------------------------------------------------------
// nothing to do - needed because we want it to be private
//----------------------------------------------------------------------------

TagDictionary :: TagDictionary() {
}

//----------------------------------------------------------------------------
// nothing to do - we don't own registed objects
//----------------------------------------------------------------------------

TagDictionary :: ~TagDictionary() {
}

//----------------------------------------------------------------------------
// the dictionary singleton
//----------------------------------------------------------------------------

TagDictionary * TagDictionary :: Instance() {
	static TagDictionary dict;
	return & dict;
}

//----------------------------------------------------------------------------
// add a creaton method for a generator
//----------------------------------------------------------------------------

void TagDictionary :: AddGen( const string & name, TagDictEntry * e ) {
	if ( HasTag( name, mGenMap ) ) {
		// flag error somehow
	}
	mGenMap.insert( std::make_pair( name, e ) );
}

//----------------------------------------------------------------------------
// add creation method for a data source
//----------------------------------------------------------------------------

void TagDictionary :: AddDS( const string & name, TagDictEntry * e ) {
	if ( HasTag( name, mDSMap ) ) {
		// flag error somehow
	}
	mDSMap.insert( std::make_pair( name, e ) );
}

//----------------------------------------------------------------------------
// create datasource from XML
//----------------------------------------------------------------------------

DataSource * TagDictionary :: CreateDS( const ALib::XMLElement * e ) {
	string name = e->Name();
	MapType::iterator it = mDSMap.find( name );
	if ( it == mDSMap.end() ) {
		throw Exception( "Unknown tag: " + name );
	}
	return it->second->CreateDS( e );
}

//----------------------------------------------------------------------------
// create generator from XML
//----------------------------------------------------------------------------

Generator * TagDictionary :: CreateGen( const ALib::XMLElement * e ) {
	assert( e );
	string name = e->Name();
	MapType::iterator it = mGenMap.find( name );
	if ( it == mGenMap.end() ) {
		throw Exception( "Unknown tag: " + name );
	}
	return it->second->CreateGen( e );
}

//----------------------------------------------------------------------------
// see if dictionary knows a tag
//----------------------------------------------------------------------------

bool TagDictionary :: HasTag( const string & tag, const MapType & m ) {
	return m.find( tag ) != m.end();
}

//----------------------------------------------------------------------------

} // namespace


// end

