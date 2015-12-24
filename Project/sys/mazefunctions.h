////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
///////////////////////////////mazefunctions.h//////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//    See mazefunctions.c
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

typedef struct _matchStages MATCHSTAGES;

//Struct for comparison if two stages. Compares stage a with stage b (with stage
//a is bigger than stage b) and returns the Position in wich stage b matches
//into stage a
struct _matchStages {
    uint8_t stage_a;
    uint8_t stage_b;
    POS match;
    int8_t accordance;
    uint8_t ambiguity;
};

////////////////////////////////////////////////////////////////////////////////

extern uint8_t maze_alignDir(uint8_t dir);

extern uint8_t maze_cmpCoords(COORD *_coordA, COORD *_coordB);

extern void maze_setTile(COORD *_coord, TILE *_tile);

extern TILE *maze_getTile(COORD *_coord);

extern void maze_clearTile(COORD *_coord, TILE *_clear);

extern void maze_corrTile(COORD *_coordA, COORD *_coordB);

extern void maze_setVictim(COORD *_coord, int8_t dir, int8_t value);

extern void maze_corrVictim(COORD *_coord, int8_t dir, int8_t value);

extern int8_t maze_getVictim(COORD *_coord, uint8_t dir);

extern void maze_setWall(COORD *_coord, int8_t dir, int8_t value);

extern void maze_corrWall(COORD *_coord, int8_t dir, int8_t value);

extern int8_t maze_getWall(COORD *_coord, uint8_t dir);

extern void maze_setGround(COORD *_coord, int8_t dir, int8_t value);

extern void maze_corrGround(COORD *_coord, int8_t dir, int8_t value);

extern int8_t maze_getGround(COORD *_coord, int8_t dir);

extern void maze_setObstacle(COORD *_coord, int8_t dir, int8_t value);

extern void maze_corrObstacle(COORD *_coord, int8_t dir, int8_t value);

extern int8_t maze_getObstacle    (COORD *_coord, int8_t dir);

extern uint8_t maze_tileIsVisitable(COORD *_coord, int8_t dir);

extern void maze_setBeenthere(COORD *_coord, int8_t dir, uint8_t value);

extern int8_t maze_getBeenthere(COORD *_coord, int8_t dir);

extern void maze_setDepthsearch(COORD *_coord, int8_t dir, uint8_t value);

extern uint8_t maze_getDepthsearch(COORD *_coord, int8_t dir);

extern int16_t maze_findNearestTile(COORD *_start, COORD *_goal);

extern int16_t maze_findPath(COORD *_start, COORD *_goal);

extern void maze_clearDepthsearch(void);

extern void maze_clearGround(void);

extern void maze_clearWalls(void);

extern void maze_rotateStage(uint8_t stage);

extern void maze_matchStages(MATCHSTAGES *matchStages);

extern void maze_stageGetOuterSize(COORD *_size);

extern void maze_stageGetInnerSize(COORD *_size);

extern void maze_stageGetSize(COORD *_size);

extern uint8_t maze_stageGetVisitedTiles(uint8_t stage);

extern uint16_t maze_GetVisitedTiles(void);

extern void maze_clearStage(uint8_t stage, TILE *clear);

extern void maze_clearRamp(void);

extern void maze_clear(TILE *clear);

extern void maze_setCheckpoint(COORD *_coord, int8_t dir);

extern COORD *maze_getCheckpoint(COORD *_coord);

extern void maze_setRamp(COORD *_coord, int8_t ramp_dir, int8_t tile_dir, int8_t set);

extern uint8_t maze_getRampDir(int8_t pos_z);

extern uint8_t maze_getRampPosDir(COORD *_coord);

extern uint8_t maze_getRampPosDirAtDir(COORD *_coord, int8_t dir);

extern COORD *maze_getRamp(int8_t pos_z);

extern COORD *maze_getStartPos(void);

extern void maze_init(void);

extern void maze_chgOffset(int8_t whichOff, int8_t pos_z, int8_t cnt);

extern int8_t maze_adaptOffset(COORD *_coord);
