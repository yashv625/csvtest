//---------------------------------------------------------------------------
// dmk_xmlutil.cpp
//
// utilities for working with xml
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_str.h"
#include "dmk_xmlutil.h"
#include "dmk_strings.h"
#include "dmk_fileman.h"
#include <iostream>
#include <cstdarg>

using std::string;
using std::vector;


namespace DMK {

//----------------------------------------------------------------------------
// Exception to use for reporting XML related errors, contains message amd
// pointer to tag that caused the error.
//----------------------------------------------------------------------------

XMLError :: XMLError( const string & msg, const ALib::XMLElement * e  )
	: Exception( FmtMsg( msg, e ) ) {
}

//----------------------------------------------------------------------------
// Format error message with info from tag.
//----------------------------------------------------------------------------

string XMLError :: FmtMsg(  const string & msg, const ALib::XMLElement * e ) {
	return "XML tag " + ALib::SQuote( e->Name())  + " " +  msg
				+ " in " + e->File()
				+ " at line " + ALib::Str( e->Line() );
}

//----------------------------------------------------------------------------
// Check tag has required attributes or throw.
//----------------------------------------------------------------------------

void RequireAttrs( const ALib::XMLElement * e, const string & attrs ) {
	ALib::CommaList cl( attrs );
	for ( unsigned int i = 0; i < cl.Size(); i++ ) {
		if ( ! e->HasAttr( cl.At( i ) )) {
			throw XMLError( "missing attribute "
							+ ALib::SQuote( cl.At( i ) ), e );
		}
	}
}

//----------------------------------------------------------------------------
// Check tag has only allowed attributes or throw
//----------------------------------------------------------------------------

void AllowAttrs( const ALib::XMLElement * e, const string & attrs ) {
	ALib::CommaList cl( attrs );
	for ( unsigned int i = 0; i < e->AttrCount(); i++ ) {
		if ( ! cl.Contains( e->AttrName( i ) )) {
			throw XMLError( "invalid attribute "
							+ ALib::SQuote( e->AttrName( i ) ), e );
		}
	}
}

//----------------------------------------------------------------------------
// Check we do not have certain attribs
//----------------------------------------------------------------------------

void ForbidAttrs( const ALib::XMLElement * e, const std::string & attrs ) {
	ALib::CommaList cl( attrs );
	for ( unsigned int i = 0; i < e->AttrCount(); i++ ) {
		if (  cl.Contains( e->AttrName( i ) )) {
			throw XMLError( "invalid attribute "
							+ ALib::SQuote( e->AttrName( i ) ), e );
		}
	}
}

//----------------------------------------------------------------------------
// Check tag has any required child tags (at first level only)
//----------------------------------------------------------------------------

void RequireChildTags( const ALib::XMLElement * e, const string & tags ) {
	ALib::CommaList cl( tags );
	for ( unsigned int i = 0; i < cl.Size(); i++ ) {
		if ( ! e->ChildElement( cl.At( i ) )) {
			throw XMLError( "required child tag "
							+ ALib::SQuote( cl.At( i ) )
							+ " not found"
							, e );
		}
	}
}

//----------------------------------------------------------------------------
/// Check tag has exactly N kids
//----------------------------------------------------------------------------

void RequireChildCount( const ALib::XMLElement * e, unsigned int n ) {
	if ( e->ChildCount() != n ) {
		throw XMLError( "requires exactlty " + ALib::Str( n )
							+ " child elements", e );
	}
}

//----------------------------------------------------------------------------
// Check element has only tags on allowed list
//----------------------------------------------------------------------------

void AllowChildTags( const ALib::XMLElement * e, const string & tags ) {
	ALib::CommaList cl( tags );
	for ( unsigned int i = 0; i < e->ChildCount(); i++ ) {
		const ALib::XMLElement * ce = e->ChildElement( i );
		if ( ce && ! cl.Contains( ce->Name() ) ) {
			throw XMLError( "invalid child tag "
							+ ALib::SQuote( ce->Name() ), e );
		}
	}
}

//----------------------------------------------------------------------------
// Check tag has mo content
//----------------------------------------------------------------------------

void ForbidChildren( const ALib::XMLElement * e ) {
	if ( e->ChildCount() != 0 ) {
		throw XMLError( "cannot have content", e );
	}
}

//----------------------------------------------------------------------------
// Check there are some child tags
//----------------------------------------------------------------------------

void RequireChildren( const ALib::XMLElement * e ) {
	if ( e->ChildCount() == 0 ) {
		throw XMLError( "missing content", e );
	}
}

//----------------------------------------------------------------------------
// Get all text content for a tag, adding newlines.
//----------------------------------------------------------------------------

string ChildText( const ALib::XMLElement * e ) {
	string s;
	for ( unsigned int i = 0; i < e->ChildCount(); i++ ) {
		const ALib::XMLText * ct = e->ChildText( i );
		if ( ct ) {
			if ( s != "" ) {
				s += "\n";
			}
			s += ct->Text();
		}
	}
	return s;
}

//----------------------------------------------------------------------------
// Get comma list from attribute
//----------------------------------------------------------------------------

ALib::CommaList ListFromAttrib( const ALib::XMLElement * e,
									const std::string & name ) {
	return ALib::CommaList( e->AttrValue( name, "" ) );
}

//----------------------------------------------------------------------------
// Get value of "order" attribute
//----------------------------------------------------------------------------

FieldList GetOrder( const ALib::XMLElement * e, const string & def ) {
	return FieldList( e->AttrValue( ORDER_ATTRIB, def ) );
}


//----------------------------------------------------------------------------
// Get signed integer value of attribute
//----------------------------------------------------------------------------

int GetInt( const ALib::XMLElement * e,
						const string & attr,
						const string & def ) {

	string ns = e->AttrValue( attr, def );
	if ( ! ALib::IsInteger( ns ) ) {
		throw XMLError( "excpected integer value for "
							+ ALib::SQuote( attr ), e );
	}
	return ALib::ToInteger( ns );
}

//----------------------------------------------------------------------------
// Get double value of attribute
//----------------------------------------------------------------------------

double GetReal( const ALib::XMLElement * e,
						const string & attr,
						const string & def ) {

	string ns = e->AttrValue( attr, def );
	if ( ! ALib::IsNumber( ns ) ) {
		throw XMLError( "excpected real number value for "
							+ ALib::SQuote( attr ), e );
	}
	return ALib::ToReal( ns );
}

//----------------------------------------------------------------------------
// Get boolean in "yes" or "no" form.
//----------------------------------------------------------------------------

bool GetBool( const ALib::XMLElement * e, const string & attr,
							const string & def ) {

	string s = e->AttrValue( attr, def );
	if ( s != YES_STR && s != NO_STR ) {
		throw XMLError( "invalid boolean value " + ALib::SQuote( s )
						+ " for attribute " + ALib::SQuote( attr ) , e );
	}
	return s == YES_STR;
}

//----------------------------------------------------------------------------
// Get value of "random" setting with default of yes.
// Note: default always used to be "no" - may cause problems.
//----------------------------------------------------------------------------

bool GetRandom( const ALib::XMLElement * e, const string & defval ) {
	return GetBool( e, RANDOM_ATTRIB, defval == "" ? YES_STR : defval );
}

//----------------------------------------------------------------------------
// Get count of rows to output. The special symbol ALL_STR is used to
// indicate that all rows are required.
// Mote: changing away from this to make things that would have returned
// ALL_STR return 1 instead - may cause problems.
//----------------------------------------------------------------------------

int GetCount( const ALib::XMLElement * e ) {
	string s = e->AttrValue( COUNT_ATTRIB, ALL_STR );
	if ( s == ALL_STR ) {
		return -1;
	}
	else {
		int n = GetInt( e, COUNT_ATTRIB );
		if ( n < 0 ) {
			throw XMLError( ALib::SQuote( COUNT_ATTRIB )
								+ " cannot be negative", e );
		}
		return n;
	}
}

//----------------------------------------------------------------------------
// Construct comma-separated list of string values from null terminated list
//----------------------------------------------------------------------------

string AttrList( const char * s, ... ) {
	string csl;
	const char * val = s;
	va_list begin;
	va_start( begin, s );
	while( val != 0 ) {
		if ( csl != "" ) {
			csl += ',';
		}
		csl += val;
		val = va_arg( begin, const char * );
	}
	va_end( begin );
	return csl;
}

//----------------------------------------------------------------------------
// Get value of "output" attribute, which shoould be file name else return
// special string indicating stdout if not there.
//----------------------------------------------------------------------------

string GetOutputFile( const ALib::XMLElement * e ) {
	if ( e->HasAttr( OUT_ATTRIB ) ) {
		string s = e->AttrValue( OUT_ATTRIB, "" );
		return s;
	}
	else {
		return FileManager::Instance().StdOutName();
	}
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

#ifdef DMK_TEST

#include "a_myth.h"
#include "a_xmlparser.h"
using namespace ALib;
using namespace DMK;
using namespace std;

DEFSUITE( "XML" );

//----------------------------------------------------------------------------

const char * const XML1 =
	"<model number='42'>\n"
		"<say>\n"
			"this is some text\n"
			"this is some more\n"
		"</say>\n"
	"</model>\n";

//----------------------------------------------------------------------------

DEFTEST( ParseOnly ) {
	XMLTreeParser tp;
	XMLElement * e = tp.Parse( XML1 );
	FAILEQ( e, 0 );
	FAILNE( e->Name(), "model" );
}

DEFTEST( AttrUtil ) {
	XMLTreeParser tp;
	XMLElement * e = tp.Parse( XML1 );
	MUST_THROW( RequireAttrs( e, "foo" ) );
	AllowAttrs( e, "foo,number" );;
}

DEFTEST( GetInt ) {
	XMLTreeParser tp;
	XMLElement * e = tp.Parse( XML1 );
	int n = GetInt( e, "number" );
	FAILNE( n, 42 );
}

DEFTEST( AttrList ) {
	string s = AttrList( "foo", "bar", 0 );
	FAILNE( s, "foo,bar" );
}

#endif

//----------------------------------------------------------------------------


// end

