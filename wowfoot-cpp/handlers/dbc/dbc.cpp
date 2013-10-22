#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <unistd.h>
#include "libs/mpq_libmpq04.h"
#include "libs/dbcfile.h"
#include "dbc.h"
#include "../wowfoot-ex/src/config.h"
#include "../wowfoot-ex/src/dbcList.h"
#include <stdio.h>
#include <fcntl.h>
#include "util/criticalSection.h"

static CriticalSectionLoadGuard sCS;

void DBC::load(bool verbose) {
	LOCK_AND_LOAD;

	loadMpqFiles(verbose);
}
