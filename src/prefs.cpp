#include <iostream>

#include "prefs.h"

#include "viewer_window.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <boost/foreach.hpp>

bool loadPrefs(const boost::filesystem::path & path) {
	//open file, and deserialize
	std::ifstream f(path.string());

	if(f.fail()) {
		std::cerr << "Couldn't open properties file: " << path.string() << std::endl;
		return false;
	}

	boost::property_tree::ptree prefs;
	read_json(f, prefs);

	BOOST_FOREACH(boost::property_tree::ptree::value_type & window_detail_val, prefs.get_child("windows")) {
        showViewerWindow(window_detail_val.second);
    }

	return true;
}

bool savePrefs(const boost::filesystem::path & path) {

	boost::property_tree::ptree prefs;
	boost::property_tree::ptree windows = getViewerWindowsDetails();
    
    {
    std::ofstream f("/tmp/windows.txt");
    write_json(f, windows);
    }
    
    prefs.put("author", "Lee C. Baker");
    prefs.put("compile_date", __DATE__ " " __TIME__);
    prefs.add_child("windows", windows);

	//serialize and save to file
	std::ofstream f(path.string());
    if(f.fail()) {
        return false;
    }
	write_json(f, prefs);

	return false == f.fail();
}