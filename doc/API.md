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
