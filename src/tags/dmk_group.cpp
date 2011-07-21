//---------------------------------------------------------------------------
// dmk_group.cpp
//
// Group fields and related counters etc.
// Note that sorting perrformed to do grouping is not a general purpose
// sort and won't work correctly for numbers etc.
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
#include "dmk_random.h"

using std::string;
using std::vector;

namespace DMK {

//----------------------------------------------------------------------------

const char * const GROUP_TAG 			= "group";
const char * const RESET_ATTR 			= "reset";
const char * const FIELDS_ATTR 		= "fields";


//----------------------------------------------------------------------------

class Group : public CompositeDataSource {

	public:

		Group( const FieldList & order,
				const FieldList & fields,
				const FieldList & reset  );

		Row Get();
		int Size();

		static DataSource * FromXML( const ALib::XMLElement * e );


	private:

		void DoReset();
		Row GetNonGroup();
		void DoSort();

		FieldList  mFields, mReset;
		Row mLast;
		Rows mSorted;
		unsigned int mPos;

};

//----------------------------------------------------------------------------
// Register group tag
//----------------------------------------------------------------------------

static RegisterDS <Group> regrs1_(GROUP_TAG );

//----------------------------------------------------------------------------
// Can specify fields that form group and sources to reset (not implemented)
//----------------------------------------------------------------------------

Group :: Group( const FieldList & order,
				const FieldList & fields,
				const FieldList & reset  )
	: CompositeDataSource( order ), mFields( fields ), mReset( reset ), mPos(0) {
}

//----------------------------------------------------------------------------
// Size is that of the group source
//----------------------------------------------------------------------------

int Group :: Size() {
	return SourceAt(0)->Size();
}

//----------------------------------------------------------------------------
// Get a rowe from sources not part of group
//----------------------------------------------------------------------------

Row Group :: GetNonGroup() {
	Row r;
	for ( unsigned int i = 1; i < SourceCount(); i++ ) {
		r.AppendRow( SourceAt(i)->Get() );
	}
	return r;
}

//----------------------------------------------------------------------------
// Reset non-group sources
//----------------------------------------------------------------------------

void Group :: DoReset() {
	for ( unsigned int i = 1; i < SourceCount(); i++ ) {
		SourceAt(i)->Reset();
	}
}

//----------------------------------------------------------------------------
// Group the fields by sorting them
//----------------------------------------------------------------------------

struct Sorter {

	Sorter( const FieldList & f ) : mFields(f) {}
	bool operator()( const Row & r1, const Row & r2 ) const {
		return Cmp( r1, r2, mFields ) < 0;
	}
	FieldList mFields;

};

void Group :: DoSort() {
	for ( unsigned int i = 0; i < SourceAt(0)->Size(); i++ ) {
		mSorted.push_back( SourceAt(0)->Get() );
	}
	Sorter s( mFields );
	std::sort( mSorted.begin(), mSorted.end(), s );
}

//----------------------------------------------------------------------------
// Get group source & if start of new group, reset other sources
//----------------------------------------------------------------------------

Row Group :: Get() {

	if ( mSorted.size() == 0 ) {
		DoSort();
	}

	Row gr = mSorted[ mPos++ ];
	mPos %= mSorted.size();

	if ( Cmp( mLast, gr, mFields ) != 0 ) {
		DoReset();
		mLast = gr;
	}

	gr.AppendRow( GetNonGroup() );
	return Order( gr );
}


//----------------------------------------------------------------------------
// Create from xml. There must be at least a single source, which must
// have a size.
//----------------------------------------------------------------------------

DataSource * Group :: FromXML( const ALib::XMLElement * e ) {
	RequireChildren( e );
	AllowAttrs( e, AttrList( ORDER_ATTRIB, FIELDS_ATTR, RESET_ATTR, 0 ));
	FieldList fields( e->AttrValue( FIELDS_ATTR, "" ) );
	FieldList reset( e->AttrValue( RESET_ATTR, "" ) );
	std::auto_ptr <Group> gp( new Group( GetOrder( e ), fields, reset ) );
	gp->AddChildSources( e );
	return gp.release();
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

#ifdef DMK_TEST

#include "a_myth.h"
using namespace ALib;
using namespace DMK;

DEFSUITE( "Group" );

// no tests yet

#endif

//----------------------------------------------------------------------------

// end

