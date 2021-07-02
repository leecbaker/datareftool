# DataRefTool API

DRT has a search API for 3rd party plugins. With this, other plugins can request a list of commands or datarefs, and perform searches in the same way that a user can in a DRT window.

## Example code

An example plugin is provided in `src/drt_client/`. This plugin queries DRT 5 seconds after the plugin has loaded; it make 5 different searches, writing results out to Log.txt and to some files on the filesystem. Most use cases will match one of the examples in `example_queries.c`.

## Compiling the API

To use DRT's API, copy `src/drt_client/drt_client.h` and `src/drt_client/drt_client.h` into your project. They should compile with any C or C++ compiler.

## Using the API

To search DRT's list of commands and datarefs, you'll need to do the following:

1. Write a callback function for handling search results from DRT (see `DRTSearchResultCallback`)
2. Put your query parameters in to the `DRTSearchParameters` structure
3. Call `DRTPerformSearch()`
4. Check the return value of `DRTPerformSearch()` to ensure that the search was successful

There are several examples of using the API in `src/drt_client/example_queries.c`.

```c
// This function will be called once for every result.
// Refcon is a value that you provide- you can use this to pass information to the
// callback.
// Name is the name of a dataref or command.
void results_callback(void * refcon, const char * name) {
    ...
}

// An example of how to perform a search
void perform_search() {
    DRTSearchParameters search_params;
    search_params.struct_size = sizeof(DRTSearchParameters);
    search_params.refcon = ...; // You can use this to pass data to your
    search_params.callback = results_callback; // The callback defined above

    // Option flags (see drt_client.h). These flags correspond to the buttons in DRT.
    search_params.flags = 
        (DRT_API_SEARCH_DATAREFS | DRT_API_SEARCH_COMMANDS) |
        DRT_API_SEARCH_TERM_CASE_SENSITIVE | 
        DRT_API_DETECT_CHANGES_NONE |
        DRT_API_SEARCH_TERM_TEXT;

    search_params.search_term = ""; // no search term, so you get all results

    // Perform the search itself
    int search_success = DRTPerformSearch(&search_params);

    // Check the return value.
    switch(search_success) {
        case DRT_SEARCH_RESULT_SUCCESS:
            // yay
            break;
        case DRT_SEARCH_RESULT_PLUGIN_NOT_FOUND:
            // DRT is not installed, or an older version of DRT without the search API
            break;
        case DRT_SEARCH_RESULT_INVALID_PARAMETER:
            // One of the values provided in the DRTSearchParameters structure was invalid
            break;
        default:
            // Another return value; none are currently defined.
            break;
    }

    print_search_success(search_success);

    fclose(file_pointer);
}
```
