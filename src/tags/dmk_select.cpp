//---------------------------------------------------------------------------
// dmk_select.cpp
//
// select/case construct for
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

const char * const SELECT_TAG 		= "select";
const char * const CASE_TAG 		= "case";
const char * const VALUES_ATTR		= "values";

//----------------------------------------------------------------------------
// Select uses first source to select from a number of case sources
//----------------------------------------------------------------------------

class DSSelect : public CompositeDataSource {

	public:

		DSSelect( const FieldList & order );

		Row Get();
		int Size();

		static DataSource * FromXML( const ALib::XMLElement * e );

	private:

		static void CheckKids( const ALib::XMLElement * e );
};

//----------------------------------------------------------------------------
// cases only work in  select
//----------------------------------------------------------------------------

class DSCase : public CompositeDataSource {

	public:

		DSCase( const FieldList & order, const std::string & values );

		bool Match( const Row & r ) const;

		static DataSource * FromXML( const ALib::XMLElement * e );

	private:

		ALib::CommaList  mValues;
};

//----------------------------------------------------------------------------
// Register tags
//----------------------------------------------------------------------------

static RegisterDS <DSSelect> regrs1_( SELECT_TAG );
static RegisterDS <DSCase> regrs2_( CASE_TAG );


//----------------------------------------------------------------------------
// select only has order
//----------------------------------------------------------------------------

DSSelect :: DSSelect( const FieldList & order )
	: CompositeDataSource( order )  {
}

//----------------------------------------------------------------------------
// Get row from first source then compare with cases until one matches and
// get row from that. Return concatenation of the two rows
//----------------------------------------------------------------------------

Row DSSelect  :: Get() {
	Row r = SourceAt(0)->Get();
	for ( unsigned int i = 1; i < SourceCount(); i++ ) {
		DSCase * cs = dynamic_cast <DSCase *>( SourceAt(i) );
		if ( cs->Match( r ) ) {
			return Order( r.AppendRow( cs->Get() ) );
		}
	}
	throw Exception( "No default case" );
}

//----------------------------------------------------------------------------
// No size info
//----------------------------------------------------------------------------

int DSSelect :: Size() {
	return 1;
}

//----------------------------------------------------------------------------
// Check at least two sources, and that all but first are cases
//----------------------------------------------------------------------------

void DSSelect :: CheckKids( const ALib::XMLElement * e ) {
	if ( e->ChildCount()  < 2 ) {
		XMLERR( e, "Invalid select  sructure" );
	}
	for ( unsigned int i = 0; i < e->ChildCount(); i++ ) {
		const ALib::XMLElement * ce = e->ChildElement(i);
		if ( ce ) {
			if ( i == 0 && ce->Name() != CASE_TAG ) {
				continue;
			}
			if ( i != 0 && ce->Name() == CASE_TAG ) {
				continue;
			}
		}
		XMLERR( e, "Invalid select  sructure" );
	}
}

//----------------------------------------------------------------------------
// Create select from xml
//----------------------------------------------------------------------------

DataSource * DSSelect :: FromXML( const ALib::XMLElement * e ) {
	AllowAttrs( e, AttrList( ORDER_ATTRIB,  0 ) );
	RequireChildren( e );
	CheckKids( e );
	std::auto_ptr <DSSelect> sl( new DSSelect( GetOrder( e ) ) );
	sl->AddChildSources( e );
	return sl.release();
}

//----------------------------------------------------------------------------
// Case has order & list of values it matches
//----------------------------------------------------------------------------

DSCase :: DSCase( const FieldList & order, const string & values )
	: CompositeDataSource( order ), mValues( values ) {
}

//----------------------------------------------------------------------------
// Perfortm match with first field of source
//----------------------------------------------------------------------------

bool DSCase :: Match( const Row & r ) const {
	if ( mValues.Size() == 0 ) {
		return true;
	}

	for ( unsigned int i = 0; i < mValues.Size(); i++ ) {
		if ( r.At(0) == mValues.At(i) ) {
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------------
// Build from XML
//----------------------------------------------------------------------------

DataSource * DSCase :: FromXML( const ALib::XMLElement * e ) {

	if ( e->Parent() == 0 || e->Parent()->Name() != SELECT_TAG ) {
		XMLERR( e, "case must be part of select" );
	}

	AllowAttrs( e, AttrList( ORDER_ATTRIB, VALUES_ATTR, 0 ) );
	RequireChildren( e );
	string values = e->AttrValue( VALUES_ATTR, "" );

	std::auto_ptr <DSCase> cs( new DSCase( GetOrder( e ), values ) );
	cs->AddChildSources( e );
	return cs.release();
}


//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

#ifdef DMK_TEST

#include "a_myth.h"
using namespace ALib;
using namespace DMK;

DEFSUITE( "Select" );

DEFTEST( SelectTest ) {
	string xml =
		"<select>\n"
			"<row values='foo,bar' />\n"
			"<case values='foo' >\n"
				"<row values='that was foo' />\n"
			"</case>\n"
			"<case values='bar' >\n"
				"<row values='that was bar' />\n"
			"</case>\n"
		"</select>\n";

	XMLPtr xp( xml );
	DSSelect * s = (DSSelect *) DSSelect::FromXML( xp );
	Row r = s->Get();
}

#endif

//----------------------------------------------------------------------------

// end

