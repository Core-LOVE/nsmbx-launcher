/*
 * Description: simple read only ini parser
 *     History: yang@haipo.me, 2013/06/13, create
 */

#include <ctype.h>
#include <SDL2/SDL.h>

#include "ini.h"

static const Uint8 utfbom[3] = {0xEF, 0xBB, 0xBF};

enum { Space = 0x01, Special = 0x02, INIParamEq = 0x04 };

static const Uint8 charTraits[256] =
{
    /* Space: '\t', '\n', '\r', ' ' */
    /* Special: '\n', '\r', '"', ';', '=', '\\' */
    /* INIParamEq: ':', '=' */

    0, 0, 0, 0, 0, 0, 0, 0, 0, Space, Space | Special, 0, 0, Space | Special,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, Space, 0, Special,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, INIParamEq,
    Special, 0, Special | INIParamEq, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, Special, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

Uint8 IS_SPACE(char c)
{
    return (charTraits[(Uint8)(c)] & Space);
}

Uint8 IS_SPECIAL(char c)
{
    return (charTraits[(Uint8)(c)] & Special);
}

Uint8 IS_INIEQUAL(char c)
{
    return (Uint8)(charTraits[(Uint8)(c)] & INIParamEq);
}

/* Strip whitespace chars off end of given string, in place. Return s. */
char *rstrip(char *s)
{
    char *p = s + SDL_strlen(s);

    while(p > s && IS_SPACE(*--p))
        *p = '\0';

    return s;
}

/* Return pointer to first non-whitespace char in given string. */
char *lskip(char *s)
{
    while(*s && IS_SPACE(*s))
        s++;
    return (char *)(s);
}

char *lrtrim(char *s)
{
    char *p;
    while(*s && IS_SPACE(*s))
        s++;

    p = s + SDL_strlen(s);

    while(p > s && IS_SPACE(*--p))
        *p = '\0';

    return s;
}

char *removeQuotes(char *begin, char *end)
{
    if((*begin == '\0') || (begin == end))
        return begin;

    if((*begin == '"') && (begin + 1 != end))
        begin++;
    else
        return begin;

    if(*(end - 1) == '"')
        *(end - 1) = '\0';

    return begin;
}

static SDL_bool is_comment(char **line)
{
    char *content = *line;
    char *end;

    while(SDL_isspace(*content))
        ++content;

    if(*content == ';' || *content == '#' || *content == '\0')
        return SDL_TRUE;

    end = content + SDL_strlen(content) - 1;
    while(SDL_isspace(*end))
        *end-- = '\0';

    *line = content;

    return SDL_FALSE;
}


static Sint32 rw_getline(char **lineptr, size_t *n, SDL_RWops *stream)
{
    size_t nn = 0;
    char c;
    char *cur;
    *lineptr = SDL_malloc(2048);
    cur = *lineptr;

    do
    {
        if(SDL_RWread(stream, &c, 1, 1) < 1)
        {
            if(nn == 0)
            {
                SDL_free(*lineptr);
                *lineptr = NULL;
                return -1;
            }
            break;
        }
        if(c == '\r')
            continue;
        if(c == '\n')
            break;
        nn++;
        *cur = c;
        cur++;
    } while(nn < 2048);

    *cur = '\0';
    *n = nn;

    return (Sint32)nn;
}

static Sint32 _getline(char **lineptr, size_t *n, SDL_RWops *stream)
{
    char *_line = NULL;
    char *next_line;
    Sint32 next_len;
    Sint32 need_len;
    size_t _n    = 0;
    Sint32 len = rw_getline(lineptr, n, stream);

    if(len == -1)
        return -1;

    while(len >= 2 && (*lineptr)[len - 2] == '\\')
    {
        if(rw_getline(&_line, &_n, stream) == -1)
        {
            SDL_free(_line);
            return 0;
        }

        next_line = _line;

        while(SDL_isspace(*next_line))
            ++next_line;

        next_len = (Sint32)SDL_strlen(next_line);
        need_len = len - 1 + next_len + 1;

        if((Sint32)*n < need_len)
        {
            while((Sint32)*n < need_len)
                *n *= 2;

            *lineptr = SDL_realloc(*lineptr, *n);
            if(*lineptr == NULL)
            {
                SDL_free(_line);

                return -1;
            }
        }

        if(SDL_isspace((*lineptr)[len - 3]))
            (*lineptr)[len - 2] = '\0';
        else
            (*lineptr)[len - 2] = ' ';
        (*lineptr)[len - 1] = '\0';

        strcat(*lineptr, next_line);
        len = (Sint32)SDL_strlen(*lineptr);
    }

    if(_line)
        SDL_free(_line);

    return len;
}

void ini_free(ini_t *handler)
{
    struct ini_arg *arg_curr;
    struct ini_arg *arg_next;
    struct ini_section *curr = handler;
    struct ini_section *next = NULL;

    while(curr)
    {
        next = curr->next;

        arg_curr = curr->args;
        arg_next = NULL;

        while(arg_curr)
        {
            arg_next = arg_curr->next;

            SDL_free(arg_curr->name);
            SDL_free(arg_curr->value);
            SDL_free(arg_curr);

            arg_curr = arg_next;
        }

        SDL_free(curr->name);
        SDL_free(curr);

        curr = next;
    }

    return;
}

static void ini_print(ini_t *handler)
{
    (void)handler;
# ifdef DEBUG
    struct ini_section *curr = handler;

    while(curr)
    {
        if(curr->name == NULL)
            continue;

        printf("[%s]\n", curr->name);

        struct ini_arg *arg = curr->args;

        while(arg)
        {
            if(arg->name == NULL || arg->value == NULL)
                continue;

            printf("    %-20s = %s\n", arg->name, arg->value);
            arg = arg->next;
        }

        curr = curr->next;
    }
# endif

    return;
}

static struct ini_section *create_section(struct ini_section *head, char *name)
{
    struct ini_section *p = SDL_calloc(1, sizeof(struct ini_section));

    if(p == NULL)
    {
        ini_free(head);

        return NULL;
    }

    if((p->name = SDL_strdup(name)) == NULL)
    {
        ini_free(head);

        return NULL;
    }

    return p;
}

static struct ini_section *find_section(struct ini_section *head, char *name)
{
    struct ini_section *curr = head;

    while(curr)
    {
        if(curr->name && SDL_strcmp(curr->name, name) == 0)
            return curr;

        curr = curr->next;
    }

    return NULL;
}

static struct ini_arg *create_arg(struct ini_section *head, char *name, char *value)
{
    struct ini_arg *p = SDL_calloc(1, sizeof(struct ini_arg));

    if(p == NULL)
    {
        ini_free(head);

        return NULL;
    }

    if((p->name = SDL_strdup(name)) == NULL)
    {
        ini_free(head);

        return NULL;
    }

    if((p->value = SDL_strdup(value)) == NULL)
    {
        ini_free(head);

        return NULL;
    }

    return p;
}

static struct ini_arg *find_arg(struct ini_section *curr, char *name)
{
    struct ini_arg *arg = curr->args;

    while(arg)
    {
        if(arg->name && SDL_strcmp(arg->name, name) == 0)
            return arg;
        arg = arg->next;
    }

    return NULL;
}

ini_t *ini_load(char *path)
{
    struct ini_section *head = NULL;
    struct ini_section *prev = NULL;
    struct ini_section *curr = NULL;

    struct ini_arg *arg_curr = NULL;
    struct ini_arg *arg_prev = NULL;

    char magic[4];
    char *line  = NULL;
    size_t   n  = 0;
    Sint32 len = 0;

    char *delimiter;
    char *name;
    char *name_end;
    char *value;

    SDL_RWops *fp = SDL_RWFromFile(path, "r");
    if(fp == NULL)
        return NULL;

    /* Try to skip UTF8 BOM */
    if(SDL_RWread(fp, magic, 1, 3) < 3)
        SDL_RWseek(fp, 0, RW_SEEK_SET);

    if(SDL_memcmp(magic, utfbom, 3) != 0)
        SDL_RWseek(fp, 0, RW_SEEK_SET); /* no BOM detected */

    while((len = _getline(&line, &n, fp)) != -1)
    {
        char *s = line;

        if(is_comment(&s))
            continue;

        len = (Sint32)SDL_strlen(s);

        if(len >= 3 && s[0] == '[' && s[len - 1] == ']')
        {
            char *name = s + 1;
            while(SDL_isspace(*name))
                ++name;

            name_end = s + len - 1;
            *name_end-- = '\0';
            while(SDL_isspace(*name_end))
                *name_end-- = '\0';

            if((curr = find_section(head, name)) == NULL)
            {
                if((curr = create_section(head, name)) == NULL)
                {
                    SDL_free(line);

                    return NULL;
                }

                if(head == NULL)
                    head = curr;
                if(prev != NULL)
                    prev->next = curr;

                prev = curr;
                arg_prev = NULL;
            }
            else
            {
                arg_prev = curr->args;
                while(arg_prev->next != NULL)
                    arg_prev = arg_prev->next;
            }

            continue;
        }

        delimiter = strchr(s, '=');
        if(delimiter == NULL)
            continue;
        *delimiter = '\0';

        name = s;
        name_end = delimiter - 1;
        while(SDL_isspace(*name_end))
            *name_end-- = '\0';

        value = delimiter + 1;
        while(SDL_isspace(*value))
            value++;

        if(curr == NULL)
        {
            if((curr = create_section(head, "global")) == NULL)
            {
                SDL_free(line);
                return NULL;
            }

            if(head == NULL)
                head = curr;
            prev = curr;
            arg_prev = NULL;
        }

        if((arg_curr = find_arg(curr, name)) == NULL)
        {
            arg_curr = create_arg(head, name, value);
            if(arg_curr == NULL)
            {
                SDL_free(line);

                return NULL;
            }

            if(arg_prev)
                arg_prev->next = arg_curr;
            if(curr->args == NULL)
                curr->args = arg_curr;

            arg_prev = arg_curr;
        }
        else
        {
            char *old_value = arg_curr->value;
            if((arg_curr->value = SDL_strdup(value)) == NULL)
            {
                ini_free(head);
                SDL_free(line);
                return NULL;
            }
            SDL_free(old_value);
        }
    }

    SDL_free(line);
    SDL_RWclose(fp);

    if(head == NULL)
    {
        if((head = SDL_calloc(1, sizeof(struct ini_section))) == NULL)
            return NULL;
    }

    ini_print(head);

    return head;
}

int ini_read_str(ini_t *handler, char *section, char *name, char **value, char *default_value)
{
    struct ini_section *curr;

    if(!handler && name && value)
    {
        if(default_value)
        {
            *value = SDL_strdup(default_value);
            if(*value == NULL)
                return -1;
        }
        else
            *value = NULL;
        return -1;
    }

    if(!handler || !name || !value)
        return -1;

    if(section == NULL || *section == 0)
        section = "global";

    curr = handler;

    while(curr)
    {
        if(curr->name && SDL_strcmp(section, curr->name) == 0)
            break;

        curr = curr->next;
    }

    if(curr)
    {
        struct ini_arg *arg = curr->args;

        while(arg)
        {
            if(arg->name && arg->value && SDL_strcmp(arg->name, name) == 0)
            {
                *value = SDL_strdup(removeQuotes(arg->value, arg->value + SDL_strlen(arg->value)));
                if(*value == NULL)
                    return -1;
                return 0;
            }

            arg = arg->next;
        }
    }

    if(default_value)
    {
        *value = SDL_strdup(default_value);
        if(*value == NULL)
            return -1;
    }
    else
        *value = NULL;

    return 1;
}

static char *sstrncpy(char *dest, const char *src, size_t n)
{
    if(n == 0)
        return dest;

    dest[0] = 0;

    return strncat(dest, src, n - 1);
}

int ini_read_strn(ini_t *handler,
                  char *section, char *name, char *value, size_t n, char *default_value)
{
    char *s = NULL;
    int ret = ini_read_str(handler, section, name, &s, default_value);
    if(ret < 0)
        return ret;

    memset(value, 0, n);

    if(s)
    {
        sstrncpy(value, s, n);
        SDL_free(s);
    }

    return ret;
}

static int ini_read_num(ini_t *handler,
                        char *section, char *name, void *value, SDL_bool is_unsigned)
{
    char *s = NULL;
    int ret = ini_read_str(handler, section, name, &s, NULL);
    if(ret == 0)
    {
        if(is_unsigned)
            *(unsigned long long int *)value = SDL_strtoull(s, NULL, 0);
        else
            *(long long int *)value = SDL_strtoll(s, NULL, 0);

        SDL_free(s);
    }

    return ret;
}

# define INI_READ_SIGNED(type) do { \
    long long int v; \
    int ret = ini_read_num(handler, section, name, &v, SDL_FALSE); \
    if (ret == 0) { \
        *value = (type)v; \
    } \
    else if (ret > 0) { \
        *value = default_value; \
    } \
    return ret; \
} while (0)

# define INI_READ_UNSIGNED(type) do { \
    unsigned long long int v; \
    int ret = ini_read_num(handler, section, name, &v, SDL_TRUE); \
    if (ret == 0) { \
        *value = (type)v; \
    } \
    else if (ret > 0) { \
        *value = default_value; \
    } \
    return ret; \
} while (0)

int ini_read_int(ini_t *handler,
                 char *section, char *name, int *value, int default_value)
{
    INI_READ_SIGNED(int);
}

int ini_read_unsigned(ini_t *handler,
                      char *section, char *name, unsigned *value, unsigned default_value)
{
    INI_READ_UNSIGNED(unsigned);
}

int ini_read_int8(ini_t *handler,
                  char *section, char *name, int8_t *value, int8_t default_value)
{
    INI_READ_SIGNED(int8_t);
}

int ini_read_uint8(ini_t *handler,
                   char *section, char *name, uint8_t *value, uint8_t default_value)
{
    INI_READ_UNSIGNED(uint8_t);
}

int ini_read_int16(ini_t *handler,
                   char *section, char *name, int16_t *value, int16_t default_value)
{
    INI_READ_SIGNED(int16_t);
}

int ini_read_uint16(ini_t *handler,
                    char *section, char *name, uint16_t *value, uint16_t default_value)
{
    INI_READ_UNSIGNED(uint16_t);
}

int ini_read_int32(ini_t *handler,
                   char *section, char *name, int32_t *value, int32_t default_value)
{
    INI_READ_SIGNED(int32_t);
}

int ini_read_uint32(ini_t *handler,
                    char *section, char *name, uint32_t *value, uint32_t default_value)
{
    INI_READ_UNSIGNED(uint32_t);
}

int ini_read_int64(ini_t *handler,
                   char *section, char *name, int64_t *value, int64_t default_value)
{
    INI_READ_SIGNED(int64_t);
}

int ini_read_uint64(ini_t *handler,
                    char *section, char *name, uint64_t *value, uint64_t default_value)
{
    INI_READ_UNSIGNED(uint64_t);
}

int ini_read_float(ini_t *handler,
                   char *section, char *name, float *value, float default_value)
{
    char *s = NULL;
    int ret = ini_read_str(handler, section, name, &s, NULL);
    if(ret == 0)
    {
        *value = (float)SDL_strtod(s, NULL);
        SDL_free(s);
    }
    else if(ret > 0)
        *value = default_value;

    return ret;
}

int ini_read_double(ini_t *handler,
                    char *section, char *name, double *value, double default_value)
{
    char *s = NULL;
    int ret = ini_read_str(handler, section, name, &s, NULL);
    if(ret == 0)
    {
        *value = strtod(s, NULL);
        SDL_free(s);
    }
    else if(ret > 0)
        *value = default_value;

    return ret;
}

int ini_read_bool(ini_t *handler,
                  char *section, char *name, SDL_bool *value, SDL_bool default_value)
{
    char *s = NULL;
    int ret = ini_read_str(handler, section, name, &s, NULL);

    if(ret == 0)
    {
        int i;
        for(i = 0; s[i]; ++i)
            s[i] = (char)SDL_tolower((int)s[i]);

        if(strcmp(s, "true") == 0)
            *value = SDL_TRUE;
        else if(strcmp(s, "false") == 0)
            *value = SDL_FALSE;
        else
            *value = default_value;

        SDL_free(s);
    }
    else if(ret > 0)
        *value = default_value;

    return ret;
}

