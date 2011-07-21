//---------------------------------------------------------------------------
// dmk_fileman.h
//
// file stream management
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_DMK_FILEMAN_H
#define INC_DMK_FILEMAN_H

#include "dmk_base.h"
#include "dmk_xmlutil.h"
#include <map>

namespace DMK {

//----------------------------------------------------------------------------



class FileManager {

	public:

		FileManager( std::ostream & defout );
		~FileManager();

		static FileManager & Instance();

		void Clear();

		std::string HideName() const;
		std::string StdOutName() const;

		std::ostream & GetStream( const std::string & fname );

	private:

		typedef std::map <std::string, std::ostream *> NameMapType;
		NameMapType mNameMap;
		std::ostream & mDefOut;
		static FileManager * mInstance;

};


//----------------------------------------------------------------------------

} // namespace

#endif

