//
// Created by Tao Chiang on 5/17/2019.
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
#include <math.h>

#include <windows.h>
#include <olectl.h>
#include <mmsystem.h>
#include <wingdi.h>
#include <ole2.h>
#include <ocidl.h>
#include <winuser.h>

#include "model.h"
#include "consts.h"
#include "drawers.h"

#include "imgui.h"

#include "file_system_linked_list.h"

Checkerboard checkerboard;
// store the colors of block, white as 0, (x,y),  extended space are for easier(lazier) check...
tetrimino ctetri;

int Score = 0;
static int Mark[4] = {-1, -1, -1, -1};
static Checkerboard lastCheckerboard;
static Checkerboard clearCheckerboard;
tetrimino que[2];
tetrimino HoldedTetri;
bool is_game_over = FALSE;
bool isHoldLegal = TRUE;
double globalSpeed;

//MenuBar============================================

//===================================================

static int countScore[] = {0, 100, 200, 500, 1000};

static void game ();
static void flash ();
static Checkerboard ClearLine (Checkerboard checkerboard, int row);
static Checkerboard ClearLines (Checkerboard checkerboard);
static Checkerboard RemoveLines (Checkerboard checkerboard1);
static Checkerboard RemoveLine (Checkerboard checkerboard1, int row);
static tetrimino NextTetri ();

tetrimino generateTetrimino (int type, int direction)
{
    tetrimino tetri;

    tetri.x = LEFTBAR + 6;
    tetri.y = HEIGHT;
    tetri.type = type;
    tetri.direction = direction;
    tetri.color = TETRI_COLOR[type];
    tetri.yVelocity = 0;
    tetri.isPaused = FALSE;

    return tetri;
}
void timerEventHandler (int timerID)
{

    switch (timerID) {
        case GAME:game ();
            break;
        case CheckerboardFLASH :flash ();
            break;
        case GAMEOVER:drawCheckerBoard (checkerboard);
            drawUI (Score, que[1]);
            break;
        case DEBUG:printf ("%f", GetWindowWidth ());
            break;
    }
}
static void game ()
{

    static int time = 0;
    if (ctetri.yVelocity == 0 && !ctetri.isPaused) {
        ctetri = NextTetri ();
        ctetri = tetriMaintainer_on_gravity (time, ctetri);
    }

    time = (time + 1) % ERA; // !!!
    Clean ();

    drawCheckerBoard (checkerboard);
    ctetri = tetriMaintainer_on_gravity (time, ctetri);
    DrawShadow (HardDrop (ctetri));

    drawTetri (ctetri);
    drawUI (Score, que[1]);

    if (ctetri.yVelocity == 0 && !ctetri.isPaused) {
        Settle (ctetri); //add tetri to checker board
        globalSpeed = INIT_SPEED + INIT_SPEED * (Score / LevelGap); //update speed
        isHoldLegal = TRUE;
    }
    if (ctetri.isPaused) {
        DrawPulseBoard ();
    }
    if (is_game_over) {
        GameOver ();
    }
}
static void flash ()
{
    static int times = 0;
    if (times % 2 == 0) {
        drawCheckerBoard (lastCheckerboard);
    } else {
        drawCheckerBoard (clearCheckerboard);
    }
    drawUI (Score, que[1]);
    times++;
    if (times >= 6) {
        cancelTimer (CheckerboardFLASH);
        startTimer (GAME, 10);
        times = 0;
    }
}

tetrimino tetriMaintainer_on_gravity (int time, tetrimino tetri)
{
    static int curTime = 0;
    static double dy = 0;
    int dt;
    if (tetri.yVelocity == 0 && !tetri.isPaused) {
        tetri.yVelocity = globalSpeed;
    }
    tetrimino last = tetri;
    if (time > curTime) {
        dt = (time - curTime);
    } else {
        dt = (time + ERA - curTime);
    }

    dy += tetri.yVelocity * dt;
    if (dy >= 1) {
        tetri.y -= 1;
        dy = 0;
    }

    curTime = time;

    if (!check_collision (tetri)) {
        return tetri;
    } else {
        last.yVelocity = 0;
        return last;
    }

}

void Settle (tetrimino tetri)
{
    checkerboard = Settle_Tetri (tetri, checkerboard);
    lastCheckerboard = checkerboard;
    clearCheckerboard = ClearLines (checkerboard);
    if (Mark[0] != -1) {
        cancelTimer (GAME);
        startTimer (CheckerboardFLASH, 100);
    }
    checkerboard = RemoveLines (checkerboard);
    if (CheckTop () == FALSE) {
        is_game_over = TRUE;
    }
}

bool CheckTop ()
{
    bool isTopped = 0;
    for (int i = 1; i <= 12; i++) {
        if (checkerboard.block[i][19]) {
            isTopped = 1;
        }
    }
    if (isTopped) {
        return FALSE;
    } else {
        return TRUE;
    }
}

static Checkerboard ClearLines (Checkerboard checkerboard1)
{
    int num = 0;
    int i, j, line_ok;
    for (i = 1; i <= 18; i++) {
        line_ok = TRUE;
        for (j = 1; j <= 12; j++) {
            if (!checkerboard1.block[j][i]) {
                line_ok = FALSE;
                break;
            }
        }
        if (line_ok) {
            checkerboard1 = ClearLine (checkerboard1, i);
            Mark[num++] = i;
        }
    }
    Score += countScore[num];
    return checkerboard1;
}

static Checkerboard ClearLine (Checkerboard checkerboard1, int row)
{
    int j;
    for (j = 1; j <= 12; j++)
        checkerboard1.block[j][row] = 0;

    return checkerboard1;
}
static Checkerboard RemoveLines (Checkerboard checkerboard1)
{
    int i, j;
    for (i = 1, j = 0; i <= 18;) {
        if (i == Mark[j] - j) {
            checkerboard1 = RemoveLine (checkerboard1, i);
            Mark[j] = -1;
            j++;
        } else {
            i++;
        }
    }
    return checkerboard1;
}
static Checkerboard RemoveLine (Checkerboard checkerboard1, int row)
{
    int i, j;
    for (i = row; i <= 18; i++) {
        for (j = 1; j <= 12; j++)
            checkerboard1.block[j][i] = checkerboard1.block[j][i + 1];
    }
    return checkerboard1;
}

tetrimino NextTetri ()
{
    que[0] = que[1];
    que[1] = tetriRandom ();
    return que[0];
}

tetrimino tetriRandom ()
{
    static int last = 0;
    int type;
    do {
        type = rand () % 7;
        if (type == 0) type = 7;
    } while (last == type);
    last = type;
    int direction = rand () % 2;
    return generateTetrimino (type, direction);
}

static Checkerboard generateInitCheckerboard ()
{
    int i, j;
    Checkerboard EmptyCheckerboard;
    for (i = 0; i < 14; i++)
        for (j = 0; j < 25; j++)
            EmptyCheckerboard.block[i][j] = 0;
    for (i = 0; i < 20; i++)
        EmptyCheckerboard.block[0][i] = EmptyCheckerboard.block[13][i] = 1;
    for (i = 1; i < 13; i++)
        EmptyCheckerboard.block[i][0] = 1; // block_color[i][19] =
    // rewrite the boundary as 1
    for (i = 1; i <= 12; i++) {
        for (j = 1; j < 20; j++)
            EmptyCheckerboard.block[i][j] = 0;
    }

    return EmptyCheckerboard;
}

void InitModel ()
{
    cancelTimer (GAMEOVER);
    checkerboard = generateInitCheckerboard ();
    ctetri = tetriRandom ();
    ctetri.yVelocity = INIT_SPEED;
    Score = 0;

    globalSpeed = INIT_SPEED;
    is_game_over = FALSE;
    isHoldLegal = TRUE;
    HoldedTetri = generateTetrimino (0, 0);
    que[0] = tetriRandom ();
    que[1] = tetriRandom ();
    //For MenuBar
    //For Game STATE
    setMenuColors ("Black", "White", "Light Gray", "White", 1);
}

bool check_collision (tetrimino tetri)
{
    switch (tetri.direction) {
        case 0:
            for (int i = 0; i < 4; i++) {
                if (checkerboard.block[tetri.x - LEFTBAR + 1 + typeInfo[tetri.type][i][0]][tetri.y
                                                                                           + typeInfo[tetri.type][i][1]
                                                                                           + 1])
                    return TRUE;
            }
            return FALSE;
            break;
        case 1:
            for (int i = 0; i < 4; i++) {
                if (checkerboard.block[tetri.x - LEFTBAR + 1 - typeInfo[tetri.type][i][1]][tetri.y
                                                                                           + typeInfo[tetri.type][i][0]
                                                                                           + 1])
                    return TRUE;
            }
            return FALSE;
            break;
        case 2:
            for (int i = 0; i < 4; i++) {
                if (checkerboard.block[tetri.x - LEFTBAR + 1 - typeInfo[tetri.type][i][0]][tetri.y
                                                                                           - typeInfo[tetri.type][i][1]
                                                                                           + 1])
                    return TRUE;
            }
            return FALSE;
            break;
        case 3:
            for (int i = 0; i < 4; i++) {
                if (checkerboard.block[tetri.x - LEFTBAR + 1 + typeInfo[tetri.type][i][1]][tetri.y
                                                                                           - typeInfo[tetri.type][i][0]
                                                                                           + 1])
                    return TRUE;
            }
            return FALSE;
            break;
    }
}

Checkerboard Settle_Tetri (tetrimino tetri, Checkerboard checker)
{
    switch (tetri.direction) {
        case 0:
            for (int i = 0; i < 4; i++) {
                checker.block[tetri.x - LEFTBAR + 1 + typeInfo[tetri.type][i][0]][tetri.y + typeInfo[tetri.type][i][1]
                                                                                  + 1] = tetri.type;
            }
            break;
        case 1:
            for (int i = 0; i < 4; i++) {
                checker.block[tetri.x - LEFTBAR + 1 - typeInfo[tetri.type][i][1]][tetri.y + typeInfo[tetri.type][i][0]
                                                                                  + 1] = tetri.type;
            }
            break;
        case 2:
            for (int i = 0; i < 4; i++) {
                checker.block[tetri.x - LEFTBAR + 1 - typeInfo[tetri.type][i][0]][tetri.y - typeInfo[tetri.type][i][1]
                                                                                  + 1] = tetri.type;
            }
            break;
        case 3:
            for (int i = 0; i < 4; i++) {
                checker.block[tetri.x - LEFTBAR + 1 + typeInfo[tetri.type][i][1]][tetri.y - typeInfo[tetri.type][i][0]
                                                                                  + 1] = tetri.type;
            }
            break;
    }
    return checker;

}

tetrimino HardDrop (tetrimino tetri)
{
    while (!check_collision (tetri)) {
        tetri.y--;
    }
    tetri.y++;
    return tetri;
}

tetrimino Restart ()
{
    if (is_game_over) {
        is_game_over = FALSE;
        startTimer (GAME, 10);
    }

    InitModel ();
    drawUI (0, que[1]);

}

tetrimino HoldEventHandler (tetrimino temp)
{
    if (isHoldLegal) {
        if (HoldedTetri.type == 0) {
            HoldedTetri = temp;
            temp = que[1];
        } else {
            temp = HoldedTetri;
            temp.yVelocity = globalSpeed;
            HoldedTetri = ctetri;
        }
        HoldedTetri.y = 18;
        isHoldLegal = FALSE;
    }
    return temp;
}

tetrimino PauseEventHandler (tetrimino temp)
{
    if (!temp.isPaused) {
        temp.isPaused = TRUE;
        temp.yVelocity = 0;
    } else {
        temp.yVelocity = globalSpeed;
        temp.isPaused = FALSE;
    }
    return temp;
}

void ExitGame ()
{
    exit (0);
    //To laucher
}

void GameOver ()
{
    is_game_over = TRUE;
    cancelTimer (GAME);
    userNode *rank_list = Load_Rank ();
    rank_list = Add_Node (rank_list, Score, "game_debug");
    write_Rank (rank_list);
    char buffer[32];
    sprintf (buffer, "Thanks for playing, your score is %d!", Score);
    MessageBoxA (NULL, buffer, "Game Over", MB_ICONINFORMATION);
    startTimer (GAMEOVER, 10);
}