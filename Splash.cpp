

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