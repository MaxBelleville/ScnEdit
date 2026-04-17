#ifndef CENT_H_
#define CENT_H_
#include "include/util.h"
class CEnt
{
public:
	u32 srefidx;            //index into a solid, like doors
	u32 n_fields;
	core::array<u32> keylengths;
	core::array<u32> vallengths;
	core::array<int64_t> entsad;
	int indx = 0;

	struct field
	{
		char key[512];
		char value[512];
	};

	field* fields = nullptr;

	inline const char* getField(const char* key)
	{
		for (int i = 0; i < n_fields; i++) {
			if (str_equiv(fields[i].key, key))
				return fields[i].value;
		}

		return nullptr;
	}

	inline const char* getValue(const char* value)
	{
		for (field* fi = fields; fi != &fields[n_fields]; fi++)
			if (str_equiv(fi->value, value))
				return fi->key;

		return nullptr;
	}

	inline void setField(const char* key, const char* val)
	{
		for (field* fi = fields; fi != &fields[n_fields]; fi++)
			if (str_equiv(fi->key, key))
				strcpy_s(fi->value, val);
	}

	inline CEnt() : srefidx(0), n_fields(0)
	{
		keylengths.clear();
		vallengths.clear();
		entsad.clear();
		if (fields) delete[] fields;
		fields = nullptr;
	}

	inline ~CEnt()
	{
		if (fields) {
			delete[] fields;
			fields = nullptr;
		}
	}
};

#endif
