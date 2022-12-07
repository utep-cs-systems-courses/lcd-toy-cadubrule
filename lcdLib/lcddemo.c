/** \file lcddemo.c
 *  \brief A simple demo that draws a string and square
 */
#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"
#include <math.h>
#include <stdio.h>
/** Initializes everything, clears the screen, draws "hello" and a square */
int
main()
{
  configureClocks();
  lcd_init();
  u_char width = screenWidth, height = screenHeight;

  clearScreen(COLOR_BLUE);

  //drawString5x7(20,20, "hello", COLOR_GREEN, COLOR_RED);

  //fillRectangle(30,30, 60, 60, COLOR_ORANGE);

  int fcol = screenWidth/2, frow = screenHeight/2;
  /*  
  for(int i = 0;i<30;i++){
    for(int j = i;j>0;j--){
      drawPixel(fcol+j, frow+i, COLOR_BLACK);
      drawPixel(fcol-j, frow-i, COLOR_BLACK);
      drawPixel(fcol+j, frow-i, COLOR_RED);
      drawPixel(fcol-j, frow+i, COLOR_RED);
     
  }
  }
  */
  drawRhombus(fcol,frow,20);
  
}

void drawRhombus(int fcol,int frow,int size){
  //fcol-=size;
frow-=size;
  for(int i = 0;i<size;i++){
    for(int j = i;j>0;j--){
      drawPixel(fcol+j, frow+i, COLOR_BLACK);
      //drawPixel(fcol-j, frow-i, COLOR_BLACK);
      //drawPixel(fcol+j, frow-i, COLOR_RED);
       drawPixel(fcol-j, frow+i, COLOR_RED);
     
  }
  }
  // fcol-=(size*2);
frow+=(size*2);
  for(int i = 0;i<size;i++){
    for(int j = i;j>0;j--){
      // drawPixel(fcol+j, frow+i, COLOR_BLACK);
      drawPixel(fcol-j, frow-i, COLOR_BLACK);
      drawPixel(fcol+j, frow-i, COLOR_RED);
      // drawPixel(fcol-j, frow+i, COLOR_RED);
     
  }
  }
}

