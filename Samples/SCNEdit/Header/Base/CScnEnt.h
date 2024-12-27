#ifndef CSCNENT_H_
#define CSCNENT_H_
#include "util.h"
class CScnEnt
{
public:
	u32 srefidx;            //index into a solid, like doors
	u32 n_fields;
	core::array<u32> keylengths;
	core::array<u32> vallengths;
	core::array<int64_t> entsad;
	int indx;

	struct field
	{
		char key[512];
		char value[512];
	};

	field * fields = nullptr;

	const char * getField(const char * key)
	{
		for (field * fi = fields; fi!=&fields[n_fields]; fi++)
			if (str_equiv(fi->key,key))
				return fi->value;

		return NULL;
	}

	const char* getValue(const char* value)
	{
		for (field* fi = fields; fi != &fields[n_fields]; fi++)
			if (str_equiv(fi->value, value))
				return fi->key;

		return NULL;
	}


	void setField(const char* key, const char* val)
	{
		for (field* fi = fields; fi != &fields[n_fields]; fi++)
			if (str_equiv(fi->key, key))
				strcpy_s(fi->value, val);
	}


	CScnEnt() : srefidx(0), n_fields(0)
	{}

   ~CScnEnt()
	{
		if (fields)
			delete [] fields;
	}
};

#endif
