#pragma once
#include <array>
#include <cassert>
#include <chrono>
#include <cstring>
#include <vector>
#include <string>
#include "XPLMDataAccess.h"

#define PREVIEW_DATAREF_ARRAY_COUNT 4
#define PREVIEW_DATAREF_BYTEARRAY_COUNT 20

enum class dataref_src_t {
	FILE,
	AIRCRAFT,
	PLUGIN,
	USER_MSG,
};

class DataRefRecord {
	std::string name;
	std::chrono::system_clock::time_point last_updated;
	std::chrono::system_clock::time_point last_updated_big;
	union {
		float f_val;
		double lf_val;
		int i_val;
		std::array<float,PREVIEW_DATAREF_ARRAY_COUNT> fv_val;
		std::array<int,PREVIEW_DATAREF_ARRAY_COUNT> iv_val;
		std::array<uint8_t,PREVIEW_DATAREF_ARRAY_COUNT> b_val;
	};

	XPLMDataTypeID type;
	XPLMDataRef ref;
	dataref_src_t source;
	int array_length = 1;
	size_t array_hash = -1;

public:
	DataRefRecord(const std::string & name, XPLMDataRef ref, dataref_src_t source);

	/// @return true if updated, false if not
	std::string getValueString() const;
	std::string getDisplayString() const;
	dataref_src_t getSource() const { return source; }
	bool update(const std::chrono::system_clock::time_point current_time);
	const std::string & getName() const { return name; }
	const std::chrono::system_clock::time_point & getLastUpdateTime() const { return last_updated; }
	const std::chrono::system_clock::time_point & getLastBigUpdateTime() const { return last_updated_big; }
	bool writable() const;

	bool isDouble() const { return 0 != (xplmType_Double & type); }
	bool isFloat() const { return 0 != (xplmType_Float & type); }
	bool isInt() const { return 0 != (xplmType_Int & type); }

	bool isArray() const { return xplmType_FloatArray & type || xplmType_IntArray & type; }
	void updateArrayLength();

	void setDouble(double d) { assert(isDouble()); XPLMSetDatad(ref, d); }
	void setFloat(float f) { assert(isFloat()); XPLMSetDataf(ref, f); }
	void setInt(int i) { assert(isInt()); XPLMSetDatai(ref, i); }
};


bool addUserDataref(const std::string & name, dataref_src_t source);
int addUserDatarefs(const std::vector<std::string> & names, dataref_src_t source);
bool loadDatarefsFile();
void cleanupDatarefs();
void datarefUpdate();
void doDatarefSearch(const std::string & search_term, bool regex, bool case_insensitive, bool changed_recently, bool only_big_changes, std::vector<DataRefRecord *> & data_out);
