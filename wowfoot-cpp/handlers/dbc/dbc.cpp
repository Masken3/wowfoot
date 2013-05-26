#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <unistd.h>
#include "libs/mpq_libmpq04.h"
#include "libs/dbcfile.h"
#include "dbc.h"
#include "../wowfoot-ex/src/config.h"
#include <stdio.h>
#include <fcntl.h>
#include "util/criticalSection.h"

static CriticalSectionLoadGuard sCS;

static const char * const CONF_mpq_list[] = {
	"common.MPQ",
	"common-2.MPQ",
	"lichking.MPQ",
	"expansion.MPQ",
	"patch.MPQ",
	"patch-2.MPQ",
	"patch-3.MPQ",
	"patch-4.MPQ",
	"patch-5.MPQ",
};

static bool fileExists(const char* name) {
	int fd = open(name, 0);
	if(fd < 0)
		return false;
	int res = close(fd);
	assert(res == 0);
	return true;
}

static void LoadCommonMPQFiles() {
	char filename[512];
	int count = sizeof(CONF_mpq_list)/sizeof(char*);
	for(int i = 0; i < count; ++i)
	{
		sprintf(filename, WOW_INSTALL_DIR "Data/%s", CONF_mpq_list[i]);
		if(fileExists(filename))
			new MPQArchive(filename);
	}
}

void DBC::load() {
	LOCK_AND_LOAD;

	printf("Opening MPQ files:\n");
	// the objects need to survive past this function.
	new MPQArchive(WOW_INSTALL_DIR "Data/" WOW_LOCALE "/locale-" WOW_LOCALE ".MPQ");
	new MPQArchive(WOW_INSTALL_DIR "Data/" WOW_LOCALE "/patch-" WOW_LOCALE ".MPQ");
	new MPQArchive(WOW_INSTALL_DIR "Data/" WOW_LOCALE "/patch-" WOW_LOCALE "-2.MPQ");
	new MPQArchive(WOW_INSTALL_DIR "Data/" WOW_LOCALE "/patch-" WOW_LOCALE "-3.MPQ");
	LoadCommonMPQFiles();
}
