#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdio.h>
#include <curl/curl.h>
#include <fstream>
#include <vector>
#include <string>
#include <tidy/tidy.h>
#include <tidy/buffio.h>

using namespace std;

#define LOG printf
#define IN_FILE_ON_LINE LOG("%s:%i:\n", __FILE__, __LINE__)

static void fatalError() __attribute((noreturn));
static void testUrl(const string& url);
static void parse(const char* html, size_t size);

#define TCM(func) do { CURLMcode _res = (func); if(_res != CURLM_OK) { IN_FILE_ON_LINE;\
	LOG("CURLM error %i (%s)\n", _res, curl_multi_strerror(_res)); fatalError(); } } while(0)
#define TCE(func) do { CURLcode _res = (func); if(_res != CURLE_OK) { IN_FILE_ON_LINE;\
	LOG("CURLE error %i (%s)\n", _res, curl_easy_strerror(_res)); fatalError(); } } while(0)
#define TCP(func) do { void* _res = (func); if(_res == NULL) { IN_FILE_ON_LINE;\
	fatalError(); } } while(0)

int main() {
	printf("Loading URLs...\n");
	ifstream urlFile("urlsToTest.txt");
	vector<string> urls;
	while(urlFile.good()) {
		string s;
		getline(urlFile, s);
		if(!s.empty())
			urls.push_back(s);
	}
	printf("%" PRIuPTR " urls loaded.\n", urls.size());

	for(size_t i=0; i<urls.size(); i++) {
		testUrl(urls[i]);
		//printf("done.\n");
		//abort();	//temp
	}
	return 0;
}

static void testUrl(const string& url) {
	static char* mem;
	static size_t memSize;
	printf("%s\n", url.c_str());
	FILE* memStream = open_memstream(&mem, &memSize);
	CURL* curl;
	TCP(curl = curl_easy_init());
	//TCE(curl_easy_setopt(curl, CURLOPT_SHARE, sCurlsh));
	TCE(curl_easy_setopt(curl, CURLOPT_URL, url.c_str()));
	TCE(curl_easy_setopt(curl, CURLOPT_WRITEDATA, memStream));
	//TCE(curl_easy_setopt(curl, CURLOPT_FILETIME, 1));
	//TCE(curl_easy_setopt(curl, CURLOPT_USERAGENT, DEFAULT_USERAGENT));
	TCE(curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0));

	TCE(curl_easy_perform(curl));

	fflush(memStream);

	parse(mem, memSize);

	curl_easy_cleanup(curl);
	fclose(memStream);
}

static void fatalError() {
	abort();
}

static void parse(const char* html, size_t size) {
	const char* input = html;
	//TidyBuffer output;
	TidyBuffer errbuf;
	int rc = -1;
	//Bool ok;

	TidyDoc tdoc = tidyCreate();                     // Initialize "document"
	//tidyBufInit( &output );
	tidyBufInit( &errbuf );

	//ok = tidyOptSetBool( tdoc, TidyXhtmlOut, yes );  // Convert to XHTML
	//if ( ok )
		rc = tidySetErrorBuffer( tdoc, &errbuf );      // Capture diagnostics
	if ( rc >= 0 )
		rc = tidyParseString( tdoc, input );           // Parse the input
	//if ( rc >= 0 )
	//	rc = tidyCleanAndRepair( tdoc );               // Tidy it up!
	if ( rc >= 0 )
		rc = tidyRunDiagnostics( tdoc );               // Kvetch
	//if ( rc > 1 )                                    // If error, force output.
	//	rc = ( tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1 );
	//if ( rc >= 0 )
	//	rc = tidySaveBuffer( tdoc, &output );          // Pretty Print

	if ( rc >= 0 )
	{
		if ( rc > 0 )
			printf( "\nDiagnostics:\n\%s", errbuf.bp );
		//printf( "\\nAnd here is the result:\\n\\n\%s", output.bp );
	}
	else
		printf( "A severe error (\%d) occurred.\\n", rc );

	//tidyBufFree( &output );
	tidyBufFree( &errbuf );
	tidyRelease( tdoc );
	if(rc != 0)
		abort();
}
