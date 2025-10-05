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


#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"

/*
 * Local functions
 */
void	list_group_chosen	args( ( CHAR_DATA *ch ) );

/* used to get new skills */
void
do_gain( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *trainer;
    int gn = 0, sn = 0;

    if (IS_NPC(ch))
	return;

    /* find a trainer */
    for ( trainer = ch->in_room->people; 
	  trainer != NULL; 
	  trainer = trainer->next_in_room)
	if (IS_NPC(trainer) && IS_SET(trainer->act,ACT_GAIN))
	    break;

    if ( trainer == NULL || !can_see( ch, trainer ) )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return;
    }

    if ( !xIS_SET( trainer->pIndexData->train, ch->class ) )
    {
        ch_printf( ch, "%s cannot do that here.\n\r",
                   capitalize( aoran( class_table[ch->class].name ) ) );
        return;
    }

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	do_function(trainer, &do_say, "Pardon me?");
	return;
    }



    if (!str_prefix(arg,"convert"))
    {
	if (ch->practice < 10)
	{
	    act("$N tells you 'You are not yet ready.'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

	act( "$N helps you apply your practice to training.",
		ch,NULL,trainer,TO_CHAR);
	ch->practice -= 10;
	ch->train +=1 ;
	return;
    }

else if (!str_prefix(arg,"revert"))
{
if (ch->train < 1)
{
    act("$N tells you 'You are not yet ready.'",
    ch,NULL,trainer,TO_CHAR);
    return;
}

act( "$N helps you apply your training to practice.",
    ch,NULL,trainer,TO_CHAR);
ch->train -= 1;
ch->practice +=10 ;
return;
    } else {
act("$N tells you 'I do not understand...'",ch,NULL,trainer,TO_CHAR);
return;
}



}


/* RT spells and skills show the players spells (or skills) */

void do_spells(CHAR_DATA *ch, char *argument)
{
    BUFFER *buffer;
    char arg[MAX_INPUT_LENGTH];
    char spell_list[LEVEL_HERO + 1][MAX_STRING_LENGTH];
    char spell_columns[LEVEL_HERO + 1];
    int sn, level, min_lev = 1, max_lev = LEVEL_HERO, mana;
    bool fAll = FALSE, found = FALSE;
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
        return;

    if (argument[0] != '\0')
    {
	fAll = TRUE;

	if (str_prefix(argument,"all"))
	{
	    argument = one_argument(argument,arg);
	    if (!is_number(arg))
	    {
		send_to_char("Arguments must be numerical or all.\n\r",ch);
		return;
	    }
	    max_lev = atoi(arg);

	    if (max_lev < 1 || max_lev > LEVEL_HERO)
	    {
		sprintf(buf,"Levels must be between 1 and %d.\n\r",LEVEL_HERO);
		send_to_char(buf,ch);
		return;
	    }

	    if (argument[0] != '\0')
	    {
		argument = one_argument(argument,arg);
		if (!is_number(arg))
		{
		    send_to_char("Arguments must be numerical or all.\n\r",ch);
		    return;
		}
		min_lev = max_lev;
		max_lev = atoi(arg);

		if (max_lev < 1 || max_lev > LEVEL_HERO)
		{
		    sprintf(buf,
			"Levels must be between 1 and %d.\n\r",LEVEL_HERO);
		    send_to_char(buf,ch);
		    return;
		}

		if (min_lev > max_lev)
		{
		    send_to_char("That would be silly.\n\r",ch);
		    return;
		}
	    }
	}
    }


    /* initialize data */
    for (level = 0; level < LEVEL_HERO + 1; level++)
    {
        spell_columns[level] = 0;
        spell_list[level][0] = '\0';
    }

    for (sn = 0; sn < top_skill; sn++)
    {
        if (skill_table[sn].name == NULL )
	    break;

	if ((level = get_multiclass_skill_level(ch, sn)) < LEVEL_HERO + 1
	&&  (fAll || multiclass_meets_skill_level(ch, sn))
	&&  level >= min_lev && level <= max_lev
	&&  skill_table[sn].spell_fun != spell_null
	&&  ch->pcdata->skill[sn].percent > 0
	&&  (skill_table[sn].rating[ch->class] > 0 || is_skill_available_to_multiclass(ch, sn)))
        {
	    found = TRUE;
	    level = get_multiclass_skill_level(ch, sn);
	    if (ch->level < level)
	    	sprintf(buf,"%-18s n/a      ", skill_table[sn].name);
	    else
	    {
		mana = UMAX(skill_table[sn].min_mana,
		    100/(2 + ch->level - level));
	        sprintf(buf,"%-18s  %3d mana  ",skill_table[sn].name,mana);
	    }

	    if (spell_list[level][0] == '\0')
          	sprintf(spell_list[level],"\n\rLevel %2d: %s",level,buf);
	    else /* append */
	    {
          	if ( ++spell_columns[level] % 2 == 0)
		    strcat(spell_list[level],"\n\r          ");
          	strcat(spell_list[level],buf);
	    }
	}
    }

    /* return results */

    if (!found)
    {
      	send_to_char("No spells found.\n\r",ch);
      	return;
    }

    buffer = new_buf();
    for (level = 0; level < LEVEL_HERO + 1; level++)
      	if (spell_list[level][0] != '\0')
	    add_buf(buffer,spell_list[level]);
    add_buf(buffer,"\n\r");
    page_to_char(buf_string(buffer),ch);
    free_buf(buffer);
}

void do_skills(CHAR_DATA *ch, char *argument)
{
    BUFFER *buffer;
    char arg[MAX_INPUT_LENGTH];
    char skill_list[LEVEL_HERO + 1][MAX_STRING_LENGTH];
    char skill_columns[LEVEL_HERO + 1];
    int sn, level, min_lev = 1, max_lev = LEVEL_HERO;
    bool fAll = FALSE, found = FALSE;
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
      return;

    if (argument[0] != '\0')
    {
	fAll = TRUE;

	if (str_prefix(argument,"all"))
	{
	    argument = one_argument(argument,arg);
	    if (!is_number(arg))
	    {
		send_to_char("Arguments must be numerical or all.\n\r",ch);
		return;
	    }
	    max_lev = atoi(arg);

	    if (max_lev < 1 || max_lev > LEVEL_HERO)
	    {
		sprintf(buf,"Levels must be between 1 and %d.\n\r",LEVEL_HERO);
		send_to_char(buf,ch);
		return;
	    }

	    if (argument[0] != '\0')
	    {
		argument = one_argument(argument,arg);
		if (!is_number(arg))
		{
		    send_to_char("Arguments must be numerical or all.\n\r",ch);
		    return;
		}
		min_lev = max_lev;
		max_lev = atoi(arg);

		if (max_lev < 1 || max_lev > LEVEL_HERO)
		{
		    sprintf(buf,
			"Levels must be between 1 and %d.\n\r",LEVEL_HERO);
		    send_to_char(buf,ch);
		    return;
		}

		if (min_lev > max_lev)
		{
		    send_to_char("That would be silly.\n\r",ch);
		    return;
		}
	    }
	}
    }


    /* initialize data */
    for (level = 0; level < LEVEL_HERO + 1; level++)
    {
        skill_columns[level] = 0;
        skill_list[level][0] = '\0';
    }

    for (sn = 0; sn < top_skill; sn++)
    {
        if (skill_table[sn].name == NULL )
	    break;

	if ((level = get_multiclass_skill_level(ch, sn)) < LEVEL_HERO + 1
	&&  (fAll || multiclass_meets_skill_level(ch, sn))
	&&  level >= min_lev && level <= max_lev
	&&  skill_table[sn].spell_fun == spell_null
	&&  (ch->pcdata->skill[sn].percent > 0 || is_skill_available_to_multiclass(ch, sn))
	&&  (skill_table[sn].rating[ch->class] > 0 || is_skill_available_to_multiclass(ch, sn)))
        {
	    found = TRUE;
	    level = get_multiclass_skill_level(ch, sn);
	    if (ch->level < level)
	    	sprintf(buf,"%-18s n/a      ", skill_table[sn].name);
	    else
	    	sprintf(buf,"%-18s %3d%%      ",skill_table[sn].name,
		    ch->pcdata->skill[sn].percent);

	    if (skill_list[level][0] == '\0')
          	sprintf(skill_list[level],"\n\rLevel %2d: %s",level,buf);
	    else /* append */
	    {
          	if ( ++skill_columns[level] % 2 == 0)
		    strcat(skill_list[level],"\n\r          ");
          	strcat(skill_list[level],buf);
	    }
	}
    }

    /* return results */

    if (!found)
    {
      	send_to_char("No skills found.\n\r",ch);
      	return;
    }

    buffer = new_buf();
    for (level = 0; level < LEVEL_HERO + 1; level++)
      	if (skill_list[level][0] != '\0')
	    add_buf(buffer,skill_list[level]);
    add_buf(buffer,"\n\r");
    page_to_char(buf_string(buffer),ch);
    free_buf(buffer);
}

/* shows skills, groups and costs (only if not bought) */
void list_group_costs(CHAR_DATA *ch)
{
    char buf[100];
    int gn,sn,col;

    if (IS_NPC(ch))
	return;

    col = 0;

    sprintf(buf,"%-18s %-5s %-18s %-5s %-18s %-5s\n\r","group","cp","group","cp","group","cp");
    send_to_char(buf,ch);

    for (gn = 0; gn < MAX_GROUP; gn++)
    {
	if (group_table[gn].name == NULL)
	    break;

        if (!ch->gen_data->group_chosen[gn] 
	&&  !ch->pcdata->group_known[gn]
	&&  group_table[gn].rating[ch->class] > 0)
	{
	    sprintf(buf,"%-18s %-5d ",group_table[gn].name,
				    group_table[gn].rating[ch->class]);
	    send_to_char(buf,ch);
	    if (++col % 3 == 0)
		send_to_char("\n\r",ch);
	}
    }
    if ( col % 3 != 0 )
        send_to_char( "\n\r", ch );
    send_to_char("\n\r",ch);

    col = 0;

    sprintf(buf,"%-18s %-5s %-18s %-5s %-18s %-5s\n\r","skill","cp","skill","cp","skill","cp");
    send_to_char(buf,ch);

    for (sn = 0; sn < top_skill; sn++)
    {
        if (skill_table[sn].name == NULL)
            break;

        if (!ch->gen_data->skill_chosen[sn] 
	&&  ch->pcdata->skill[sn].percent == 0
	&&  skill_table[sn].spell_fun == spell_null
	&&  skill_table[sn].rating[ch->class] > 0)
        {
            sprintf(buf,"%-18s %-5d ",skill_table[sn].name,
                                    skill_table[sn].rating[ch->class]);
            send_to_char(buf,ch);
            if (++col % 3 == 0)
                send_to_char("\n\r",ch);
        }
    }
    if ( col % 3 != 0 )
        send_to_char( "\n\r", ch );
    send_to_char("\n\r",ch);

    sprintf(buf,"Creation points: %d\n\r",ch->pcdata->points);
    send_to_char(buf,ch);
    sprintf(buf,"Experience per level: %d\n\r",
	    exp_per_level(ch,ch->gen_data->points_chosen));
    send_to_char(buf,ch);
    return;
}


void list_group_chosen(CHAR_DATA *ch)
{
    char buf[100];
    int gn,sn,col;

    if (IS_NPC(ch))
        return;

    col = 0;

    sprintf(buf,"%-18s %-5s %-18s %-5s %-18s %-5s","group","cp","group","cp","group","cp\n\r");
    send_to_char(buf,ch);

    for (gn = 0; gn < MAX_GROUP; gn++)
    {
        if (group_table[gn].name == NULL)
            break;

        if (ch->gen_data->group_chosen[gn] 
	&&  group_table[gn].rating[ch->class] > 0)
        {
            sprintf(buf,"%-18s %-5d ",group_table[gn].name,
                                    group_table[gn].rating[ch->class]);
            send_to_char(buf,ch);
            if (++col % 3 == 0)
                send_to_char("\n\r",ch);
        }
    }
    if ( col % 3 != 0 )
        send_to_char( "\n\r", ch );
    send_to_char("\n\r",ch);

    col = 0;

    sprintf(buf,"%-18s %-5s %-18s %-5s %-18s %-5s","skill","cp","skill","cp","skill","cp\n\r");
    send_to_char(buf,ch);

    for (sn = 0; sn < top_skill; sn++)
    {
        if (skill_table[sn].name == NULL)
            break;

        if (ch->gen_data->skill_chosen[sn] 
	&&  skill_table[sn].rating[ch->class] > 0)
        {
            sprintf(buf,"%-18s %-5d ",skill_table[sn].name,
                                    skill_table[sn].rating[ch->class]);
            send_to_char(buf,ch);
            if (++col % 3 == 0)
                send_to_char("\n\r",ch);
        }
    }
    if ( col % 3 != 0 )
        send_to_char( "\n\r", ch );
    send_to_char("\n\r",ch);

    sprintf(buf,"Creation points: %d\n\r",ch->gen_data->points_chosen);
    send_to_char(buf,ch);
    sprintf(buf,"Experience per level: %d\n\r",
	    exp_per_level(ch,ch->gen_data->points_chosen));
    send_to_char(buf,ch);
    return;
}
/*
int
exp_per_level( CHAR_DATA *ch, int points )
{
    int expl;
    int	inc;
    int	mult;

    if ( IS_NPC( ch ) )
	return 1000; 

    expl = 1000;
    inc = 500;
    //mult = race_table[ch->race].class_mult[ch->class];
    mult = ch->level
    if ( mult == 0 )
        mult = 100;

    if ( points < 40 )
	return 1000 * mult / 100;

    points -= 40;

    while ( points > 9 )
    {
	expl += inc;
        points -= 10;
        if (points > 9)
	{
	    expl += inc;
	    inc *= 2;
	    points -= 10;
	}
    }

    expl += points * inc / 10;  

    return expl * mult / 100;
}
*/
int
exp_per_level( CHAR_DATA *ch, int points )
{
    int current_level;
    
    if ( IS_NPC( ch ) )
	return 1000; 

    /* Use multiclass system for PCs */
    /* Note: points parameter is ignored in multiclass system */
    current_level = get_total_level( ch );
    
    if ( current_level >= 20 )
        return 999999; /* Can't level beyond 20 */
        
    /* Return the total experience needed for the next level */
    return get_multiclass_exp_requirement( ch, current_level + 1 );
}

/* this procedure handles the input parsing for the skill generator */
bool parse_gen_groups(CHAR_DATA *ch,char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    int gn,sn,i;

    if (argument[0] == '\0')
	return FALSE;

    argument = one_argument(argument,arg);

    if (!str_prefix(arg,"help"))
    {
	if (argument[0] == '\0')
	{
	    do_function(ch, &do_help, "group help");
	    return TRUE;
	}

        do_function(ch, &do_help, argument);
	return TRUE;
    }

    if (!str_prefix(arg,"add"))
    {
	if (argument[0] == '\0')
	{
	    send_to_char("You must provide a skill name.\n\r",ch);
	    return TRUE;
	}

	gn = group_lookup(argument);
	if (gn != -1)
	{
	    if (ch->gen_data->group_chosen[gn]
	    ||  ch->pcdata->group_known[gn])
	    {
		send_to_char("You already know that group!\n\r",ch);
		return TRUE;
	    }

	    if (group_table[gn].rating[ch->class] < 1)
	    {
	  	send_to_char("That group is not available.\n\r",ch);
	 	return TRUE;
	    }

	    /* Close security hole */
	    if (ch->gen_data->points_chosen + group_table[gn].rating[ch->class]
		> 300)
	    {
		send_to_char(
		    "You cannot take more than 300 creation points.\n\r", ch);
		return TRUE;
	    }

	    sprintf(buf,"%s group added\n\r",group_table[gn].name);
	    send_to_char(buf,ch);
	    ch->gen_data->group_chosen[gn] = TRUE;
	    ch->gen_data->points_chosen += group_table[gn].rating[ch->class];
	    gn_add(ch,gn);
	    ch->pcdata->points += group_table[gn].rating[ch->class];
	    return TRUE;
	}

	sn = skill_lookup(argument);
	if (sn != -1)
	{
	    if (ch->gen_data->skill_chosen[sn]
	    ||  ch->pcdata->skill[sn].percent > 0)
	    {
		send_to_char("You already know that skill!\n\r",ch);
		return TRUE;
	    }

	    if (skill_table[sn].rating[ch->class] < 1
	    ||  skill_table[sn].spell_fun != spell_null)
	    {
		send_to_char("That skill is not available.\n\r",ch);
		return TRUE;
	    }

	    /* Close security hole */
	    if (ch->gen_data->points_chosen + skill_table[sn].rating[ch->class]
		> 300)
	    {
		send_to_char(
		    "You cannot take more than 300 creation points.\n\r", ch);
		return TRUE;
	    }
	    sprintf(buf, "%s skill added\n\r",skill_table[sn].name);
	    send_to_char(buf,ch);
	    ch->gen_data->skill_chosen[sn] = TRUE;
	    ch->gen_data->points_chosen += skill_table[sn].rating[ch->class];
	    ch->pcdata->skill[sn].percent = 1;
	    ch->pcdata->skill[sn].learned_age = GET_AGE( ch );
	    ch->pcdata->points += skill_table[sn].rating[ch->class];
	    return TRUE;
	}

	send_to_char("No skills or groups by that name...\n\r",ch);
	return TRUE;
    }

    if (!strcmp(arg,"drop"))
    {
	if (argument[0] == '\0')
  	{
	    send_to_char("You must provide a skill to drop.\n\r",ch);
	    return TRUE;
	}

	gn = group_lookup(argument);
	if (gn != -1 && ch->gen_data->group_chosen[gn])
	{
	    send_to_char("Group dropped.\n\r",ch);
	    ch->gen_data->group_chosen[gn] = FALSE;
	    ch->gen_data->points_chosen -= group_table[gn].rating[ch->class];
	    gn_remove(ch,gn);
	    for (i = 0; i < MAX_GROUP; i++)
	    {
		if (ch->gen_data->group_chosen[gn])
		    gn_add(ch,gn);
	    }
	    ch->pcdata->points -= group_table[gn].rating[ch->class];
	    return TRUE;
	}

	sn = skill_lookup(argument);
	if (sn != -1 && ch->gen_data->skill_chosen[sn])
	{
	    send_to_char("Skill dropped.\n\r",ch);
	    ch->gen_data->skill_chosen[sn] = FALSE;
	    ch->gen_data->points_chosen -= skill_table[sn].rating[ch->class];
	    ch->pcdata->skill[sn].percent = 0;
	    ch->pcdata->skill[sn].learned_age = 0;
	    ch->pcdata->points -= skill_table[sn].rating[ch->class];
	    return TRUE;
	}

	send_to_char("You haven't bought any such skill or group.\n\r",ch);
	return TRUE;
    }

    if (!str_prefix(arg,"premise"))
    {
	do_function(ch, &do_help, "premise");
	return TRUE;
    }

    if (!str_prefix(arg,"list"))
    {
	list_group_costs(ch);
	return TRUE;
    }

    if (!str_prefix(arg,"learned"))
    {
	list_group_chosen(ch);
	return TRUE;
    }

    if (!str_prefix(arg,"info"))
    {
	do_function(ch, &do_groups, argument);
	return TRUE;
    }

    return FALSE;
}


/* shows all groups, or the sub-members of a group */
void
do_groups( CHAR_DATA *ch, char *argument )
{
    BUFFER *	pBuf;
    int		gn;
    int		sn;
    int		col;

    if ( IS_NPC( ch ) )
	return;

    col = 0;

    if ( *argument == '\0' )
    {   /* show all groups ch has */
	pBuf = new_buf( );
	for ( gn = 0; gn < MAX_GROUP; gn++ )
        {
	    if ( group_table[gn].name == NULL )
		break;
	    if ( ch->pcdata->group_known[gn] )
	    {
		buf_printf( pBuf,"%-20s ", group_table[gn].name );
		if ( ++col % 3 == 0 )
		    add_buf( pBuf, "\n\r" );
	    }
        }
        if ( col % 3 != 0 )
            add_buf( pBuf,"\n\r" );
        buf_printf( pBuf, "Creation points: %d\n\r", ch->pcdata->points );
	page_to_char( buf_string( pBuf ), ch );
	free_buf( pBuf );
	return;
    }

    if ( !str_cmp( argument, "all" ) )	/* show all groups */
    {
        pBuf = new_buf( );
        for ( gn = 0; gn < MAX_GROUP; gn++ )
        {
            if ( group_table[gn].name == NULL )
                break;
	    buf_printf( pBuf,"%-20s ", group_table[gn].name );
	    if (++col % 3 == 0)
            	add_buf( pBuf, "\n\r" );
        }
        if ( col % 3 != 0 )
            add_buf( pBuf, "\n\r" );
	page_to_char( buf_string( pBuf ), ch );
	free_buf( pBuf );
	return;
    }

    /* show the sub-members of a group */
    gn = group_lookup( argument );
    if ( gn == -1 )
    {
	send_to_char( "No group of that name exists.\n\r", ch );
	send_to_char(
	    "Type 'groups all' or 'info all' for a full listing.\n\r", ch );
	return;
    }

    pBuf = new_buf( );
    for ( sn = 0; sn < MAX_IN_GROUP; sn++ )
    {
	if ( group_table[gn].spells[sn] == NULL )
	    break;
	if ( group_table[gn].spells[sn][0] == '\0' )
	    continue;
	buf_printf( pBuf, "%-20s ", group_table[gn].spells[sn] );
	if ( ++col % 3 == 0 )
	    add_buf( pBuf, "\n\r" );
    }
    if ( col % 3 != 0 )
        add_buf( pBuf, "\n\r" );
    page_to_char( buf_string( pBuf ), ch );
    free_buf( pBuf );
}


/* checks for skill improvement */
void
check_improve( CHAR_DATA *ch, int sn, bool success, int multiplier )
{
    int chance;
    char buf[100];

    if ( IS_NPC( ch ) )
	return;

    if ( ch->level < skill_table[sn].skill_level[ch->class]
    ||   skill_table[sn].rating[ch->class] == 0
    ||   ch->pcdata->skill[sn].percent == 0
    ||   ch->pcdata->skill[sn].percent == 100 )
	return;  /* skill is not known */ 

    /* record a usage, successful or unsuccessful */
    ch->pcdata->skill[sn].usage++;
    ch->pcdata->skill[sn].used_age = GET_AGE( ch );

    /* check to see if the character has a chance to learn */
    chance = 10 * int_app[get_curr_stat(ch,STAT_INT)].learn;
    chance /= (		multiplier
		*	skill_table[sn].rating[ch->class] 
		*	4);
    chance += ch->level;

    if ( number_range( 1, 1000 ) > chance )
	return;

    /* now that the character has a CHANCE to learn, see if they really have */	

    if ( success )
    {
	chance = URANGE( 5, 100 - ch->pcdata->skill[sn].percent, 95 );
	if ( number_percent( ) < chance )
	{
	    sprintf( buf, "You have become better at %s!\n\r",
		    skill_table[sn].name );
	    send_to_char( buf, ch );
	    ch->pcdata->skill[sn].percent++;
	    gain_exp( ch, 2 * skill_table[sn].rating[ch->class] );
	}
    }

    else
    {
	chance = URANGE( 5, ch->pcdata->skill[sn].percent / 2, 30 );
	if ( number_percent( ) < chance )
	{
	    sprintf( buf,
		"You learn from your mistakes, and your %s skill improves.\n\r",
		skill_table[sn].name );
	    send_to_char(buf,ch);
	    ch->pcdata->skill[sn].percent += number_range( 1, 3 );
	    ch->pcdata->skill[sn].percent = UMIN( ch->pcdata->skill[sn].percent, 100 );
	    gain_exp( ch, 2 * skill_table[sn].rating[ch->class] );
	}
    }
}

/* returns a group index number given the name */
int group_lookup( const char *name )
{
    int gn;

    for ( gn = 0; gn < MAX_GROUP; gn++ )
    {
        if ( group_table[gn].name == NULL )
            break;
        if ( LOWER(name[0]) == LOWER(group_table[gn].name[0])
        &&   !str_prefix( name, group_table[gn].name ) )
            return gn;
    }

    return -1;
}

/* returns a group index number given the name
   exact match required */
int group_find( const char *name )
{
    int gn;

    for ( gn = 0; gn < MAX_GROUP; gn++ )
    {
        if ( group_table[gn].name == NULL )
            break;
        if ( LOWER(name[0]) == LOWER(group_table[gn].name[0])
        &&   !str_cmp( name, group_table[gn].name ) )
            return gn;
    }

    return -1;
}

/* recursively adds a group given its number -- uses group_add */
void gn_add( CHAR_DATA *ch, int gn)
{
    int i;

    ch->pcdata->group_known[gn] = TRUE;
    for ( i = 0; i < MAX_IN_GROUP; i++)
    {
        if (group_table[gn].spells[i] == NULL)
            break;
        group_add(ch,group_table[gn].spells[i],FALSE);
    }
}

/* recusively removes a group given its number -- uses group_remove */
void gn_remove( CHAR_DATA *ch, int gn)
{
    int i;

    ch->pcdata->group_known[gn] = FALSE;

    for ( i = 0; i < MAX_IN_GROUP; i ++)
    {
	if (group_table[gn].spells[i] == NULL)
	    break;
	group_remove(ch,group_table[gn].spells[i]);
    }
}


/* use for processing a skill or group for addition  */
void
group_add( CHAR_DATA *ch, const char *name, bool deduct)
{
    int sn;
    int gn;

    if ( IS_NPC( ch ) ) /* NPCs do not have skills */
	return;

    if ( ch->pcdata == NULL || ch->pcdata->skill == NULL ) /* Safety check */
	return;

    if ( IS_NULLSTR( name ) ) /* Do nothing if no name given */
        return;

    sn = skill_lookup( name );

    if ( sn != -1 )
    {
	if ( ch->pcdata->skill[sn].percent == 0 ) /* i.e. not known */
	{
	    ch->pcdata->skill[sn].percent = 1;
	    ch->pcdata->skill[sn].learned_age = GET_AGE( ch );
	    if ( deduct )
	   	ch->pcdata->points += skill_table[sn].rating[ch->class]; 
	}
	return;
    }

    /* now check groups */

    gn = group_lookup( name );

    if ( gn != -1 )
    {
	if ( ch->pcdata->group_known[gn] == FALSE )  
	{
	    ch->pcdata->group_known[gn] = TRUE;
	    if ( deduct )
		ch->pcdata->points += group_table[gn].rating[ch->class];
	}
	
	/* Debug output */
	{
	    char buf[MAX_STRING_LENGTH];
	    sprintf( buf, "DEBUG: group_add calling gn_add for group %d (name=%s)\n\r", gn, name );
	    send_to_char( buf, ch );
	}
	
	gn_add( ch, gn ); /* make sure all skills in the group are known */
	
	send_to_char( "DEBUG: gn_add completed\n\r", ch );
    }
    else
    {
	char buf[MAX_STRING_LENGTH];
	sprintf( buf, "DEBUG: group_lookup failed for '%s'\n\r", name );
	send_to_char( buf, ch );
    }
}


/* used for processing a skill or group for deletion -- no points back! */
void
group_remove( CHAR_DATA *ch, const char *name )
{
    int sn;
    int gn;

    if ( IS_NPC( ch ) ) /* NPCs do not have skills */
	return;

    if ( ch->pcdata == NULL || ch->pcdata->skill == NULL ) /* Safety check */
	return;

    sn = skill_lookup( name );

    if ( sn != -1 )
    {
	ch->pcdata->skill[sn].percent = 0;
	ch->pcdata->skill[sn].learned_age = 0;
	return;
    }

    /* now check groups */

    gn = group_lookup( name );

    if ( gn != -1 && ch->pcdata->group_known[gn] == TRUE )
    {
	ch->pcdata->group_known[gn] = FALSE;
	gn_remove( ch, gn );  /* be sure to call gn_add on all remaining groups */
    }
}


int
get_prereq_ability( CHAR_DATA *ch, int sn )
{
    int ability;
    int index;
    int psn;

    if ( !IS_NPC( ch ) )
	return 100;

    ability = 100;

    for ( index = 0; index < MAX_PREREQ; index++ )
    {
	psn = skill_table[sn].prereq[index];
	if ( psn == 0 )
	    continue;
	if ( ch->level < skill_table[psn].skill_level[ch->class] )
	    return 0;
	ability = UMIN( ability, ch->pcdata->skill[psn].percent );
    }

    return ability;
}

void do_engage(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;

  /* Check for skill.  */
  if (   (get_skill(ch,gsn_engage) == 0 )
      || (    !IS_NPC(ch)
          &&  (ch->level < skill_table[gsn_engage].skill_level[ch->class])))
    {
      send_to_char("Engage?  You're not even dating!\n\r",ch);  /* Humor. :)  */
      return;
    }

  /* Must be fighting.  */
  if (ch->fighting == NULL)
    {
      send_to_char("You're not fighting anyone.\n\r",ch);
      return;
    }

  one_argument( argument, arg );

  /* Check for argument.  */
  if (arg[0] == '\0')
    {
      send_to_char("Engage who?\n\r",ch);
      return;
    }

  /* Check for victim.  */
  if ((victim = get_char_room(ch,arg)) == NULL)
    {
      send_to_char("Shadowbox some other time.\n\r",ch);
      return;
    }

  if (victim == ch)
    {
      send_to_char("Attacking yourself in combat isn't a smart thing.\n\r",ch);
      return;
    }

  if (ch->fighting == victim)
    {
      send_to_char("You're already pummelling that target as hard as you can!\n\r",ch);
      return;
    }

  /* Check for safe.  */
  if (is_safe(ch, victim))
    return;

  /* Check for charm.  */
  if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
      act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
      return;
    }

  /* This lets higher-level characters engage someone that isn't already fighting them.
     Quite powerful.  Raise level as needed.  */
  if ((victim->fighting != ch) && (ch->level < 20))
    {
      send_to_char("But they're not fighting you!\n\r",ch);
      return;
    }

  /* Get chance of success, and allow max 95%.  */
  chance = get_skill(ch,gsn_engage);
  chance = UMIN(chance,95);

  if (number_percent() < chance)
    {
      /* It worked!  */
      stop_fighting(ch,FALSE);

      set_fighting(ch,victim);
      if (victim->fighting == NULL)
        set_fighting(victim,ch);

      check_improve(ch,gsn_engage,TRUE,3);
      act("$n has turned $s attacks toward you!",ch,NULL,victim,TO_VICT);
      act("You turn your attacks toward $N.",ch,NULL,victim,TO_CHAR);
      act("$n has turned $s attacks toward $N!",ch,NULL,victim,TO_NOTVICT);
    }
  else
    {
      /* It failed!  */
      send_to_char("You couldn't get your attack in.\n\r",ch);
      check_improve(ch,gsn_engage,FALSE,3);
    }
}