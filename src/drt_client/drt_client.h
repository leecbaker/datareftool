#ifndef _DRT_CLIENT_H_
#define _DRT_CLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DRT search client API
 * This API enables third party plugins to search datarefs using DRT.
 */

/*
 * Search result type
 * When perming a search, indicate whether you want just datarefs, just commands,
 * or both. You need to pass at least one of these flags if you want to get results back.
 */
#define DRT_API_SEARCH_DATAREFS 0x1
#define DRT_API_SEARCH_COMMANDS 0x2

/*
 * Case sensitivity
 * Pass one of these as a flag to indicate if the search term
 * provided is case sensitive or not.
 */
#define DRT_API_SEARCH_TERM_CASE_INSENSITIVE 0x0
#define DRT_API_SEARCH_TERM_CASE_SENSITIVE 0x100

/*
 * Detect changes
 * To see recently-changed datarefs, pass in one of these flags. The behaviour
 * of detecting big changes vs. small changes is defined in the DRT README.
 */
#define DRT_API_DETECT_CHANGES_NONE 0x0
#define DRT_API_DETECT_CHANGES_SMALL 0x200
#define DRT_API_DETECT_CHANGES_LARGE 0x400

/*
 * Regular expression search term
 * To search using a regex search term, pass the DRT_API_REGEX. If you pass this flag
 * but pass an invalid regular expression, the search term will be interpreted as
 * text instead.
 */
#define DRT_API_SEARCH_TERM_TEXT 0x0
#define DRT_API_SEARCH_TERM_REGEX 0x800

/*
 * Search result callback
 * When perfoming a search, this function is called once for each search result.
 *
 * refcon is a user-provided pointer provided in the DRTSearchParameters structure;
 * you can use this to pass your application's data to the callback function.
 * 
 * ref_name is the name of the command or dataref.
 */
typedef void (* DRTSearchResultCallback)(void * refcon, const char * ref_name);

/* Search parameters for the requested search */
typedef struct DRTSearchParameters_ {
    /* Set this equal to the struct size */
    int struct_size;

    /* Optional user-defined data to pass to the callback */
    void * refcon;

    /* Callback function, called for each search result. If set to
     * nullptr, only the number of results will be returned via the
     * result_count field.
     */
    DRTSearchResultCallback callback;

    /* Search term. Must be a valid null-terminated string. Multiple
     * search terms can be separated by spaces, just as in DRT.
     */
    const char * search_term;

    /* Flags to set search options */
    int flags;

    /* Output- this is the number of search results */
    int result_count;
} DRTSearchParameters;

/* Search return values */

/* Indicates success- a search was performed */
#define DRT_SEARCH_RESULT_SUCCESS 0

/* The DRT plugin couldn't be found, or is too old to support the search API */
#define DRT_SEARCH_RESULT_PLUGIN_NOT_FOUND -1

/* A search was no perfomed as the struct passed in was invalid */
#define DRT_SEARCH_RESULT_INVALID_PARAMETER -2

/* Perform a search. */
int DRTPerformSearch(DRTSearchParameters * search_parameters);

#ifdef __cplusplus
}
#endif

#endif /* _DRT_CLIENT_H_ */
