#include "config.h"
#include "libs/mpq_libmpq04.h"

int main() {
	MPQArchive locale(WOW_INSTALL_DIR"Data/"WOW_LOCALE"/locale-"WOW_LOCALE".MPQ");
	MPQFile wmc("DBFilesClient\\WorldMapContinent.dbc");
	printf("size: %i\n", wmc.getSize());

	return 0;
}
