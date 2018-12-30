/*****************************************************************************/
/* Template for version 2.0 plugins.                                         */
/* Contact: yann.chachkoff@myrealbox.com                                     */
/*****************************************************************************/
/* That code is placed under the GNU General Public Licence (GPL)            */
/* (C)2001-2005 by Chachkoff Yann (Feel free to deliver your complaints)     */
/*****************************************************************************/
/*  CrossFire, A Multiplayer game for X-windows                              */
/*                                                                           */
/*  Copyright (C) 2000 Mark Wedel                                            */
/*  Copyright (C) 1992 Frank Tore Johansen                                   */
/*                                                                           */
/*  This program is free software; you can redistribute it and/or modify     */
/*  it under the terms of the GNU General Public License as published by     */
/*  the Free Software Foundation; either version 2 of the License, or        */
/*  (at your option) any later version.                                      */
/*                                                                           */
/*  This program is distributed in the hope that it will be useful,          */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/*  GNU General Public License for more details.                             */
/*                                                                           */
/*  You should have received a copy of the GNU General Public License        */
/*  along with this program; if not, write to the Free Software              */
/*  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                */
/*                                                                           */
/*****************************************************************************/

/* First let's include the header file needed                                */

#include <plugin_template.h>
#include <stdarg.h>
#ifndef __CEXTRACT__
#include <plugin_template_proto.h>
#endif

CFPContext *context_stack;

CFPContext *current_context;

static int current_command = -999;

void initContextStack(void) {
    current_context = NULL;
    context_stack = NULL;
}

void pushContext(CFPContext *context) {
    CFPContext *stack_context;

    if (current_context == NULL) {
        context_stack = context;
        context->down = NULL;
    } else {
        context->down = current_context;
    }
    current_context = context;
}

CFPContext *popContext(void) {
    CFPContext *oldcontext;

    if (current_context != NULL) {
        oldcontext = current_context;
        current_context = current_context->down;
        return oldcontext;
    } else
        return NULL;
}

CF_PLUGIN int initPlugin(const char *iversion, f_plug_api gethooksptr) {
    cf_init_plugin(gethooksptr);

    cf_log(llevDebug, PLUGIN_VERSION " init\n");

    /* Place your initialization code here */
    return 0;
}

CF_PLUGIN void *getPluginProperty(int *type, ...) {
    va_list args;
    const char *propname;
    int i, size;
    command_array_struct *rtn_cmd;
    char *buf;

    va_start(args, type);
    propname = va_arg(args, const char *);

    if (!strcmp(propname, "command?")) {
        const char *cmdname;
        cmdname = va_arg(args, const char *);
        rtn_cmd = va_arg(args, command_array_struct *);
        va_end(args);

        /** Check if plugin handles custom command */
        return NULL;
    } else if (!strcmp(propname, "Identification")) {
        buf = va_arg(args, char *);
        size = va_arg(args, int);
        va_end(args);
        snprintf(buf, size, PLUGIN_NAME);
        return NULL;
    } else if (!strcmp(propname, "FullName")) {
        buf = va_arg(args, char *);
        size = va_arg(args, int);
        va_end(args);
        snprintf(buf, size, PLUGIN_VERSION);
        return NULL;
    }
    va_end(args);
    return NULL;
}

CF_PLUGIN int runPluginCommand(object *op, char *params) {
    return -1;
}

CF_PLUGIN int postInitPlugin(void) {
    cf_log(llevDebug, PLUGIN_VERSION" post init\n");
    initContextStack();
    /* Pick the global events you want to monitor from this plugin */
/*
    cf_system_register_global_event(EVENT_BORN, PLUGIN_NAME, globalEventListener);
    cf_system_register_global_event(EVENT_CLOCK, PLUGIN_NAME, globalEventListener);
    cf_system_register_global_event(EVENT_CRASH, PLUGIN_NAME, globalEventListener);
    cf_system_register_global_event(EVENT_PLAYER_DEATH, PLUGIN_NAME, globalEventListener);
    cf_system_register_global_event(EVENT_GKILL, PLUGIN_NAME, globalEventListener);
    cf_system_register_global_event(EVENT_LOGIN, PLUGIN_NAME, globalEventListener);
    cf_system_register_global_event(EVENT_LOGOUT, PLUGIN_NAME, globalEventListener);
    cf_system_register_global_event(EVENT_MAPENTER, PLUGIN_NAME, globalEventListener);
    cf_system_register_global_event(EVENT_MAPLEAVE, PLUGIN_NAME, globalEventListener);
    cf_system_register_global_event(EVENT_MAPRESET, PLUGIN_NAME, globalEventListener);
    cf_system_register_global_event(EVENT_REMOVE, PLUGIN_NAME, globalEventListener);
    cf_system_register_global_event(EVENT_SHOUT, PLUGIN_NAME, globalEventListener);
    cf_system_register_global_event(EVENT_TELL, PLUGIN_NAME, globalEventListener);
    cf_system_register_global_event(EVENT_MUZZLE, PLUGIN_NAME, globalEventListener);
    cf_system_register_global_event(EVENT_KICK, PLUGIN_NAME, globalEventListener);
*/
    return 0;
}

CF_PLUGIN void *globalEventListener(int *type, ...) {
    va_list args;
    static int rv = 0;
    CFPContext *context;
    context = malloc(sizeof(CFPContext));
    char *buf;
    player *pl;
    object *op;

    va_start(args, type);
    context->event_code = va_arg(args, int);
    printf("****** Global event listener called ***********\n");
    printf("- Event code: %d\n", context->event_code);

    context->message[0] = 0;

    context->who = NULL;
    context->activator = NULL;
    context->third = NULL;
    context->event = NULL;
    rv = context->returnvalue = 0;
    switch (context->event_code) {
    case EVENT_CRASH:
        printf("Unimplemented for now\n");
        break;

    case EVENT_BORN:
        context->activator = va_arg(args, object *);
        break;

    case EVENT_PLAYER_DEATH:
        context->who = va_arg(args, object *);
        context->activator = va_arg(args, object *);
        break;

    case EVENT_GKILL:
        context->who = va_arg(args, object *);
        context->activator = va_arg(args, object *);
        break;

    case EVENT_LOGIN:
        pl = va_arg(args, player *);
        context->activator = pl->ob;
        buf = va_arg(args, char *);
        if (buf != 0)
            strcpy(context->message, buf);
        break;

    case EVENT_LOGOUT:
        pl = va_arg(args, player *);
        context->activator = pl->ob;
        buf = va_arg(args, char *);
        if (buf != 0)
            strcpy(context->message, buf);
        break;

    case EVENT_REMOVE:
        context->activator = va_arg(args, object *);
        break;

    case EVENT_SHOUT:
        context->activator = va_arg(args, object *);
        buf = va_arg(args, char *);
        if (buf != 0)
            strcpy(context->message, buf);
        break;

    case EVENT_MUZZLE:
        context->activator = va_arg(args, object *);
        buf = va_arg(args, char *);
        if (buf != 0)
            strcpy(context->message, buf);
        break;

    case EVENT_KICK:
        context->activator = va_arg(args, object *);
        buf = va_arg(args, char *);
        if (buf != 0)
            strcpy(context->message, buf);
        break;

    case EVENT_MAPENTER:
        context->activator = va_arg(args, object *);
        break;

    case EVENT_MAPLEAVE:
        context->activator = va_arg(args, object *);
        break;

    case EVENT_CLOCK:
        break;

    case EVENT_MAPRESET:
        buf = va_arg(args, char *);
        if (buf != 0)
            strcpy(context->message, buf);
        break;

    case EVENT_TELL:
        context->activator = va_arg(args, object *);
        buf = va_arg(args, char *);
        if (buf != 0)
            strcpy(context->message, buf);
        context->third = va_arg(args, object *);
        break;
    }
    va_end(args);
    context->returnvalue = 0;

    pushContext(context);
    /* Put your plugin action(s) here */
    context = popContext();
    rv = context->returnvalue;
    free(context);
    cf_log(llevDebug, "*********** Execution complete ****************\n");

    return &rv;
}

CF_PLUGIN void *eventListener(int *type, ...) {
    static int rv = 0;
    va_list args;
    char *buf;
    CFPContext *context;

    context = malloc(sizeof(CFPContext));

    context->message[0] = 0;

    va_start(args, type);

    context->who = va_arg(args, object *);
    context->activator = va_arg(args, object *);
    context->third = va_arg(args, object *);
    buf = va_arg(args, char *);
    if (buf != 0)
        strcpy(context->message, buf);
    context->fix = va_arg(args, int);
    context->event = va_arg(args, object *);
    context->event_code = context->event->subtype;
    strncpy(context->options, context->event->name, sizeof(context->options));
    context->returnvalue = 0;
    va_arg(args, talk_info *); /* ignored for now */
    va_end(args);

    pushContext(context);
    /* Put your plugin action(s) here */
    context = popContext();
    rv = context->returnvalue;
    free(context);
    cf_log(llevDebug, "Execution complete");
    return &rv;
}

CF_PLUGIN int closePlugin(void) {
    cf_log(llevDebug, PLUGIN_VERSION " closing\n");
    return 0;
}
