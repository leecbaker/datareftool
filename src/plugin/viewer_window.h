#pragma once
#include <boost/property_tree/ptree.hpp>

class ViewerWindow;

void showViewerWindow();
void showViewerWindow(bool show_dr, bool show_cr);
void showViewerWindow(const boost::property_tree::ptree & window_details);
boost::property_tree::ptree getViewerWindowsDetails();

class RefRecord;
void updateWindowsPerFrame(const std::vector<RefRecord *> & new_refs, std::vector<RefRecord *> & changed_crs, std::vector<RefRecord *> & changed_drs);
void closeViewerWindow(const ViewerWindow * window);
void closeViewerWindows();
size_t countViewerWindows();

void setAllWindowsInVr(bool in_vr);
