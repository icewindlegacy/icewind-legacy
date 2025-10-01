#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "songs.h"

/*
 * The kludgy global is for spells who want more stuff from command line.
 * It should be of type "const char *" too, but that'll involve a *lot* of work.
 */
char *target_name;

/* This function lists songs and their levels...the {Y and {G stuff is for color
 * remove it if you dont have color
 */
void do_songs(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int i;

    sprintf(buf, "`P[`G%-18s %5s`P]`X\n\r", "Song Name", "Level");
    send_to_char(buf,ch);

    for (i = 0; i < MAX_SONGS; i++)
    {
	sprintf(buf, "`P[`G%-20s `B%3d`P]`X\n\r", song_table[i].listname, song_table[i].level);
	send_to_char(buf,ch);
    }
    return;
}

int
song_lookup( CHAR_DATA *ch, const char *name )
{
    /* finds a spell the character can cast if possible */
    int songnum, found = -1;

    for ( songnum = 0; songnum < top_skill; songnum++ )
    {
	if ( song_table[songnum].name == NULL )
	    break;
	if ( LOWER( name[0]) == LOWER( song_table[songnum].name[0] )
	&&  !str_prefix( name, song_table[songnum].name ) )
	{
	    if ( found == -1)
		found = songnum;
	  if ( LOWER(name[0]) == LOWER(song_table[songnum].name[0])
	&&   !str_cmp( name, song_table[songnum].name ) )
	    return songnum;
	}
    }
    return found;
}

/* looks up a song */
/*
int song_lookup( const char *name )
{
    int songnum;

    for ( songnum = 0; songnum < top_skill; songnum++ )
    {
	if ( song_table[songnum].name == NULL )
	    break;
	if ( LOWER(name[0]) == LOWER(song_table[songnum].name[0])
	&&   !str_cmp( name, song_table[songnum].name ) )
	    return songnum;
    }

    return -1;
}
*/

/* actually starts the playing process */
void do_play( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *instrument;
    int songnum, chance, mana, level;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    void *vo;
    int target;

    target_name = one_argument( argument, arg1 );
    one_argument( target_name, arg2 );

    

    if ((chance = get_skill(ch,gsn_music)) == 0)
    {
	send_to_char("You have no clue how to play music.\n\r", ch);
	return;
    }

    if ((instrument = get_eq_char(ch, WEAR_HOLD)) == NULL)
    {
	send_to_char("You aren't carrying an instrument.\n\r", ch);
	return;
    }

    if (instrument->item_type != ITEM_INSTRUMENT)
    {
	send_to_char("You aren't carrying an instrument.\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	send_to_char("Play what?\n\r", ch);
	return;
    }

    songnum = song_lookup(ch, arg1);

    if (songnum == -1)
    {
	send_to_char("That isn't a song.\n\r", ch);
	return;
    }

    if (ch->level < song_table[songnum].level)
    {
	send_to_char("You haven't been able to master that song yet.\n\r", ch);
	return;
    }

    /*
	Below:  This basically lets there be songs that you can play while resting
	and songs that must be standing to play.  This way you cant play a damaging
	song to start a fight while sitting/resting
    */

    if (ch->position < song_table[songnum].minimum_position)
    {
	send_to_char("You need to be standing up to play that song.\n\r", ch);
	return;
    }

    /*
	Below: This can be changed to an algorith to allow for varied mana
	based on a comparison to level of spell vs players level, etc
    */
/*
     * Locate targets.
     */
    victim	= NULL;
    obj		= NULL;
    vo		= NULL;
    target	= TARGET_NONE;

    switch ( song_table[songnum].target )
    {
    default:
	bug( "Do_play: bad target for songnum %d.", songnum );
	return;

    case TAR_IGNORE:
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( arg2[0] == '\0' )
	{
	    if ( ( victim = ch->fighting ) == NULL )
	    {
		send_to_char( "Play a song at whom?\n\r", ch );
		return;
	    }
	}
	else
	{
	    if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
	    {
		send_to_char( "They aren't here.\n\r", ch );
		return;
	    }
	}

	if ( !IS_NPC(ch) )
	{

            if (is_safe(ch,victim) && victim != ch)
	    {
		send_to_char("Not on that target.\n\r",ch);
		return;
	    }
	check_killer(ch,victim);
	}

        if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
	{
	    send_to_char( "You can't do that on your own follower.\n\r",
		ch );
	    return;
	}

	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_DEFENSIVE:
	if ( arg2[0] == '\0' )
	{
	    victim = ch;
	}
	else
	{
	    if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
	    {
		send_to_char( "They aren't here.\n\r", ch );
		return;
	    }
	}

	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_SELF:
	if ( arg2[0] != '\0' && !is_name( target_name, ch->name ) )
	{
	    send_to_char( "You cannot play this songl on another.\n\r", ch );
	    return;
	}

	vo = (void *) ch;
	target = TARGET_CHAR;
	break;

    case TAR_OBJ_INV:
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "What should the song be played upon?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_here( ch, target_name ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}

	vo = (void *) obj;
	target = TARGET_OBJ;
	break;

    case TAR_OBJ_CHAR_OFF:
	if (arg2[0] == '\0')
	{
	    if ( ( victim = ch->fighting ) == NULL )
	    {
		send_to_char("Play the song at whom or what?\n\r",ch);
		return;
	    }

	    target = TARGET_CHAR;
	}
	else if ((victim = get_char_room(ch,target_name)) != NULL)
	{
	    target = TARGET_CHAR;
	}

	if (target == TARGET_CHAR) /* check the sanity of the attack */
	{
	    if(is_safe_spell(ch,victim,FALSE) && victim != ch)
	    {
		send_to_char("Not on that target.\n\r",ch);
		return;
	    }

            if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
            {
                send_to_char( "You can't do that on your own follower.\n\r",
                    ch );
                return;
            }

	    if (!IS_NPC(ch))
		check_killer(ch,victim);

	    vo = (void *) victim;
 	}
	else if ((obj = get_obj_here(ch,target_name)) != NULL)
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	else
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
	break;

    case TAR_OBJ_CHAR_DEF:
        if (arg2[0] == '\0')
        {
            vo = (void *) ch;
            target = TARGET_CHAR;
        }
        else if ((victim = get_char_room(ch,target_name)) != NULL)
        {
            vo = (void *) victim;
            target = TARGET_CHAR;
	}
	else if ((obj = get_obj_carry(ch,target_name,ch)) != NULL)
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	else
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
	break;
    }

    mana = song_table[songnum].min_mana;

    if (!IS_NPC(ch) && ch->mana < mana)
    {
	send_to_char("You don't have enough mana.\n\r", ch);
	return;
    }

    act("$n plays a melody on $s $p.",ch,instrument,NULL,TO_ROOM);
    act("You play a melody on your $p.",ch,instrument,NULL,TO_CHAR);
    if ( !IS_IMMORTAL( ch ) )
    WAIT_STATE( ch, song_table[songnum].beats );

    chance = chance - (20 / (1 + ch->level - song_table[songnum].level));

	/* average of level of player and level of instrument */
    level = (ch->level + instrument->level) / 2;
    

    if (number_percent() > chance)
    {
	ch->mana -= mana / 2;
	act("$n's fingers slip and the song ends abruptly.",ch,NULL,NULL,TO_ROOM);
	send_to_char("Your fingers slip and the song ends abruptly.\n\r", ch);
	check_improve(ch,gsn_music,FALSE,1);
	return;
    }
    else /* actually start playing the song */
    {
	ch->mana -= mana;
	if (IS_NPC(ch))
	    (*song_table[songnum].song_fun) ( songnum, level, ch, vo, target );
	else
	    (*song_table[songnum].song_fun) ( songnum, 3 * level/4, ch, vo, target );
	check_improve(ch,gsn_music,TRUE,1);
    }

    return;
}
