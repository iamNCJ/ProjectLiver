//
// Created by liujunliang on 2019/5/13.
//
#include "graphics.h"
#include "extgraph.h"
#include "genlib.h"
#include "simpio.h"
#include "conio.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include <windows.h>
#include <olectl.h>
#include <mmsystem.h>
#include <wingdi.h>
#include <ole2.h>
#include <ocidl.h>
#include <winuser.h>

#include "handlers.c"
#include "drawers.c"
#include "consts.c"



string TETRI_COLOR[8] = {
        "",//for null
        "BLUE",
        "DarkBlue",
        "Orange",
        "Yellow",
        "Green",
        "Purple",
        "Red"
};

tetrimino NaT;//Not a Tetrimino
tetrimino generateTetrimino(int type, int direction);

void timerEventHandler(int timerID);
tetrimino tetriMaintainer_on_gravity(int time, tetrimino tetri);


void Main(){
    NaT = generateTetrimino(0, 0); // Not a Tetri

    SetWindowTitle("Tetris");

    SetWindowSize(BLOCKSIZE*WIDTH,BLOCKSIZE*HEIGHT);
    InitGraphics();
    InitConsole();

    drawInit();
    registerTimerEvent(timerEventHandler);
    //registerMouseEvent(mouseEventHandler);
    //registerKeyboardEvent(keyboardEventHandler);

    tetrimino tetri = generateTetrimino(1, 0);
    tetriMaintainer_on_gravity(-1, tetri);

    registerTimerEvent(timerEventHandler);

    startTimer(MAINTAINER, 16);
}

tetrimino generateTetrimino(int type, int direction){
    tetrimino tetri;

    tetri.x = WIDTH/2;
    tetri.y = HEIGHT;
    tetri.type = type;
    tetri.direction = direction;
    tetri.color = TETRI_COLOR[type];

    if(type){
        drawTetri(tetri);
    }
    return tetri;
}

void timerEventHandler(int timerID){
    static int time = 0;
    tetrimino tetri;
    time = (time+1)%10000; // !!!
    Clean();
    drawInit();
    tetri = tetriMaintainer_on_gravity(time, NaT);
    drawTetri(tetri);
}

tetrimino tetriMaintainer_on_gravity(int time, tetrimino tetri){
    static int curTime = 0;
    static tetrimino curTetri;
    int dt;

    if(tetri.type && time < 0){
        curTetri = tetri;
        return NaT;
    }

    if(time >= 0 && curTetri.type) {
        if (time > curTime) {
            dt = time - curTime;
        } else {
            dt = time + 1000 - curTime;
        }
        if(dt == 24){
        curTetri.y -= 1;
        curTime = time;
        }
        return curTetri;
    }
}

