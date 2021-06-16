#include "search_api.h"

#include "../drt_client/drt_client.h"

#include "drt_plugin.h"

void performSearchFromMessage(DRTSearchParameters * search_parameters) {

    //v1- struct size better match the expected size
    if(search_parameters->struct_size != 40) {
        search_parameters->result_count = DRT_SEARCH_RESULT_INVALID_PARAMETER;
        return;
    }

    // interpret flags
    bool change_detection_on = search_parameters->flags & (DRT_API_DETECT_CHANGES_SMALL | DRT_API_DETECT_CHANGES_LARGE);
    bool change_detection_big = search_parameters->flags & DRT_API_DETECT_CHANGES_LARGE;
    bool case_sensitive = search_parameters->flags & DRT_API_SEARCH_TERM_CASE_SENSITIVE;
    bool include_cr = search_parameters->flags & DRT_API_SEARCH_COMMANDS;
    bool include_dr = search_parameters->flags & DRT_API_SEARCH_DATAREFS;
    bool is_regex = search_parameters->flags & DRT_API_SEARCH_TERM_REGEX;

    SearchParams params;
    params.setUseRegex(is_regex);
    params.setIncludeRefs(include_cr, include_dr);
    params.setCaseSensitive(case_sensitive);
    params.setChangeDetection(change_detection_on, change_detection_big);
    params.setSearchTerms(search_parameters->search_term);

    // perform search
    const RefRecords & refs = plugin->getRefs();
    std::vector<RefRecord *> results;
    params.freshSearch(results, refs.getAllCommandrefs(), refs.getAllDatarefs());

    // return results
    search_parameters->result_count = results.size();

    if(search_parameters->callback) {
        for(RefRecord * rr: results) {
            if(false == rr->isBlacklisted()) {
                search_parameters->callback(search_parameters->refcon, rr->getName().c_str());
            }
        }
    }
}