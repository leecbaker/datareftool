#pragma once

extern "C" {
    struct DRTSearchParameters_;
    typedef struct DRTSearchParameters_ DRTSearchParameters;
};

void performSearchFromMessage(DRTSearchParameters * search_parameters);
