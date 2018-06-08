// Include libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h> // usleep
#include "/usr/include/graphics.h"
// Other Linker Options: -lXbgi -lX11 -lm

#include <time.h>
clock_t start, end;
double cpu_time_used;
// start=clock();
// end=clock();
// cpu_time_used=((double)(end-start))/CLOCKS_PER_SEC;

double radians(double degrees) {
  return degrees*M_PI/180;
}
double degrees(double radians) {
  return radians*180/M_PI;
}

int SIZE=25;                // Grid square size
int MARGIN=12;              // Grid square size
int LEAF_SIZE=10;           // Small heart leaf
int SEED_SIZE=3;            // Small circle

int animateProbability=50;  // 5 to 99% probability
int width=750;              // Area width
int height=600;             // Area height
int sizeX=30;               // Grid squares (width)
int sizeY=24;               // Grid squares (height)
int dirsX[30][24];          // X direction {-1,0,1}
int dirsY[30][24];          // Y direction {-1,0,1}
bool isBranch[30][24];      // Is branch?
int points_X[720];          // X Point List
int points_Y[720];          // Y Point List
int N;                      // List size
int nextPtr;                // List iterator (next point)

void bezier3(int x0,int y0,int x1,int y1,int x2,int y2,int x3,int y3)
{
  int t,px,py,lastpx,lastpy,px0,py0,px1,py1;

  // Cubic bezier curve using 10 segments
  px=x0;
  py=y0;
  px1=x0*1000;
  py1=py*1000;
  for (t=1;t<=10;t++) {
    lastpx=px;
    lastpy=py;
    px=(10-t)*(10-t)*(10-t)*x0+3*(10-t)*(10-t)*t*x1+3*(10-t)*t*t*x2+t*t*t*x3;
    py=(10-t)*(10-t)*(10-t)*y0+3*(10-t)*(10-t)*t*y1+3*(10-t)*t*t*y2+t*t*t*y3;
    // DXF segment
    px0=px1;
    py0=py1;
    px1=px;
    py1=py;
    fprintf(F1,"0\n")  ;fprintf(F1,"LINE\n");                   // A segment
    fprintf(F1,"8\n")  ;fprintf(F1,"0\n");                      // Layer 0
    fprintf(F1,"62\n") ;fprintf(F1,"0\n");                      // Colour 0
    fprintf(F1,"10\n") ;fprintf(F1,"%0.3lf\n",px0/1000.0);      // X (start)
    fprintf(F1,"20\n") ;fprintf(F1,"%0.3lf\n",py0/1000.0);      // Y (start)
    fprintf(F1,"30\n") ;fprintf(F1,"%0.3lf\n",0.0);             // Z (start)
    fprintf(F1,"11\n") ;fprintf(F1,"%0.3lf\n",px1/1000.0);      // X (end)
    fprintf(F1,"21\n") ;fprintf(F1,"%0.3lf\n",py1/1000.0);      // Y (end)
    fprintf(F1,"31\n") ;fprintf(F1,"%0.3lf\n",0.0);             // Z (end)
    px=(px+500)/1000;
    py=(py+500)/1000;
    line(lastpx,lastpy,px,py);
  }
}

void drawArc(int x1,int y1,int xc,int yc,int x2,int y2)
{
  // A nice curve from three points
  bezier3(x1,y1,(x1+xc)/2,(y1+yc)/2,(x2+xc)/2,(y2+yc)/2,x2,y2);
}

bool canAnimateX(int x, int y) {
  int newX;

  newX=x+dirsX[x][y];
  // Check if legal
  return (newX>=0)&&(newX<sizeX)&&(dirsX[newX][y]==0);
}

bool canAnimateY(int x, int y)
{
  int newY;

  newY=y+dirsY[x][y];
  // Check if legal
  return (newY>=0)&&(newY<sizeY)&&(dirsY[x][newY]==0);
}

bool canAnimateXY(int x, int y)
{
  int newX,newY;

  newX=x+dirsX[x][y];
  newY=y+dirsY[x][y];
  // Check if legal
  return
    (newX>=0)&&(newX<sizeX)&&(newY>=0)&&(newY<sizeY)&&
    (dirsY[newX][newY]==0)&&
    ((newX+dirsX[newX][y]!=x)||(y+dirsY[newX][y]!=newY))&&
    ((x+dirsX[x][newY]!=newX)||(newY+dirsY[x][newY]!=y));
}

void animateX(int x, int y) {
  int newX,px0,py0,px1,py1,px2,py2;

  // Go horizontal with curves
  newX=x+dirsX[x][y];
  isBranch[x][y]=true;
  dirsX[newX][y]=dirsX[x][y];
  dirsY[newX][y]=-dirsY[x][y];
  points_X[N]=newX;
  points_Y[N]=y;
  N++;
  // Show
  px0=SIZE*x+MARGIN;
  py0=SIZE*y+MARGIN;
  px1=(SIZE*x+SIZE*newX)/2+MARGIN;
  py1=SIZE*y+SIZE*dirsY[x][y]/2+MARGIN;
  px2=SIZE*newX+MARGIN;
  py2=SIZE*y+MARGIN;
  drawArc(px0,py0,px1,py1,px2,py2);
}

void animateY(int x, int y) {
  int newY,px0,py0,px1,py1,px2,py2;

  // Go vertical with curves
  newY=y+dirsY[x][y];
  isBranch[x][y]=true;
  dirsX[x][newY]=-dirsX[x][y];
  dirsY[x][newY]=dirsY[x][y];
  points_X[N]=x;
  points_Y[N]=newY;
  N++;
  // show
  px0=SIZE*x+MARGIN;
  py0=SIZE*y+MARGIN;
  px1=SIZE*x+SIZE*dirsX[x][y]/2+MARGIN;
  py1=(py0+SIZE*newY)/2+MARGIN;
  px2=SIZE*x+MARGIN;
  py2=SIZE*newY+MARGIN;
  drawArc(px0,py0,px1,py1,px2,py2);
}

void animateXY(int x, int y) {
  int newX,newY,px0,py0,px1,py1;

  // Go straight diagonally
  newX=x+dirsX[x][y];
  newY=y+dirsY[x][y];
  isBranch[x][y]=true;
  dirsX[newX][newY]=dirsX[x][y];
  dirsY[newX][newY]=dirsY[x][y];
  points_X[N]=newX;
  points_Y[N]=newY;
  N++;
  // Show
  px0=SIZE*x+MARGIN;
  py0=SIZE*y+MARGIN;
  px1=SIZE*newX+MARGIN;
  py1=SIZE*newY+MARGIN;
  line(px0,py0,px1,py1);
}

void drawLeaf(int x,int y)
{
  int px,py;

  px=x*SIZE+MARGIN;
  py=y*SIZE+MARGIN;
  bezier3(
    px, py,
    px-dirsX[x][y]*LEAF_SIZE*2/8, py+dirsY[x][y]*LEAF_SIZE*2/8,
    px-dirsX[x][y]*LEAF_SIZE*3/8, py+dirsY[x][y]*LEAF_SIZE*4/8,
    px-dirsX[x][y]*LEAF_SIZE*1/8, py+dirsY[x][y]*LEAF_SIZE*6/8
  );
  bezier3(
    px-dirsX[x][y]*LEAF_SIZE*1/8, py+dirsY[x][y]*LEAF_SIZE*6/8,
    px+dirsX[x][y]*LEAF_SIZE*1/8, py+dirsY[x][y]*LEAF_SIZE*8/8,
    px+dirsX[x][y]*LEAF_SIZE*6/8, py+dirsY[x][y]*LEAF_SIZE*6/8,
    px+dirsX[x][y]*LEAF_SIZE*8/8, py+dirsY[x][y]*LEAF_SIZE*8/8
  );
  bezier3(
    px+dirsX[x][y]*LEAF_SIZE*8/8, py+dirsY[x][y]*LEAF_SIZE*8/8,
    px+dirsX[x][y]*LEAF_SIZE*6/8, py+dirsY[x][y]*LEAF_SIZE*6/8,
    px+dirsX[x][y]*LEAF_SIZE*8/8, py+dirsY[x][y]*LEAF_SIZE*1/8,
    px+dirsX[x][y]*LEAF_SIZE*6/8, py-dirsY[x][y]*LEAF_SIZE*1/8
  );
  bezier3(
    px+dirsX[x][y]*LEAF_SIZE*6/8, py-dirsY[x][y]*LEAF_SIZE*1/8,
    px+dirsX[x][y]*LEAF_SIZE*4/8, py-dirsY[x][y]*LEAF_SIZE*3/8,
    px+dirsX[x][y]*LEAF_SIZE*2/8, py-dirsY[x][y]*LEAF_SIZE*2/8,
    px, py
  );
}

void animate(int x, int y)
 {
  int i,r;

  // Select a direction randomly (may fail to fine an empty cell)
  for (i=0;i<12;i++) {
    r=rand()%3;
    if (r==0) {
      if (canAnimateX(x,y)) animateX(x,y);
    } else if (r==1) {
      if (canAnimateY(x,y)) animateY(x,y);
    } else if (r==2) {
      if (canAnimateXY(x,y)) animateXY(x,y);
    }
  }
}

void addPoint(int x,int y)
{
  int i,r,px,py;
  // Add a new point - NEEDS MORE WORK!
  for (i=0;i<12;i++) {
    dirsX[x][y]=(rand()%2)*2-1;
    dirsY[x][y]=(rand()%2)*2-1;
    r=rand()%3;
    if (r==0) {
      if (canAnimateX(x,y)) animateX(x,y);
    } else if (r==1) {
      if (canAnimateY(x,y)) animateY(x,y);
    } else if (r==2) {
      if (canAnimateXY(x,y)) animateXY(x,y);
    }
  }
  points_X[N]=x;
  points_Y[N]=y;
  N++;
  // Show
  px=x*SIZE+MARGIN;
  py=y*SIZE+MARGIN;
  if (isBranch[x][y]==true) {
    fillellipse(px,py,SEED_SIZE,SEED_SIZE);
  } else {
    circle(px,py,SEED_SIZE*2);
    isBranch[x][y]=true;
  }
}

void draw() {
  int x,y,r,n;

  // While there are vacant cells
  do {

      nextPtr=0;
      while (nextPtr<N) {
        // Get next point from list - may need to repeat here!
        x=points_X[nextPtr];
        y=points_Y[nextPtr];
        nextPtr++;
        if (rand()%100<animateProbability) animate(x,y);
      }

    // Count vacant cells
    n=0;
    for (x=0;x<sizeX;x++) {
      for (y=0;y<sizeY;y++) {
        if (dirsX[x][y]==0) n++;
      }
    }
    if (n>0) {
      // Nothing to do,so add a random point (efficiently)
      // Randomly select a vacant cell
      r=rand()%n+1;
      // Find the cell
      n=0;
      for (x=0;x<sizeX;x++) {
        for (y=0;y<sizeY;y++) {
          if (dirsX[x][y]==0) n++;
          if (n==r) {
            addPoint(x,y);
            break;
          }
        }
        if (n==r) break;
      }
      // Reset next to the beginning of the list
      nextPtr=0;
    }
  } while (n>0);

  // Set terminals as leaves
  nextPtr=0;
  while (nextPtr<N) {
    // Get next point from list
    x=points_X[nextPtr];
    y=points_Y[nextPtr];
    nextPtr++;
    // If terminal then add a leaf
    if (!isBranch[x][y]) drawLeaf(x,y);
  }
}

void initialize() {
  int x,y;

  animateProbability=rand()%95+5; // 5% to 99% probability

  // Make all barren
  for (x=0;x<sizeX;x++) {
    for (y=0;y<sizeY;y++) {
      dirsX[x][y]=1;
      dirsY[x][y]=1;
      isBranch[x][y]=true;
    }
  }
  // Big circle - make fertile
  for (x=0;x<sizeX;x++) {
    for (y=0;y<sizeY;y++) {
      if ((x-15)*(x-15)+(y-12)*(y-12)<=9*9) {
        dirsX[x][y]=0;
        dirsY[x][y]=0;
        isBranch[x][y]=false;
      }
    }
  }
  circle(SIZE*15+MARGIN,SIZE*12+MARGIN,SIZE*9+MARGIN);

  // Small circle - make barren
  for (x=0;x<sizeX;x++) {
    for (y=0;y<sizeY;y++) {
      if ((x-17)*(x-17)+(y-12)*(y-12)<=6*6) {
        dirsX[x][y]=1;
        dirsY[x][y]=1;
        isBranch[x][y]=true;
      }
    }
  }
  circle(SIZE*17+MARGIN,SIZE*12+MARGIN,SIZE*6-MARGIN);

  N=0;
  nextPtr=0;
  addPoint(23,12);
}

int main(void)
{
  // Set up display
  initwindow(750,600);
  setbkcolor(WHITE);
  setcolor(BLACK);
  setlinestyle(SOLID_LINE,EMPTY_FILL,THICK_WIDTH);
  setfillstyle(SOLID_FILL,BLACK);

  // Ready to go!
  cleardevice();
  initialize();
  draw();

  printf("Done - Enter to exit");
  getchar();
  closegraph();

  return(0);
}
