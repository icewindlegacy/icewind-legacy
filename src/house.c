/***************************************************************************
 *  Icewind Legacy - House System
 *  Based on code from ROM 2.4 by Russ Taylor
 *  Original house code by Dalsor of AWW (As the Wheel Weaves)
 ***************************************************************************/

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "recycle.h"
#include "olc.h"
#include "interp.h"

/* Forward declarations for OLC functions */
RESET_DATA *add_new_reset( ROOM_INDEX_DATA *pRoom, char letter, int arg1, int arg2, int arg3, int arg4 );

/* Forward declarations */
void do_asave args( ( CHAR_DATA *ch, char *argument ) );


void save_house args( ( ) );
void load_house args( ( ) );
void home_sell ( CHAR_DATA *ch, int cost );
CHAR_DATA *  find_architect     args( ( CHAR_DATA * ch ) );
HOUSE_DATA *house_list;
HOUSE_DATA *new_house(void);
void free_house(HOUSE_DATA *hOuse);

#define MAX_HORESET	10
#define MAX_HMRESETS	10

#define VNUM_START 1200
#define VNUM_STOP  1400

CHAR_DATA * find_architect ( CHAR_DATA *ch )
{
    CHAR_DATA * architect;

    for ( architect = ch->in_room->people; architect != NULL; architect = architect->next_in_room )
    {
        if (!IS_NPC(architect))
            continue;

        if (architect->spec_fun == spec_lookup( "spec_architect" ) )
            return architect;
    }  

    if ( architect == NULL || architect->spec_fun != spec_lookup( "spec_architect" ))
    {
        send_to_char("You can't do that here, find an Architect.`X\n\r", ch);
        return NULL;
    }

    if ( architect->fighting != NULL )
    {
        send_to_char("Wait until the fighting stops.`X\n\r",ch );
        return NULL;
    }

    return NULL;
}        

void do_objbuy(CHAR_DATA *ch, char *argument)
{
    char arg[MSL];
    char arg1[MSL];
    char buf[MSL];
    OBJ_DATA *obj;
    HOUSE_DATA *hOuse;
    int i;
 
    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg1 );

    if ( !HAS_HOME( ch ) )
    {
        send_to_char("If you only had a home...`X\n\r",ch);
        return;
    }

    if ( !IS_HOME( ch ) )
    {
	send_to_char("You gotta be in your house.`X\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	send_to_char("Syntax: objbuy <list>`X\n\rSyntax: objbuy buy <name>`X\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"list"))
    {
	send_to_char("The following objects are available for purchase:`X\n\r",ch);
        send_to_char("`GName:             Cost:`X\n\r", ch );
        send_to_char("`P========================`X\n\r", ch );
        for ( i = 0; house_table[i].name != NULL; i++ )
        {
             if ( house_table[i].type != OBJ_VNUM )
                 continue;
             printf_to_char( ch, "%-10s        %-5d`X\n\r", house_table[i].name, house_table[i].cost );
        }

        return;
    }

    if (!str_cmp(arg, "buy"))
    {
        if (arg1[0] == '\0')
        {
            send_to_char( "To buy an item type objbuy buy <item>.`X\n\r",ch );
            return;
        }
         
        for (i = 0; house_table[i].name != NULL; i++)           
        {
             if (is_name(arg1, house_table[i].name) && house_table[i].type == OBJ_VNUM)         
             {
                 if (ch->money.gold >= house_table[i].cost) 
                 {
                     if( ch->pcdata->horesets > MAX_HORESET )
                     {
  	                send_to_char("Sorry you can only have 10 resetting objects.`X\n\r",ch);
	                return;
                     }

                     ch->pcdata->horesets++;
                     ch->money.gold -= house_table[i].cost;  
          
                     if ( (obj = create_object( get_obj_index( house_table[i].vnum ), ch->level ) )  == NULL ) 
                     {
                         send_to_char( "That object could not be found contact an imm.`X\n\r", ch );
                         return;
                     }
                     obj_to_room( obj, ch->in_room );

                     /* Add reset for the object */
                     add_new_reset( ch->in_room, 'O', house_table[i].vnum, -1, ch->in_room->vnum, 0 );
                     SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );

                     // Create a new entry for each object purchased
                     hOuse = new_house();
                     hOuse->oname = str_dup( ch->name );
                     hOuse->objname = str_dup( obj->name );
                     hOuse->ovalue = house_table[i].cost;
                     hOuse->mvalue = 0; // No mob value for object entries
                     hOuse->next = house_list;
                     house_list = hOuse;

                     ch->pcdata->hchange = TRUE;
                     save_house();
                     do_asave(ch, "changed");

          	     return;
                 }

                 else
                 {
                    printf_to_char( ch, "Sorry %s but you need %d gold for that.`X\n\r", ch->name, house_table[i].cost );
                    return;
                 }
             }
        }
    }

    send_to_char( "To buy an item type objbuy buy <item>.`X\n\r",ch );
    return;
}

void do_mobbuy(CHAR_DATA *ch, char *argument)
{
    char arg[MSL];
    char arg1[MSL];
    CHAR_DATA *mob;
    char buf[MSL];  
    HOUSE_DATA *hOuse; 
    int i;
 
    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg1 );

    if ( !HAS_HOME( ch ) )
    {
        send_to_char("If you only had a home...`X\n\r",ch);
        return;
    }

    if ( !IS_HOME( ch ) )
    {
	send_to_char("You gotta be in your house.`X\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
        send_to_char("Syntax: mobbuy <list>`X\n\rSyntax: mobbuy buy <name>`X\n\r",ch); 
	return;
    }

    if (!str_cmp(arg,"list"))
    {
	send_to_char("The following mobiles are available for purchase:`X\n\r",ch);
        send_to_char("`GName:             Cost:`X\n\r", ch );
        send_to_char("`P========================`X\n\r", ch );
        for ( i = 0; house_table[i].name != NULL; i++ )
        {
             if ( house_table[i].type != MOB_VNUM )
                 continue;
             printf_to_char( ch, "%-10s        %-5d`X\n\r", house_table[i].name, house_table[i].cost );
        }

        return;

    }

    if (!str_cmp(arg, "buy"))
    {
        if (arg1[0] == '\0')
        {
            send_to_char( "To buy an item type mobbuy buy <item>.`X\n\r",ch );
            return;
        }           

        for (i = 0; house_table[i].name != NULL; i++)
        {        
             if (is_name(arg1, house_table[i].name) && house_table[i].type == MOB_VNUM)
             {
                 if (ch->money.gold >= house_table[i].cost)
                 {                  
                     mob = create_mobile(get_mob_index(house_table[i].vnum) );
                 
                     if ( mob == NULL ) 
                     {
                         send_to_char( "That mob could not be found contact an imm.`X\n\r", ch );
                         return;
                     }            

                     if( ch->pcdata->hmresets > MAX_HMRESETS )
                     {      
                       	send_to_char("Sorry you can only have 5 resetting mobiles.`X\n\r",ch);
	                return;
                     }

                     ch->pcdata->hmresets++;
                     ch->money.gold -= house_table[i].cost;

                     mob = create_mobile( get_mob_index( house_table[i].vnum ) );
                     char_to_room( mob, ch->in_room );

                     /* Add reset for the mobile */
                     add_new_reset( ch->in_room, 'M', house_table[i].vnum, -1, ch->in_room->vnum, 1 );
                     SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );
          
                     // Create a new entry for each mobile purchased
                     hOuse = new_house();
                     hOuse->oname = str_dup( ch->name );
                     hOuse->mobname = str_dup( mob->name );
                     hOuse->mvalue = house_table[i].cost;
                     hOuse->ovalue = 0; // No object value for mobile entries
                     hOuse->next = house_list;
                     house_list = hOuse;

                     ch->pcdata->hchange = TRUE;
                     save_house();
                     do_asave(ch, "changed");

                     return;                            
                 }

                 else
                 {

                   printf_to_char( ch, "Sorry %s but you need %d gold for a %s.`X\n\r", ch->name, house_table[i].cost, house_table[i].name );                       
                   return;
                 }
             }
        }
    }
       
    send_to_char("Syntax: mobbuy <list>`X\n\rSyntax: mobbuy buy <name>`X\n\r",ch); 
    return; 
}

void do_house( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *architect;
    char arg[MIL];
    int osec;
    char buf[MSL];
    int i,i2;
    int cost = 100000;
    HOUSE_DATA *hOuse;

    architect = find_architect( ch );

    if (!architect)
        return;
 
    argument = one_argument( argument, arg );

    if (IS_NPC(ch))
    {
	send_to_char("Sorry, you'll have to just wander, mobile.`X\n\r",ch);
	return;
    }

    if ( arg[0] == '\0' && HAS_HOME( ch ) )
    {
        send_to_char( "You already bought a house. To sell it type house <sell>.`X\n\r", ch );
        send_to_char( "You can also value your house. Type house <value>`X\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "sell" ) && HAS_HOME( ch ) )
    {
        home_sell( ch, cost );
        return;
    }

    if ( !str_cmp( arg, "value" ) && HAS_HOME( ch ) )
    {
        int value = 0, objects = 0, mobs = 0;
       
        for ( hOuse = house_list; hOuse != NULL; hOuse = hOuse->next )
        {                                                     
             if ( !str_cmp( ch->name, hOuse->oname ) ) 
             {
                 value += ( hOuse->ovalue + hOuse->mvalue );
                 if ( hOuse->ovalue > 0 ) objects++;
                 if ( hOuse->mvalue > 0 ) mobs++;
             }
        }

        printf_to_char( ch, "You have %d objects and %d mobs in your house.`X\n\r", objects, mobs );
        printf_to_char( ch, "Your house is valued at a total of %d gold.`X\n\r", ( cost / 2 ) + value );
        return;        
    }

    if ( HAS_HOME( ch ) )
    {
        send_to_char( "You already own a house. To sell it type house <sell>.`X\n\r", ch );
        return;
    }

    if (ch->money.gold < cost)
    {
        printf_to_char( ch, "I'm sorry but it cost %d gold to buy a house.`X\n\r", cost );
	return;
    }

    i = VNUM_START;

    while (get_room_index(i) != NULL)
    {
	i++;

	if (i > VNUM_STOP - 5)
	{
            send_to_char("Sorry all of the house vnums are used up! Tell an immortal.`X\n\r", ch );
	    return;
	}
    }

    send_to_char("Okay... Attempting creation of your home.`X\n\r\n\r",ch);
    ch->money.gold -= cost;
    osec = ch->pcdata->security;  
    ch->pcdata->security = 5;

    // Create the central room first at the next available vnum
    sprintf(buf,"%d",i);
    if (!redit_create(ch, buf))
    {
        send_to_char("Unable to create central room. Contact an Immortal.`X\n\r",ch);
        ch->pcdata->security = osec;
        return;
    }
    
    // Move to the newly created room
    char_from_room(ch);
    char_to_room(ch, get_room_index(i));
    
    // Set the room name
    sprintf(buf,"%s's Home",ch->name);
    redit_name(ch,buf);
    
    // Create the 4 connected rooms using dig
    // Room layout:
    //      3
    //      |
    //   2--1--4
    //      |
    //      5
    
    // Dig west (room 2)
    sprintf(buf,"dig %d",i+1);
    if (!redit_west(ch, buf))
    {
        send_to_char("Unable to create west room. Contact an Immortal.`X\n\r",ch);
        ch->pcdata->security = osec;
        return;
    }
    
    // Dig north (room 3) 
    sprintf(buf,"dig %d",i+2);
    if (!redit_north(ch, buf))
    {
        send_to_char("Unable to create north room. Contact an Immortal.`X\n\r",ch);
        ch->pcdata->security = osec;
        return;
    }
    
    // Dig east (room 4)
    sprintf(buf,"dig %d",i+3);
    if (!redit_east(ch, buf))
    {
        send_to_char("Unable to create east room. Contact an Immortal.`X\n\r",ch);
        ch->pcdata->security = osec;
        return;
    }
    
    // Dig up (room 5)
    sprintf(buf,"dig %d",i+4);
    if (!redit_up(ch, buf))
    {
        send_to_char("Unable to create up room. Contact an Immortal.`X\n\r",ch);
        ch->pcdata->security = osec;
        return;
    }
    if ( osec > 1 )
        ch->pcdata->security = osec;
    send_to_char("\n\rHURRAY! Your house was made successfully.`X\n\r",ch);
    ch->pcdata->h_vnum = i;
    // TODO: Save area
    do_asave(ch, "changed");
    // TODO: Edit done
    return;
}

void do_hname( CHAR_DATA *ch, char *argument )
{
    int cost = 500;

    if ( !HAS_HOME( ch ) )
    {
        send_to_char("If you only had a home...`X\n\r",ch);
        return;
    }

    if ( !IS_HOME( ch ) )
    {
        send_to_char( "You've got to be in your house to do that.`X\n\r", ch );
	return;

    }

    if (ch->money.gold < cost)
    {
        printf_to_char( ch, "It cost %d to change the name of a room in your house.`X\n\r", cost );
	return;
    }

    if (argument[0] == '\0')
    {
	send_to_char("Change the name to what?`X\n\r",ch);
	return;
    }

    ch->money.gold -= cost;
    
    // Update the room name directly
    free_string( ch->in_room->name );
    ch->in_room->name = str_dup( argument );
    send_to_char("Room name changed.`X\n\r", ch);
    SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );
    do_asave(ch, "changed");
    return;
}

void do_hdesc( CHAR_DATA *ch, char *argument )
{
    int cost = 500;

    if ( !HAS_HOME( ch ) )
    {
        send_to_char("If you only had a home...`X\n\r",ch);
        return;
    }

    if ( !IS_HOME( ch ) )
    {
	send_to_char("You gotta be in your house.`X\n\r",ch);
	return;
    }

    if (ch->money.gold < cost)
    {
        printf_to_char( ch, "It cost %d gold to change your description.`X\n\r", cost );
	return;
    }

    ch->money.gold -= cost;
    send_to_char("Ok.`X\n\r",ch);
    ch->pcdata->hchange	= TRUE;
    string_append(ch, &ch->in_room->description);
    do_asave(ch, "changed");
    return;
}

void do_home( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *location;

    if (IS_NPC(ch) && IS_SET(ch->act,ACT_PET))
    {
        send_to_char("Only players can go home.`X\n\r",ch);
        return;
    }

    if ( !HAS_HOME( ch ) )
    {
	send_to_char("If you only had a home...`X\n\r",ch);
	return;
    }

    if(IS_NPC(ch))
	location = get_room_index(ch->master->pcdata->h_vnum);
    else
	location = get_room_index(ch->pcdata->h_vnum);

    if (location == NULL)
    {
        send_to_char( "Yikes write a note to immortal and let them know your house is Null.`X\n\r", ch );
	return;
    }

    act( "$n prays for transportation!", ch, 0, 0, TO_ROOM );

    if ( ch->in_room == location )
        return;
   
    if ( IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_AFFECTED(ch, AFF_CURSE))
    {
        send_to_char( "ShadowStorm has forsaken you.`X\n\r", ch );
	return;
    }

    if ( ( victim = ch->fighting ) != NULL )
    {
	int lose,skill;

	skill = get_skill(ch,skill_lookup("recall"));

	if ( number_percent() < 80 * skill / 100 )
	{
	    check_improve(ch,skill_lookup("recall"),FALSE,6);
	    WAIT_STATE( ch, 4 );
	    sprintf( buf, "You failed!.`X\n\r");
	    send_to_char( buf, ch );
	    return;
	}

	lose = (ch->desc != NULL) ? 25 : 50;
	gain_exp( ch, 0 - lose );
	check_improve(ch,skill_lookup("recall"),TRUE,4);
	sprintf( buf, "You recall from combat!  You lose %d exps.`X\n\r", lose );
	send_to_char( buf, ch );
	stop_fighting( ch, TRUE );
    }

    act( "$n disappears.", ch, NULL, NULL, TO_ROOM );

    char_from_room( ch );
    char_to_room( ch, location );
    act( "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );

    if (ch->pet != NULL)
        do_home(ch->pet,"");
 
    if (ch->mount != NULL)
    {
        char_from_room( ch->mount );
        char_to_room( ch->mount, ch->in_room );
    }   

    return;
}

void do_invite(CHAR_DATA *ch,char *argument)
{
    CHAR_DATA *victim;
    char buf[MSL];

    if (argument[0] == '\0')
    {
	send_to_char("Invite whom to your home?`X\n\r",ch);
	return;
    }

    if ((victim = get_char_world(ch,argument)) == NULL)
    {
	send_to_char("Who is that?`X\n\r",ch);
	return;
    }

    if ( !IS_HOME( ch ) )
    {
        send_to_char("Can't invite someone to your house when you aren't in it.`X\n\r",ch);
	return;
    }

    if (ch == victim)
    {
	send_to_char("Lonely person, huh?`X\n\r",ch);
	return;
    }
 
    sprintf(buf,"%s has invited you to come to their house!`X\n\r",ch->name);
    send_to_char(buf,victim);

    send_to_char("You have invited them to come in.`X\n\r",ch);
    victim->pcdata->hinvite = ch->pcdata->h_vnum;
}

void do_house_join(CHAR_DATA *ch,char *argument)
{
    int ohvnum = 0;

    if (ch->pcdata->hinvite == 0)
    {
	send_to_char("You haven't been invited anywhere.`X\n\r",ch);
	return;
    }

    ohvnum = ch->pcdata->h_vnum;
    ch->pcdata->h_vnum = ch->pcdata->hinvite;
    do_home(ch,"");
    ch->pcdata->h_vnum = ohvnum;
    ch->pcdata->hinvite = 0;
}

void home_sell( CHAR_DATA *ch, int cost )
{
    char buf[MSL];
    int i, t = 0;
    HOUSE_DATA *hOuse;
    int value = 0;

    send_to_char( "Ok. Attempting to sell your home.`X\n\r\n\r", ch );
    
    // Calculate value of furniture and mobs
    for ( hOuse = house_list; hOuse != NULL; hOuse = hOuse->next )
    {                                                     
         if ( !str_cmp( ch->name, hOuse->oname ) ) 
         {
             value += ( hOuse->ovalue + hOuse->mvalue );
             t++;
         }
    }

    // Delete rooms
    for ( i = ch->pcdata->h_vnum; i < ch->pcdata->h_vnum + 5; i++ )
    {
         ROOM_INDEX_DATA *pRoom = get_room_index(i);
         if (pRoom != NULL)
         {
             delete_room(pRoom);
             send_to_char("Room deleted.`X\n\r", ch);
         }
    }            

    if ( ch->pcdata->security <= 5 )
        ch->pcdata->security = 0;
    
    // Give player half the base cost plus value of furniture/mobs
    int total_gold = ( cost / 2 ) + value;
    ch->money.gold += total_gold;
    ch->pcdata->h_vnum = 0;
    
    send_to_char( "\n\rYou have successfully sold your house.`X\n\r", ch );
    printf_to_char( ch, "You receive %d gold for selling it.`X\n\r", total_gold );
    printf_to_char( ch, "You had a total of %d mobs and objects in your house.`X\n\r", t );
    
    save_house();
    return;
}

void do_boot( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( argument[0] == '\0' )
    {     
        send_to_char( "Who would you like to boot out of your house?`X\n\r", ch );
        return;
    }

    if ((victim = get_char_world(ch,argument)) == NULL)
    {
        send_to_char("Who is that?`X\n\r",ch);
        return;
    }                  
    
    if ( !IS_HOME( ch ) )
    {
        send_to_char("You can't boot someone if your not in your house!`X\n\r", ch );
        return;
    }            

    if ((victim->in_room->vnum < ch->pcdata->h_vnum) || (victim->in_room->vnum > ch->pcdata->h_vnum + 4))
    {
        send_to_char("You can't boot someone if their not in your house!`X\n\r",ch);
        return;
    }       

    printf_to_char( ch, "You get in %s's face and yell at %s to get out of your house!`X\n\r", victim->name, victim->sex == 0 ? "It" : victim->sex == 1 ? "Him" : "Her" );
    printf_to_char( victim, "%s begins yelling at you, and tells you to get out of %s house!`X\n\r", ch->name, ch->sex == 0 ? "It's" : ch->sex == 1 ? "His" : "Her" );  
    printf_to_char( victim, "You have been kicked out of %s's house!`X\n\r", ch->name );  
    do_recall( victim, "" );
    return;
}

void save_house()
{
  FILE *fp;
  HOUSE_DATA *hOuse;

  if ( ( fp = file_open( HOUSE_FILE, "w" ) ) == NULL )
  {
      logf2("The house file is gone!`X\n\r");
      return;
  }              

  for ( hOuse = house_list; hOuse != NULL; hOuse = hOuse->next )
  {
       fprintf(fp,"Oname %s~\n", hOuse->oname );
       fprintf(fp,"Objname %s~\n", hOuse->objname );
       fprintf(fp,"Mobname %s~\n", hOuse->mobname );
       fprintf(fp,"Ovalue %d\n", hOuse->ovalue );
       fprintf(fp,"Mvalue %d\n\n", hOuse->mvalue );
 }

 fprintf(fp, "$\n");
 logf2("House saved.");
 file_close(fp);
 return;
}


void load_house()
{
  FILE *fp;
  char *word;
  HOUSE_DATA *hlist;
  HOUSE_DATA *hOuse;

  if (file_exists(HOUSE_FILE))
  {
      fp = file_open( HOUSE_FILE, "r" );

      hlist = NULL;

      for( ; ; )
      {                      
          word = feof( fp ) ? "End" : fread_word( fp );

          if(word[0] == '$' )
             return;

          if (!str_cmp(word, "Oname" ) )
          {
              hOuse = new_house();
              if (house_list == NULL)
                  house_list = hOuse;
              else
                  hlist->next = hOuse;
              hlist = hOuse;
              hOuse->oname = str_dup(fread_string(fp));
          }                                 
         
          if (!str_cmp(word, "Objname" ) )
              hOuse->objname = str_dup(fread_string(fp));
            
          if (!str_cmp(word, "Mobname" ) )
              hOuse->mobname = str_dup(fread_string(fp));

          if (!str_cmp(word, "Ovalue" ) )
              hOuse->ovalue = fread_number(fp);
          
          if (!str_cmp(word, "Mvalue" ) )
              hOuse->mvalue = fread_number(fp);
      }
  }

  return;

}

