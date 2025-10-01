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