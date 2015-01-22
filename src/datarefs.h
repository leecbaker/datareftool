#pragma once
#include <chrono>
#include <vector>
#include <string>
#include "XPLMDataAccess.h"


class DataRefRecord {
	std::string name;
	std::chrono::system_clock::time_point last_updated;
	union {
		float f_val;
		double lf_val;
		int i_val;
	};

	XPLMDataTypeID type;
	XPLMDataRef ref;

public:
	DataRefRecord(const std::string & name, XPLMDataRef ref) : name(name), ref(ref) {
		type = 	XPLMGetDataRefTypes(ref);
	}

	/// @return true if updated, false if not
	std::string getValueString() const;
	std::string getDisplayString() const;
	bool update(const std::chrono::system_clock::time_point current_time);
	const std::string & getName() const { return name; }
	const std::chrono::system_clock::time_point & getLastUpdated() const { return last_updated; }
};

bool loadDatarefs();
void cleanupDatarefs();
void datarefUpdate();
void doDatarefSearch(const std::string & search_term, bool regex, bool case_insensitive, bool changed_recently, std::vector<DataRefRecord *> & data_out);