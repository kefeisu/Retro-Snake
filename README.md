# Retro-Snake
Retro Snake coded in C++ and XWindows


-----MAKE SURE THE COMPUTER RUNNING THE PROGRAM HAS HELVETICA (FONT) LISCENCED ON X11-------

Game developed/compiled/runs on APPLE OSX El Capitain (xQuartz) 

Use --arrow keys-- or --WASD-- to move the snake;
Press 1 to decrement the speed of the snake in real time (min 1)
Press 2 to increment the speed of the snake in real time (max 10)
Press 3 to decrement framerate by 5 FPS in real time (min 1)
Press 4 to increment framerate by 10 FPS in real time (max 100)

Press --P-- to pause/unpause the game;
Press --O-- to "Toggle walls On/Off" **

** Default setting for walls is off;
** When walls are off, the snake phases through the border to appear on the other side;
** When walls are on, if the snake collides with the border of the window, the game ends;


The length of the snake is displayed in the upper left corner (length is synonymous with score)
The speed and FPS are displayed in the bottom left corner
The snake grows by a length of 5 everytime a fruit is eaten.
