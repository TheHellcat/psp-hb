/*
 *  Galaga ASCII Clone Toolbox
 *  by ---==> HELLCAT <==---
 *  -----------------------------
 *  collection of supplemental and supporting functions
 *  for gHost's Galaga ASCII Clone game
 ********************************************************
 **
 **  header include
 **
 ********************************************************/


//
// structs and defs
////////////////////

typedef struct HcGacToolsShipInfo
{
  int posx;       // current X position of ship
  int posy;       // current Y position of ship
  int originx;    // originating X for current algorithm
  int originy;    // originating Y for current algorithm
  int algo;       // current algorithm to use for calculating actual position
  int tindex;     // current time index (for current algo in use)
  char face;      // char used for drawing ship
  int color;      // color of ship
  int wave;       // number of the enemy-wave the ship is part of
  int shipno;     // number of the ship in the wave (which one is it in the wave)
  int killpoints; // point rewarded for killing this ship
} HcGacToolsShipInfo;

typedef struct HcGacToolsWaveInfo
{
  int waveno;     // number of the wave
  int shipcount;  // how many ships are in this wave
  int originX;    // starting X position of the first ship of this wave
  int originY;    // starting Y position of the first ship of this wave
  HcGacToolsShipInfo* ships[];  // pointer to array of ships this wave is made of
} HcGacToolsWaveInfo;


//
// functions
////////////////////

/*
 * hcGacToolsCalcShipPos - calculate position of (enemy) ship based on origin position and time
 *
 * params:  originX   - X coord. of ships starting point for this algo
 *          originY   - Y coord. of ships starting point for this algo
 *          wavealgo  - algorithm / formula to use for calculating position
 *          timeindex - how much time passed since starting point (valid are 0 - 100)
 *          shipinfo  - pointer to a HcGacToolsShipInfo struct to receive position data
 *          postfilterfunction - pointer to a function called after position has been
 *                               calculated but before returning it, for post procesing
 *                               the final position
 *                               can be NULL if not used
 *
 * returns: position of ship into posx and posy of shipinfo struct passed
 *
 */         
void hcGacToolsCalcShipPos( int originX, int originY, int wavealgo, int timeindex, HcGacToolsShipInfo* shipinfo, void* postfilterfunction );


/*
 * hcGacToolsSetTMax - set the new maximum timescale value (end of timescale)
 *
 * params:  new_max - the new maximum/ending value of the timescale
 *
 * returns: nothing
 *
 */
void hcGacToolsSetTMax( int new_max );


/*
 * hcGacToolsGetTMax - get the current maximum timescale value (end of timescale)
 *
 * params:  none
 *
 * returns: current t_max value
 *
 */
int hcGacToolsGetTMax( void );
