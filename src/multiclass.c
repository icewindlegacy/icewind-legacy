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
#include <stdlib.h>
#include <string.h>
#include "merc.h"

/*
 * Recalculate multiclass_count from class_levels array
 */
void recalculate_multiclass_count( CHAR_DATA *ch )
{
    int i;
    int count = 0;
    
    if ( IS_NPC( ch ) )
        return;
        
    /* Count how many classes have levels > 0 */
    for ( i = 0; i < MAX_CLASS; i++ )
    {
        if ( ch->class_levels[i] > 0 )
            count++;
    }
    
    ch->multiclass_count = count;
}

/*
 * Initialize multiclass data for a new character
 */
void init_multiclass( CHAR_DATA *ch )
{
    int i;
    
    if ( IS_NPC( ch ) )
        return;
        
    ch->multiclass_count = 1;
    ch->primary_class = ch->class;
    ch->secondary_class = -1;
    ch->tertiary_class = -1;
    ch->pcdata->pending_class_choice = FALSE;
    
    /* Initialize all class levels to 0 */
    for ( i = 0; i < MAX_CLASS; i++ )
        ch->class_levels[i] = 0;
        
    /* Set primary class to current character level */
    /* For new characters, ch->level will be 1 */
    /* For existing characters, we preserve their current level */
    ch->class_levels[ch->primary_class] = ch->level;
}

/*
 * Get the total character level (sum of all class levels)
 */
int get_total_level( CHAR_DATA *ch )
{
    int total = 0;
    int i;
    
    if ( IS_NPC( ch ) )
        return ch->level;
        
    for ( i = 0; i < MAX_CLASS; i++ )
        total += ch->class_levels[i];
        
    return total;
}

/*
 * Get the level in a specific class
 */
int get_class_level( CHAR_DATA *ch, int class_num )
{
    if ( IS_NPC( ch ) || class_num < 0 || class_num >= MAX_CLASS )
        return 0;
        
    return ch->class_levels[class_num];
}

/*
 * Check if character can add a new class
 */
bool can_add_class( CHAR_DATA *ch )
{
    if ( IS_NPC( ch ) )
        return FALSE;
        
    return ( ch->multiclass_count < 3 );
}

/*
 * Add a new class to the character
 */
bool add_class( CHAR_DATA *ch, int class_num )
{
    if ( IS_NPC( ch ) || class_num < 0 || class_num >= MAX_CLASS )
        return FALSE;
        
    if ( !can_add_class( ch ) )
        return FALSE;
        
    /* Check if class is already taken */
    if ( ch->class_levels[class_num] > 0 )
        return FALSE;
        
    /* Add the class */
    ch->class_levels[class_num] = 1;
    ch->multiclass_count++;
    
    if ( ch->secondary_class == -1 )
        ch->secondary_class = class_num;
    else if ( ch->tertiary_class == -1 )
        ch->tertiary_class = class_num;
    
    /* Add base group skills for the new class */
    if ( class_table[class_num].base_group != NULL && class_table[class_num].base_group[0] != '\0' )
    {
        char buf[MAX_STRING_LENGTH];
        sprintf( buf, "DEBUG: Adding base group '%s' for class %s\n\r", 
                class_table[class_num].base_group, class_table[class_num].name );
        send_to_char( buf, ch );
        
        group_add( ch, class_table[class_num].base_group, TRUE );
        
        send_to_char( "DEBUG: group_add completed\n\r", ch );
    }
    else
    {
        char buf[MAX_STRING_LENGTH];
        sprintf( buf, "DEBUG: No base group for class %s (base_group=%s)\n\r", 
                class_table[class_num].name, 
                class_table[class_num].base_group ? class_table[class_num].base_group : "NULL" );
        send_to_char( buf, ch );
    }
        
    return TRUE;
}

/*
 * Check if a skill is available to a multiclass character
 * Returns TRUE if the skill is available to any of the character's classes
 */
bool is_skill_available_to_multiclass( CHAR_DATA *ch, int sn )
{
    int i;
    
    if ( IS_NPC( ch ) )
        return FALSE;
        
    /* Check if skill is available to any of the character's classes */
    for ( i = 0; i < MAX_CLASS; i++ )
    {
        if ( ch->class_levels[i] > 0 )
        {
            if ( skill_table[sn].rating[i] > 0 )
                return TRUE;
        }
    }
    
    return FALSE;
}

/*
 * Get the minimum skill level requirement for a multiclass character
 * Returns the lowest level requirement across all the character's classes
 */
int get_multiclass_skill_level( CHAR_DATA *ch, int sn )
{
    int i;
    int min_level = LEVEL_HERO + 1;
    
    if ( IS_NPC( ch ) )
        return skill_table[sn].skill_level[ch->class];
        
    /* Find the minimum skill level requirement across all classes */
    for ( i = 0; i < MAX_CLASS; i++ )
    {
        if ( ch->class_levels[i] > 0 && skill_table[sn].rating[i] > 0 )
        {
            if ( skill_table[sn].skill_level[i] < min_level )
                min_level = skill_table[sn].skill_level[i];
        }
    }
    
    /* If no class has the skill, return the primary class requirement */
    if ( min_level == LEVEL_HERO + 1 )
        return skill_table[sn].skill_level[ch->class];
        
    return min_level;
}

/*
 * Check if a multiclass character meets the level requirement for a skill
 * Returns TRUE if the character's level in any class that has the skill meets the requirement
 */
bool multiclass_meets_skill_level( CHAR_DATA *ch, int sn )
{
    int i;
    
    if ( IS_NPC( ch ) )
        return ch->level >= skill_table[sn].skill_level[ch->class];
        
    /* Check if any of the character's classes meets the level requirement */
    for ( i = 0; i < MAX_CLASS; i++ )
    {
        if ( ch->class_levels[i] > 0 && skill_table[sn].rating[i] > 0 )
        {
            if ( ch->class_levels[i] >= skill_table[sn].skill_level[i] )
                return TRUE;
        }
    }
    
    return FALSE;
}

/*
 * Get the skill rating for a multiclass character
 * Returns the rating from the class that has the skill (preferring the class with the skill)
 */
int get_multiclass_skill_rating( CHAR_DATA *ch, int sn )
{
    int i;
    
    if ( IS_NPC( ch ) )
        return skill_table[sn].rating[ch->class];
        
    /* Find the first class that has the skill */
    for ( i = 0; i < MAX_CLASS; i++ )
    {
        if ( ch->class_levels[i] > 0 && skill_table[sn].rating[i] > 0 )
        {
            return skill_table[sn].rating[i];
        }
    }
    
    /* If no class has the skill, return the primary class rating */
    return skill_table[sn].rating[ch->class];
}

/*
 * Get the effective class for skill/spell calculations
 * This returns the class with the highest level
 */
int get_effective_class( CHAR_DATA *ch )
{
    int i;
    int max_level = 0;
    int effective_class = 0;
    
    if ( IS_NPC( ch ) )
        return ch->class;
        
    for ( i = 0; i < MAX_CLASS; i++ )
    {
        if ( ch->class_levels[i] > max_level )
        {
            max_level = ch->class_levels[i];
            effective_class = i;
        }
    }
    
    return effective_class;
}

/*
 * Calculate experience required for next level with multiclass penalties
 */
int get_multiclass_exp_requirement( CHAR_DATA *ch, int target_level )
{
    int base_exp;
    int penalty;
    
    if ( target_level < 2 || target_level > 20 )
        return 0;
        
    /* Get base experience for this level */
    base_exp = get_base_exp_for_level( target_level );
    
    /* Apply 20% penalty for each extra class */
    /* For example: 1 fighter, 1 rogue, 1 barbarian = 2 extra classes = 40% penalty */
    penalty = (ch->multiclass_count - 1) * 20;
    
    return base_exp + (base_exp * penalty / 100);
}

/*
 * Check if character is ready to level up
 */
bool is_ready_to_level( CHAR_DATA *ch )
{
    int current_total = get_total_level( ch );
    int required_exp;
    
    if ( current_total >= 20 )
        return FALSE;
        
    required_exp = get_multiclass_exp_requirement( ch, current_total + 1 );
    
    return ( ch->exp >= required_exp );
}

/*
 * Show available classes for multiclass selection
 */
void show_multiclass_choices( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    int i;
    int count = 0;
    
    /* Add extra spacing to make it more visible */
    send_to_char( "\n\r\n\r\n\r", ch );
    send_to_char( "`P===========================================================================\n\r", ch );
    send_to_char( "`P===========================================================================\n\r", ch );
    send_to_char( "`R*** LEVEL UP AVAILABLE! ***\n\r", ch );
    send_to_char( "`GYou have gained enough experience to advance! You may now choose:\n\r", ch );
    
    /* Show all existing classes for advancement */
    send_to_char( "\n\r`GAdvance in existing classes:\n\r", ch );
    for ( i = 0; i < MAX_CLASS; i++ )
    {
        if ( ch->class_levels[i] > 0 )
        {
            count++;
            sprintf( buf, "`W%2d.`G %s (level %d)\n\r", count, class_table[i].name, ch->class_levels[i] );
            send_to_char( buf, ch );
        }
    }
    
    /* Show available new classes if player can add more */
    if ( can_add_class( ch ) )
    {
        send_to_char( "\n\r`GAdd new class:\n\r", ch );
        
        for ( i = 0; i < MAX_CLASS; i++ )
        {
            if ( ch->class_levels[i] == 0 && class_table[i].name[0] != '\0' )
            {
                count++;
                sprintf( buf, "`W%2d.`G %s\n\r", count, class_table[i].name );
                send_to_char( buf, ch );
            }
        }
    }
    
    send_to_char( "\n\r`P===========================================================================\n\r", ch );
    send_to_char( "`P===========================================================================\n\r", ch );
    send_to_char( "`WType 'multiclass <number>' to make your choice (e.g., 'multiclass 1')\n\r", ch );
    send_to_char( "`WYour choice? `X", ch );
}

/*
 * Handle multiclass level up
 */
void handle_multiclass_levelup( CHAR_DATA *ch, int choice )
{
    char buf[MAX_STRING_LENGTH];
    int i;
    int class_count = 0;
    int selected_class = -1;
    
    /* First, count existing classes to determine if choice is for existing or new class */
    int existing_class_count = 0;
    for ( i = 0; i < MAX_CLASS; i++ )
    {
        if ( ch->class_levels[i] > 0 )
            existing_class_count++;
    }
    
    if ( choice <= existing_class_count )
    {
        /* Advance in existing class */
        int class_index = 0;
        for ( i = 0; i < MAX_CLASS; i++ )
        {
            if ( ch->class_levels[i] > 0 )
            {
                class_index++;
                if ( class_index == choice )
                {
                    ch->class_levels[i]++;
                    sprintf( buf, "You advance in %s to level %d!\n\r", 
                            class_table[i].name, 
                            ch->class_levels[i] );
                    send_to_char( buf, ch );
                    break;
                }
            }
        }
    }
    else if ( choice > existing_class_count && can_add_class( ch ) )
    {
        /* Add new class */
        int new_class_count = 0;
        for ( i = 0; i < MAX_CLASS; i++ )
        {
            if ( ch->class_levels[i] == 0 && class_table[i].name[0] != '\0' )
            {
                new_class_count++;
                if ( new_class_count == (choice - existing_class_count) )
                {
                    add_class( ch, i );
                    sprintf( buf, "You begin studying %s!\n\r", class_table[i].name );
                    send_to_char( buf, ch );
                    break;
                }
            }
        }
    }
    else
    {
        send_to_char( "Invalid choice. Please try again.\n\r", ch );
        show_multiclass_choices( ch );
        return;
    }
    
    /* Update character level */
    ch->level = get_total_level( ch );
    ch->pcdata->pending_class_choice = FALSE;
    
    /* Remove PLR_NOEXP flag to allow exp gain again */
    REMOVE_BIT( ch->act, PLR_NOEXP );
    
    /* Apply level up benefits */
    advance_level( ch, FALSE );
    
    sprintf( buf, "You are now a level %d character.\n\r", ch->level );
    send_to_char( buf, ch );
}

/*
 * Get class display string for multiclass characters
 */
void get_multiclass_display( CHAR_DATA *ch, char *buf )
{
    int i;
    bool first = TRUE;
    bool has_class_levels = FALSE;
    
    if ( IS_NPC( ch ) )
    {
        sprintf( buf, "%s", class_table[ch->class].who_name );
        return;
    }
    
    buf[0] = '\0';
    
    /* Check if character has any class levels set */
    for ( i = 0; i < MAX_CLASS; i++ )
    {
        if ( ch->class_levels[i] > 0 )
        {
            has_class_levels = TRUE;
            break;
        }
    }
    
    /* If no class levels are set, fall back to ch->class */
    if ( !has_class_levels )
    {
        sprintf( buf, "%s %d", class_table[ch->class].who_name, ch->level );
        return;
    }
    
    /* Display multiclass information */
    for ( i = 0; i < MAX_CLASS; i++ )
    {
        if ( ch->class_levels[i] > 0 )
        {
            if ( !first )
                strcat( buf, "/" );
            sprintf( buf + strlen(buf), "%s %d", class_table[i].who_name, ch->class_levels[i] );
            first = FALSE;
        }
    }
}

/*
 * Handle levelup command
 */
void do_levelup( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch ) )
    {
        send_to_char( "Only players can use levelup.\n\r", ch );
        return;
    }
    
    if ( !IS_SET( ch->act, PLR_NOEXP ) )
    {
        send_to_char( "You are not ready to level up.\n\r", ch );
        return;
    }
    
    if ( !is_ready_to_level( ch ) )
    {
        send_to_char( "You don't have enough experience to level up.\n\r", ch );
        return;
    }
    
    /* Set the pending class choice flag so multiclass command works */
    ch->pcdata->pending_class_choice = TRUE;
    
    /* Show the multiclass choices */
    show_multiclass_choices( ch );
}

/*
 * Handle multiclass command
 */
void do_multiclass( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int choice;
    
    if ( IS_NPC( ch ) )
    {
        send_to_char( "Only players can use multiclass.\n\r", ch );
        return;
    }
    
    if ( !ch->pcdata->pending_class_choice )
    {
        send_to_char( "You are not ready to choose a class advancement.\n\r", ch );
        return;
    }
    
    one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
        show_multiclass_choices( ch );
        return;
    }
    
    if ( !is_number( arg ) )
    {
        send_to_char( "Please enter a number for your choice.\n\r", ch );
        show_multiclass_choices( ch );
        return;
    }
    
    choice = atoi( arg );
    handle_multiclass_levelup( ch, choice );
}
