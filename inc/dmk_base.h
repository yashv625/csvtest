//---------------------------------------------------------------------------
// dmk_base.h
//
// basic stuff for dmk3
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_DMK_BASE_H
#define INC_DMK_BASE_H

#include "a_base.h"
#include "a_except.h"
#include "a_xmltree.h"
#include <string>
#include <vector>
#include <iosfwd>

namespace DMK {

//----------------------------------------------------------------------------
// Base exceptions for DMK - XML errors have their own.
//----------------------------------------------------------------------------

class Exception : public ALib::Exception {

	public:

		Exception( const std::string & msg )
			: ALib::Exception( msg ) {}
};

//----------------------------------------------------------------------------
// Used all over the place
//----------------------------------------------------------------------------

typedef std::vector <std::string> Strings;


//----------------------------------------------------------------------------

}	// namespace


#endif

