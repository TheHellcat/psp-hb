/*
 *  Galaga ASCII Clone Toolbox
 *  by ---==> HELLCAT <==---
 *  -----------------------------
 *  collection of supplemental and supporting functions
 *  for gHost's Galaga ASCII Clone game
 ********************************************************/

#include <math.h>
#include "gactools.h"


int t_max = 100;


void hcGacToolsCalcShipPos( int originX, int originY, int wavealgo, int timeindex, HcGacToolsShipInfo* shipinfo, void* postfilterfunction )
{
	// note: about the sine: http://www.cplusplus.com/reference/clibrary/cmath/sin/
  int part;
  int i, j;
  int r, t;
  int arc;
  int x, y;
  int a, b, c;

  // fly a circle
  if( wavealgo == 1 )
  {
  	// determine what quarter of the circle we're in
    if( timeindex <= (t_max/4) ) { part = 1; }
    else if( timeindex <= ((t_max/4)*2) ) { part = 2; }
    else if( timeindex <= ((t_max/4)*3) ) { part = 3; }
    else if( timeindex <= t_max ) { part = 4; }
    
    // cut current time index down into quarter #1
    t = timeindex - ((t_max/4)*(part-1);
    
    // get how many degrees into the circle the current position is
    arc = 90 * ( timeindex / (t_max/4) );
    
    // now get the resulting x and y on the outline of the circle
    r = 20; // define radius of circle here
    b = 180 - arc - 90;
    c = sin(90);
    x = r * sin(arc) / c;
    y = r * sin(b) / c;
    
    // map current coordinates back into the correct quarter of the circle
    a = x; b = y;
    if( part == 2 ) { x = b; y = 0 - a; }
    if( part == 3 ) { x = 0 - a; y = 0 - b; }
    if( part == 4 ) { x = 0 - b; y = a; }
    
    // rotate current x/y to desired starting point's quarter
    i = x;
    x = 0 - y;
    y = i;
    
    // shift x and y to actual starting point of movement
    // (shift 0,0 point to the outline of the circle where movement starts)
    x += r;
    
    // store results
    shipinfo.posx = x + originX;
    shipinfo.posy = y + originY;
  }
}


void hcGacToolsSetTMax( int new_max )
{
  t_max = new_max;
}


int hcGacToolsGetTMax( void )
{
  return t_max;
}
