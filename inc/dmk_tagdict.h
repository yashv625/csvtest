//---------------------------------------------------------------------------
// dmk_tagdict.h
//
// dictionary for lookup & creation of dmk model enyties and data sources
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_DMK_TAGDICT_H
#define INC_DMK_TAGDICT_H

#include "a_xmltree.h"
#include "dmk_base.h"
#include "dmk_model.h"
#include "dmk_source.h"
#include <map>

namespace DMK {

//----------------------------------------------------------------------------
// Entry provides create from XML functions
//----------------------------------------------------------------------------

struct TagDictEntry {
	TagDictEntry() {}
	virtual ~TagDictEntry() {}
	virtual Generator * CreateGen( const ALib::XMLElement * e ) = 0;
	virtual DataSource * CreateDS( const ALib::XMLElement * e ) = 0;
};

//----------------------------------------------------------------------------
// Singleton dictiomary used to map tah name to creation function
//----------------------------------------------------------------------------

class TagDictionary {

	public:

		~TagDictionary();

		void AddGen( const std::string & name,
								TagDictEntry * e );
		void AddDS( const std::string & name,
								TagDictEntry * e );

		DataSource * CreateDS( const ALib::XMLElement * e );
		Generator * CreateGen( const ALib::XMLElement * e );

		static TagDictionary * Instance();

	private:

		typedef std::map <std::string, TagDictEntry *> MapType;

		TagDictionary();

		bool HasTag( const std::string & tag, const MapType & m );

		MapType mGenMap;
		MapType mDSMap;
};

//----------------------------------------------------------------------------
// Create instances of this type to perform registration with dictionary.
//----------------------------------------------------------------------------

template <typename T> struct RegisterGen : public TagDictEntry {

	RegisterGen( const std::string & name ) {
		TagDictionary::Instance()->AddGen( name, this );
	}

	Generator * CreateGen( const ALib::XMLElement * e ) {
		return T::FromXML( e );
	}

	DataSource * CreateDS( const ALib::XMLElement * e ) {
		return 0;
	}
};

//----------------------------------------------------------------------------
// Create instances of this struct to register data sources
//----------------------------------------------------------------------------


template <typename T> struct RegisterDS : public TagDictEntry {

	RegisterDS( const std::string & name ) {
		TagDictionary::Instance()->AddDS( name, this );
	}

	Generator * CreateGen( const ALib::XMLElement * e ) {
		return 0;
	}

	DataSource * CreateDS( const ALib::XMLElement * e ) {
		return T::FromXML( e );
	}
};


//----------------------------------------------------------------------------

} // namespace

#endif

