//---------------------------------------------------------------------------
// dmk_modman.h
//
// model manager for DMK.
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_DMK_MODMAN_H
#define INC_DMK_MODMAN_H

#include "dmk_base.h"
#include "dmk_model.h"
#include "a_xmltree.h"

namespace DMK {

//----------------------------------------------------------------------------

class ModelManager {

	public:

		enum ModelMode { mmCheckOnly, mmGenOnly, mmGenForm };

		~ModelManager();

		void AddModel( Model * m, ModelMode mm );
		void AddModelFromFile( const std::string & fname, ModelMode mm );
		void Clear();

		Model * FindModel( const std::string & name ) const;
		Generator * FindGen( const std::string & name ) const;

		void RunModels( std::ostream & os );

		static ModelManager * Instance();


	private:

		ModelManager();

		bool ProcessInclude(  ALib::XMLElement * m );
		void ReplaceFromFile( ALib::XMLElement * e, const std::string & fname );

		struct MME {

			Model * mModel;
			ModelMode mMode;

			MME( Model * model, ModelMode mm )
				: mModel( model ), mMode( mm ) {}

		};

		std::vector <MME> mModels;

};

//----------------------------------------------------------------------------

} // namespace

#endif

