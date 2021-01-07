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


static void startGame(App *app)
{
    char *args[] = {0, 0, 0, 0};
    int argc = 1;
    args[0] = app->m_gamePath;

    if(app->optNoSound)
        args[argc++] = "--no-sound";
    if(app->optFrameSkip)
        args[argc++] = "--frameskip";

    executeProcess(app->m_gamePath, args);
}

static void startEditor(App *app)
{
    char *args[] = {0, 0};
    args[0] = app->m_editorPath;
    executeProcess(app->m_editorPath, args);
}



static void initMenuItem(Menu *m, size_t i, const char *label, int x, int y, MenuAction action)
{
    m->s_menu[i].label = label;
    m->s_menu[i].x = x;
    m->s_menu[i].y = y;
    getTextBlockSize(label, &m->s_menu[i].w, &m->s_menu[i].h);
    m->s_menu[i].selected = SDL_FALSE;
    m->s_menu[i].choosen = SDL_FALSE;
    m->s_menu[i].action = action;
}

static void initCheckBox(Menu *m, size_t i, const char *label, int x, int y, SDL_bool *target)
{
    m->s_cb[i].label = label;
    m->s_cb[i].x = x;
    m->s_cb[i].y = y;
    getTextBlockSize(label, &m->s_cb[i].w, &m->s_cb[i].h);
    m->s_cb[i].w += 20; /* a width of check itself */
    m->s_cb[i].selected = SDL_FALSE;
    m->s_cb[i].checkState = *target;
    m->s_cb[i].dstValue = target;
}

void initMenu(Menu *m, App *a)
{
    initMenuItem(m, 0, "Start game", 20, 370, startGame);
    initMenuItem(m, 1, "Editor", 20, 400, startEditor);
    m->s_menu_count = 2;

    m->s_menu_keypos = -1;

    initCheckBox(m, 0, "No Sound", 10, 10, &a->optNoSound);
    initCheckBox(m, 1, "Frameskip", 10, 38, &a->optFrameSkip);
    m->s_cb_count = 2;
}

void unInitMenu(Menu *m)
{
    m->s_menu_count = 0;
    m->s_cb_count = 0;
}

SDL_bool checkCollision(MenuItem *it, int x, int y)
{
    if(x < it->x)
        return SDL_FALSE;
    if(y < it->y)
        return SDL_FALSE;
    if(x > it->x + it->w)
        return SDL_FALSE;
    if(y > it->y + it->h)
        return SDL_FALSE;
    return SDL_TRUE;
}

SDL_bool checkCollisionCB(MenuCheckBox *it, int x, int y)
{
    if(x < it->x)
        return SDL_FALSE;
    if(y < it->y)
        return SDL_FALSE;
    if(x > it->x + it->w)
        return SDL_FALSE;
    if(y > it->y + it->h)
        return SDL_FALSE;
    return SDL_TRUE;
}

void drawFader(App *a)
{
    SDL_Rect r;
    r.x = 0;
    r.y = 0;
    r.w = a->m_windowWidth;
    r.h = a->m_windowHeight;
    SDL_SetRenderDrawColor(a->m_gRenderer, 0, 0, 0, (255 - (Uint8)a->fadeLevel));
    SDL_RenderFillRect(a->m_gRenderer, &r);
}

void renderMenu(Menu *m, App *app)
{
    size_t i;
    Uint8 r = 255, g = 255, b = 255, a = 255;

    /* Background */
    renderTextureS(app, 0, 0, app->m_back);
    /* Splash logo */
    renderTextureS(app, 50, 80, app->m_splash);

    /* Menu items */
    for(i = 0; i < m->s_menu_count; i++)
    {
        if(m->s_menu[i].choosen)
        {
            g = 128;
            b = 128;
        }
        else
        {
            g = 255;
            b = 255;
            r = m->s_menu[i].selected ? 128 : 255;
        }
        printText(app, m->s_menu[i].label, m->s_menu[i].x, m->s_menu[i].y, r, g, b, a);
    }

    for(i = 0; i < m->s_cb_count; i++)
    {
        r = m->s_cb[i].selected ? 128 : 255;
        printText(app, m->s_cb[i].checkState ? "X" : "-", m->s_cb[i].x, m->s_cb[i].y, r, g, b, a);
        printText(app, m->s_cb[i].label, m->s_cb[i].x + 20, m->s_cb[i].y, r, g, b, a);
    }
}

void processMenuMouseMove(Menu *m, int x, int y)
{
    size_t i;
    m->s_menu_keypos = -1;
    for(i = 0; i < m->s_menu_count; i++)
    {
        m->s_menu[i].selected = checkCollision(&m->s_menu[i], x, y);
        m->s_menu_keypos = (int)i;
    }

    for(i = 0; i < m->s_cb_count; i++)
    {
        m->s_cb[i].selected = checkCollisionCB(&m->s_cb[i], x, y);
    }
}

void processMenuMousePress(Menu *m, App *a, int x, int y)
{
    size_t i;
    for(i = 0; i < m->s_menu_count; i++)
    {
        if(checkCollision(&m->s_menu[i], x, y))
        {
            m->s_menu[i].action(a);
            m->s_menu[i].choosen = SDL_TRUE;
            a->m_working = 0;
            break;
        }
    }

    for(i = 0; i < m->s_cb_count; i++)
    {
        if(checkCollisionCB(&m->s_cb[i], x, y))
        {
            m->s_cb[i].checkState = !m->s_cb[i].checkState;
            *(m->s_cb[i].dstValue) = m->s_cb[i].checkState;
            break;
        }
    }
}

void processMenuKeyboard(Menu *m, App *a, int key)
{
    size_t i;
    for(i = 0; i < m->s_menu_count; i++)
        m->s_menu[m->s_menu_keypos].selected = SDL_FALSE;

    switch(key)
    {
    case SDL_SCANCODE_DOWN:
        if(m->s_menu_keypos < 0)
            m->s_menu_keypos = 0;
        else
        {
            m->s_menu_keypos++;
            if(m->s_menu_keypos >= (int)m->s_menu_count)
                m->s_menu_keypos = 0;
        }
        m->s_menu[m->s_menu_keypos].selected = SDL_TRUE;
        break;

    case SDL_SCANCODE_UP:
        if(m->s_menu_keypos < 0)
            m->s_menu_keypos = (int)(m->s_menu_count - 1);
        else
        {
            m->s_menu_keypos--;
            if(m->s_menu_keypos < 0)
                m->s_menu_keypos = (int)(m->s_menu_count - 1);
        }
        m->s_menu[m->s_menu_keypos].selected = SDL_TRUE;
        break;

    case SDL_SCANCODE_RETURN:
    case SDL_SCANCODE_KP_ENTER:
        if(m->s_menu_keypos >= 0 && m->s_menu_keypos < (int)m->s_menu_count)
        {
            m->s_menu[m->s_menu_keypos].action(a);
            m->s_menu[m->s_menu_keypos].choosen = SDL_TRUE;
            a->m_working = 0;
        }
        break;

    case SDL_SCANCODE_ESCAPE:
        a->m_working = 0;
        break;
    }
}
