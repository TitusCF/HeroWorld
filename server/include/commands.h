/**
 * @file
 * Defines and structures related to commands the player can send.
 */

#ifndef COMMANDS_H
#define COMMANDS_H

/*
 * Crossfire commands
 *      ++Jam
 *
 * ''', run and fire-keys are parsed separately (cannot be overrided).
 */


/* The initialized arrays were removed from this file and are now
 * in commands.c.  Initializing the arrays in any header file
 * is stupid, as it means that header file can only be included
 * in one source file (so what is the point of putting them in a header
 * file then?).  Header files should be used like this one - to declare
 * the structures externally - they actual structures should resided/
 * be initialized in one of the source files.
 */

/**
 * One command function.
 * @param op
 * the player executing the command
 * @param params
 * the command parameters; empty string if no commands are given; leading and
 * trailing spaces have been removed
 */
typedef void (*command_function)(object *op, const char *params);

/** Represents one command. */
typedef struct {
    const char *name;         /**< Command name. */
    command_function func;    /**< Pointer to command function. */
    float time;               /**< How long it takes to execute this command. */
} command_array_struct;

extern command_array_struct Commands[], WizCommands[], CommunicationCommands[];

extern const int CommandsSize, WizCommandsSize, CommunicationCommandSize;

#endif /* COMMANDS_H */
