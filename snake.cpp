/*
    Snake
 
 - - - - - - - - - - - - - - - - - - - - - -
 
 Commands to compile and run:
 
 g++ -o snake snake.cpp -L/usr/X11R6/lib -lX11 -lstdc++
 ./snake
 
 Note: the -L option and -lstdc++ may not be needed on some machines.
 */

#include <iostream>
#include <list>
#include <cstdlib>
#include <sys/time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>

/*
 * Header files for X functions
 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>

using namespace std;

/*
 * Global game state variables
 */
static int FPS = 30;
static int speed = 4;
static int grow = 0;
const int growsize = 5;

const int Border = 1;
const int BufferSize = 10;
const int width = 800;
const int height = 600;
const int second = 1000000;
const int BoardWidth = 40;
const int BoardHeight = 30;
const int BoardSize = BoardWidth * BoardHeight;
const int PixelWidth = width/BoardWidth;
const int PixelHeight = height/BoardHeight;

/* 
 *Resizing Ratios
int pwr = PixelWidth/20; // PWR: Pixel Width Ratio;
int phr = PixelHeight/20; // PHR: Pixel Height Ratio;
 */


/* ======================================= X Display ======================================= */

/*
 * Information to draw on the window.
 */
struct XInfo {
    Display	 *display;
    int		 screen;
    Window	 window;
    GC		 gc[10];
    Pixmap pixmap;      // double buffer
    int		width;		// size of window
    int		height;
};


/*
 * Function to put out a message on error exits.
 */
void error( string str ) {
    cerr << str << endl;
    exit(0);
}


/*
 * An abstract class representing displayable things.
 */
class Displayable {
public:
   virtual void paint(XInfo &xinfo) = 0;
};

/* ======================================= Game Logic ======================================= */

static bool gameover = true;
static bool pausee = false;
static bool walls = false;

enum cell {empty = -1, fruit = -3, head = -2, snakeSegment};

enum direction {north = 1, south = -1, east = 2, west = -2};

int arrayPosition(int xpos, int ypos) {
    return xpos + ypos * BoardWidth;
}

bool outOfBound(int xpos, int ypos) {
    return (xpos < 0 || xpos >= BoardWidth) ||
            (ypos < 0 || ypos >= BoardHeight);
}

void drawApple(XInfo &xinfo, int xpos, int ypos) {
    XFillArc(xinfo.display, xinfo.pixmap, xinfo.gc[3], xpos, ypos, PixelWidth, PixelHeight, 0, 360*64);
    XDrawArc(xinfo.display, xinfo.pixmap, xinfo.gc[0], xpos, ypos, PixelWidth, PixelHeight, 0, 360*64);
    XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[5], (xpos+8), ypos, 6, 6);
    XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[5], (xpos+5), (ypos+3), 6, 6);
    XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[2], (xpos+12), (ypos+6), 4, 5);
}

class Board: public Displayable {
    friend class Snake;
    /*
     * board is internally represented as an array of ints
     * where the index = xpos + ypos*BoardWidth
     */
    int *array;
public:
    virtual void paint(XInfo &xinfo) {
        for (int i = 0; i < BoardSize; ++i) {
            if(array[i] == fruit) {
                drawApple(xinfo,i%BoardWidth*PixelWidth, i/BoardWidth*PixelHeight);
            } else if (array[i] != empty) {
                XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[1], i%BoardWidth*PixelWidth, i/BoardWidth*PixelHeight, PixelWidth, PixelHeight);
                XDrawRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[0], i%BoardWidth*PixelWidth, i/BoardWidth*PixelHeight, PixelWidth, PixelHeight);
            }
        }
    }
    
    Board(){
        array = new int[BoardSize];
        for (int i = 0; i < BoardSize; ++i) {
            array[i] = empty;
        }
    }
    
    void genFruit(){
        int x, y;
        do {
            x = rand() % BoardWidth;
            y = rand() % BoardHeight;
        }  while (array[arrayPosition(x,y)] != empty);
        array[arrayPosition(x,y)] = fruit;
    }
    
    void reset() {
        for (int i = 0; i < BoardSize; ++i) {
            array[i] = empty;
        }
    }
    
    ~Board() {
        delete [] array;
    }
};

class Snake : public Displayable {
    int xhead;
    int yhead;
    int tailPosition;
    int length;
    int direction;
    Board *theBoard;
public:
    virtual void paint(XInfo &xinfo) {
        
        ostringstream score;
        score << "length: " << length;
        string msg = score.str();
        ostringstream frames;
        ostringstream fast;
        frames << "FPS: " << FPS;
        fast << "speed: " << speed;
        string one = frames.str();
        string two = fast.str();
        
        //Displays Information: Length of snake; framerate; speed.
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[0], 25, height-25, one.c_str(), one.length());
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[0], 25, height-40, two.c_str(), two.length());
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[0], 25, 25, msg.c_str(), msg.length() );
        if(walls) {
            XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[0], width-100, height-25, "walls: on", 9);
            XDrawRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[0], 2, 2, width-4, height-4);
        }
        else XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[0], width-100, height-25, "walls: off", 10);
        
        // Decor for the snake head: draw two eyes and a tongue orienting the right direction
        int i = arrayPosition(xhead,yhead);
        int eye1x, eye1y, eye2x, eye2y, tongue1x, tongue1y, tongue2x, tongue2y;
        switch( direction ) {
            case north:
                eye1x = i%BoardWidth*PixelWidth + 4;
                eye1y = i/BoardWidth*PixelHeight + 3;
                eye2x = eye1x + 8;
                eye2y = eye1y;
                tongue1x = eye1x + 6;
                tongue1y = eye1y - 4;
                tongue2x = tongue1x;
                tongue2y = tongue1y - 6;
                XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[3], tongue2x+2, tongue2y-1, 3, 3);
                XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[3], tongue2x-3, tongue2y-1, 3, 3);
                break;
            case south:
                eye1x = i%BoardWidth*PixelWidth + 4;
                eye1y = i/BoardWidth*PixelHeight + 12;
                eye2x = eye1x + 8;
                eye2y = eye1y;
                tongue1x = eye1x + 6;
                tongue1y = eye1y + 8;
                tongue2x = tongue1x;
                tongue2y = tongue1y + 7;
                XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[3], tongue2x+2, tongue2y-1, 3, 3);
                XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[3], tongue2x-3, tongue2y-1, 3, 3);
                break;
            case east:
                eye1x = i%BoardWidth*PixelWidth + 12;
                eye1y = i/BoardWidth*PixelHeight + 4;
                eye2x = eye1x;
                eye2y = eye1y + 8;
                tongue1x = eye1x + 8;
                tongue1y = eye1y + 6;
                tongue2x = tongue1x + 7;
                tongue2y = tongue1y;
                XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[3], tongue2x-1, tongue2y+2, 3, 3);
                XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[3], tongue2x-1, tongue2y-3, 3, 3);
                break;
            case west:
                eye1x = i%BoardWidth*PixelWidth + 5;
                eye1y = i/BoardWidth*PixelHeight + 4;
                eye2x = eye1x;
                eye2y = eye1y + 8;
                tongue1x = eye1x - 6;
                tongue1y = eye1y + 5;
                tongue2x = tongue1x - 6;
                tongue2y = tongue1y;
                XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[3], tongue2x-1, tongue2y+2, 3, 3);
                XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[3], tongue2x-1, tongue2y-3, 3, 3);
                break;
        }
        XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[0], eye1x, eye1y, 4, 4); // eyes
        XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[0], eye2x, eye2y, 4, 4);
        XDrawLine(xinfo.display, xinfo.pixmap, xinfo.gc[3], tongue1x, tongue1y, tongue2x, tongue2y); // tongue
        return;
    }
    
    Snake(Board *b){
        // Sets a snake on the board of length 5;
        theBoard = b;
        direction = east;
        length = 5;
        xhead = BoardWidth/2;
        yhead = BoardHeight/2;
        tailPosition = arrayPosition(xhead-4,yhead);
        theBoard->array[arrayPosition(xhead-4, yhead)] = arrayPosition(xhead-3,yhead);
        theBoard->array[arrayPosition(xhead-3, yhead)] = arrayPosition(xhead-2,yhead);
        theBoard->array[arrayPosition(xhead-2, yhead)] = arrayPosition(xhead-1,yhead);
        theBoard->array[arrayPosition(xhead-1, yhead)] = arrayPosition(xhead,yhead);
        theBoard->array[arrayPosition(xhead, yhead)] = -2;
        theBoard->genFruit();
    }

    
    void changeDirection(int dir) { // turns left of right
        if(direction == dir || direction == -dir) return;
        else direction = dir;
    }
    
    void move(XInfo &xinfo) {
        
        int oldHead = arrayPosition(xhead, yhead);
        int oldTail = tailPosition;
        
        switch(direction) {
            case north: --yhead; break;
            case south: ++yhead; break;
            case east: ++xhead; break;
            case west: --xhead; break;
        }
        
        if(outOfBound(xhead,yhead)) {
            gameover = true;
            switch(direction) {
                case north: ++yhead; break;
                case south: --yhead; break;
                case east: --xhead; break;
                case west: ++xhead; break;
            }
            return;
        }
        
        int newHead = arrayPosition(xhead, yhead);
        int cell = theBoard->array[newHead];
        
        switch (cell) {
            case empty: {
                theBoard->array[oldHead] = newHead;
                theBoard->array[newHead] = -2;
                if(grow <= 0 ) {
                    tailPosition = theBoard->array[oldTail];
                    theBoard->array[oldTail] = empty;
                }
                else --grow;
                return; }
            case fruit: {
                length += growsize;
                theBoard->array[oldHead] = newHead;
                theBoard->array[newHead] = -2;
                theBoard->genFruit();
                grow = growsize;
                return; }
            default:
                gameover = true;
                return;
        }
    }
    
    void moveTwo(XInfo &xinfo) {
        
        int oldHead = arrayPosition(xhead, yhead);
        int oldTail = tailPosition;
        
        switch(direction) {
            case north: yhead = (yhead - 1 + BoardHeight)% BoardHeight; break;
            case south: yhead = (yhead + 1)% BoardHeight; break;
            case east: xhead = (xhead + 1)% BoardWidth; break;
            case west: xhead = (xhead - 1 + BoardWidth)% BoardWidth; break;
        }
        
        int newHead = arrayPosition(xhead, yhead);
        int cell = theBoard->array[newHead];
        
        // SMALL SEMANTIC GLITCH*
        switch (cell) {
            case empty: {
                theBoard->array[oldHead] = newHead;
                theBoard->array[newHead] = -2;
                if(grow <= 0 ) {
                    tailPosition = theBoard->array[oldTail];
                    theBoard->array[oldTail] = empty;
                }
                else -- grow;
                return; }
            case fruit: {
                length += growsize;
                theBoard->array[oldHead] = newHead;
                theBoard->array[newHead] = -2;
                theBoard->genFruit();
                grow = growsize-1;
                return; }
            default:
                gameover = true;
                return;
        }
    }
    
    void reset() {
        direction = east;
        length = 5;
        xhead = BoardWidth/2;
        yhead = BoardHeight/2;
        tailPosition = arrayPosition(xhead-4,yhead);
        theBoard->array[arrayPosition(xhead-4, yhead)] = arrayPosition(xhead-3,yhead);
        theBoard->array[arrayPosition(xhead-3, yhead)] = arrayPosition(xhead-2,yhead);
        theBoard->array[arrayPosition(xhead-2, yhead)] = arrayPosition(xhead-1,yhead);
        theBoard->array[arrayPosition(xhead-1, yhead)] = arrayPosition(xhead,yhead);
        theBoard->array[arrayPosition(xhead, yhead)] = -2;
        theBoard->genFruit();
    }
    
};


/* ======================================= Initialization ======================================= */

list<Displayable *> dList;           // list of Displayables
Board *board = new Board();
Snake snake(board);


/*
 * Initialize X and create a window
 */
void initX(int argc, char *argv[], XInfo &xInfo) {
    dList.push_front(&snake);
    dList.push_front(board);
    
    XSizeHints hints;
    unsigned long white, black;
    
    /*
     * Display opening uses the DISPLAY	environment variable.
     * It can go wrong if DISPLAY isn't set, or you don't have permission.
     */
    xInfo.display = XOpenDisplay( "" );
    if ( !xInfo.display )	{
        error( "Can't open display." );
    }
    
    /*
     * Find out some things about the display you're using.
     */
    xInfo.screen = DefaultScreen( xInfo.display );
    
    
    // olive
    XColor color;
    Colormap colormap;
    char olive[] = "#556B2F";
    colormap = DefaultColormap(xInfo.display, 0);
    XParseColor(xInfo.display, colormap, olive, &color);
    XAllocColor(xInfo.display, colormap, &color);
    
    //goldenrod
    XColor color1;
    Colormap colormap1;
    char goldenrod[] = "#B8860B";
    colormap1 = DefaultColormap(xInfo.display, 0);
    XParseColor(xInfo.display, colormap1, goldenrod, &color1);
    XAllocColor(xInfo.display, colormap1, &color1);
    
    //crimson
    XColor color2;
    Colormap colormap2;
    char crimson[] = "#CD5C5C";// alternatives -> #A52A2A #B03060 #CD5C5C
    colormap2 = DefaultColormap(xInfo.display, 0);
    XParseColor(xInfo.display, colormap2, crimson, &color2);
    XAllocColor(xInfo.display, colormap2, &color2);
    
    white = XWhitePixel( xInfo.display, xInfo.screen );
    black = XBlackPixel( xInfo.display, xInfo.screen );
    
    
    hints.x = 100;
    hints.y = 100;
    hints.width = 800;
    hints.height = 600;
    hints.flags = PPosition | PSize;
    
    xInfo.window = XCreateSimpleWindow(
                                       xInfo.display,				// display where window appears
                                       DefaultRootWindow( xInfo.display ), // window's parent in window tree
                                       hints.x, hints.y,			// upper left corner location
                                       hints.width, hints.height,	// size of the window
                                       Border,						// width of window's border
                                       black,						// window border colour
                                       color.pixel);					// window background colour
    
    
    XSetStandardProperties(
                           xInfo.display,		// display containing the window
                           xInfo.window,		// window whose properties are set
                           "Retro Snake",		// window's title
                           "Animate",			// icon's title
                           None,				// pixmap for the icon
                           argv, argc,			// applications command line args
                           &hints );			// size hints for the window
    
    /*
     * Create Graphics Contexts
     */
    
    Font font;
    font = XLoadFont(xInfo.display, "6x9");
    
    
    int i = 0; // black
    xInfo.gc[i] = XCreateGC(xInfo.display, xInfo.window, 0, 0);
    XSetForeground(xInfo.display, xInfo.gc[i], BlackPixel(xInfo.display, xInfo.screen));
    XSetBackground(xInfo.display, xInfo.gc[i], WhitePixel(xInfo.display, xInfo.screen));
    XSetFont(xInfo.display, xInfo.gc[i], font);
    XSetFillStyle(xInfo.display, xInfo.gc[i], FillSolid);
    XSetLineAttributes(xInfo.display, xInfo.gc[i],
                       2, LineSolid, CapButt, JoinRound);
    
    
    i = 1; //goldenrod
    xInfo.gc[i] = XCreateGC(xInfo.display, xInfo.window, 0, 0);
    XSetForeground(xInfo.display, xInfo.gc[i], color1.pixel);
    XSetBackground(xInfo.display, xInfo.gc[i], WhitePixel(xInfo.display, xInfo.screen));
    XSetFillStyle(xInfo.display, xInfo.gc[i], FillSolid);
    XSetLineAttributes(xInfo.display, xInfo.gc[i],
                       18, LineSolid, CapButt, JoinMiter);
    
    i = 2; //white
    xInfo.gc[i] = XCreateGC(xInfo.display, xInfo.window, 0, 0);
    XSetForeground(xInfo.display, xInfo.gc[i], WhitePixel(xInfo.display, xInfo.screen));
    XSetBackground(xInfo.display, xInfo.gc[i], WhitePixel(xInfo.display, xInfo.screen));
    XSetFont(xInfo.display, xInfo.gc[i], font);
    XSetFillStyle(xInfo.display, xInfo.gc[i], FillSolid);
    XSetLineAttributes(xInfo.display, xInfo.gc[i],
                       1, LineSolid, CapButt, JoinMiter);
    
    i = 3; // crimson
    xInfo.gc[i] = XCreateGC(xInfo.display, xInfo.window, 0, 0);
    XSetForeground(xInfo.display, xInfo.gc[i], color2.pixel);
    XSetBackground(xInfo.display, xInfo.gc[i], WhitePixel(xInfo.display, xInfo.screen));
    XSetFillStyle(xInfo.display, xInfo.gc[i], FillSolid);
    XSetLineAttributes(xInfo.display, xInfo.gc[i],
                       3, LineSolid, CapButt, JoinMiter);
    
    i = 4; // white; helvatical font size 360;
    string name = "-*-helvetica-bold-r-*-*-*-360-150-150-*-*-*-*";
    
    Font font1 = XLoadFont(xInfo.display, name.c_str());
    
    xInfo.gc[i] = XCreateGC(xInfo.display, xInfo.window, 0, 0);
    XSetForeground(xInfo.display, xInfo.gc[i], WhitePixel(xInfo.display, xInfo.screen));
    XSetBackground(xInfo.display, xInfo.gc[i], WhitePixel(xInfo.display, xInfo.screen));
    XSetFont(xInfo.display, xInfo.gc[i], font1);
    XSetFillStyle(xInfo.display, xInfo.gc[i], FillSolid);
    XSetLineAttributes(xInfo.display, xInfo.gc[i],
                       1, LineSolid, CapButt, JoinMiter);
    
    
    i = 5; //olive
    xInfo.gc[i] = XCreateGC(xInfo.display, xInfo.window, 0, 0);
    XSetForeground(xInfo.display, xInfo.gc[i], color.pixel);
    XSetBackground(xInfo.display, xInfo.gc[i], WhitePixel(xInfo.display, xInfo.screen));
    XSetFillStyle(xInfo.display, xInfo.gc[i], FillSolid);
    XSetLineAttributes(xInfo.display, xInfo.gc[i],
                       3, LineSolid, CapButt, JoinMiter);
    
    // Pixmap initiation
    int depth = DefaultDepth(xInfo.display, DefaultScreen(xInfo.display));
    xInfo.pixmap = XCreatePixmap(xInfo.display, xInfo.window, hints.width, hints.height, depth);
    xInfo.width = hints.width;
    xInfo.height = hints.height;
    
    
    XSelectInput(xInfo.display, xInfo.window,
                 ButtonPressMask | KeyPressMask |
                 PointerMotionMask |
                 EnterWindowMask | LeaveWindowMask |
                 StructureNotifyMask);  // for resize events
    
    /*
     * Don't paint the background -- reduce flickering
     */
    XSetWindowBackgroundPixmap(xInfo.display, xInfo.window, None);
    
    /*
     * Put the window on the screen.
     */
    XMapRaised( xInfo.display, xInfo.window );
    XFlush(xInfo.display);
}

/* ======================================= Startup Screen ======================================= */

class Splash : public Displayable {
    int studentNumber;

public:
    
    virtual void paint(XInfo &xinfo) {
        
        XFillRectangle(xinfo.display, xinfo.window, xinfo.gc[5], // background color
                       0, 0, xinfo.width, xinfo.height);
        XDrawLine(xinfo.display, xinfo.window, xinfo.gc[1], 450, 100, 659, 100); // snake
        XDrawLine(xinfo.display, xinfo.window, xinfo.gc[1], 650, 100, 650, 200);
        XDrawLine(xinfo.display, xinfo.window, xinfo.gc[1], 141, 200, 659, 200);
        XDrawLine(xinfo.display, xinfo.window, xinfo.gc[1], 150, 200, 150, 300);
        XDrawLine(xinfo.display, xinfo.window, xinfo.gc[1], 141, 300, 300, 300);
        XFillRectangle(xinfo.display, xinfo.window, xinfo.gc[0], 293, 293, 4, 4); // eyes
        XFillRectangle(xinfo.display, xinfo.window, xinfo.gc[0], 293, 303, 4, 4);
        XDrawLine(xinfo.display, xinfo.window, xinfo.gc[3], 300, 300, 309, 300); // tongue
        XFillRectangle(xinfo.display, xinfo.window, xinfo.gc[3], 308, 297, 3, 3);
        XFillRectangle(xinfo.display, xinfo.window, xinfo.gc[3], 308, 302, 3, 3);
        XFillArc(xinfo.display, xinfo.window, xinfo.gc[3], 370, 290, PixelWidth, PixelHeight, 0, 360*64);// apple
        XDrawArc(xinfo.display, xinfo.window, xinfo.gc[0], 370, 290, PixelWidth, PixelHeight, 0, 360*64);
        XFillRectangle(xinfo.display, xinfo.window, xinfo.gc[5], 378, 290, 6, 6);
        XFillRectangle(xinfo.display, xinfo.window, xinfo.gc[5], 375, 293, 6, 6);
        XFillRectangle(xinfo.display, xinfo.window, xinfo.gc[2], 382, 296, 4, 5);
        XDrawString(xinfo.display, xinfo.window, xinfo.gc[2], 350, 450, "PRESS SPACE TO PLAY", 19); //text
        XDrawString(xinfo.display, xinfo.window, xinfo.gc[2], 280, 490, "Eat the Fruit and Avoid Biting Yourself", 39); //text
        XDrawString(xinfo.display, xinfo.window, xinfo.gc[2], 320, 500, "Use WASD or Arrow Keys to Move", 30); //text
        XDrawString(xinfo.display, xinfo.window, xinfo.gc[2], 10, 590, "Created by: k4su (KeFei Su)", 27);
        XDrawString(xinfo.display, xinfo.window, xinfo.gc[2], 675, 590, "Student ID: 20613990", 20);
        XDrawString(xinfo.display, xinfo.window, xinfo.gc[4], 275, 175, "SNAKE", 5); // title
        XDrawString(xinfo.display, xinfo.window, xinfo.gc[4], 300, 250, "GAME", 4);
    }
    
    Splash(){
        studentNumber = 20613990;
    }
    
    ~Splash() {
    }
    
};


/* ======================================= Event Loop ======================================= */

Splash splash;

/*
 * Function to repaint a display list
 */
void repaint( XInfo &xinfo) {
    list<Displayable *>::const_iterator begin = dList.begin();
    list<Displayable *>::const_iterator end = dList.end();
    
    XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[5],
                   0, 0, xinfo.width, xinfo.height);
    
    // draw display list
    while( begin != end ) {
        Displayable *d = *begin;
        d->paint(xinfo);
        begin++;
    }
    // copy buffer to window
    XCopyArea(xinfo.display, xinfo.pixmap, xinfo.window, xinfo.gc[0],
              0, 0, xinfo.width, xinfo.height,  // region of pixmap to copy
              0, 0); // position to put top left corner of pixmap in window
    XFlush( xinfo.display );
}

void handleKeyPress(XInfo &xinfo, XEvent &event) {
    KeySym key;
    char text[BufferSize];
    
    /*
     * Exit when 'q' is typed.
     * This is a simplified approach that does NOT use localization.
     */
    int i = XLookupString(
                          (XKeyEvent *)&event, 	// the keyboard event
                          text, 					// buffer when text will be written
                          BufferSize, 			// size of the text buffer
                          &key, 					// workstation-independent key symbol
                          NULL );					// pointer to a composeStatus structure (unused)
    if ( i == 1) {
        //printf("Got key press -- %c\n", text[0]);
        if (text[0] == 'q') {
            error("Terminating normally.");
        } else if(text[0] == 'w' || text[0] == 'W') {
            snake.changeDirection(north);
        } else if(text[0] == 'a' || text[0] == 'A') {
            snake.changeDirection(west);
        } else if(text[0] == 's' || text[0] == 'S') {
            snake.changeDirection(south);
        } else if(text[0] == 'd' || text[0] == 'D') {
            snake.changeDirection(east);
        } else if (text[0] == '1') {
            if (speed != 1) --speed;
        } else if (text[0] == '2') {
            if (speed < 10) ++speed;
        } else if (text[0] == '3') {
            if(FPS <= 5) FPS = 1;
            else FPS -= 5;
        } else if (text[0] == '4') {
            if(FPS >= 100) FPS = 100;
            else FPS += 10;
        } else if (text[0] == ' ') {
            if ( gameover ) {
            gameover = false;
            board->reset();
            snake.reset();
            }
        } else if(text[0] == 'o' || text[0] == 'O') {
            walls = !walls;
        } else if(text[0] == 'p' || text[0] == 'P') {
            if ( !gameover ) pausee = !pausee;
        } else if(text[0] == 'r' || text[0] == 'R') {
            if( pausee ) {
                board->reset();
                snake.reset();
                pausee = false;
            }
        }
    }
    if ((key >= XK_Left) && (key <= XK_Down)){
        switch(key){
            case XK_Left: snake.changeDirection(west);break;
            case XK_Up: snake.changeDirection(north);break;
            case XK_Right: snake.changeDirection(east);break;
            case XK_Down: snake.changeDirection(south);break;
        }
    }
}

void handleResize(XInfo &xinfo, XEvent &event) {
    XConfigureEvent xce = event.xconfigure;
    //fprintf(stderr, "Handling resize  w=%d  h=%d\n", xce.width, xce.height);
    if (xce.width > xinfo.width || xce.height > xinfo.height) {
        XFreePixmap(xinfo.display, xinfo.pixmap);
        int depth = DefaultDepth(xinfo.display, DefaultScreen(xinfo.display));
        xinfo.pixmap = XCreatePixmap(xinfo.display, xinfo.window, xce.width, xce.height, depth);
        xinfo.width = xce.width;
        xinfo.height = xce.height;
        
        /* code below for risizing
          width = xinfo.width;
          height = xinfo.height;
          PixelWidth = width/BoardWidth;
          PixelHeight = height/BoardHeight;
        */
        
        repaint(xinfo);
    }
}

void handleAnimation(XInfo &xinfo, int inside) {
    usleep(second/(speed*speed));
    if (walls) snake.move(xinfo);
    else snake.moveTwo(xinfo);
}

// get microseconds
unsigned long now() {
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

void eventLoop(XInfo &xinfo) {
    
    if ( gameover ) return;
    
    XEvent event;
    unsigned long lastRepaint = 0;
    int inside = 0;
    
    lastRepaint = now();
    while( !gameover ) {
        while (XPending(xinfo.display) > 0) {
            XNextEvent( xinfo.display, &event );
            // cout << "event.type=" << event.type << "\n";
            switch( event.type ) {
                case KeyPress:
                    handleKeyPress(xinfo, event);
                    break;
                case EnterNotify:
                    inside = 1;
                    break;
                case LeaveNotify:
                    inside = 0;
                    break;
                case ConfigureNotify:
                    handleResize(xinfo, event);
                    break;
            }
        }
        while( pausee ) {
            XCopyArea(xinfo.display, xinfo.pixmap, xinfo.window, xinfo.gc[0], 0, 0, xinfo.width, xinfo.height, 0, 0);
            XDrawString(xinfo.display, xinfo.window, xinfo.gc[2], 370, 300, "GAME PAUSED", 11);
            XDrawString(xinfo.display, xinfo.window, xinfo.gc[2], 350, 350, "PRESS P TO UNPAUSE", 18);
            XDrawString(xinfo.display, xinfo.window, xinfo.gc[2], 350, 375, "PRESS R TO RESTART", 18);
            
            while (XPending(xinfo.display) > 0) {
                XNextEvent( xinfo.display, &event );
                // cout << "event.type=" << event.type << "\n";
                switch( event.type ) {
                    case KeyPress:
                        handleKeyPress(xinfo, event);
                    case ConfigureNotify:
                        handleResize(xinfo, event);
                        break;
                }
            }
            
            usleep(second/5);
            
        }
        
        handleAnimation(xinfo, inside);
        if (lastRepaint+(second/FPS) < now()) {
            repaint(xinfo);
            lastRepaint = now();
        }
        
    }
}

void playGame(XInfo &xinfo) {
    XEvent event;
    
    while( gameover ) {
        splash.paint(xinfo);
        
        while(XPending(xinfo.display) > 0) {
            XNextEvent(xinfo.display, &event);
            
            switch ( event.type ) {
                case KeyPress:
                    handleKeyPress(xinfo, event);
                    break;
                case ConfigureNotify:
                    handleResize(xinfo, event);
                    break;
            }
        }
        
        usleep(second/5);
    }
    
    while( true ) {
        eventLoop(xinfo); // game starts here
        XCopyArea(xinfo.display, xinfo.pixmap, xinfo.window, xinfo.gc[0], 0, 0, xinfo.width, xinfo.height, 0, 0);
        XDrawString(xinfo.display, xinfo.window, xinfo.gc[2], 380, 300, "YOU DIED", 8);
        XDrawString(xinfo.display, xinfo.window, xinfo.gc[2], 330, 350, "PRESS SPACE TO PLAY AGAIN", 25);
        while(XPending(xinfo.display) > 0) {
            XNextEvent(xinfo.display, &event);
            
            switch ( event.type ) {
                case KeyPress:
                    handleKeyPress(xinfo, event);
                    break;
                case ConfigureNotify:
                    handleResize(xinfo, event);
                    break;
            }
        }
        usleep(second/5);
    }
    
}

/* ======================================= Main Code ======================================= */

void settings(int argc, char*argv[]) {
    if (argc == 1) return;
    if (argc != 3) {
        cerr << "Inccorect number of arguments" << endl;
        return;
    }
    
    int frames = atoi(argv[1]);
    int fast = atoi(argv[2]);
    if (frames <= 0 || frames > 100 || fast > 10 || fast <= 0) {
        cerr << "Invalid arguments" << endl;
        return;
    }
    FPS = frames;
    speed = fast;
}

/*
 * Start executing here.
 *	 First initialize window.
 *	 Next loop responding to events.
 *	 Exit forcing window manager to clean up - cheesy, but easy.
 */
int main ( int argc, char *argv[] ) {
    settings( argc, argv );
    XInfo xInfo;
    initX(argc, argv, xInfo);
    playGame(xInfo);
    XCloseDisplay(xInfo.display);
    delete board;
    exit(0);
}
