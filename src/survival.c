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
/******************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,           *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.      *
 *                                                                            *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael             *
 *  Chastain, Michael Quan, and Mitchell Tse.                                 *
 *                                                                            *
 *  In order to use any part of this Merc Diku Mud, you must comply with      *
 *  both the original Diku license in 'license.doc' as well the Merc          *
 *  license in 'license.txt'.  In particular, you may not remove either of    *
 *  these copyright notices.                                                  *
 *                                                                            *
 *  Much time and thought has gone into this software and you are             *
 *  benefitting.  We hope that you share your changes too.  What goes         *
 *  around, comes around.                                                     *
 *****************************************************************************/

/******************************************************************************
*	ROM 2.4 is copyright 1993-1998 Russ Taylor			                      *
*	ROM has been brought to you by the ROM consortium		                  *
*	    Russ Taylor (rtaylor@hypercube.org)				                      *
*	    Gabrielle Taylor (gtaylor@hypercube.org)			                  *
*	    Brian Moore (zump@rom.org)					                          *
*	By using this code, you have agreed to follow the terms of the	          *
*	ROM license, in the file Rom24/doc/rom.license			                  *
******************************************************************************/
/* dwatch egate artifice fdmud code copyright 2002-2008 Bob Kircher	  */

/******************************************************************************
 * Survival Commands
 *
 * This file contains the commands for survival related skills.
 *
 * Foraging:
 * - Gathers food and other resources from the environment.
 *
 * Tracking:
 * - Tracks monsters and other characters through the environment.
 *
 * Fishing:
 * - Fishes for food, giving different fish based on environment.
 *
 * Hunting:
 * - Allows you to spot game animals and hunt them.
 *
 * Building:
 * - Builds shelters to protect from the elements.
 *
 * Gathering:
 * - Gathers wood for building shelters and cooking, and making fire.
 *
 * Butchering:
 * - Butchers animals for their meat.
 *
 * Skinning:
 * - Skins animals for their hide.
 *
 *****************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include "merc.h"
#include "tables.h"

#define PATH_MAX_DIR  10  // Extended directions: n,e,s,w,u,d,ne,nw,se,sw
#define BITS_PER_INT  32
#define MAX_TRACK_ROOMS 1000  // Reasonable limit for tracking
#define PATH_IS_FLAG(flag, bit)  ((unsigned)flag[bit/BITS_PER_INT]>>bit%BITS_PER_INT&01)
#define PATH_SET_FLAG(flag, bit) (flag[bit/BITS_PER_INT] |= 1 << bit%BITS_PER_INT)


void do_track(CHAR_DATA *ch, char *argument);
int find_path(CHAR_DATA *ch, ROOM_INDEX_DATA *from, ROOM_INDEX_DATA *to, int max_depth);

void debug_print(CHAR_DATA *ch, const char *fmt, ...)
{
#ifdef DEBUG_TRACK
    va_list args;
    char buf[MSL];

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (ch)
        send_to_char(buf, ch);
    else
        fprintf(stderr, "%s", buf);
#endif
}

void do_track(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    int skill;
    int direction;

    if ((skill = get_skill(ch, gsn_track)) == 0 || IS_NPC(ch))
    {
        send_to_char("You do not know how to track.\n\r", ch);
        return;
    }

    if (argument[0] == '\0')
    {
        send_to_char("Track whom?\n\r", ch);
        return;
    }

    victim = get_char_world(ch, argument);
    if (!victim)
    {
        send_to_char("No one around by that name.\n\r", ch);
        return;
    }

    if (ch->in_room == victim->in_room)
    {
        act("$N is here!", ch, NULL, victim, TO_CHAR);
        return;
    }

    // Check for Pass Without Trace
    if (IS_AFFECTED(victim, AFF_PASS_WITHOUT_TRACE))
    {
        send_to_char("You cannot find any trace of that person.\n\r", ch);
        return;
    }

    act("$n carefully sniffs the air.", ch, NULL, NULL, TO_ROOM);
    WAIT_STATE(ch, skill_table[gsn_track].beats);

    // Determine max search depth based on skill (reasonable limits)
    skill = UMIN(skill / 4 + ch->level / 5, 50);  // Cap at 50 rooms max

    direction = find_path(ch, ch->in_room, victim->in_room, skill);

    if (direction == -1)
        ch_printf(ch, "Unable to find a path to %s.\n\r", PERS(victim, ch));
    else
        ch_printf(ch, "%s is %s from here.\n\r", PERS(victim, ch), dir_name[direction]);

    check_improve(ch, gsn_track, direction != -1, 1);
}

// Fixed BFS pathfinding using ROM hash table iteration
int find_path(CHAR_DATA *ch, ROOM_INDEX_DATA *from, ROOM_INDEX_DATA *to, int max_depth)
{
    ROOM_INDEX_DATA **queue = NULL;
    ROOM_INDEX_DATA *room;
    ROOM_INDEX_DATA *next_room;
    ROOM_INDEX_DATA *current_room;
    int *visited = NULL;
    int front = 0, back = 0;
    int *depth_arr = NULL;
    int *parent_vnum = NULL;
    int *parent_dir = NULL;
    int i, dir;
    int max_vnum = 0;
    int visited_size;
    int result = -1;

    if (!from || !to || max_depth <= 0)
        return -1;

    // Find maximum vnum - use a safe default for ROM
    max_vnum = 32767;  // Standard ROM vnum range

    if (max_vnum < 1)
        max_vnum = 32767;  // Fallback

    // Calculate size for visited bitfield
    visited_size = (max_vnum / BITS_PER_INT) + 1;

    // Allocate arrays with bounds checking
    visited = calloc(visited_size, sizeof(int));
    queue = calloc(MAX_TRACK_ROOMS, sizeof(ROOM_INDEX_DATA *));
    depth_arr = calloc(max_vnum + 1, sizeof(int));
    parent_vnum = calloc(max_vnum + 1, sizeof(int));
    parent_dir = calloc(max_vnum + 1, sizeof(int));

    if (!visited || !queue || !depth_arr || !parent_vnum || !parent_dir)
    {
        send_to_char("Tracking failed: memory error.\n\r", ch);
        goto cleanup;
    }

    debug_print(ch, "DEBUG: Starting BFS from room %d to room %d, max depth %d\n", 
                from->vnum, to->vnum, max_depth);

    // Initialize BFS
    front = back = 0;
    queue[back++] = from;
    
    // Bounds check for vnum before setting visited flag
    if (from->vnum >= 0 && from->vnum <= max_vnum)
    {
        PATH_SET_FLAG(visited, from->vnum);
        depth_arr[from->vnum] = 0;
        parent_vnum[from->vnum] = -1;  // Mark as root
    }

    while (front < back && back < MAX_TRACK_ROOMS)
    {
        current_room = queue[front++];
        
        if (!current_room || current_room->vnum < 0 || current_room->vnum > max_vnum)
            continue;

        int cur_depth = depth_arr[current_room->vnum];
        
        debug_print(ch, "DEBUG: Visiting room %d at depth %d\n", current_room->vnum, cur_depth);

        // Don't go beyond max depth
        if (cur_depth >= max_depth)
            continue;

        // Check all directions
        for (dir = 0; dir < PATH_MAX_DIR; dir++)
        {
            EXIT_DATA *pexit = current_room->exit[dir];
            
            debug_print(ch, "DEBUG: Checking direction %d from room %d\n", dir, current_room->vnum);
            
            if (!pexit)
            {
                debug_print(ch, "DEBUG: No exit data in direction %d from room %d\n", dir, current_room->vnum);
                continue;
            }
            
            // Try both common ROM exit structures
            next_room = NULL;
            if (pexit->u1.to_room)
                next_room = pexit->u1.to_room;
            else if (pexit->to_room)
                next_room = pexit->to_room;
            
            if (!next_room)
            {
                debug_print(ch, "DEBUG: Exit %d exists but no destination room from room %d\n", dir, current_room->vnum);
                continue;
            }

            debug_print(ch, "DEBUG: Found exit %s (%d) from room %d to room %d\n", 
                       dir_name[dir], dir, current_room->vnum, next_room->vnum);
            
            // Bounds check
            if (!next_room || next_room->vnum < 0 || next_room->vnum > max_vnum)
                continue;

            // Skip if already visited
            if (PATH_IS_FLAG(visited, next_room->vnum))
            {
                debug_print(ch, "DEBUG: Room %d already visited, skipping\n", next_room->vnum);
                continue;
            }

            // Skip closed/locked doors unless character can pass
            if (IS_SET(pexit->exit_info, EX_CLOSED))
            {
                debug_print(ch, "DEBUG: Exit to room %d is closed, skipping\n", next_room->vnum);
                continue;
            }

            // Mark as visited and add to queue
            PATH_SET_FLAG(visited, next_room->vnum);
            depth_arr[next_room->vnum] = cur_depth + 1;
            parent_vnum[next_room->vnum] = current_room->vnum;
            parent_dir[next_room->vnum] = dir;

            debug_print(ch, "DEBUG: Added room %d to queue at depth %d\n", next_room->vnum, cur_depth + 1);

            // Found target?
            if (next_room == to)
            {
                debug_print(ch, "DEBUG: Found target at depth %d\n", cur_depth + 1);
                
                // Backtrack to find the FIRST direction from starting room
                int trace_vnum = next_room->vnum;
                
                debug_print(ch, "DEBUG: Backtracking from target room %d\n", trace_vnum);
                
                // Follow parent chain back to find the room that's directly connected to start
                while (parent_vnum[trace_vnum] != -1 && parent_vnum[trace_vnum] != from->vnum)
                {
                    debug_print(ch, "DEBUG: Backtrack: room %d came from room %d via dir %d\n", 
                               trace_vnum, parent_vnum[trace_vnum], parent_dir[trace_vnum]);
                    trace_vnum = parent_vnum[trace_vnum];
                }
                
                // Now trace_vnum should be the room directly connected to 'from'
                // Find which direction from 'from' leads to trace_vnum
                for (i = 0; i < PATH_MAX_DIR; i++)
                {
                    if (from->exit[i] && 
                        ((from->exit[i]->u1.to_room && from->exit[i]->u1.to_room->vnum == trace_vnum) ||
                         (from->exit[i]->to_room && from->exit[i]->to_room->vnum == trace_vnum)))
                    {
                        debug_print(ch, "DEBUG: First step is direction %d (%s) to room %d\n", 
                                   i, dir_name[i], trace_vnum);
                        result = i;
                        goto cleanup;
                    }
                }
                
                debug_print(ch, "DEBUG: ERROR: Could not find direction from start room to %d\n", trace_vnum);
                result = -1;
                goto cleanup;
            }

            // Add to queue if there's space
            if (back < MAX_TRACK_ROOMS)
                queue[back++] = next_room;
        }
    }

    debug_print(ch, "DEBUG: BFS completed, no path found\n");

cleanup:
    if (visited) free(visited);
    if (queue) free(queue);
    if (depth_arr) free(depth_arr);
    if (parent_vnum) free(parent_vnum);
    if (parent_dir) free(parent_dir);
    
    return result;
}

void
do_forage( CHAR_DATA *ch, char *argument )
{
    const struct foraging_entry *table = NULL;
    const struct foraging_entry *entry;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    int chance;
    int count = 0;
    int i;
    int sn;

    if ( IS_NPC(ch) )
    {
	send_to_char( "You don't know how to forage.\n\r", ch );
	return;
    }

    sn = gsn_forage;
    if ( (chance = get_skill(ch, sn)) == 0 )
    {
	send_to_char( "You don't know how to forage.\n\r", ch );
	return;
    }

    /* Seasonal modifiers */
    switch ( time_info.month )
    {
        case 2: case 3: case 4:  /* Spring - better foraging */
            chance += 10;
            break;
        case 8: case 9: case 10: /* Fall - 10% more failure */
            chance -= 10;
            break;
        case 0: case 1: case 11: /* Winter - extremely scarce */
            chance -= 40;
            break;
    }

    WAIT_STATE( ch, skill_table[gsn_forage].beats );

    /* Determine which foraging table to use based on sector type */
    switch ( ch->in_room->sector_type )
    {
	case SECT_FOREST:
	    table = forest_foraging_table;
	    break;
	case SECT_DESERT:
	    table = desert_foraging_table;
	    break;
	case SECT_FIELD:
	    table = field_foraging_table;
	    break;
	case SECT_MOUNTAIN:
	    table = mountain_foraging_table;
	    break;
	case SECT_HILLS:
	    table = hills_foraging_table;
	    break;
	default:
	    send_to_char( "You search around but find nothing of use here.\n\r", ch );
	    return;
    }

    /* Count entries in the table */
    for ( i = 0; table[i].name != NULL; i++ )
	count++;

    if ( count == 0 )
    {
	send_to_char( "You search around but find nothing of use here.\n\r", ch );
	return;
    }

    /* Check skill success */
    if ( number_percent() > chance )
    {
	send_to_char( "You search around but fail to find anything useful.\n\r", ch );
	check_improve( ch, sn, FALSE, 1 );
	return;
    }

    /* Select random entry from table */
    entry = &table[number_range( 0, count - 1 )];

    /* Create the foraged item */
    pObjIndex = get_obj_index( OBJ_VNUM_FORAGED );
    if ( pObjIndex == NULL )
    {
	send_to_char( "Something went wrong with foraging.\n\r", ch );
	return;
    }

    obj = create_object( pObjIndex, 0 );
    if ( obj == NULL )
    {
	send_to_char( "Something went wrong with foraging.\n\r", ch );
	return;
    }

    /* Set object properties */
    free_string( obj->name );
    obj->name = str_dup( entry->name );
    free_string( obj->short_descr );
    obj->short_descr = str_dup( entry->short_descr );
    free_string( obj->description );
    obj->description = str_dup( entry->long_descr );

    /* Set food values */
    obj->value[0] = 1;  /* food value */
    obj->value[1] = 3;  /* hours to decay */
    obj->value[4] = 15; /* nutrition */

    /* Give item to character */
    obj_to_char( obj, ch );

    /* Send messages */
    act( "You forage around and find $p.", ch, obj, NULL, TO_CHAR );
    act( "$n forages around and finds $p.", ch, obj, NULL, TO_ROOM );

    /* Spring bonus - chance for extra item */
    if ( (time_info.month >= 2 && time_info.month <= 4) && number_percent() < 15 )
    {
        obj = create_object( pObjIndex, 0 );
        if ( obj != NULL )
        {
            free_string( obj->name );
            obj->name = str_dup( entry->name );
            free_string( obj->short_descr );
            obj->short_descr = str_dup( entry->short_descr );
            free_string( obj->description );
            obj->description = str_dup( entry->long_descr );
            obj->value[0] = 1;  /* food value */
            obj->value[1] = 3;  /* hours to decay */
            obj->value[4] = 15; /* nutrition */
            obj_to_char( obj, ch );
            send_to_char( "You find extra forage!\n\r", ch );
        }
    }

    /* Improve skill */
    check_improve( ch, sn, TRUE, 1 );

    return;
}


void
spawn_hunted_animal( CHAR_DATA *ch )
{
    const struct hunted_animal_entry *entry;
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *mob;
    int chance = 5; /* Base 5% chance */
    int count = 0;
    int i;

    /* Only spawn if character is hunting */
    if ( !IS_SET(ch->act, PLR_HUNTING) )
	return;

    /* Only spawn in forest areas */
    if ( ch->in_room->sector_type != SECT_FOREST )
	return;

    /* Seasonal modifiers */
    switch ( time_info.month )
    {
        case 2: case 3: case 4:  /* Spring - animals spawn twice as often */
            chance *= 2;
            break;
        case 8: case 9: case 10: /* Fall - 25% less often */
            chance = chance * 3 / 4;
            break;
        case 0: case 1: case 11: /* Winter - no animals (hibernation) */
            return; /* Don't spawn any animals in winter */
    }

    /* Don't spawn if there are already too many mobs in the room */
    for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
	if ( !IS_NPC(mob) )
	    count++;
    
    if ( count > 3 ) /* Limit to 3+ players in room */
	return;

    /* Adjust chance based on affects */
    if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
	chance += 10;
    if ( IS_AFFECTED(ch, AFF_SNEAK) )
	chance += 10;
    if ( IS_AFFECTED(ch, AFF_FAERIE_FIRE) )
	chance -= 15;

    /* Ensure minimum chance */
    if ( chance < 1 )
	chance = 1;

    /* Roll for spawn */
    if ( number_percent() > chance )
	return;

    /* Count entries in table */
    count = 0;
    for ( i = 0; hunted_animal_table[i].name != NULL; i++ )
	count++;

    if ( count == 0 )
	return;

    /* Select random entry */
    entry = &hunted_animal_table[number_range( 0, count - 1 )];

    /* Safety check */
    if ( entry->name == NULL )
	return;

    /* Create the mobile */
    pMobIndex = get_mob_index( MOB_VNUM_HUNTED );
    if ( pMobIndex == NULL )
	return;

    mob = create_mobile( pMobIndex );
    if ( mob == NULL )
	return;

    /* Set race and level based on entry */
    if ( !str_cmp( entry->name, "fox" ) )
	mob->race = race_fox;
    else if ( !str_cmp( entry->name, "bear" ) )
	mob->race = race_bear;
    else if ( !str_cmp( entry->name, "wolf" ) )
	mob->race = race_wolf;
    else if ( !str_cmp( entry->name, "rabbit" ) )
	mob->race = race_cat; /* Using cat as closest to rabbit */
    else if ( !str_cmp( entry->name, "owl" ) )
	mob->race = race_bat; /* Using bat as closest to owl */
    
    mob->level = 1;
    
    /* Set stats using DICE_EASY */
    set_mob_dice( mob->pIndexData, DICE_EASY );

    /* Set mobile properties AFTER create_mobile and set_mob_dice */
    free_string( mob->name );
    mob->name = str_dup( entry->name );
    free_string( mob->short_descr );
    mob->short_descr = str_dup( entry->short_descr );
    free_string( mob->long_descr );
    mob->long_descr = str_dup( entry->long_descr );

    /* Add to room */
    char_to_room( mob, ch->in_room );

    /* Send message to room */
    act( "A $T appears in the area.", ch, NULL, entry->short_descr, TO_ROOM );

    return;
}

void
do_fish( CHAR_DATA *ch, char *argument )
{
    const struct fish_entry *table = NULL;
    const struct fish_entry *entry;
    OBJ_DATA *rod;
    OBJ_DATA *fish;
    OBJ_INDEX_DATA *pObjIndex;
    int chance;
    int count = 0;
    int i;
    int sn;

    if ( IS_NPC(ch) )
    {
	send_to_char( "You don't know how to fish.\n\r", ch );
	return;
    }

    sn = gsn_fishing;
    if ( (chance = get_skill(ch, sn)) == 0 )
    {
	send_to_char( "You don't know how to fish.\n\r", ch );
	return;
    }

    /* Seasonal modifiers */
    switch ( time_info.month )
    {
        case 2: case 3: case 4:  /* Spring - 10% higher success rate */
            chance += 10;
            break;
        case 8: case 9: case 10: /* Fall - 10% more failure */
            chance -= 10;
            break;
        case 0: case 1: case 11: /* Winter - only in rivers and oceans (not frozen) */
            if ( ch->in_room->sector_type != SECT_RIVER && ch->in_room->sector_type != SECT_OCEAN )
            {
                send_to_char( "The water is frozen over - you can only fish in rivers and oceans during winter.\n\r", ch );
                return;
            }
            break;
    }

    /* Check if holding a fishing rod */
    rod = get_eq_char( ch, WEAR_HOLD );
    if ( rod == NULL || rod->item_type != ITEM_FISHING_ROD )
    {
	send_to_char( "You need to be holding a fishing rod to fish.\n\r", ch );
	return;
    }

    /* Check if in a fishable area */
    if ( ch->in_room->sector_type != SECT_RIVER &&
	 ch->in_room->sector_type != SECT_LAKE &&
	 ch->in_room->sector_type != SECT_OCEAN )
    {
	send_to_char( "You can only fish in rivers, lakes, or oceans.\n\r", ch );
	return;
    }

    /* Add wait state */
    if ( !IS_IMMORTAL(ch) )
	WAIT_STATE( ch, skill_table[sn].beats );

    /* Determine which fish table to use based on sector type */
    switch ( ch->in_room->sector_type )
    {
	case SECT_RIVER:
	    table = river_fish_table;
	    break;
	case SECT_LAKE:
	    table = lake_fish_table;
	    break;
	case SECT_OCEAN:
	    table = ocean_fish_table;
	    break;
	default:
	    send_to_char( "You can't fish here.\n\r", ch );
	    return;
    }

    /* Count entries in the table */
    count = 0;
    for ( i = 0; table[i].name != NULL; i++ )
	count++;

    if ( count == 0 )
    {
	send_to_char( "There don't seem to be any fish here.\n\r", ch );
	return;
    }

    /* Cast line message */
    act( "You cast your line into the water.", ch, NULL, NULL, TO_CHAR );
    act( "$n casts $s fishing line into the water.", ch, NULL, NULL, TO_ROOM );

    /* Check skill success */
    if ( number_percent() > chance )
    {
	send_to_char( "You don't get any bites.\n\r", ch );
	check_improve( ch, sn, FALSE, 1 );
	return;
    }

    /* Select random fish from table */
    entry = &table[number_range( 0, count - 1 )];

    /* Check if fish size is too big for rod */
    if ( entry->size > rod->value[0] )
    {
	send_to_char( "You feel a strong tug on your line!\n\r", ch );
	send_to_char( "The fish is too strong and breaks your line!\n\r", ch );
	send_to_char( "You quickly replace the line and cast again.\n\r", ch );
	check_improve( ch, sn, FALSE, 1 );
	return;
    }

    /* Create the fish */
    pObjIndex = get_obj_index( OBJ_VNUM_FORAGED );
    if ( pObjIndex == NULL )
    {
	send_to_char( "Something went wrong with fishing.\n\r", ch );
	return;
    }

    fish = create_object( pObjIndex, 0 );
    if ( fish == NULL )
    {
	send_to_char( "Something went wrong with fishing.\n\r", ch );
	return;
    }

    /* Set fish properties */
    free_string( fish->name );
    fish->name = str_dup( entry->name );
    free_string( fish->short_descr );
    fish->short_descr = str_dup( entry->short_descr );
    free_string( fish->description );
    fish->description = str_dup( entry->long_descr );

    /* Set food values */
    fish->value[0] = 1;  /* food value */
    fish->value[1] = 4;  /* hours to decay */
    fish->value[4] = 20; /* nutrition */

    /* Give fish to character */
    obj_to_char( fish, ch );

    /* Send messages */
    act( "You reel in $p!", ch, fish, NULL, TO_CHAR );
    act( "$n reels in $p!", ch, fish, NULL, TO_ROOM );

    /* Improve skill */
    check_improve( ch, sn, TRUE, 1 );

    return;
}

void
do_hunt( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
    {
	send_to_char( "You don't know how to hunt.\n\r", ch );
	return;
    }

    if ( IS_SET(ch->act, PLR_HUNTING) )
    {
	REMOVE_BIT( ch->act, PLR_HUNTING );
	send_to_char( "You stop hunting for animals.\n\r", ch );
    }
    else
    {
	SET_BIT( ch->act, PLR_HUNTING );
	send_to_char( "You begin hunting for animals.\n\r", ch );
    }

    return;
}

void do_butcher(CHAR_DATA *ch, char *argument)
{

    /* Butcher skill, created by Argawal */
    /* Original Idea taken fom Carrion Fields Mud */
    /* If you have an interest in this skill, feel free */
    /* to use it in your mud if you so desire. */
    /* All I ask is that Argawal is credited with creating */
    /* this skill, as I wrote it from scratch. */

    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    int numst = 0;
    OBJ_DATA *steak;
    OBJ_DATA *obj;

    one_argument(argument, arg);

    if(get_skill(ch,gsn_butcher)==0)
    {
       send_to_char("Butchering is beyond your skills.\n\r",ch);
       return;
    }

    if(arg[0]=='\0')
    {
       send_to_char("Butcher what?\n\r",ch);
       return;
    }

    obj = get_obj_list( ch, arg, ch->in_room->contents ); 
    if ( obj == NULL )
    {
        send_to_char( "It's not here.\n\r", ch ); 
        return; 
    }

    if( (obj->item_type != ITEM_CORPSE_NPC)
        && (obj->item_type!=ITEM_CORPSE_PC) )
    {
        send_to_char( "You can only butcher corpses.\n\r", ch ); 
        return; 
    }

    /* create and rename the steak */
    buf[0]='\0';
    strcat(buf,"A steak of ");
    strcat(buf,str_dup(obj->short_descr));
    strcat(buf," is here.");

    steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 ); 

    steak->description=str_dup(buf);
    steak->value[0] = ch->level / 2;
    steak->value[1] = ch->level;

    buf[0]='\0';
    strcat(buf,"A steak of ");
    strcat(buf,str_dup(obj->short_descr));

    steak->short_descr=str_dup(buf);

    /* Check the skill roll, and put a random ammount of steaks here. */

    if(number_percent( ) < get_skill(ch,gsn_butcher))
    { 
       numst = dice(1,4);
       switch(numst)
       {
       case 1:
         steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 );
         obj_to_room( steak, ch->in_room );
         act( "$n butchers a corpse and creates a steak.", ch, steak, NULL, TO_ROOM );
         act( "You butcher a corpse and create a steak.", ch, steak, NULL, TO_CHAR );
         break;
       case 2: 
         steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 ); 
         obj_to_room( steak, ch->in_room );
         steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 ); 
         obj_to_room( steak, ch->in_room );
         act( "$n butchers a corpse and creates two steaks.", ch, steak, NULL, TO_ROOM );
         act( "You butcher a corpse and create two steaks.", ch, steak, NULL, TO_CHAR );
         break; 
       case 3:
         steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 ); 
         obj_to_room( steak, ch->in_room );
         steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 ); 
         obj_to_room( steak, ch->in_room );
         steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 ); 
         obj_to_room( steak, ch->in_room );
         act( "$n butchers a corpse and creates three steaks.", ch, steak, NULL, TO_ROOM );
         act( "You butcher a corpse and create three steaks.", ch, steak, NULL, TO_CHAR );
         break;
       case 4:
         steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 ); 
         obj_to_room( steak, ch->in_room );
         steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 ); 
         obj_to_room( steak, ch->in_room );
         steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 ); 
         obj_to_room( steak, ch->in_room );
         steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 ); 
         obj_to_room( steak, ch->in_room );
         act( "$n butchers a corpse and creates four steaks.", ch, steak, NULL, TO_ROOM );
         act( "You butcher a corpse and create four steaks.", ch, steak, NULL, TO_CHAR );
         break;
      } 
      check_improve(ch,gsn_butcher,TRUE,1);

    }   
    else
    {
       act( "$n fails to butcher a corpse, and destroys it.", ch, steak, NULL, TO_ROOM );
       act( "You fail to butcher a corpse, and destroy it.", ch, steak, NULL, TO_CHAR );
       check_improve(ch,gsn_butcher,FALSE,1);
    } 
    /* dump items caried */
    /* Taken from the original ROM code and added into here. */

    if ( obj->item_type == ITEM_CORPSE_PC )
    {   /* save the contents */ 
       {

            OBJ_DATA *t_obj, *next_obj; 
            for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
            {
                next_obj = t_obj->next_content; 
                obj_from_obj(t_obj); 
                if (obj->in_obj) /* in another object */
                    obj_to_obj(t_obj,obj->in_obj); 
                else if (obj->carried_by) /* carried */
                    if (obj->wear_loc == WEAR_FLOAT)
                        if (obj->carried_by->in_room == NULL)
                            extract_obj(t_obj); 
                        else
                            obj_to_room(t_obj,obj->carried_by->in_room); 
                    else
                        obj_to_char(t_obj,obj->carried_by); 
               else if (obj->in_room == NULL) /* destroy it */
                    extract_obj(t_obj); 
                else /* to a room */
                    obj_to_room(t_obj,obj->in_room); 
           }
      }

  }

    if ( obj->item_type == ITEM_CORPSE_NPC )
    {
       {
            OBJ_DATA *t_obj, *next_obj; 
            for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
            {
                next_obj = t_obj->next_content; 
                obj_from_obj(t_obj); 
                if (obj->in_obj) /* in another object */
                    obj_to_obj(t_obj,obj->in_obj); 
                else if (obj->carried_by) /* carried */
                    if (obj->wear_loc == WEAR_FLOAT)
                        if (obj->carried_by->in_room == NULL)
                            extract_obj(t_obj); 
                        else
                            obj_to_room(t_obj,obj->carried_by->in_room); 
                    else
                        obj_to_char(t_obj,obj->carried_by); 
                else if (obj->in_room == NULL) /* destroy it */
                    extract_obj(t_obj); 
                else /* to a room */
                    obj_to_room(t_obj,obj->in_room); 
         }
     }
  }

    /* Now remove the corpse */
    extract_obj(obj);
    return;
}

void do_skin(CHAR_DATA *ch, char *argument)
{

    /* Butcher skill, created by Argawal */
    /* Original Idea taken fom Carrion Fields Mud */
    /* If you have an interest in this skill, feel free */
    /* to use it in your mud if you so desire. */
    /* All I ask is that Argawal is credited with creating */
    /* this skill, as I wrote it from scratch. */

    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    int numst = 0;
    OBJ_DATA *steak;
    OBJ_DATA *obj;

    one_argument(argument, arg);

    if(get_skill(ch,gsn_skin)==0)
    {
       send_to_char("Skinning is beyond your skills.\n\r",ch);
       return;
    }

    if(arg[0]=='\0')
    {
       send_to_char("Skin what?\n\r",ch);
       return;
    }

    obj = get_obj_list( ch, arg, ch->in_room->contents ); 
    if ( obj == NULL )
    {
        send_to_char( "It's not here.\n\r", ch ); 
        return; 
    }

    if( (obj->item_type != ITEM_CORPSE_NPC)
        && (obj->item_type!=ITEM_CORPSE_PC) )
    {
        send_to_char( "You can only skin corpses.\n\r", ch ); 
        return; 
    }

    /* create and rename the steak */
    buf[0]='\0';
    strcat(buf,"A skin of ");
    strcat(buf,str_dup(obj->short_descr));
    strcat(buf," is here.");

    steak = create_object( get_obj_index( OBJ_VNUM_SKIN ), 0 ); 

    steak->description=str_dup(buf);
    steak->value[0] = ch->level / 2;
    steak->value[1] = ch->level;

    buf[0]='\0';
    strcat(buf,"A skin of ");
    strcat(buf,str_dup(obj->short_descr));

    steak->short_descr=str_dup(buf);

    /* Check the skill roll, and put a random ammount of steaks here. */

    if(number_percent( ) < get_skill(ch,gsn_skin))
    { 
       numst = dice(1,4);
       switch(numst)
       {
       case 1:
         steak = create_object( get_obj_index( OBJ_VNUM_SKIN ), 0 );
         obj_to_room( steak, ch->in_room );
         act( "$n skins a corpse and creates a hide.", ch, steak, NULL, TO_ROOM );
         act( "You skin a corpse and create a hide.", ch, steak, NULL, TO_CHAR );
         break;
       case 2: 
         steak = create_object( get_obj_index( OBJ_VNUM_SKIN ), 0 ); 
         obj_to_room( steak, ch->in_room );
         act( "$n skins a corpse and creates a hide.", ch, steak, NULL, TO_ROOM );
         act( "You skin a corpse and creates a hide.", ch, steak, NULL, TO_CHAR );
         break; 
       case 3:
         steak = create_object( get_obj_index( OBJ_VNUM_SKIN ), 0 ); 
         obj_to_room( steak, ch->in_room );
         act( "$n skins a corpse and creates a hide.", ch, steak, NULL, TO_ROOM );
         act( "You skin a corpse and create a hide.", ch, steak, NULL, TO_CHAR );
         break;
       case 4:
         steak = create_object( get_obj_index( OBJ_VNUM_SKIN ), 0 ); 
         obj_to_room( steak, ch->in_room );
         act( "$n skins a corpse and creates a hide.", ch, steak, NULL, TO_ROOM );
         act( "You skin a corpse and create a hide.", ch, steak, NULL, TO_CHAR );
         break;
      } 
      check_improve(ch,gsn_skin,TRUE,1);

    }   
    else
    {
       act( "$n fails to skin a corpse, and destroys it.", ch, steak, NULL, TO_ROOM );
       act( "You fail to skin a corpse, and destroy it.", ch, steak, NULL, TO_CHAR );
       check_improve(ch,gsn_skin,FALSE,1);
    } 
    /* dump items caried */
    /* Taken from the original ROM code and added into here. */

    if ( obj->item_type == ITEM_CORPSE_PC )
    {   /* save the contents */ 
       {

            OBJ_DATA *t_obj, *next_obj; 
            for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
            {
                next_obj = t_obj->next_content; 
                obj_from_obj(t_obj); 
                if (obj->in_obj) /* in another object */
                    obj_to_obj(t_obj,obj->in_obj); 
                else if (obj->carried_by) /* carried */
                    if (obj->wear_loc == WEAR_FLOAT)
                        if (obj->carried_by->in_room == NULL)
                            extract_obj(t_obj); 
                        else
                            obj_to_room(t_obj,obj->carried_by->in_room); 
                    else
                        obj_to_char(t_obj,obj->carried_by); 
               else if (obj->in_room == NULL) /* destroy it */
                    extract_obj(t_obj); 
                else /* to a room */
                    obj_to_room(t_obj,obj->in_room); 
           }
      }

  }

    if ( obj->item_type == ITEM_CORPSE_NPC )
    {
       {
            OBJ_DATA *t_obj, *next_obj; 
            for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
            {
                next_obj = t_obj->next_content; 
                obj_from_obj(t_obj); 
                if (obj->in_obj) /* in another object */
                    obj_to_obj(t_obj,obj->in_obj); 
                else if (obj->carried_by) /* carried */
                    if (obj->wear_loc == WEAR_FLOAT)
                        if (obj->carried_by->in_room == NULL)
                            extract_obj(t_obj); 
                        else
                            obj_to_room(t_obj,obj->carried_by->in_room); 
                    else
                        obj_to_char(t_obj,obj->carried_by); 
                else if (obj->in_room == NULL) /* destroy it */
                    extract_obj(t_obj); 
                else /* to a room */
                    obj_to_room(t_obj,obj->in_room); 
         }
     }
  }

    /* Now remove the corpse */
    extract_obj(obj);
    return;
}

/* Gather firewood from forest areas */
void do_gather( CHAR_DATA *ch, char *argument )
{
    const struct foraging_entry *entry;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    int chance;
    int count = 0;
    int i;
    
    if ( ch->in_room == NULL )
    {
        send_to_char( "You are nowhere.\n\r", ch );
        return;
    }
    
    if ( ch->in_room->sector_type != SECT_FOREST )
    {
        send_to_char( "You can only gather firewood in forests.\n\r", ch );
        return;
    }
    
    if ( ch->move < 10 )
    {
        send_to_char( "You are too tired to gather firewood.\n\r", ch );
        return;
    }
    
    /* Use move points */
    ch->move -= 10;
    
    /* Base chance of success */
    chance = 60;
    
    /* Seasonal modifiers */
    switch ( time_info.month )
    {
        case 2: case 3: case 4:  /* Spring - better gathering */
            chance += 10;
            break;
        case 8: case 9: case 10: /* Fall - worse gathering */
            chance -= 10;
            break;
        case 0: case 1: case 11: /* Winter - very scarce */
            chance -= 30;
            break;
    }
    
    /* Count entries in the table */
    for ( i = 0; uk_wood_table[i].name != NULL; i++ )
        count++;
    
    if ( count == 0 )
    {
        send_to_char( "You search around but find nothing of use here.\n\r", ch );
        return;
    }
    
    if ( number_percent() < chance )
    {
        /* Select random entry from table */
        entry = &uk_wood_table[number_range( 0, count - 1 )];
        
        /* Create the firewood item */
        pObjIndex = get_obj_index( OBJ_VNUM_FIREWOOD );
        if ( pObjIndex == NULL )
        {
            send_to_char( "Something went wrong with gathering.\n\r", ch );
            return;
        }
        
        obj = create_object( pObjIndex, 0 );
        if ( obj == NULL )
        {
            send_to_char( "Something went wrong with gathering.\n\r", ch );
            return;
        }
        
        /* Set object properties */
        free_string( obj->name );
        obj->name = str_dup( entry->name );
        free_string( obj->short_descr );
        obj->short_descr = str_dup( entry->short_descr );
        free_string( obj->description );
        obj->description = str_dup( entry->long_descr );
        
        /* Set as firewood (trash type) */
        obj->item_type = ITEM_FIREWOOD;
        
        /* Give item to character */
        obj_to_char( obj, ch );
        
        /* Send messages */
        act( "You gather $p from the forest.", ch, obj, NULL, TO_CHAR );
        act( "$n gathers $p from the forest.", ch, obj, NULL, TO_ROOM );
        
        /* Spring bonus - chance for extra wood */
        if ( (time_info.month >= 2 && time_info.month <= 4) && number_percent() < 20 )
        {
            obj = create_object( pObjIndex, 0 );
            if ( obj != NULL )
            {
                free_string( obj->name );
                obj->name = str_dup( entry->name );
                free_string( obj->short_descr );
                obj->short_descr = str_dup( entry->short_descr );
                free_string( obj->description );
                obj->description = str_dup( entry->long_descr );
                obj->item_type = ITEM_FIREWOOD;
                obj_to_char( obj, ch );
                send_to_char( "You find extra firewood!\n\r", ch );
            }
        }
    }
    else
    {
        send_to_char( "You search but find no suitable firewood.\n\r", ch );
    }
    
    return;
}

/* Build a temporary shelter */
void do_build_shelter( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *firewood;
    OBJ_DATA *shelter;
    int move_cost = 50;
    
    if ( ch->in_room == NULL )
    {
        send_to_char( "You are nowhere.\n\r", ch );
        return;
    }
    
    if ( ch->move < move_cost )
    {
        send_to_char( "You are too tired to build a shelter.\n\r", ch );
        return;
    }
    
    /* Check for firewood (any wood branches) */
    firewood = get_obj_carry( ch, "firewood", ch );
    if ( firewood == NULL )
    {
        send_to_char( "You need wood branches to build a shelter.\n\r", ch );
        return;
    }
    
    /* Check if there's already a shelter in the room */
    for ( shelter = ch->in_room->contents; shelter != NULL; shelter = shelter->next_content )
    {
        if ( shelter->pIndexData->vnum == OBJ_VNUM_SHELTER )
        {
            send_to_char( "There is already a shelter here.\n\r", ch );
            return;
        }
    }
    
    /* Use move points */
    ch->move -= move_cost;
    
    /* Remove firewood */
    extract_obj( firewood );
    
    /* Create shelter */
    {
        OBJ_INDEX_DATA *obj_index = get_obj_index( OBJ_VNUM_SHELTER );
        if ( obj_index == NULL )
        {
            send_to_char( "You cannot build a shelter here.\n\r", ch );
            return;
        }
        shelter = create_object( obj_index, 0 );
    }
    
    obj_to_room( shelter, ch->in_room );
    
    act( "You build a temporary shelter from the firewood.", ch, NULL, NULL, TO_CHAR );
    act( "$n builds a temporary shelter from firewood.", ch, NULL, NULL, TO_ROOM );
    
    return;
}

void do_blanket( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *skin1, *skin2;
    OBJ_DATA *blanket;
    OBJ_INDEX_DATA *pObjIndex;
    
    if ( ch->in_room == NULL )
    {
	send_to_char( "You are nowhere.\n\r", ch );
	return;
    }
    
    if ( ch->move < 15 )
    {
	send_to_char( "You are too tired to make a blanket.\n\r", ch );
	return;
    }
    
    /* Find first skin */
    for ( skin1 = ch->carrying; skin1 != NULL; skin1 = skin1->next_content )
    {
	if ( skin1->pIndexData->vnum == OBJ_VNUM_SKIN )
	    break;
    }
    if ( skin1 == NULL )
    {
	send_to_char( "You need 2 skins to make a blanket.\n\r", ch );
	return;
    }
    
    /* Find second skin */
    for ( skin2 = ch->carrying; skin2 != NULL; skin2 = skin2->next_content )
    {
	if ( skin2->pIndexData->vnum == OBJ_VNUM_SKIN && skin2 != skin1 )
	    break;
    }
    if ( skin2 == NULL )
    {
	send_to_char( "You need 2 skins to make a blanket.\n\r", ch );
	return;
    }
    
    ch->move -= 15;
    
    /* Remove both skins */
    extract_obj( skin1 );
    extract_obj( skin2 );
    
    /* Create blanket */
    pObjIndex = get_obj_index( OBJ_VNUM_BLANKET );
    if ( pObjIndex == NULL )
    {
	bug( "do_blanket: OBJ_VNUM_BLANKET not found", 0 );
	send_to_char( "You cannot make a blanket right now.\n\r", ch );
	return;
    }
    
    blanket = create_object( pObjIndex, 0 );
    obj_to_char( blanket, ch );
    
    act( "You make a warm blanket from two skins.", ch, NULL, NULL, TO_CHAR );
    act( "$n makes a warm blanket from two skins.", ch, NULL, NULL, TO_ROOM );
    
    return;
}


