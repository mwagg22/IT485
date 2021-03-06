#include "gfc_input.h"
#include "gfc_list.h"
#include "simple_logger.h"
#include <simple_json.h>

static List *gfc_input_list = NULL;
static const Uint8 * gfc_input_keys = NULL;
static Uint8 * gfc_input_old_keys = NULL;
static int gfc_input_key_count = 0;

Input *gfc_input_get_by_name(const char *name);


void gfc_input_delete(Input *in)
{
    if (!in)return;
    if (in->onDelete != NULL)
    {
        in->onDelete(in->data);
    }
    gfc_list_delete(in->keyCodes);// data in the list is just integers
    free(in);
}

Input *gfc_input_new()
{
    Input *in = NULL;
    in = (Input *)malloc(sizeof(Input));
    if (!in)
    {
        slog("Failed to allocate space for input");
        return NULL;
    }
    memset(in,0,sizeof(Input));
    in->keyCodes = gfc_list_new();
    return in;
}

void gfc_input_set_callbacks(
    char *command,
    void (*onPress)(void *data),
    void (*onHold)(void *data),
    void (*onRelease)(void *data),
    void (*onDelete)(void *data),
    void *data
)
{
    Input *in;
    if (!command)return;
    in = gfc_input_get_by_name(command);
    if (!in)return;
    in->onPress = onPress;
    in->onHold = onHold;
    in->onRelease = onRelease;
    in->onDelete = onDelete;
    in->data = data;
}

void gfc_input_close()
{
    gfc_input_commands_purge();
    gfc_list_delete(gfc_input_list);
    gfc_input_list = NULL;
    if (gfc_input_old_keys)
    {
        free(gfc_input_old_keys);
    }
}

void gfc_input_commands_purge()
{
    Uint32 c,i;
    void *data;
    c = gfc_list_get_count(gfc_input_list);
    for (i = 0;i < c;i++)
    {
        data = gfc_list_get_nth(gfc_input_list,i);
        if (!data)continue;
        gfc_input_delete((Input*)data);
    }
    gfc_list_delete(gfc_input_list);
    gfc_input_list = gfc_list_new();
}

void gfc_input_update_command(Input *command)
{
    Uint32 c,i;
    SDL_Scancode kc;
    int old = 0, new = 0;
    if (!command)return;
    c = gfc_list_get_count(command->keyCodes);
    if (!c)return;// no commands to update this with, do nothing
    for (i = 0; i < c; i++)
    {
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
        kc = (SDL_Scancode)gfc_list_get_nth(command->keyCodes,i);
        if (!kc)continue;
        if(gfc_input_old_keys[kc])old++;
        if(gfc_input_keys[kc])new++;
    }
    if ((old == c)&&(new == c))
    {
        command->state = IET_Hold;
        if (command->onHold)
        {
            command->onHold(command->data);
        }
    }
    else if ((old == c)&&(new != c))
    {
        command->state = IET_Release;
        if (command->onRelease)
        {
            command->onRelease(command->data);
        }
    }
    else if ((old != c)&&(new == c))
    {
        command->state = IET_Press;
        command->pressTime = SDL_GetTicks();
        if (command->onPress)
        {
            command->onPress(command->data);
        }
    }
    else
    {
        command->state = IET_Idle;
    }
}

Input *gfc_input_get_by_name(const char *name)
{
    Uint32 c,i;
    Input *in;
    if (!name)
    {
        return NULL;
    }
    c = gfc_list_get_count(gfc_input_list);
    for (i = 0;i < c;i++)
    {
        in = (Input *)gfc_list_get_nth(gfc_input_list,i);
        if (!in)continue;
        if (gfc_line_cmp(in->command,name)==0)
        {
            return in;
        }
    }
    return 0;
}

Uint8 gfc_input_command_pressed(const char *command)
{
    Input *in;
    in = gfc_input_get_by_name(command);
    if ((in)&&(in->state == IET_Press))return 1;
    return 0;
}

Uint8 gfc_input_command_held(const char *command)
{
    Input *in;
    in = gfc_input_get_by_name(command);
    if ((in)&&(in->state == IET_Hold))return 1;
    return 0;
}

Uint8 gfc_input_command_released(const char *command)
{
    Input *in;
    in = gfc_input_get_by_name(command);
    if ((in)&&(in->state == IET_Release))return 1;
    return 0;
}

Uint8 gfc_input_command_down(const char *command)
{
    Input *in;
    in = gfc_input_get_by_name(command);
    if (in)
    {
        if(in->state)return 1;
    }
    return 0;
}

InputEventType gfc_input_command_get_state(const char *command)
{
    Input *in;
    in = gfc_input_get_by_name(command);
    if (!in)return 0;
    return in->state;
}


void gfc_input_update()
{
    Uint32 c,i;
    void *data;

    memcpy(gfc_input_old_keys,gfc_input_keys,sizeof(Uint8)*gfc_input_key_count);
    SDL_PumpEvents();   // update SDL's internal event structures
    gfc_input_keys = SDL_GetKeyboardState(&gfc_input_key_count);

    c = gfc_list_get_count(gfc_input_list);
    for (i = 0;i < c;i++)
    {
        data = gfc_list_get_nth(gfc_input_list,i);
        if (!data)continue;
        gfc_input_update_command((Input*)data);        
    }
}

void gfc_input_init(char *configFile)
{
    if (gfc_input_list != NULL)
    {
        slog("gfc_input_init: error, gfc_input_list not NULL");
        return;
    }
    gfc_input_list = gfc_list_new();
    gfc_input_keys = SDL_GetKeyboardState(&gfc_input_key_count);
    if (!gfc_input_key_count)
    {
        slog("failed to get keyboard count!");
    }
    else
    {
        gfc_input_old_keys = (Uint8*)malloc(sizeof(Uint8)*gfc_input_key_count);
        memcpy(gfc_input_old_keys,gfc_input_keys,sizeof(Uint8)*gfc_input_key_count);
    }
    atexit(gfc_input_close);
    gfc_input_commands_load(configFile);
}

SDL_Scancode gfc_input_key_to_scancode(const char * buffer)
{
    int F = 0;
    SDL_Scancode kc = -1;
    if (strlen(buffer) == 1)
    {
        //single letter code
        if ((buffer[0] >= 'a')&&(buffer[0] <= 'z'))
        {
            kc = SDL_SCANCODE_A + buffer[0] - 'a';
        }
        else if (buffer[0] == '0')
        {
            kc = SDL_SCANCODE_0;
        }
        else if (buffer[0] == '1')
        {
            kc = SDL_SCANCODE_1;
        }
        else if (buffer[0] == '2')
        {
            kc = SDL_SCANCODE_2;
        }
        else if (buffer[0] == '3')
        {
            kc = SDL_SCANCODE_3;
        }
        else if (buffer[0] == '4')
        {
            kc = SDL_SCANCODE_4;
        }
        else if (buffer[0] == '5')
        {
            kc = SDL_SCANCODE_5;
        }
        else if (buffer[0] == '6')
        {
            kc = SDL_SCANCODE_6;
        }
        else if (buffer[0] == '7')
        {
            kc = SDL_SCANCODE_7;
        }
        else if (buffer[0] == '8')
        {
            kc = SDL_SCANCODE_8;
        }
        else if (buffer[0] == '9')
        {
            kc = SDL_SCANCODE_9;
        }
        else if (buffer[0] == '-')
        {
            kc = SDL_SCANCODE_MINUS;
        }
        else if (buffer[0] == '=')
        {
            kc = SDL_SCANCODE_EQUALS;
        }
        else if (buffer[0] == '[')
        {
            kc = SDL_SCANCODE_LEFTBRACKET;
        }
        else if (buffer[0] == ']')
        {
            kc = SDL_SCANCODE_RIGHTBRACKET;
        }
        else if (buffer[0] == '.')
        {
            kc = SDL_SCANCODE_PERIOD;
        }
        else if (buffer[0] == ',')
        {
            kc = SDL_SCANCODE_COMMA;
        }
        else if (buffer[0] == ';')
        {
            kc = SDL_SCANCODE_SEMICOLON;
        }
        else if (buffer[0] == '\\')
        {
            kc = SDL_SCANCODE_BACKSLASH;
        }
        else if (buffer[0] == '/')
        {
            kc = SDL_SCANCODE_SLASH;
        }
        else if (buffer[0] == '\'')
        {
            kc = SDL_SCANCODE_APOSTROPHE;
        }
        else if (buffer[0] == ';')
        {
            kc = SDL_SCANCODE_SEMICOLON;
        }
        else if (buffer[0] == '`')
        {
            kc = SDL_SCANCODE_GRAVE;
        }
        else if ((buffer[0] >= ' ')&&(buffer[0] <= '`'))
        {
            kc = SDL_SCANCODE_SPACE + buffer[0] - ' ';
        }
    }
    else
    {
        if (buffer[0] == 'F')
        {
            F = atoi(&buffer[1]);
            if (F <= 12)
            {
                kc = SDL_SCANCODE_F1 + F - 1; 
            }
            else if (F <= 24)
            {
                kc = SDL_SCANCODE_F13 + F - 1; 
            }
        }
        else if (strcmp(buffer,"BACKSPACE") == 0)
        {
            kc = SDL_SCANCODE_BACKSPACE;
        }
        else if (strcmp(buffer,"LALT") == 0)
        {
            kc = SDL_SCANCODE_LALT;
        }
        else if (strcmp(buffer,"RALT") == 0)
        {
            kc = SDL_SCANCODE_RALT;
        }
        else if (strcmp(buffer,"LSHIFT") == 0)
        {
            kc = SDL_SCANCODE_LSHIFT;
        }
        else if (strcmp(buffer,"RSHIFT") == 0)
        {
            kc = SDL_SCANCODE_RSHIFT;
        }
        else if (strcmp(buffer,"LCTRL") == 0)
        {
            kc = SDL_SCANCODE_LCTRL;
        }
        else if (strcmp(buffer,"RCTRL") == 0)
        {
            kc = SDL_SCANCODE_RCTRL;
        }
        else if (strcmp(buffer,"TAB") == 0)
        {
            kc = SDL_SCANCODE_TAB;
        }
        else if (strcmp(buffer,"RETURN") == 0)
        {
            kc = SDL_SCANCODE_RETURN;
        }
        else if (strcmp(buffer,"BACKSPACE") == 0)
        {
            kc = SDL_SCANCODE_BACKSPACE;
        }
        else if (strcmp(buffer,"ESCAPE") == 0)
        {
            kc = SDL_SCANCODE_ESCAPE;
        }
    }
    if (kc == -1)
    {
        slog("no input mapping available for %s",buffer);
    }
    return kc;
}

Uint8 gfc_input_key_pressed(const char *key)
{
    SDL_Scancode kc;
    kc = gfc_input_key_to_scancode(key);
    if (kc == -1)return 0;
    if ((!gfc_input_old_keys[kc])&&(gfc_input_keys[kc]))return 1;
    return 0;
}

Uint8 gfc_input_key_released(const char *key)
{
    SDL_Scancode kc;
    kc = gfc_input_key_to_scancode(key);
    if (kc == -1)return 0;
    if ((gfc_input_old_keys[kc])&&(!gfc_input_keys[kc]))return 1;
    return 0;
}

Uint8 gfc_input_key_held(const char *key)
{
    SDL_Scancode kc;
    kc = gfc_input_key_to_scancode(key);
    if (kc == -1)return 0;
    if ((gfc_input_old_keys[kc])&&(gfc_input_keys[kc]))return 1;
    return 0;
}

Uint8 gfc_input_key_down(const char *key)
{
    SDL_Scancode kc;
    kc = gfc_input_key_to_scancode(key);
    if (kc == -1)return 0;
    if (gfc_input_keys[kc])
    {
        return 1;
    }
    return 0;
}

void gfc_input_parse_command_json(SJson *command)
{
    SJson *value,*list;
    const char * buffer;
    Input *in;
    int count,i;
    SDL_Scancode kc = 0;
    if (!command)return;
    value = sj_object_get_value(command,"command");
    if (!value)
    {
        slog("input command missing 'command' key");
        return;
    }
    in = gfc_input_new();
    if (!in)return;
    buffer = sj_get_string_value(value);
    gfc_line_cpy(in->command,buffer);
    list = sj_object_get_value(command,"keys");
    count = sj_array_get_count(list);
    for (i = 0; i< count; i++)
    {
        value = sj_array_get_nth(list,i);
        if (!value)continue;
        buffer = sj_get_string_value(value);
        if (strlen(buffer) == 0)
        {
            slog("error in key list, empty value");
            continue;   //error
        }
        kc = gfc_input_key_to_scancode(buffer);
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
        if (kc != -1)
        {
            gfc_list_append(in->keyCodes,(void *)kc);
        }
    }
    gfc_list_append(gfc_input_list,(void *)in);
}

void gfc_input_commands_load(char *configFile)
{
    SJson *json;
    SJson *commands;
    SJson *value;
    int count,i;
    if (!configFile)return;
    json = sj_load(configFile);
    if (!json)return;
    commands = sj_object_get_value(json,"commands");
    if (!commands)
    {
        slog("config file %s does not contain 'commands' object",configFile);
        sj_free(json);
        return;
    }
    count = sj_array_get_count(commands);
    for (i = 0; i< count; i++)
    {
        value = sj_array_get_nth(commands,i);
        if (!value)continue;
        gfc_input_parse_command_json(value);
    }
    sj_free(json);
}


/*eol@eof*/
