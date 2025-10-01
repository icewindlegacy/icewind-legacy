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

/* SAMPLE SONG! This song casts armor and bless on the player, and theres
   an 80% chance for each spell for each person in the room that they will
   receive the effect also
*/

void song_of_huma( int songnum, int level, CHAR_DATA *ch,void *vo, int target )
{
    CHAR_DATA *vch;
    AFFECT_DATA af1, af2;

    af1.where = TO_AFFECTS;
    af1.type = skill_lookup("bless");
    af1.level = level;
    af1.duration = level/2;
    af1.location = APPLY_HITROLL;
    af1.modifier = level/8;
    af1.bitvector = 0;

    af2.where = TO_AFFECTS;
    af2.type = skill_lookup("armor");
    af2.level = level;
    af2.duration = level/2;
    af2.location = APPLY_AC;
    af2.modifier = -20;
    af2.bitvector = 0;

    if (!is_affected( ch, af1.type ))
    {
	act("$n glows briefly.",ch,NULL,NULL,TO_ROOM);
	send_to_char("You glow briefly.\n\r", ch);
	affect_to_char(ch,&af1);
    }

    if (!is_affected( ch, af2.type ))
    {
	act("$n is suddenly surrounded by a glowing suit of armor.",ch,NULL,NULL,TO_ROOM);
	send_to_char("You are suddenly surrounded by a glowing suit of armor.\n\r", ch);
	affect_to_char(ch,&af2);
    }

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
	if (ch == vch)
	    continue;

	if (!IS_NPC(vch))
	{
	    if (number_percent() <= 80 && !is_affected( vch, af1.type ))
	    {
		act("$N glows briefly.",ch,NULL,vch,TO_ROOM);
		act("$N glows briefly.",ch,NULL,vch,TO_CHAR);
		send_to_char("You glow briefly.\n\r", vch);
		affect_to_char( vch, &af1 );
	    }
	    if (number_percent() <= 80 && !is_affected( vch, af2.type ))
	    {
		act("$N is suddenly surrounded by a glowing suit of armor.",ch,NULL,vch,TO_ROOM);
		act("$N is suddenly surrounded by a glowing suit of armor.",ch,NULL,vch,TO_CHAR);
		send_to_char("You are suddenly surrounded by a glowing suit of armor.\n\r", vch);
		affect_to_char( vch, &af2 );
	    }
	}
    }

    return;
}

void song_of_serenity( int songnum, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *vch;
    int mlevel = 0;
    int count = 0;
    int high_level = 0;
    int chance;
    AFFECT_DATA af;

    /* get sum of all mobile levels in the room */
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
	if (vch->position == POS_FIGHTING)
	{
	    count++;
	    if (IS_NPC(vch))
	      mlevel += vch->level;
	    else
	      mlevel += vch->level/2;
	    high_level = UMAX(high_level,vch->level);
	}
    }

    /* compute chance of stopping combat */
    chance = 4 * level - high_level + 2 * count;

    if (IS_IMMORTAL(ch)) /* always works */
      mlevel = 0;

    if (number_range(0, chance) >= mlevel)  /* hard to stop large fights */
    {
	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
   	{
	    if (IS_NPC(vch) && (IS_SET(vch->imm_flags,IMM_MAGIC) ||
				IS_SET(vch->act,ACT_UNDEAD)))
	      return;

	    if (IS_AFFECTED(vch,AFF_CALM) || IS_AFFECTED(vch,AFF_BERSERK)
	    ||  is_affected(vch,skill_lookup("frenzy")))
	      return;

	    send_to_char("A wave of calm passes over you.\n\r",vch);

	    if (vch->fighting || vch->position == POS_FIGHTING)
	      stop_fighting(vch,FALSE);


	    af.where = TO_AFFECTS;
	    af.type = skill_lookup("calm");
  	    af.level = level;
	    af.duration = level/4;
	    af.location = APPLY_HITROLL;
	    if (!IS_NPC(vch))
	      af.modifier = -5;
	    else
	      af.modifier = -2;
	    af.bitvector = AFF_CALM;
	    affect_to_char(vch,&af);

	    af.location = APPLY_DAMROLL;
	    affect_to_char(vch,&af);
	}
    }
}

void song_of_burning(int songnum,int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] =
    {
	 0,
	 0,  0,  0,  0,	14,	17, 20, 23, 26, 29,
	29, 29, 30, 30,	31,	31, 32, 32, 33, 33,
	34, 34, 35, 35,	36,	36, 37, 37, 38, 38,
	39, 39, 40, 40,	41,	41, 42, 42, 43, 43,
	44, 44, 45, 45,	46,	46, 47, 47, 48, 48
    };
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim,DAM_FIRE) )
	dam /= 2;
    damage( ch, victim, dam, songnum, DAM_FIRE,TRUE);
    return;
}

void song_of_readiness( int songnum, int level, CHAR_DATA *ch,void *vo, int target )
{
    CHAR_DATA *vch;
    AFFECT_DATA af1, af2;

    af1.where = TO_AFFECTS;
    af1.type = skill_lookup("sanctuary");
    af1.level = level;
    af1.duration = level/2;
    af1.location = APPLY_HITROLL;
    af1.modifier = level/8;
    af1.bitvector = 0;

    af2.where = TO_AFFECTS;
    af2.type = skill_lookup("haste");
    af2.level = level;
    af2.duration = level/2;
    af2.location = APPLY_AC;
    af2.modifier = -20;
    af2.bitvector = 0;

    if (!is_affected( ch, af1.type ))
    {
	act("$n glows briefly.",ch,NULL,NULL,TO_ROOM);
	send_to_char("You glow briefly.\n\r", ch);
	affect_to_char(ch,&af1);
    }

    if (!is_affected( ch, af2.type ))
    {
	act("$n is moving faster and their aura glows white.",ch,NULL,NULL,TO_ROOM);
	send_to_char("You begin moving faster and your aura glows white.\n\r", ch);
	affect_to_char(ch,&af2);
    }

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
	if (ch == vch)
	    continue;

	if (!IS_NPC(vch))
	{
	    if (number_percent() <= 80 && !is_affected( vch, af1.type ))
	    {
		act("$N glows briefly.",ch,NULL,vch,TO_ROOM);
		act("$N glows briefly.",ch,NULL,vch,TO_CHAR);
		send_to_char("You glow briefly.\n\r", vch);
		affect_to_char( vch, &af1 );
	    }
	    if (number_percent() <= 80 && !is_affected( vch, af2.type ))
	    {
		act("$N is moving faster and their aura glows white.",ch,NULL,vch,TO_ROOM);
		act("$N is moving faster and their aura glows white.",ch,NULL,vch,TO_CHAR);
		send_to_char("You are suddenly surrounded by a glowing suit of armor.\n\r", vch);
		affect_to_char( vch, &af2 );
	    }
	}
    }

    return;
}
