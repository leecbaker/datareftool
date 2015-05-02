#pragma once
#include <cassert>
#include <chrono>
#include <vector>
#include <string>
#include "XPLMDataAccess.h"

#define PREVIEW_DATAREF_ARRAY_COUNT 4
#define PREVIEW_DATAREF_BYTEARRAY_COUNT 20

enum class dataref_src_t {
	FILE,
	USER_MSG,
};

class DataRefRecord {
	std::string name;
	std::chrono::system_clock::time_point last_updated;
	union {
		float f_val;
		double lf_val;
		int i_val;
		float fv_val[PREVIEW_DATAREF_ARRAY_COUNT];
		int iv_val[PREVIEW_DATAREF_ARRAY_COUNT];
		char b_val[PREVIEW_DATAREF_BYTEARRAY_COUNT];
	};

	XPLMDataTypeID type;
	XPLMDataRef ref;
	dataref_src_t source;

public:
	DataRefRecord(const std::string & name, XPLMDataRef ref, dataref_src_t source) : name(name), ref(ref), source(source) {
		type = 	XPLMGetDataRefTypes(ref);
	}

	/// @return true if updated, false if not
	std::string getValueString() const;
	std::string getDisplayString() const;
	dataref_src_t getSource() const { return source; }
	bool update(const std::chrono::system_clock::time_point current_time);
	const std::string & getName() const { return name; }
	const std::chrono::system_clock::time_point & getLastUpdated() const { return last_updated; }
	bool writable() const;

	bool isDouble() const { return 0 != (xplmType_Double & type); }
	bool isFloat() const { return 0 != (xplmType_Float & type); }
	bool isInt() const { return 0 != (xplmType_Int & type); }

	void setDouble(double d) { assert(isDouble()); XPLMSetDatad(ref, d); }
	void setFloat(float f) { assert(isFloat()); XPLMSetDataf(ref, f); }
	void setInt(int i) { assert(isInt()); XPLMSetDatai(ref, i); }
};


bool addUserDataref(const std::string & name);
bool loadDatarefsFile();
void cleanupDatarefs();
void datarefUpdate();
void doDatarefSearch(const std::string & search_term, bool regex, bool case_insensitive, bool changed_recently, std::vector<DataRefRecord *> & data_out);