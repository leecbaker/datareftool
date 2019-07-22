#pragma once

#include <json.hpp>

class ViewerWindow;

void showViewerWindow(bool show_dr, bool show_cr);
void showViewerWindow(const nlohmann::json & window_details = {});
nlohmann::json getViewerWindowsDetails();

class RefRecord;
void updateWindowsPerFrame(const std::vector<RefRecord *> & new_refs, std::vector<RefRecord *> & changed_crs, std::vector<RefRecord *> & changed_drs);
void closeViewerWindow(const ViewerWindow * window);
void closeViewerWindows();
size_t countViewerWindows();

void setAllWindowsInVr(bool in_vr);
