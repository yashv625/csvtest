//---------------------------------------------------------------------------
// dmk_run.h
//
// run dmk using command line params
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_DMK_RUN_H
#define INC_DMK_RUN_H

#include "dmk_base.h"
#include "dmk_model.h"
#include "a_xmlparser.h"
#include "a_env.h"

namespace DMK {

//----------------------------------------------------------------------------


class DMKRun {

	public:

		DMKRun( int argc, char *argv[] );
		~DMKRun();

		int Run();

	private:

		void SeedRNG();
		ALib::CommandLine mCmdLine;
		ALib::XMLTreeParser mParser;
		DMK::ModelBuilder mBuilder;
};



//----------------------------------------------------------------------------



} // namespace



#endif

