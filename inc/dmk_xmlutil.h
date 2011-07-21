//---------------------------------------------------------------------------
// dmk_xmlutil.h
//
// various xml utility functions used by dmk
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_DMK_XMLUTIL_H
#define INC_DMK_XMLUTIL_H

#include "dmk_base.h"
#include "dmk_fieldlist.h"
#include "a_str.h"
#include "a_xmlparser.h"
#include <sstream>

namespace DMK {

//----------------------------------------------------------------------------
// Exception class used by folowing functions
//----------------------------------------------------------------------------

class XMLError : public Exception {

	public:

		XMLError( const std::string & msg, const ALib::XMLElement * e  );
		static std::string FmtMsg(  const std::string & msg,
									const ALib::XMLElement * e );

};

//----------------------------------------------------------------------------
// Handy macro to construct & throw exception.
//----------------------------------------------------------------------------

#define XMLERR( xml, msg )								\
{														\
	std::ostringstream os_;								\
	os_ << msg;											\
	throw XMLError( os_.str(), xml );					\
}

//----------------------------------------------------------------------------
// These all raise XML exceptions if their conditions fail
//----------------------------------------------------------------------------

void RequireAttrs( const ALib::XMLElement * e, const std::string & attrs );
void AllowAttrs( const ALib::XMLElement * e, const std::string & attrs );
void ForbidAttrs( const ALib::XMLElement * e, const std::string & attrs );

void RequireChildTags( const ALib::XMLElement * e, const std::string & tags );
void AllowChildTags( const ALib::XMLElement * e, const std::string & tags );

void RequireChildCount( const ALib::XMLElement * e, unsigned int n );

void RequireChildren( const ALib::XMLElement * e );
void ForbidChildren( const ALib::XMLElement * e );

ALib::CommaList ListFromAttrib( const ALib::XMLElement * e,
									const std::string & name );

FieldList GetOrder( const ALib::XMLElement * e, const std::string & def = "" );

int GetInt( const ALib::XMLElement * e, const std::string & attr,
							const std::string & def = "" );
double  GetReal( const ALib::XMLElement * e, const std::string & attr,
							const std::string & def = "" );

bool GetBool( const ALib::XMLElement * e, const std::string & attr,
							const std::string & def = "" );
bool GetRandom( const ALib::XMLElement * e, const std::string & defval = "" );

int GetCount( const ALib::XMLElement * e );

std::string GetOutputFile( const ALib::XMLElement * e );

std::string AttrList( const char * s, ... );
std::string ChildText( const ALib::XMLElement * e );

//----------------------------------------------------------------------------
// This is used for testing - handles element lifetime & error reporting
//----------------------------------------------------------------------------

class XMLPtr {

	CANNOT_COPY( XMLPtr );

	public:

		XMLPtr( const std::string & xml ) : mPtr( 0 ) {
			ALib::XMLTreeParser p;
			mPtr = p.Parse( xml );
			if ( mPtr == 0 ) {
				throw Exception( "Bad XML: " + xml  );
			}
		}

		~XMLPtr() {
			delete mPtr;
		}

		operator ALib::XMLElement *() {
			return mPtr;
		}

		ALib::XMLElement * operator ->() {
			return mPtr;
		}

	private:

		ALib::XMLElement * mPtr;
};

//----------------------------------------------------------------------------

} // namespace



#endif

