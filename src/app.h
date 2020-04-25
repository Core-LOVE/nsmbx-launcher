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

#ifndef APP_H
#define APP_H

#include <SDL2/SDL.h>

struct Menu_t;
typedef struct Menu_t Menu;

typedef struct App_t
{
    SDL_Window *m_window;
    SDL_Renderer *m_gRenderer;

    SDL_Texture *m_font;

    SDL_Texture *m_back;
    SDL_Texture *m_splash;

    char *m_windowTitle;
    int m_windowWidth;
    int m_windowHeight;

    int m_working;

    char *m_gamePath;
    char *m_editorPath;

    SDL_bool optNoSound;
    SDL_bool optFrameSkip;

    SDL_Event m_event;
    int fadeLevel;
} App;

extern int executeProcess(const char *path, char * const argv[]);

extern void initApp(App *a);
extern int initSdl(void);
extern void quitSdl(App *a);
extern int isSdlError(void);

extern void loadSetup(App *a);

extern int initWindow(App *a);
extern int initFont(App *a);

extern int initTextures(App *a);

extern void processEvent(Menu *m, App *a);
extern void waitEvents(Menu *m, App *a);
extern void doEvents(Menu *m, App *a);

extern void renderTexture(App *app, int xDst, int yDst, int wDst, int hDst,
                          SDL_Texture *t,
                          int xSrc, int ySrc, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
extern void renderTextureS(App *app, int x, int y, SDL_Texture *t);
extern void printText(App *app, const char *text, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
extern void getTextBlockSize(const char *text, int *w, int *h);

#endif /* APP_H */
