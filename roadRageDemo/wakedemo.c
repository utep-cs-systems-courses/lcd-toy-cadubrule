#include <msp430.h>
#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"

// WARNING: LCD DISPLAY USES P1.0.  Do not touch!!! 

#define LED BIT6		/* note that bit zero req'd for display */

#define SW1 1
#define SW2 2
#define SW3 4
#define SW4 8

#define SWITCHES 15

char blue = 31, green = 0, red = 31;
unsigned char step = 0;

static char 
switch_update_interrupt_sense()
{
  char p2val = P2IN;
  /* update switch interrupt to detect changes from current buttons */
  P2IES |= (p2val & SWITCHES);	/* if switch up, sense down */
  P2IES &= (p2val | ~SWITCHES);	/* if switch down, sense up */
  return p2val;
}

void 
switch_init()			/* setup switch */
{  
  P2REN |= SWITCHES;		/* enables resistors for switches */
  P2IE |= SWITCHES;		/* enable interrupts from switches */
  P2OUT |= SWITCHES;		/* pull-ups for switches */
  P2DIR &= ~SWITCHES;		/* set switches' bits for input */
  switch_update_interrupt_sense();
}

int switches = 0;

void
switch_interrupt_handler()
{
  char p2val = switch_update_interrupt_sense();
  switches = ~p2val & SWITCHES;
}


// axis zero for col, axis 1 for row

short drawPos[2] = {64,140}, controlPos[2] = {65, 140};
short colVelocity = 1, colLimits[2] = {5, screenWidth-10};
short obsPos[2] = {64,81}, obsControl[2] = {64,82};
short rowVelocity = 2, rowLimit = screenHeight;

void
screen_draw_floor(){
  fillRectangle(0,screenHeight / 2,128,80,COLOR_BROWN);
  int j = 0;
  for(int i = 96;i>0;i-=2){
    fillRectangle(32+j,(screenHeight/2)-j,i,1,COLOR_GREEN);
    j++;
  }
  j=0;
  
}

void
draw_car(int col, int row, unsigned short color)
{
  if(col<screenWidth/4){
    for(int i =0;i<18;i++){
      fillRectangle(col-1-i,row-2+i,10,1,color);
    }
    return;
      }
  if(col<screenWidth/2){
    fillRectangle(col-2, row-3,10, 20, color);
    return;
  }
  if(col<(screenWidth/2+screenWidth/4)){
    fillRectangle(col-2, row-3,10, 20, color);
    return;
  }
  if(col>(screenWidth/2+screenWidth/4)){
    for(int i =0;i<18;i++){
      fillRectangle(col-1+i,row-2+i,10,1,color);
    
    }
    return;
  }
}
void
draw_obs(int col, int row, unsigned short color)
{
  fillRectangle(col-1,row-1,3,3,color);
}
void
screen_update_car()
{
  for (char axis = 0; axis < 2; axis ++) 
      if (drawPos[axis] != controlPos[axis]) // position changed? 
	draw_car(drawPos[0], drawPos[1], COLOR_BLACK); /* erase */
  for (char axis = 0; axis < 2; axis ++) 
    drawPos[axis] = controlPos[axis];
  draw_car(drawPos[0], drawPos[1], COLOR_WHITE); /* draw */
}
void
screen_update_obs()
{
  for (char axis = 0; axis < 2; axis ++) 
      if (obsPos[axis] != obsControl[axis]) // position changed? 
	draw_obs(obsPos[0], obsPos[1], COLOR_BLACK); /* erase */
  for (char axis = 0; axis < 2; axis ++) 
    obsPos[axis] = obsControl[axis];
  draw_obs(obsPos[0], obsPos[1], COLOR_RED);/* draw */
}
  

short redrawScreen = 1;
u_int controlFontColor = COLOR_GREEN;

void wdt_c_handler()
{
  static int secCount = 0;

  secCount ++;
  if (secCount >= 05) {		/* 10/sec */
    /* move car*/
    {
      short oldCol = controlPos[0];
      short newCol = oldCol;
      if(switches & SW1){
	newCol = oldCol - colVelocity;
      }
      if(switches & SW3){
	newCol = oldCol + colVelocity;
      }
      if (newCol <= colLimits[0] || newCol >= colLimits[1])
	newCol = oldCol;
      else
	controlPos[0] = newCol;
    }
    {				/* update sky */
      if (switches & SW3) green = (green + 1) % 64;
      if (switches & SW2) blue = (blue + 2) % 32;
      if (switches & SW1) red = (red - 3) % 32;
      if (step <= 80)
	step ++;
      else
	step = 0;
      secCount = 0;
    }
    if (switches & SW4) return;
    redrawScreen = 1;
  }
}
  
void update_shape();

void main()
{
  
  P1DIR |= LED;		/**< Green led on when CPU on */
  P1OUT |= LED;
  configureClocks();
  lcd_init();
  switch_init();
  
  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */
  
  clearScreen(COLOR_BLUE);
  screen_draw_floor();
  while (1) {			/* forever */
    if (redrawScreen) {
      redrawScreen = 0;
      update_shape();
    }
    P1OUT &= ~LED;	/* led off */
    or_sr(0x10);	/**< CPU OFF */
    P1OUT |= LED;	/* led on */
  }
}

void
screen_update_road()
{
  static unsigned char row = screenHeight / 2, col = screenWidth / 2;
  static char lastStep = 0;
  static char lane = 1;
    for (; lastStep <= 80; lastStep++) {
      int startCol = col - lastStep;
      int endCol = col + lastStep;
      int width = 1 + endCol - startCol;
      fillRectangle(startCol, row+lastStep, width, 1, COLOR_BLACK);
      if(lastStep%6>=3){
	drawPixel(startCol+(width/2),row+lastStep,COLOR_YELLOW);
	drawPixel(startCol+(width/2)+(width/4),row+lastStep,COLOR_YELLOW);
	drawPixel(startCol+(width/2)-(width/4),row+lastStep,COLOR_YELLOW);
      }
      if(row+lastStep == obsPos[1]){
	switch(lane){
      case 0:
	obsControl[0] = startCol + width/2 - width/4 - width/8;
	break;
      case 1:
	obsControl[0] = startCol + width/2 - width/4;
	break;
      case 2:
	obsControl[0] = startCol + width/2 + width/4;
	break;
      case 3:
	obsControl[0] = startCol + width/2;
	break;
      case 4:
	obsControl[0] = startCol + width/2 + width/4 - width/8;
	break;
      case 5:
	obsControl[0] = startCol + width/2 + width/4;
	break;
      case 6:
	obsControl[0] = startCol + width/2 + width/4 + width/8;
	break;
      }
      }
    }
    lastStep = 0;
    /*move obs*/
      obsControl[1]+= rowVelocity;
      if(obsControl[1]>=157){
	obsControl[1] = 81;
	lane = (lane+3)%7;
      }
      screen_update_obs();
      if(obsPos[1]>136){
	if(obsPos[0]-drawPos[0]<5 ||obsPos[0]-drawPos[0]>-5){
	  draw_car(drawPos[0],drawPos[1],COLOR_RED);
	}
      }
      
}
void
screen_update_sky(){
  static unsigned char row = screenHeight / 2, col = screenWidth / 2;
  static char lastStep = 0;
  
  if (step == 0 || (lastStep > step)) {
    lastStep = 0;
  } else {
    for (; lastStep <= step; lastStep++) {
      int startCol = 0;
      int endCol = 128;
      int width = 1 + endCol - startCol;
      
      // a color in this BGR encoding is BBBB BGGG GGGR RRRR
      unsigned int color = (blue << 11) | (green << 5) | red;
      
      fillRectangle(startCol, row-lastStep, width, 1, color);
    }
  }
int j = 0;
  for(int i = 96;i>0;i-=2){
    fillRectangle(32+j,(screenHeight/2)-j,i,1,COLOR_GREEN);
    j++;
  }
  j=0;
  for(int i = 64;i>0;i-=2){
    fillRectangle(j,(screenHeight/2)-j,i,1,COLOR_GREEN);
    j++;
  }
    
}
    
void
update_shape()
{
    screen_update_sky();
    screen_update_road();
    screen_update_car();
    
}
void
__interrupt_vec(PORT2_VECTOR) Port_2(){
  if (P2IFG & SWITCHES) {	      /* did a button cause this interrupt? */
    P2IFG &= ~SWITCHES;		      /* clear pending sw interrupts */
    switch_interrupt_handler();	/* single handler for all switches */
  }
}
