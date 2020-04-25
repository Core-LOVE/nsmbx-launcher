/*
 * X-Tech Launcher - a simple template game launcher
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef MENU_H
#define MENU_H

#include <stddef.h>
#include <SDL2/SDL_types.h>

struct App_t;
typedef struct App_t App;

typedef void (*MenuAction)(App *a);

typedef struct MenuItem_t
{
    const char *label;
    int x;
    int y;
    int w;
    int h;
    SDL_bool selected;
    SDL_bool choosen;
    MenuAction action;
} MenuItem;

typedef struct MenuCheckBox_t
{
    const char *label;
    int x;
    int y;
    int w;
    int h;
    SDL_bool selected;
    SDL_bool checkState;
    SDL_bool *dstValue;
} MenuCheckBox;

typedef struct Menu_t
{
    MenuItem s_menu[5];
    size_t s_menu_count;

    int s_menu_keypos;

    MenuCheckBox s_cb[5];
    size_t s_cb_count;
} Menu;

void initMenu(Menu *m, App *a);
void unInitMenu(Menu *m);
void drawFader(App *a);
void renderMenu(Menu *m, App *app);
void processMenuMouseMove(Menu *m, int x, int y);
void processMenuMousePress(Menu *m, App *a, int x, int y);
void processMenuKeyboard(Menu *m, App *a, int key);

#endif /* MENU_H */
