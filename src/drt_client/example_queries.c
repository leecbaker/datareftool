#include "example_queries.h"

#include "drt_client.h"

#include <XPLMUtilities.h>

#include <stdio.h>

// For purposes of example, print the result to a file. The open file pointer
// will be passed in refcon.
void callback_print_to_file(void * refcon, const char * name) {
    FILE * file_pointer = (FILE *) refcon;

    fprintf(file_pointer, "%s\n", name);
}

// Print the success code of the search to Log.txt
void print_search_success(int search_success_code) {
    if(DRT_SEARCH_RESULT_SUCCESS == search_success_code) {
        XPLMDebugString("DRTClientExample: Completed successfully\n");
    } else if(DRT_SEARCH_RESULT_PLUGIN_NOT_FOUND == search_success_code) {
        XPLMDebugString("DRTClientExample: DRT plugin not found\n");
    } else if(DRT_SEARCH_RESULT_INVALID_PARAMETER == search_success_code) {
        XPLMDebugString("DRTClientExample: Invalid parameter\n");
    } else {
        char message[128];
        sprintf(message, "DRTClientExample: Unknown result %d from DRTPerformSearch()\n", search_success_code);
        XPLMDebugString(message);
    }
}

void run_example_1() {
    //example 1- search for all datarefs and commandrefs, and print them to a file.
    XPLMDebugString("DRTClientExample: Running example 1\n");
    FILE * file_pointer = fopen("all.txt", "wt");

    DRTSearchParameters search_params;
    search_params.struct_size = sizeof(DRTSearchParameters);
    search_params.refcon = file_pointer;
    search_params.callback = callback_print_to_file;

    // One flag from each set of options. These flags correspond to the buttons in DRT.
    search_params.flags = 
        (DRT_API_SEARCH_DATAREFS | DRT_API_SEARCH_COMMANDS) |
        DRT_API_SEARCH_TERM_CASE_SENSITIVE | 
        DRT_API_DETECT_CHANGES_NONE |
        DRT_API_SEARCH_TERM_TEXT;

    search_params.search_term = ""; //return all of them

    int search_success = DRTPerformSearch(&search_params);

    print_search_success(search_success);

    fclose(file_pointer);
}

void run_example_2() {
    //example 2- search for only datarefs, using the term AOA
    XPLMDebugString("DRTClientExample: Running example 2\n");
    FILE * file_pointer = fopen("aoa.txt", "wt");

    DRTSearchParameters search_params;
    search_params.struct_size = sizeof(DRTSearchParameters);
    search_params.refcon = file_pointer;
    search_params.callback = callback_print_to_file;

    search_params.flags = 
        DRT_API_SEARCH_DATAREFS |
        DRT_API_SEARCH_TERM_CASE_INSENSITIVE | 
        DRT_API_DETECT_CHANGES_NONE |
        DRT_API_SEARCH_TERM_TEXT;

    search_params.search_term = "AOA"; //AOA is sometimes uppercase, and sometimes lowercase

    int search_success = DRTPerformSearch(&search_params);
    print_search_success(search_success);

    fclose(file_pointer);
}

void run_example_3() {
    //example 3- search for datarefs and commands that have the letter g, *and* either number 3 or 5
    XPLMDebugString("DRTClientExample: Running example 3\n");
    FILE * file_pointer = fopen("g35.txt", "wt");

    DRTSearchParameters search_params;
    search_params.struct_size = sizeof(DRTSearchParameters);
    search_params.refcon = file_pointer;
    search_params.callback = callback_print_to_file;

    search_params.flags = 
        (DRT_API_SEARCH_DATAREFS | DRT_API_SEARCH_COMMANDS) |
        DRT_API_SEARCH_TERM_CASE_INSENSITIVE | 
        DRT_API_DETECT_CHANGES_NONE |
        DRT_API_SEARCH_TERM_REGEX;

    search_params.search_term = "[35] g"; //AOA is sometimes uppercase, and sometimes lowercase

    int search_success = DRTPerformSearch(&search_params);
    print_search_success(search_success);

    fclose(file_pointer);
}

void run_example_4() {
    //example 4- get result count without actually getting results
    XPLMDebugString("DRTClientExample: Running example 4\n");

    DRTSearchParameters search_params;
    search_params.struct_size = sizeof(DRTSearchParameters);
    search_params.refcon = NULL;
    search_params.callback = NULL;

    search_params.flags = 
        (DRT_API_SEARCH_DATAREFS | DRT_API_SEARCH_COMMANDS) |
        DRT_API_SEARCH_TERM_CASE_INSENSITIVE | 
        DRT_API_DETECT_CHANGES_NONE |
        DRT_API_SEARCH_TERM_TEXT;

    search_params.search_term = "fuel";
    search_params.result_count = 0;

    int search_success = DRTPerformSearch(&search_params);
    print_search_success(search_success);

    char message[128];
    sprintf(message, "DRTClientExample: Found %d results\n", search_params.result_count);
    XPLMDebugString(message);
}

void run_example_5() {
    //example 5- invalid struct size means no search results
    XPLMDebugString("DRTClientExample: Running example 5\n");
    FILE * file_pointer = fopen("empty.txt", "wt");

    DRTSearchParameters search_params;
    search_params.struct_size = 1234;
    search_params.refcon = file_pointer;
    search_params.callback = callback_print_to_file;

    search_params.flags = 
        (DRT_API_SEARCH_DATAREFS | DRT_API_SEARCH_COMMANDS) |
        DRT_API_SEARCH_TERM_CASE_INSENSITIVE | 
        DRT_API_DETECT_CHANGES_NONE |
        DRT_API_SEARCH_TERM_REGEX;

    search_params.search_term = "[35] g"; //AOA is sometimes uppercase, and sometimes lowercase

    int search_success = DRTPerformSearch(&search_params);
    print_search_success(search_success);

    fclose(file_pointer);
}

void run_example_queries() {
    run_example_1();
    run_example_2();
    run_example_3();
    run_example_4();
    run_example_5();
}