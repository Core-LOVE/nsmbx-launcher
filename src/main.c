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

#include "app.h"
#include "menu.h"

int main(int argc, char **argv)
{
    App a;
    Menu m;
    int ret;

    (void)argc; (void)argv;

    /* Initialize application */
    initApp(&a);

    /* Load settings from INI files */
    loadSetup(&a);

    if(!initSdl())
        return 1;

    ret = initWindow(&a);
    if(ret > 0)
    {
        quitSdl(&a);
        return ret;
    }

    ret = initFont(&a);
    if(ret > 0)
    {
        quitSdl(&a);
        return ret;
    }

    ret = initTextures(&a);
    if(ret > 0)
    {
        quitSdl(&a);
        return ret;
    }

    initMenu(&m, &a);

    a.m_working = 1;

    while(a.fadeLevel < 255)
    {
        SDL_SetRenderDrawColor(a.m_gRenderer, 255, 255, 255, 255);
        SDL_RenderClear(a.m_gRenderer);
        renderMenu(&m, &a);
        drawFader(&a);
        SDL_RenderPresent(a.m_gRenderer);
        doEvents(&m, &a);
        SDL_Delay(10);
        a.fadeLevel += 10;
    }

    a.fadeLevel = 255;

    while(a.m_working)
    {
        SDL_SetRenderDrawColor(a.m_gRenderer, 255, 255, 255, 255);
        SDL_RenderClear(a.m_gRenderer);
        renderMenu(&m, &a);
        SDL_RenderPresent(a.m_gRenderer);
        waitEvents(&m, &a);
    }

    while(a.fadeLevel > 0)
    {
        SDL_SetRenderDrawColor(a.m_gRenderer, 255, 255, 255, 255);
        SDL_RenderClear(a.m_gRenderer);
        renderMenu(&m, &a);
        drawFader(&a);
        SDL_RenderPresent(a.m_gRenderer);
        doEvents(&m, &a);
        SDL_Delay(10);
        a.fadeLevel -= 10;
    }

    unInitMenu(&m);

    quitSdl(&a);
    return 0;
}
