#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <omp.h>

#include <tidy/tidy.h>
#include <tidy/buffio.h>

#include <curl/curl.h>

#include "util/numof.h"

#include "build/wowVersion.h"
#include "db_creature_template.h"
#include "db_gameobject_template.h"
#include "db_item.h"
#include "db_quest.h"
#if CONFIG_WOW_VERSION > 30000
#include "dbcAchievement.h"
#include "dbcCharTitles.h"
#endif
#include "dbcFaction.h"
#include "dbcItemSet.h"
#include "dbcSpell.h"
#include "dbcWorldMapArea.h"
#include "config.h"

using namespace std;

#define LOG printf
#define IN_FILE_ON_LINE LOG("%s:%i:\n", __FILE__, __LINE__)

static void fatalError() __attribute((noreturn));
static void testUrl(const string& url, ostream& tu);
static void parse(const char* html, size_t size, const char* url);
static void generateUrls(vector<string>& urls);

#define TCM(func) do { CURLMcode _res = (func); if(_res != CURLM_OK) { IN_FILE_ON_LINE;\
	LOG("CURLM error %i (%s)\n", _res, curl_multi_strerror(_res)); fatalError(); } } while(0)
#define TCE(func) do { CURLcode _res = (func); if(_res != CURLE_OK) { IN_FILE_ON_LINE;\
	LOG("CURLE error %i (%s)\n", _res, curl_easy_strerror(_res)); fatalError(); } } while(0)
#define TCP(func) do { void* _res = (func); if(_res == NULL) { IN_FILE_ON_LINE;\
	fatalError(); } } while(0)

int main() {
	vector<string> urls;
	{
	ifstream urlFile("urlsToTest.txt");
	if(urlFile.good()) {
		printf("Loading URLs...\n");
		while(urlFile.good()) {
			string s;
			getline(urlFile, s);
			if(!s.empty())
				urls.push_back(s);
		}
	} else {
		printf("Generating URLs...\n");
		generateUrls(urls);
	}
	}
	printf("%" PRIuPTR " urls.\n", urls.size());

	set<string> testedUrls;
	{
		ifstream tuFile("testedUrls.txt");
		if(tuFile.good()) {
			printf("Loading tested URLs...\n");
			while(tuFile.good()) {
				string s;
				getline(tuFile, s);
				if(!s.empty())
					testedUrls.insert(s);
			}
		}
	}
	printf("%" PRIuPTR " tested urls.\n", testedUrls.size());

	ofstream tuFile("testedUrls.txt", ios_base::out | ios_base::app);
	bool printed = false;
	#pragma omp parallel for
	for(size_t i=0; i<urls.size(); i++) {
		#pragma omp critical
		if(!printed) {
			printf("OpenMP activated, %i threads.\n", omp_get_max_threads());
			printed = true;
		}
		if(testedUrls.find(urls[i]) == testedUrls.end())
			testUrl(urls[i], tuFile);
		//printf("done.\n");
		//abort();	//temp
	}
	return 0;
}

template<class Map> void addPageUrls(const string& base_url, const char* name, Map& map, vector<string>& urls) {
	map.load();
	const string pBaseUrl = base_url+name+"=";
	for(auto itr = map.begin(); itr != map.end(); ++itr) {
		char buf[32];
		sprintf(buf, "%i", itr->first);
		urls.push_back(pBaseUrl+buf);
	}
}
#define PURLS(name, map) addPageUrls(base_url, name, map, urls)

static void generateUrls(vector<string>& urls) {
	const char* searches[] = {
		"foo",
		"bar",
		"hell",
		"the people",
	};

	char buf[64];
	sprintf(buf, "http://localhost:%i/", CONFIG_PORT);
	const string base_url = buf;

	//urls.push_back(base_url+"zones");
	urls.push_back(base_url+"quests");
	urls.push_back(base_url+"items");
	urls.push_back(base_url+"spells");

	for(size_t i=0; i<NUMOF(searches); i++) {
		urls.push_back(base_url+"search="+searches[i]);
	}

	PURLS("quest", gQuests);
	PURLS("item", gItems);
#if CONFIG_WOW_VERSION > 30000
	PURLS("title", gTitles);
	PURLS("achievement", gAchievements);
#endif
	PURLS("faction", gFactions);
	PURLS("itemset", gItemSets);
	PURLS("spell", gSpells);
	PURLS("zone", gWorldMapAreas);
	PURLS("npc", gNpcs);
	PURLS("object", gObjects);

	// write to disk
	ofstream urlFile("urlsToTest.txt");
	for(size_t i=0; i<urls.size(); i++) {
		urlFile << urls[i] << "\n";
	}
}

#define USE_OPEN_MEMSTREAM 0

#if !USE_OPEN_MEMSTREAM
struct Memstream {
	char** buf;
	size_t* size;
};

static size_t memstream_write(void* src, size_t size, size_t nmemb, void* userdata) {
	Memstream* m = (Memstream*)userdata;
	size_t srcSize = size * nmemb;
	size_t newSize = *m->size + srcSize;
	*m->buf = (char*)realloc(*m->buf, newSize + 1);
	memcpy(*m->buf + *m->size, src, srcSize);
	*m->size = newSize;
	(*m->buf)[newSize] = 0;
	return srcSize;
}
#endif

static void testUrl(const string& url, ostream& tu) {
	char* mem = NULL;
	size_t memSize = 0;
	printf("%s\n", url.c_str());

	CURL* curl;
	TCP(curl = curl_easy_init());
	TCE(curl_easy_setopt(curl, CURLOPT_URL, url.c_str()));

#if USE_OPEN_MEMSTREAM
	FILE* memStream = open_memstream(&mem, &memSize);
#else
	// WIN32 doesn't support open_memstream.
	TCE(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, memstream_write));
	Memstream ms = { &mem, &memSize };
	Memstream* memStream = &ms;
#endif
	TCE(curl_easy_setopt(curl, CURLOPT_WRITEDATA, memStream));

	//TCE(curl_easy_setopt(curl, CURLOPT_FILETIME, 1));
	//TCE(curl_easy_setopt(curl, CURLOPT_USERAGENT, DEFAULT_USERAGENT));
	TCE(curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0));

	TCE(curl_easy_perform(curl));

#if USE_OPEN_MEMSTREAM
	fflush(memStream);
#endif

	long rc;
	TCE(curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &rc));
	if(rc != 200) {
		printf("Response code: %li\n", rc);
		exit(1);
	}

	parse(mem, memSize, url.c_str());

	// todo: SGML validation.
	// todo: link validation.

	curl_easy_cleanup(curl);

#if USE_OPEN_MEMSTREAM
	fclose(memStream);
#endif
	free(mem);

	#pragma omp critical
	tu << url << endl;
}

static void fatalError() {
	exit(1);
}

static void parse(const char* html, size_t size, const char* url) {
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
		rc = tidySetCharEncoding(tdoc, "utf8");	// set encoding
	if ( rc >= 0 )
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
		if ( rc > 0 ) {
			printf("\n");
			printf("Error found in %s\n", url);
			printf( "Diagnostics:\n\%s", errbuf.bp );
#if 1
			FILE* f = fopen("dump.html", "wb");
			fwrite(html, size, 1, f);
			fclose(f);
#endif
		}
		//printf( "\\nAnd here is the result:\\n\\n\%s", output.bp );
	}
	else
		printf( "A severe error (\%d) occurred.\\n", rc );

	//tidyBufFree( &output );
	tidyBufFree( &errbuf );
	tidyRelease( tdoc );
	if(rc != 0)
		exit(1);
}
