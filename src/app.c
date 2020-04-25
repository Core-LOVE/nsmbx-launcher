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
#include "ini.h"

#include "font2.h"
#include "back.h"
#include "splash.h"

#include <locale.h>

#ifdef _WIN32
#include <windows.h>
#include <string.h>

int executeProcess(const char *path, char *const argv[])
{
    PROCESS_INFORMATION pinfo;
    BOOL success;
    wchar_t args_w[1024];
    char * const*arg;
    wchar_t *arg_w;
    wchar_t *path_w;
    STARTUPINFOW startupInfo =
    {
        sizeof( STARTUPINFO ), 0, 0, 0,
        (DWORD)CW_USEDEFAULT, (DWORD)CW_USEDEFAULT,
        (DWORD)CW_USEDEFAULT, (DWORD)CW_USEDEFAULT,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    if(!path)
        return -1;

    if(SDL_strlen(path) == 0)
        return -1;

    SDL_memset(args_w, 0, sizeof(args_w));

    path_w = (wchar_t*)SDL_iconv_utf8_ucs2(path);
    wcscat_s(args_w, 1024, L"\"");
    wcscat_s(args_w, 1024, path_w);
    wcscat_s(args_w, 1024, L"\"");
    SDL_free(path_w);

    arg = argv;
    while(*arg)
    {
        arg_w = (wchar_t*)SDL_iconv_utf8_ucs2(*arg);
        arg++;
        wcscat_s(args_w, 1024, L" ");
        wcscat_s(args_w, 1024, arg_w);
        SDL_free(arg_w);
    }

    success = CreateProcessW(0, args_w,
                             0, 0, FALSE, CREATE_UNICODE_ENVIRONMENT | CREATE_NEW_CONSOLE, 0,
                             0,
                             &startupInfo, &pinfo);
    if(success)
    {
        CloseHandle(pinfo.hThread);
        CloseHandle(pinfo.hProcess);
    }

    return 0;
}

#else

#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

int executeProcess(const char *path, char *const argv[])
{
    pid_t pid, pid2;
    struct sigaction noaction;
    int startedPipe[2];
    int pidPipe[2];
    char reply = '\0', c;
    ssize_t startResult, ret;
    int result, success;

    if(!path)
        return -1;

    if(SDL_strlen(path) == 0)
        return -1;

    SDL_Log("Executing process: [%s]", path);

    if(pipe(startedPipe) < 0)
        return -1;
    if(pipe(pidPipe) < 0)
    {
        close(startedPipe[0]);
        close(startedPipe[1]);
        return -1;
    }

    pid = fork();
    if(pid == 0)
    {
        SDL_memset(&noaction, 0, sizeof(noaction));
        noaction.sa_handler = SIG_IGN;
        sigaction(SIGPIPE, &noaction, 0);

        setsid();
        close(startedPipe[0]);
        close(pidPipe[0]);

        pid2 = fork();
        if(pid2 == 0)
        {
            close(pidPipe[1]);
            execv(path, argv);
            SDL_Log("Failed to execute %s", path);

            SDL_memset(&noaction, 0, sizeof(noaction));
            noaction.sa_handler = SIG_IGN;
            sigaction(SIGPIPE, &noaction, 0);

            c = '\1';
            ret = write(startedPipe[1], &c, 1);
            close(startedPipe[1]);

            exit(1);
        }
        else
        {
            c = '\2';
            ret = write(startedPipe[1], &c, 1);
        }

        close(startedPipe[1]);
        ret = write(pidPipe[1], (const char *)&pid2, sizeof(pid_t));
        if(ret < (ssize_t)sizeof(pid_t))
            exit(2);
        exit(1);
    }

    close(startedPipe[1]);
    close(pidPipe[1]);

    if(pid == -1)
    {
        close(startedPipe[0]);
        close(pidPipe[0]);
        return -1;
    }

    reply = '\0';
    startResult = read(startedPipe[0], &reply, 1);
    close(startedPipe[0]);
    waitpid(pid, &result, 0);
    success = (startResult != -1 && reply == '\0');
    if(success && pid)
    {
        pid_t actualPid = 0;
        if (read(pidPipe[0], (char *)&actualPid, sizeof(pid_t)) == sizeof(pid_t)) {
            SDL_Log("Started with PID %d", actualPid);
        }
    }
    close(pidPipe[0]);

    return 0;
}
#endif



void initApp(App *a)
{
    a->m_window = NULL;
    a->m_gRenderer = NULL;

    a->m_windowTitle = NULL;
    a->m_windowWidth = 600;
    a->m_windowHeight = 432;

    a->m_font = NULL;
    a->m_back = NULL;
    a->m_splash = NULL;

    a->m_gamePath = NULL;
    a->m_editorPath = NULL;

    a->m_working = 0;
    a->fadeLevel = 0;

    a->optNoSound = SDL_FALSE;
    a->optFrameSkip = SDL_FALSE;
}

int initSdl(void)
{
    Uint32 sdlInitFlags = 0;
    /* Prepare flags for SDL initialization */
#ifndef __EMSCRIPTEN__
    sdlInitFlags |= SDL_INIT_TIMER;
#endif
    sdlInitFlags |= SDL_INIT_VIDEO;
    sdlInitFlags |= SDL_INIT_EVENTS;
    sdlInitFlags |= SDL_INIT_JOYSTICK;

    /* Initialize SDL */
    return (SDL_Init(sdlInitFlags) == 0);
}

void quitSdl(App *a)
{
    if(a->m_gRenderer)
        SDL_DestroyRenderer(a->m_gRenderer);
    if(a->m_window)
        SDL_DestroyWindow(a->m_window);
    if(a->m_font)
        SDL_DestroyTexture(a->m_font);
    if(a->m_windowTitle)
        SDL_free(a->m_windowTitle);
    if(a->m_gamePath)
        SDL_free(a->m_gamePath);
    if(a->m_editorPath)
        SDL_free(a->m_editorPath);

    SDL_ClearError();
    SDL_Quit();
}

int isSdlError(void)
{
    const char *error = SDL_GetError();
    return (*error != '\0');
}

void loadSetup(App *a)
{
    ini_t *i = ini_load("launcher.ini");
    ini_read_str(i, "main", "title", &a->m_windowTitle, "<Untitled game launcher>");
    ini_read_str(i, "app", "game", &a->m_gamePath, "");
    ini_read_str(i, "app", "editor", &a->m_editorPath, "");
    ini_free(i);
}

int initWindow(App *a)
{
    SDL_ClearError();
    SDL_GL_ResetAttributes();

    /* Workaround: https://discourse.libsdl.org/t/26995 */
    setlocale(LC_NUMERIC, "C");

    a->m_window = SDL_CreateWindow(a->m_windowTitle,
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              a->m_windowWidth, a->m_windowHeight,
                              SDL_WINDOW_HIDDEN |
                              SDL_WINDOW_ALLOW_HIGHDPI);

    if(a->m_window == NULL)
    {
        SDL_Log("Can't init Window: %s", SDL_GetError());
        return 1;
    }

    if(isSdlError())
    {
        SDL_Log("Can't init Window: %s", SDL_GetError());
        return 2;
    }

    SDL_SetWindowMinimumSize(a->m_window, a->m_windowWidth, a->m_windowHeight);

    a->m_gRenderer = SDL_CreateRenderer(a->m_window, -1, SDL_RENDERER_SOFTWARE);
    if(!a->m_gRenderer)
    {
        SDL_Log("Can't init renderer: %s", SDL_GetError());
        return 3;
    }

    SDL_SetRenderDrawBlendMode(a->m_gRenderer, SDL_BLENDMODE_BLEND);
    SDL_ShowWindow(a->m_window);

    return 0;
}

void processEvent(Menu *m, App *a)
{
    switch(a->m_event.type)
    {
    case SDL_QUIT:
        a->m_working = 0;
        break;

    case SDL_MOUSEBUTTONUP:
        if(a->m_event.button.button == SDL_BUTTON_LEFT)
            processMenuMousePress(m, a, a->m_event.button.x, a->m_event.button.y);
        if(a->m_event.button.button == SDL_BUTTON_RIGHT)
            a->m_working = 0;
        break;

    case SDL_MOUSEMOTION:
        processMenuMouseMove(m, a->m_event.motion.x, a->m_event.motion.y);
        break;

    case SDL_KEYDOWN:
        processMenuKeyboard(m, a, (int)a->m_event.key.keysym.scancode);
        break;
    }
}

void waitEvents(Menu *m, App *a)
{
    if(SDL_WaitEvent(&a->m_event))
        processEvent(m, a);
    while(SDL_PollEvent(&a->m_event))
        processEvent(m, a);
}

void doEvents(Menu *m, App *a)
{
    while(SDL_PollEvent(&a->m_event))
        processEvent(m, a);
}

SDL_Texture *loadTexture(App *a, void *src, int size, SDL_bool key)
{
    SDL_RWops* font = SDL_RWFromMem(src, size);
    SDL_Surface *surface, *tempSur;
    SDL_Texture *dst = NULL;
    Uint32 rmask, gmask, bmask, amask;

    surface = SDL_LoadBMP_RW(font, 1);
    if(!surface)
    {
        SDL_Log("Can't load BMP: %s", SDL_GetError());
        return NULL;
    }

    if(key)
    {
#if SDL_BYTEORDER != SDL_BIG_ENDIAN
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
        amask = 0x000000ff;
#else
        rmask = 0x000000ff;
        gmask = 0x0000ff00;
        bmask = 0x00ff0000;
        amask = 0xff000000;
#endif

        SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 255, 0, 255));

        tempSur = SDL_CreateRGBSurface(SDL_SWSURFACE, surface->w, surface->h, 32, rmask, gmask, bmask, amask);
        SDL_BlitSurface(surface, NULL, tempSur,NULL);

        dst = SDL_CreateTextureFromSurface(a->m_gRenderer, tempSur);
        SDL_FreeSurface(surface);
        SDL_FreeSurface(tempSur);
    }
    else
    {
        dst = SDL_CreateTextureFromSurface(a->m_gRenderer, surface);
        SDL_FreeSurface(surface);
    }

    return dst;
}

int initFont(App *a)
{
    int fSize = sizeof(g_Font2_2_bmp_bank);
    a->m_font = loadTexture(a, g_Font2_2_bmp_bank, fSize, SDL_TRUE);
    if(!a->m_font)
    {
        SDL_Log("Can't init font: %s", SDL_GetError());
        return 1;
    }

    return 0;
}

int initTextures(App *a)
{
    int fSize;

    fSize = sizeof(g_back_bmp_bank);
    a->m_back = loadTexture(a, g_back_bmp_bank, fSize, SDL_FALSE);
    if(!a->m_back)
        return 1;

    fSize = sizeof(g_splash_bmp_bank);
    a->m_splash = loadTexture(a, g_splash_bmp_bank, fSize, SDL_TRUE);
    if(!a->m_splash)
        return 1;

    return 0;
}

void renderTexture(App *app, int xDst, int yDst, int wDst, int hDst,
                   SDL_Texture *t,
                   int xSrc, int ySrc, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    SDL_Rect destRect;
    SDL_Rect sourceRect;

    destRect.x = xDst;
    destRect.y = yDst;
    destRect.w = wDst;
    destRect.h = hDst;
    sourceRect.x = xSrc;
    sourceRect.y = ySrc;
    sourceRect.w = wDst;
    sourceRect.h = hDst;

    SDL_SetTextureColorMod(t, r, g, b);
    SDL_SetTextureAlphaMod(t, a);
    SDL_RenderCopyEx(app->m_gRenderer, t, &sourceRect, &destRect, 0, NULL, SDL_FLIP_NONE);
}

void renderTextureS(App *app, int x, int y, SDL_Texture *t)
{
    SDL_Rect destRect;
    SDL_Rect sourceRect;
    int w, h;
    SDL_QueryTexture(t, NULL, NULL, &w, &h);

    destRect.x = x;
    destRect.y = y;
    destRect.w = w;
    destRect.h = h;
    sourceRect.x = 0;
    sourceRect.y = 0;
    sourceRect.w = w;
    sourceRect.h = h;

    SDL_SetTextureColorMod(t, 255, 255, 255);
    SDL_SetTextureAlphaMod(t, 255);
    SDL_RenderCopyEx(app->m_gRenderer, t, &sourceRect, &destRect, 0, NULL, SDL_FLIP_NONE);
}

void printText(App *app, const char *text, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    size_t len = SDL_strlen(text);
    const char *cc = text, *c_end = text + len;
    char c;
    int B = 0, C = 0;

    for(; cc != c_end; cc++)
    {
        c = *cc;
        if(c >= 'a' && c <= 'z')
            c += 'A' - 'a';

        if(c >= 33 && c <= 126)
        {
            C = (c - 33) * 32;
            renderTexture(app, x + B, y, 18, 16, app->m_font, 2, C, r, g, b, a);
            B += 18;
            if(c == 'M')
                B += 2;
        }
        else
        {
            B += 16;
        }
    }
}

void getTextBlockSize(const char *text, int *w, int *h)
{
    size_t len = SDL_strlen(text);
    const char *cc = text, *c_end = text + len;
    char c;
    int B = 0;

    for(; cc != c_end; cc++)
    {
        c = *cc;
        if(c >= 'a' && c <= 'z')
            c += 'A' - 'a';

        if(c >= 33 && c <= 126)
        {
            B += 18;
            if(c == 'M')
                B += 2;
        }
        else
        {
            B += 16;
        }
    }

    *w = B;
    *h = 20;
}
