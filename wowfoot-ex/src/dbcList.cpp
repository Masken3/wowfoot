#include "config.h"
#include "dbcList.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include "libs/mpq_libmpq04.h"

static const char *CONF_mpq_list[]={
	"common.MPQ",
	"common-2.MPQ",
#if CONFIG_WOW_VERSION >= 30000
	"lichking.MPQ",
#endif
#if CONFIG_WOW_VERSION >= 20000
	"expansion.MPQ",
#endif
#if CONFIG_WOW_VERSION < 20000
	"base.MPQ",
	"dbc.MPQ",
	"fonts.MPQ",
	"interface.MPQ",
	"misc.MPQ",
	//"model.MPQ",
	"terrain.MPQ",
	"texture.MPQ",
	"wmo.MPQ",
#endif
	"patch.MPQ",
	"patch-2.MPQ",
	"patch-3.MPQ",
	"patch-4.MPQ",
	"patch-5.MPQ",
};

bool fileExists(const char* name) {
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

void loadMpqFiles() {
	static bool loaded = false;
	if(loaded)
		return;
	loaded = true;
	printf("Opening MPQ files:\n");
#if CONFIG_WOW_VERSION >= 20000
	MPQArchive locale(WOW_INSTALL_DIR "Data/" WOW_LOCALE "/locale-" WOW_LOCALE ".MPQ");
	MPQArchive patch(WOW_INSTALL_DIR "Data/" WOW_LOCALE "/patch-" WOW_LOCALE ".MPQ");
	MPQArchive patch2(WOW_INSTALL_DIR "Data/" WOW_LOCALE "/patch-" WOW_LOCALE "-2.MPQ");
	MPQArchive patch3(WOW_INSTALL_DIR "Data/" WOW_LOCALE "/patch-" WOW_LOCALE "-3.MPQ");
#endif
	LoadCommonMPQFiles();
}
