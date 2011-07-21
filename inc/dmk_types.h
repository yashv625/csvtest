//---------------------------------------------------------------------------
// dmk_types.h
//
// mixin types and tests  for dmk3
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_DMK_TYPES_H
#define INC_DMK_TYPES_H

#include "dmk_source.h"

namespace DMK {

//----------------------------------------------------------------------------

template <typename T>
bool IsType( const DataSource * s ) {
	return dynamic_cast <const T *>(s) != 0;
}

struct SequenceType {};

//----------------------------------------------------------------------------

}	// namespace


#endif

