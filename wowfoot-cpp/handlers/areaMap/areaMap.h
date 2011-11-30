#include "ConstMap.h"

// Grids
#define SIZE_OF_GRIDS (533.0 + 1/3.0)
#define CENTER_GRID_OFFSET (SIZE_OF_GRIDS/2)
#define MAX_NUMBER_OF_GRIDS 64
#define CENTER_GRID_ID (MAX_NUMBER_OF_GRIDS/2)

// Cells
#define MAX_NUMBER_OF_CELLS 16
#define SIZE_OF_GRID_CELL (SIZE_OF_GRIDS/MAX_NUMBER_OF_CELLS)
#define CENTER_GRID_CELL_ID (MAX_NUMBER_OF_CELLS*MAX_NUMBER_OF_GRIDS/2)
#define CENTER_GRID_CELL_OFFSET (SIZE_OF_GRID_CELL/2)

class AreaMap : public ConstMap<int, int**> {
public:
	void load() VISIBLE;
	~AreaMap();
};

extern AreaMap gAreaMap VISIBLE;
