/*
 * Advanced Weather System for MARS MUD
 * 
 * This system replaces the binary PLR_FREEZING and PLR_WET flags with
 * degrees-based conditions (COND_FREEZING and COND_WET) for more realistic
 * weather effects including:
 * 
 * - Progressive temperature effects starting at 45°F
 * - Material-based insulation (fur/wool vs metal/stone)
 * - Chattering teeth speech when freezing
 * - Cold hands dropping items
 * - Realistic recovery rates indoors/near fire
 * - Enhanced score display with condition levels
 *
 *
 *   Note: I do not use the { character for my color codes. My MUD uses `
 *         so you will need to replace my color codes with your own.
 *
 *         It also bears mentioning that my MUD has been in development
 *         for over a decade and is now pretty much a 'rom derivative'
 *         subserver rather than being a rom MUD, so you will likely have
 *         to change quite a bit to make this install cleanly into a rom MUD.
 *
 *         -Ro 
 *         Mask of Icewind Legacy MUD
 *         icewindlegacy@gmail.com
 */

/* ============================================================================
 * 1. MERC.H CHANGES
 * ============================================================================ */

/*
 * Add the condition definitions COND_WET and COND_FREEZING:
 */
#define COND_DRUNK		      0
#define COND_FULL		      1
#define COND_THIRST		      2
#define COND_HUNGER		      3
#define COND_TIRED		      4
#define COND_WET		      5
#define COND_FREEZING		      6




/*
 * Add these helper macros near the IS_DRUNK macro:
 */
#define IS_DRUNK(ch)	((ch)->pcdata->condition[COND_DRUNK] > 10 )
#define IS_WET(ch)	((ch)->pcdata->condition[COND_WET] > 0 )
#define IS_FREEZING(ch)	((ch)->pcdata->condition[COND_FREEZING] > 0 )




/* ============================================================================
 * 2. UPDATE.C CHANGES
 * ============================================================================ */

/*
 * Replace the entire weather effects section in char_update() function
 * (around line 1214) with this code:
 */

	/* Temperature effects on characters */
	if ( !IS_NPC( ch ) && ch->level < LEVEL_IMMORTAL )
	{
	    bool has_campfire = FALSE;
	    bool has_shelter = FALSE;
	    OBJ_DATA *obj;
	    int wet_level = ch->pcdata->condition[COND_WET];
	    int freezing_level = ch->pcdata->condition[COND_FREEZING];
	    bool was_freezing = (freezing_level > 0);
	    bool was_wet = (wet_level > 0);
	    
	    /* Check for campfire in room */
	    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
	    {
		if ( obj->pIndexData->vnum == OBJ_VNUM_CAMPFIRE )
		{
		    has_campfire = TRUE;
		    break;
		}
	    }
	    
	    /* Check for shelter (furniture) */
	    if ( ch->furniture_in != NULL && ch->furniture_in->item_type == ITEM_FURNITURE )
		has_shelter = TRUE;
	    
	    /* Recovery from cold and wet conditions */
	    if ( !IS_OUTSIDE( ch ) || has_campfire || has_shelter )
	    {
		/* Decrease freezing - faster recovery indoors or near fire/shelter */
		if ( freezing_level > 0 )
		{
		    int recovery_rate = 1;  /* Base recovery */
		    
		    if ( !IS_OUTSIDE( ch ) )
			recovery_rate = 3;  /* Much faster indoors */
		    else if ( has_campfire )
			recovery_rate = 4;  /* Fastest near fire */
		    else if ( has_shelter )
			recovery_rate = 2;  /* Moderate in shelter */
		    
		    gain_condition( ch, COND_FREEZING, -recovery_rate );
		    
		    int new_freezing = ch->pcdata->condition[COND_FREEZING];
		    
		    /* Send message when significantly warming up */
		    if ( freezing_level > 4 && new_freezing <= 4 )
		    {
			if ( !IS_OUTSIDE( ch ) )
			    send_to_char( "You warm up indoors.\n\r", ch );
			else if ( has_campfire )
			    send_to_char( "You warm up by the fire.\n\r", ch );
			else if ( has_shelter )
			    send_to_char( "You warm up in the shelter.\n\r", ch );
		    }
		}
		
		/* Decrease wetness - faster recovery indoors or near fire */
		if ( wet_level > 0 )
		{
		    int wet_recovery_chance = 30;  /* Base 30% chance */
		    
		    if ( !IS_OUTSIDE( ch ) )
			wet_recovery_chance = 60;  /* 60% chance indoors */
		    else if ( has_campfire )
			wet_recovery_chance = 80;  /* 80% chance near fire */
		    else if ( has_shelter )
			wet_recovery_chance = 40;  /* 40% chance in shelter */
		    
		    if ( number_percent() < wet_recovery_chance )
		    {
			gain_condition( ch, COND_WET, -1 );
			if ( wet_level > 1 && ch->pcdata->condition[COND_WET] <= 1 )
			{
			    if ( !IS_OUTSIDE( ch ) )
				send_to_char( "You dry off indoors.\n\r", ch );
			    else if ( has_campfire )
				send_to_char( "You dry off by the fire.\n\r", ch );
			    else if ( has_shelter )
				send_to_char( "You dry off in the shelter.\n\r", ch );
			}
		    }
		}
	    }
	    
	    /* Outdoor weather effects */
	    if ( IS_OUTSIDE( ch ) )
	    {
		/* Cold effects - start at 45F and below */
		if ( weather_info.temperature < 45 )
		{
		    if ( !has_campfire && !has_shelter )
		    {
			/* Get wet from snow/rain - higher wetness in worse conditions */
			if ( weather_info.sky == SKY_RAINING || weather_info.sky == SKY_LIGHTNING )
			{
			    int wet_gain = 2;  /* Base wetness gain */
			    if ( weather_info.temperature < 20 )
				wet_gain = 3;  /* Freezing rain is worse */
			    
			    gain_condition( ch, COND_WET, wet_gain );  /* Positive = increase */
			    
			    if ( !was_wet )
				send_to_char( "You get soaked by the rain!\n\r", ch );
			    else if ( ch->pcdata->condition[COND_WET] >= 8 )
				send_to_char( "You are completely drenched!\n\r", ch );
			}
			
			/* Increase freezing based on temperature - much more gradual */
			int freezing_gain = 0;
			if ( weather_info.temperature <= 0 )
			    freezing_gain = 3;  /* Extreme cold - 1 tick for chattering, damage starts immediately */
			else if ( weather_info.temperature <= 10 )
			    freezing_gain = 2;  /* Very cold - 1 tick chattering, 2 ticks damage */
			else if ( weather_info.temperature <= 28 )
			    freezing_gain = 2;  /* Cold - 1 tick chattering, 3 ticks damage */
			else if ( weather_info.temperature <= 32 )
			    freezing_gain = 1;  /* Freezing - 2 ticks chattering, 4-5 ticks damage */
			else if ( weather_info.temperature <= 40 )
			    freezing_gain = 1;  /* Chilly - slower buildup */
			else if ( weather_info.temperature <= 45 )
			    freezing_gain = 1;  /* Cool - very slow buildup */
			
			/* Wetness makes freezing worse */
			if ( wet_level > 0 )
			    freezing_gain += (wet_level / 2);
			
			/* Material effects on freezing */
			int material_modifier = 0;
			OBJ_DATA *obj;
			int i;
			
			/* Check all equipped items for material effects */
			for ( i = 0; i < MAX_WEAR; i++ )
			{
			    obj = get_eq_char( ch, i );
			    if ( obj != NULL )
			    {
				/* Insulating materials - slow freezing */
				if ( xIS_SET( obj->material, MAT_LEATHER ) ||
				     xIS_SET( obj->material, MAT_WOOD ) ||
				     xIS_SET( obj->material, MAT_STRAW ) ||
				     xIS_SET( obj->material, MAT_CLOTH ) ||
				     xIS_SET( obj->material, MAT_FIBER ) ||
				     xIS_SET( obj->material, MAT_FLAX ) ||
				     xIS_SET( obj->material, MAT_HEMP ) ||
				     xIS_SET( obj->material, MAT_SILK ) ||
				     xIS_SET( obj->material, MAT_VELVET ) )
				{
				    material_modifier -= 1;  /* Slow freezing by 1 */
				}
				
				/* Highly insulating materials - slow freezing more */
				if ( xIS_SET( obj->material, MAT_FUR ) ||
				     xIS_SET( obj->material, MAT_WOOL ) ||
				     xIS_SET( obj->material, MAT_FEATHER ) )
				{
				    material_modifier -= 2;  /* Slow freezing by 2 */
				}
				
				/* Conductive materials - speed up freezing */
				if ( xIS_SET( obj->material, MAT_STEEL ) ||
				     xIS_SET( obj->material, MAT_IRON ) ||
				     xIS_SET( obj->material, MAT_MITHRIL ) ||
				     xIS_SET( obj->material, MAT_SILVER ) ||
				     xIS_SET( obj->material, MAT_GOLD ) ||
				     xIS_SET( obj->material, MAT_COPPER ) ||
				     xIS_SET( obj->material, MAT_BRONZE ) ||
				     xIS_SET( obj->material, MAT_BRASS ) ||
				     xIS_SET( obj->material, MAT_GLASS ) ||
				     xIS_SET( obj->material, MAT_JADE ) ||
				     xIS_SET( obj->material, MAT_MARBLE ) ||
				     xIS_SET( obj->material, MAT_CRYSTAL ) ||
				     xIS_SET( obj->material, MAT_DIAMOND ) ||
				     xIS_SET( obj->material, MAT_EMERALD ) ||
				     xIS_SET( obj->material, MAT_RUBY ) ||
				     xIS_SET( obj->material, MAT_SAPPHIRE ) ||
				     xIS_SET( obj->material, MAT_PEARL ) ||
				     xIS_SET( obj->material, MAT_PLATINUM ) ||
				     xIS_SET( obj->material, MAT_TIN ) ||
				     xIS_SET( obj->material, MAT_LEAD ) ||
				     xIS_SET( obj->material, MAT_ZINC ) )
				{
				    material_modifier += 1;  /* Speed up freezing by 1 */
				}
			    }
			}
			
			/* Apply material modifier to freezing gain */
			freezing_gain += material_modifier;
			
			/* Only gain freezing if we have a gain amount */
			if ( freezing_gain > 0 )
			{
			    gain_condition( ch, COND_FREEZING, freezing_gain );
			    
			    /* Progressive warning messages */
			    int current_freezing = ch->pcdata->condition[COND_FREEZING];
			    if ( current_freezing == 1 )
				send_to_char( "You feel a chill in the air.\n\r", ch );
			    else if ( current_freezing == 2 )
				send_to_char( "You start to feel cold.\n\r", ch );
			    else if ( current_freezing == 3 )
				send_to_char( "You are getting quite cold.\n\r", ch );
			    else if ( current_freezing == 4 )
				send_to_char( "You shiver uncontrollably and your teeth chatter together.\n\r", ch );
			    else if ( current_freezing == 5 )
				send_to_char( "You are freezing and can barely feel your extremities.\n\r", ch );
			    else if ( current_freezing >= 6 )
				send_to_char( "You are in severe danger from hypothermia!\n\r", ch );
			}
			
			/* Hypothermia damage - starts at different levels based on temperature */
			int current_freezing = ch->pcdata->condition[COND_FREEZING];
			int damage_threshold = 10;  /* Default high threshold */
			
			if ( weather_info.temperature <= 0 )
			    damage_threshold = 3;  /* Damage starts at level 3 */
			else if ( weather_info.temperature <= 10 )
			    damage_threshold = 4;  /* Damage starts at level 4 */
			else if ( weather_info.temperature <= 28 )
			    damage_threshold = 5;  /* Damage starts at level 5 */
			else if ( weather_info.temperature <= 32 )
			    damage_threshold = 6;  /* Damage starts at level 6 */
			else if ( weather_info.temperature <= 40 )
			    damage_threshold = 8;  /* Damage starts at level 8 */
			else if ( weather_info.temperature <= 45 )
			    damage_threshold = 10; /* Damage starts at level 10 */
			
			if ( current_freezing >= damage_threshold )
			{
			    int damage_amount = 1;
			    if ( wet_level > 0 )
				damage_amount = 2;  /* Wet + freezing = worse */
			    
			    /* Damage scales with freezing level */
			    if ( current_freezing > damage_threshold )
				damage_amount += (current_freezing - damage_threshold);
			    
			    damage( ch, ch, damage_amount, TYPE_UNDEFINED, DAM_COLD, TRUE );
			    
			    if ( wet_level > 0 )
				send_to_char( "The freezing cold and wet conditions are deadly!\n\r", ch );
			    else
				send_to_char( "You are freezing to death!\n\r", ch );
			}
			
			/* Cold hands - drop items if not wearing gloves and very cold */
			if ( current_freezing >= 4 )
			{
			    OBJ_DATA *gloves = get_eq_char( ch, WEAR_HANDS );
			    if ( gloves == NULL || gloves->item_type != ITEM_ARMOR )
			    {
				/* Check for items in hands that might be dropped */
				OBJ_DATA *wield = get_eq_char( ch, WEAR_WIELD );
				OBJ_DATA *hold = get_eq_char( ch, WEAR_HOLD );
				OBJ_DATA *shield = get_eq_char( ch, WEAR_SHIELD );
				OBJ_DATA *dual = get_eq_char( ch, WEAR_DUAL );
				
				/* Must drop dual wield first to prevent exploits */
				if ( dual != NULL && number_percent() < 20 )
				{
				    act( "Your cold hands can't maintain their grip on $p!", ch, dual, NULL, TO_CHAR );
				    act( "$n drops $p from $s cold hands!", ch, dual, NULL, TO_ROOM );
				    unequip_char( ch, dual );
				    obj_to_room( dual, ch->in_room );
				}
				else if ( wield != NULL && number_percent() < 15 )
				{
				    act( "Your cold hands can't maintain their grip on $p!", ch, wield, NULL, TO_CHAR );
				    act( "$n drops $p from $s cold hands!", ch, wield, NULL, TO_ROOM );
				    unequip_char( ch, wield );
				    obj_to_room( wield, ch->in_room );
				}
				else if ( shield != NULL && number_percent() < 15 )
				{
				    act( "Your cold hands can't maintain their grip on $p!", ch, shield, NULL, TO_CHAR );
				    act( "$n drops $p from $s cold hands!", ch, shield, NULL, TO_ROOM );
				    unequip_char( ch, shield );
				    obj_to_room( shield, ch->in_room );
				}
				else if ( hold != NULL && number_percent() < 15 )
				{
				    act( "Your cold hands can't maintain their grip on $p!", ch, hold, NULL, TO_CHAR );
				    act( "$n drops $p from $s cold hands!", ch, hold, NULL, TO_ROOM );
				    unequip_char( ch, hold );
				    obj_to_room( hold, ch->in_room );
				}
			    }
			}
		    }
		}
		else if ( weather_info.temperature < 50 )  /* Chilly */
		{
		    /* Can get wet but not freezing */
		    if ( weather_info.sky == SKY_RAINING || weather_info.sky == SKY_LIGHTNING )
		    {
			gain_condition( ch, COND_WET, 1 );  /* Positive = increase */
			if ( !was_wet )
			    send_to_char( "You get wet from the rain.\n\r", ch );
		    }
		}
		
		/* Heat effects */
		if ( weather_info.temperature > 100 )  /* Blistering hot */
		{
		    /* Increased thirst and fatigue */
		    gain_condition( ch, COND_THIRST, -3 );
		    gain_condition( ch, COND_TIRED, -2 );
		    
		    if ( number_percent() < 10 )  /* 10% chance per tick */
			send_to_char( "The heat is oppressive and exhausting.\n\r", ch );
		}
		else if ( weather_info.temperature > 85 )  /* Hot */
		{
		    /* Moderate thirst increase */
		    gain_condition( ch, COND_THIRST, -1 );
		    gain_condition( ch, COND_TIRED, -1 );
		}
		
		/* Wet penalty - movement difficulty based on wetness level */
		int current_wet = ch->pcdata->condition[COND_WET];
		if ( current_wet > 0 && ch->move > 0 )
		{
		    int move_loss = (current_wet + 1) / 2;  /* More wet = more move loss */
		    ch->move = UMAX( 0, ch->move - move_loss );
		    if ( number_percent() < 5 )  /* 5% chance per tick */
			send_to_char( "Your wet clothes slow you down.\n\r", ch );
		}
		
		/* Freezing penalty - movement difficulty based on freezing level */
		int current_freezing = ch->pcdata->condition[COND_FREEZING];
		if ( current_freezing > 0 && ch->move > 0 )
		{
		    int move_loss = (current_freezing + 2) / 3;  /* Cold limbs make movement harder */
		    ch->move = UMAX( 0, ch->move - move_loss );
		    if ( number_percent() < 8 )  /* 8% chance per tick */
			send_to_char( "Your cold limbs make movement difficult.\n\r", ch );
		}
	    }  /* End of outdoor weather effects */
	}

/* ============================================================================
 * 3. ACT_COMM.C CHANGES
 * ============================================================================ */

/*
 * Add this chattering teeth system after the drunk system (around line 115):
 */

/*
 * Chattering teeth struct for freezing speech
 */
struct structchatter
{
    int		min_freezing_level;
    int		number_of_reps;
    char *	replacement[11];
};

struct structchatter chatter[] =
{
    { 2, 8,
        { "a", "a", "a", "A", "ah", "Ah", "ahh", "ahhh" }
    },
    { 4, 4, { "b", "b", "B", "B" } },
    { 3, 6, { "c", "c", "C", "ch", "chh", "chhh" } },
    { 4, 3, { "d", "d", "D" } },
    { 2, 4, { "e", "e", "eh", "E" } },
    { 5, 4, { "f", "f", "ff", "F" } },
    { 6, 3, { "g", "g", "G" } },
    { 4, 5, { "h", "h", "hh", "Hh", "H" } },
    { 3, 4, { "i", "i", "ii", "I" } },
    { 7, 3, { "j", "j", "J" } },
    { 5, 3, { "k", "k", "K" } },
    { 3, 3, { "l", "l", "L" } },
    { 4, 5, { "m", "m", "mm", "Mm", "M" } },
    { 3, 4, { "n", "n", "nn", "N" } },
    { 2, 4, { "o", "o", "oo", "O" } },
    { 4, 3, { "p", "p", "P" } },
    { 6, 3, { "q", "q", "Q" } },
    { 3, 4, { "r", "r", "rr", "R" } },
    { 2, 6, { "s", "ss", "sss", "Sss", "sS", "S" } },
    { 4, 4, { "t", "t", "tt", "T" } },
    { 2, 4, { "u", "u", "uh", "U" } },
    { 5, 3, { "v", "v", "V" } },
    { 4, 3, { "w", "w", "W" } },
    { 6, 3, { "x", "x", "X" } },
    { 3, 3, { "y", "y", "Y" } },
    { 2, 5,
        { "z", "z", "zz", "Zz", "Z" }
    }     
};

/*
 * Add this function after the makedrunk function (around line 609):
 */

static char *
makechatter( CHAR_DATA *ch, char *string )
{
    int		freezinglevel;
    static char	buf[MAX_INPUT_LENGTH];
    int		pos;
    int		randomnum;
    char	temp;

    freezinglevel = IS_NPC( ch ) ? 0 : (ch->pcdata ? ch->pcdata->condition[COND_FREEZING] : 0);
    if ( freezinglevel <= 0 )
        return string;

    pos = 0;
    while ( *string != '\0' )
    {
        temp = toupper( *string );
        if ( temp >= 'A' && temp <= 'Z' )
        {
            if ( freezinglevel > chatter[temp - 'A'].min_freezing_level )
            {
                randomnum = number_range( 0, chatter[temp - 'A'].number_of_reps - 1 );
                strcpy( &buf[pos], chatter[temp - 'A'].replacement[randomnum] );
                pos += strlen( chatter[temp - 'A'].replacement[randomnum] );
            }
            else
            {
                buf[pos++] = *string;
            }
        }
        else if ( temp >= '0' && temp <= '9' && number_percent( ) < freezinglevel * 3 )
        {
            buf[pos++] = number_range( '0', '9' );
        }
        else if ( is_colcode( string ) && number_percent( ) < freezinglevel * 3 )
        {
            randomnum = number_range( 1, 15 );
            buf[pos++] = *string++;
            buf[pos++] = colorcode_list[randomnum];
        }
        else
        {
            buf[pos++] = *string;
        }

        if ( pos >= MAX_INPUT_LENGTH - 10 )
            break;
        string++;
    }

    buf[pos] = '\0';
    return buf;
}

/*
 * Update these speech functions to use makechatter:
 * 
 * In do_say function (around line 1374):
 * Replace: p = stpcpy( buf, makedrunk( ch, argument ) ) - 1;
 * With:    p = stpcpy( buf, makechatter( ch, makedrunk( ch, argument ) ) ) - 1;
 * 
 * In do_sayto function (around line 1454):
 * Replace: p = stpcpy( buf, makedrunk( ch, argument ) );
 * With:    p = stpcpy( buf, makechatter( ch, makedrunk( ch, argument ) ) );
 * 
 * In whisper function (around line 1838):
 * Replace: p = stpcpy( text, makedrunk( ch, argument ) ) - 1;
 * With:    p = stpcpy( text, makechatter( ch, makedrunk( ch, argument ) ) ) - 1;
 */

/* ============================================================================
 * 4. ACT_INFO.C CHANGES
 * ============================================================================ */

/*
 * Replace the weather conditions section in show_score() function (around line 4015):
 */

	/* Weather conditions */
	found = FALSE;
	int wet_level = vch->pcdata->condition[COND_WET];
	int freezing_level = vch->pcdata->condition[COND_FREEZING];
	
	if ( wet_level > 0 )
	{
	    char *wet_desc;
	    if ( wet_level <= 2 )
		wet_desc = "`Wdamp`c";
	    else if ( wet_level <= 4 )
		wet_desc = "`Wwet`c";
	    else if ( wet_level <= 6 )
		wet_desc = "`Wdripping`c";
	    else
		wet_desc = "`Wsoaking wet`c";
	    
	    p += sprintf( p, "`GYou are %s.  ", wet_desc );
	    found = TRUE;
	}
	
	if ( freezing_level > 0 )
	{
	    char *freeze_desc;
	    if ( freezing_level == 1 )
		freeze_desc = "`Bchilled`c";
	    else if ( freezing_level == 2 )
		freeze_desc = "`Bcold`c";
	    else if ( freezing_level == 3 )
		freeze_desc = "`Bquite cold`c";
	    else if ( freezing_level == 4 )
		freeze_desc = "`Bshivering`c";
	    else if ( freezing_level == 5 )
		freeze_desc = "`Bfreezing`c";
	    else if ( freezing_level <= 7 )
		freeze_desc = "`Bhypothermic`c";
	    else
		freeze_desc = "`Bseverely hypothermic`c";
	    
	    p += sprintf( p, "`GYou are %s.  ", freeze_desc );
	    found = TRUE;
	}




/* ============================================================================
 * INSTALLATION NOTES
 * ============================================================================ */

/*
 * 1. Make sure to backup your original files before making changes
 * 2. The system requires the existing weather system to be present
 * 3. Test thoroughly in a development environment first
 * 4. The material system requires proper material flags on objects
 * 5. Recovery rates can be adjusted by changing the recovery_rate values
 * 6. Temperature thresholds can be modified in the weather effects section
 * 7. The system is designed to work with existing furniture and campfire systems
 * 
 * COMPATIBILITY:
 * - Works with existing weather system
 * - Compatible with existing furniture system
 * - Works with existing campfire objects
 * - Integrates with existing condition system
 * - Compatible with existing material system
 * 
 * CUSTOMIZATION:
 * - Adjust temperature thresholds as needed
 * - Modify recovery rates for different balance
 * - Add new materials to the insulation system
 * - Customize warning messages
 * - Adjust damage scaling
 */





