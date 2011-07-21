//---------------------------------------------------------------------------
// dmk_model.h
//
// database model for dmk
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_DMK_MODEL_H
#define INC_DMK_MODEL_H

#include "a_dict.h"
#include "a_xmltree.h"
#include "dmk_base.h"
#include "dmk_row.h"
#include "dmk_fieldlist.h"


namespace DMK {

//----------------------------------------------------------------------------
// Base for objects held in model. These may currently be generators and
// formatters.
//----------------------------------------------------------------------------

class ModelEntry {

	public:

		ModelEntry();
		virtual ~ModelEntry() = 0;
		virtual void Generate( class Model * model ) = 0;
		virtual void Discard() = 0;
};

//----------------------------------------------------------------------------
// Tag to echo string to output
//----------------------------------------------------------------------------

class Echoer : public ModelEntry {

	public:

		Echoer( const std::string & text );
		~Echoer();
		void Generate( class Model * model );
		void Discard();

	private:

		std::string mText;


};

//----------------------------------------------------------------------------
// A model consists of a number of generators. A generator contains
// a number of data sources which may, recursively, contain other data
// sources. Generators must have a name, so they can ber eferred to later.
//
// Note that Generator is ABSTRACT - it is fully implemented by the gen tag.
//----------------------------------------------------------------------------

class Generator : public  ModelEntry {

	public:

		Generator( const std::string & name, bool debug,
						const FieldList & grp = FieldList() );
		virtual ~Generator();

		std::string Name() const;

		virtual void Discard();

		virtual void AddSources( const ALib::XMLElement * e );

		void AddSource( class DataSource * s );
		unsigned int  SourceCount() const;
		class DataSource * SourceAt( unsigned int i ) const;

		int Size() const;
		Row RowAt( int i ) const;

	protected:

		virtual int GetSize();
		void AddRow( const Row & row );
		virtual Row Get();
		virtual void DebugRow( const Row & row, std::ostream & os );

		bool Debug() const;

		bool HasGroup() const;
		void DoGroup();

	private:

		std::string mName;
		bool mDebug;
		std::vector <class DataSource *> mSources;
		Rows mRows;
		FieldList mGroup;

};

//----------------------------------------------------------------------------
/// Model is a collection of generators
//----------------------------------------------------------------------------

class Model {

	public:

		Model( const std::string & name, std::ostream & defout );
		virtual ~Model();

		void Clear();

		std::string Name() const;

		Generator *  GenAt( unsigned int  ) const;
		void AddGen( Generator * g );
		Generator * FindGen( const std::string & name ) const;

		unsigned int EntryCount() const;
		ModelEntry * EntryAt( unsigned int i ) const;
		void AddEntry( ModelEntry * e );

		bool Debug() const;

		void Generate();

		void AddDef( const std::string & name, const std::string & val );
		std::string GetDefValue( const std::string & name ) const;
		std::ostream & DefaultOutput() const;

	private:

		std::string mName;
		std::vector <ModelEntry *> mEntries;
		ALib::Dictionary <std::string> mDict;
		std::ostream & mDefOut;


};

//----------------------------------------------------------------------------
/// Model builder constructs model from an XML tree
//----------------------------------------------------------------------------

class ModelBuilder {

	public:

		ModelBuilder();
		~ModelBuilder();

		Model * Build( const ALib::XMLElement * tree );

	private:

		void BuildGenerator( Model * model, const ALib::XMLElement * e );
		void BuildEchoer( Model * model, const ALib::XMLElement * e );
		void AddDefine( Model * model, const ALib::XMLElement * e );


};


} // namespace

#endif

