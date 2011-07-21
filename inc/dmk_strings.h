//---------------------------------------------------------------------------
// dmk_strings.h
//
// Common strings for DMK. Do _not_ add all tag & attribute names here or
// every change will force recompile of almost everything.
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_DMK_STRINGS_H
#define INC_DMK_STRINGS_H

namespace DMK {

//----------------------------------------------------------------------------
// major tags - source tags are in individual files
//----------------------------------------------------------------------------

const char * const DMKROOT_TAG		= "csvt";
const char * const DMKGEN_TAG		= "gen";
const char * const DMKDEF_TAG		= "def";
const char * const DMKECHO_TAG		= "echo";


//----------------------------------------------------------------------------
// Common attribute names
//----------------------------------------------------------------------------

const char * const ORDER_ATTRIB	= "order";
const char * const COUNT_ATTRIB	= "count";
const char * const FILE_ATTRIB		= "file";
const char * const RANDOM_ATTRIB	= "random";
const char * const BEGIN_ATTRIB	= "begin";
const char * const END_ATTRIB		= "end";
const char * const FIELDS_ATTRIB	= "fields";
const char * const INC_ATTRIB		= "inc";
const char * const DEBUG_ATTRIB	= "debug";
const char * const NAME_ATTRIB		= "name";
const char * const GEN_ATTRIB		= "gen";
const char * const MIN_ATTRIB		= "min";
const char * const MAX_ATTRIB		= "max";
const char * const VALUE_ATTRIB	= "value";
const char * const OUT_ATTRIB		= "output";


//----------------------------------------------------------------------------
// Common strings
//----------------------------------------------------------------------------

const char * const YES_STR			= "yes";
const char * const NO_STR			= "no";
const char * const ALL_STR			= "all";
const char * const STDOUT_STR		= "stdout";

//----------------------------------------------------------------------------
// Some other globa l values
//----------------------------------------------------------------------------

const int DMK_NOSIZE = -1;	// indicates size query not supported

} // namespace

#endif

