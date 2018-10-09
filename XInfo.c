

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