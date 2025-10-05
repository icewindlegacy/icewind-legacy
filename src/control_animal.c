/******************************************************************************
*       *****   **    **           **             ***** ***        *******    *
*    ******  ***** *****        *****          ******  * **      *       ***  *
*   **   *  *  ***** *****     *  ***         **   *  *  **     *         **  *
*  *    *  *   * **  * **         ***        *    *  *   **     **        *   *
*      *  *    *     *           *  **           *  *    *       ***          *
*     ** **    *     *           *  **          ** **   *       ** ***        *
*     ** **    *     *          *    **         ** **  *         *** ***      *
*     ** **    *     *          *    **         ** ****            *** ***    *
*     ** **    *     *         *      **        ** **  ***           *** ***  *
*     ** **    *     **        *********        ** **    **            ** *** *
*     *  **    *     **       *        **       *  **    **             ** ** *
*        *     *      **      *        **          *     **              * *  *
*    ****      *      **     *****      **     ****      ***   ***        *   *
*   *  *****           **   *   ****    ** *  *  ****    **   *  *********    *
*  *     **                *     **      **  *    **     *   *     *****      *
*  *                       *                 *               *                *
*    **                      **                **              **             *
*                                                                             *
*  MULTI                    ADVENTURER         ROLEPLAYING     SYSTEM         *
*  MARS 0.1b is copyright 2015-2024 Ro Black mars@wr3tch.org                  *
******************************************************************************/

#include <stdio.h>
#include <stddef.h>
#include <time.h>
#include "merc.h"

/* Local functions */
bool is_animal_controllable(CHAR_DATA *animal);
int find_animal_race_index(const char *name);

/*
 * Check if an animal can be controlled by druids
 */
bool is_animal_controllable(CHAR_DATA *animal)
{
    int i;
    
    if (!IS_NPC(animal))
        return FALSE;
        
    for (i = 0; control_animal_table[i].name != NULL; i++)
    {
        if (animal->race == control_animal_table[i].race)
            return TRUE;
    }
    
    return FALSE;
}

/*
 * Find the race index for an animal name
 */
int find_animal_race_index(const char *name)
{
    int i;
    
    for (i = 0; control_animal_table[i].name != NULL; i++)
    {
        if (!str_cmp(name, control_animal_table[i].name))
            return control_animal_table[i].race;
    }
    
    return -1;
}

/*
 * Control Animal command - allows druids to possess animals
 */
void do_control_animal(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *original;
    
    one_argument(argument, arg);
    
    if (arg[0] == '\0')
    {
        send_to_char("Control which animal?\n\r", ch);
        return;
    }
    
    if (ch->desc == NULL)
        return;
    
    /* Check if already controlling an animal */
    if (ch->desc->original != NULL)
    {
        send_to_char("You are already controlling an animal. Use 'release' to return to your body.\n\r", ch);
        return;
    }
    
    /* Find the animal */
    if ((victim = get_char_room(ch, arg)) == NULL)
    {
        send_to_char("You don't see that animal here.\n\r", ch);
        return;
    }
    
    if (victim == ch)
    {
        send_to_char("You can't control yourself.\n\r", ch);
        return;
    }
    
    /* Check if it's a controllable animal */
    if (!is_animal_controllable(victim))
    {
        send_to_char("You can only control certain animals.\n\r", ch);
        return;
    }
    
    /* Check if animal is already controlled */
    if (victim->desc != NULL)
    {
        send_to_char("That animal is already being controlled.\n\r", ch);
        return;
    }
    
    /* Store original character */
    original = ch;
    
    /* Switch into the animal */
    ch->desc->character = victim;
    ch->desc->original = original;
    victim->desc = ch->desc;
    ch->desc = NULL;
    
    /* Set flags and copy data */
    SET_BIT(original->act, PLR_SWITCHED);
    
    /* Copy communications */
    free_string(victim->prompt);
    if (original->prompt != NULL)
        victim->prompt = str_dup(original->prompt);
    else
        victim->prompt = NULL;
    victim->comm = original->comm;
    victim->info = original->info;
    REMOVE_BIT(victim->comm, COMM_AFK);
    victim->lines = original->lines;
    
    /* Notify the player */
    send_to_char("You take control of the animal.\n\r", victim);
    act("$n's eyes glaze over as $e takes control of $N.", original, NULL, victim, TO_ROOM);
    
    return;
}

/*
 * Release command - return to original body
 */
void do_release(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *original;
    
    if (ch->desc == NULL)
        return;
    
    if ((original = ch->desc->original) == NULL)
    {
        send_to_char("You aren't controlling an animal.\n\r", ch);
        return;
    }
    
    send_to_char("You release control of the animal and return to your body.\n\r", ch);
    
    /* Clean up animal's prompt */
    if (ch->prompt != NULL)
    {
        free_string(ch->prompt);
        ch->prompt = NULL;
    }
    
    /* Notify others */
    sprintf(buf, "$N releases control of %s.", ch->short_descr);
    act(buf, original, NULL, ch, TO_ROOM);
    
    /* Return to original body */
    REMOVE_BIT(original->act, PLR_SWITCHED);
    ch->desc->character = original;
    ch->desc->original = NULL;
    original->desc = ch->desc;
    ch->desc = NULL;
    
    /* Notify original character */
    send_to_char("You return to your body.\n\r", original);
    
    return;
}

/*
 * Control Animal Spell
 */
void spell_control_animal(int sn, int level, CHAR_DATA *ch, void *vo)
{
    CHAR_DATA *victim = (CHAR_DATA *)vo;
    CHAR_DATA *original;
    
    if (ch->desc == NULL)
        return;
    
    /* Check if already controlling an animal */
    if (ch->desc->original != NULL)
    {
        send_to_char("You are already controlling an animal.\n\r", ch);
        return;
    }
    
    /* Check if target is a controllable animal */
    if (!is_animal_controllable(victim))
    {
        send_to_char("You can only control certain animals.\n\r", ch);
        return;
    }
    
    /* Check if animal is already controlled */
    if (victim->desc != NULL)
    {
        send_to_char("That animal is already being controlled.\n\r", ch);
        return;
    }
    
    /* Check if animal is in the same room */
    if (victim->in_room != ch->in_room)
    {
        send_to_char("The animal must be in the same room as you.\n\r", ch);
        return;
    }
    
    /* Store original character */
    original = ch;
    
    /* Switch into the animal */
    ch->desc->character = victim;
    ch->desc->original = original;
    victim->desc = ch->desc;
    ch->desc = NULL;
    
    /* Set flags and copy data */
    SET_BIT(original->act, PLR_SWITCHED);
    
    /* Copy communications */
    free_string(victim->prompt);
    if (original->prompt != NULL)
        victim->prompt = str_dup(original->prompt);
    else
        victim->prompt = NULL;
    victim->comm = original->comm;
    victim->info = original->info;
    REMOVE_BIT(victim->comm, COMM_AFK);
    victim->lines = original->lines;
    
    /* Notify the player */
    send_to_char("You take control of the animal.\n\r", victim);
    act("$n's eyes glaze over as $e takes control of $N.", original, NULL, victim, TO_ROOM);
    
    return;
}
