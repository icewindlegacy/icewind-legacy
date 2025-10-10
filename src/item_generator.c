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

/*
 * Enhanced Item Generator System
 * Based on Thri's Random Item Generator
 * Adapted for Icewind Legacy with rarity tiers and special properties
 */

#include <stdio.h>
#include <time.h>
#include "merc.h"

/* Rarity definitions */
#define RARITY_COMMON     0    /* 70% chance */
#define RARITY_UNCOMMON   1    /* 20% chance */
#define RARITY_RARE       2    /* 8% chance */
#define RARITY_VERY_RARE  3    /* 1.9% chance (1 in 50) */
#define RARITY_EPIC       4    /* 0.1% chance (1 in 1000) */
#define RARITY_RELIC      5    /* 0.001% chance (1 in 100,000) */
#define RARITY_ARTIFACT   6    /* 0.0001% chance (1 in 1,000,000) */

/* Special weapon properties */
#define PROP_FLAMING      1
#define PROP_FROST        2
#define PROP_VORPAL       3
#define PROP_TALKING      4
#define PROP_TALKING_MACE 5
#define PROP_SHOCKING     6
#define PROP_PESTILENCE   7
#define PROP_POWERLEECH   8
#define PROP_POISONED     9
#define PROP_SHARP        10

/* Function prototypes */
int determine_rarity(void);
void apply_rarity_bonuses(OBJ_DATA *obj, int rarity, int level);
void add_weapon_property(OBJ_DATA *obj, int property);
void add_armor_property(OBJ_DATA *obj, int property);

/* Main generation functions */
OBJ_DATA *new_gen_armor(OBJ_DATA *obj, int level);
OBJ_DATA *new_gen_weapon(OBJ_DATA *obj, int level);
OBJ_DATA *new_gen_gold(OBJ_DATA *obj, int level);

/* Armor generation functions */
void make_armor_shield(OBJ_DATA *obj, int level, int rarity);
void make_armor_head(OBJ_DATA *obj, int level, int rarity);
void make_armor_body(OBJ_DATA *obj, int level, int rarity);
void make_armor_boots(OBJ_DATA *obj, int level, int rarity);
void make_armor_gloves(OBJ_DATA *obj, int level, int rarity);
void make_armor_arms(OBJ_DATA *obj, int level, int rarity);
void make_armor_legs(OBJ_DATA *obj, int level, int rarity);
void make_armor_waist(OBJ_DATA *obj, int level, int rarity);
void make_armor_wrist(OBJ_DATA *obj, int level, int rarity);
void make_armor_neck(OBJ_DATA *obj, int level, int rarity);
void make_armor_finger(OBJ_DATA *obj, int level, int rarity);

/* Weapon generation functions */
void make_weapon_sword(OBJ_DATA *obj, int level, int rarity);
void make_weapon_axe(OBJ_DATA *obj, int level, int rarity);
void make_weapon_mace(OBJ_DATA *obj, int level, int rarity);
void make_weapon_dagger(OBJ_DATA *obj, int level, int rarity);
void make_weapon_spear(OBJ_DATA *obj, int level, int rarity);
void make_weapon_flail(OBJ_DATA *obj, int level, int rarity);
void make_weapon_whip(OBJ_DATA *obj, int level, int rarity);
void make_weapon_polearm(OBJ_DATA *obj, int level, int rarity);
void make_weapon_hammer(OBJ_DATA *obj, int level, int rarity);
void make_weapon_longsword(OBJ_DATA *obj, int level, int rarity);
void make_weapon_staff(OBJ_DATA *obj, int level, int rarity);
void make_weapon_bow(OBJ_DATA *obj, int level, int rarity);
void make_weapon_crossbow(OBJ_DATA *obj, int level, int rarity);
void make_weapon_club(OBJ_DATA *obj, int level, int rarity);
void make_weapon_scimitar(OBJ_DATA *obj, int level, int rarity);

/*
 * Determine item rarity based on weighted random
 */
int determine_rarity(void)
{
    int roll = number_range(1, 1000000);
    
    if (roll <= 1) return RARITY_ARTIFACT;      /* 1 in 1,000,000 */
    if (roll <= 10) return RARITY_RELIC;        /* 9 in 1,000,000 (1 in 100,000) */
    if (roll <= 1000) return RARITY_EPIC;       /* 990 in 1,000,000 (1 in 1000) */
    if (roll <= 19000) return RARITY_VERY_RARE; /* 18,000 in 1,000,000 (1.8%) */
    if (roll <= 99000) return RARITY_RARE;      /* 80,000 in 1,000,000 (8%) */
    if (roll <= 290000) return RARITY_UNCOMMON; /* 200,000 in 1,000,000 (20%) */
    return RARITY_COMMON;                       /* 710,000 in 1,000,000 (71%) */
}

/*
 * Apply rarity-based bonuses to items
 */
void apply_rarity_bonuses(OBJ_DATA *obj, int rarity, int level)
{
    int bonus_multiplier = 1;
    int cost_multiplier = 1;
    
    switch (rarity)
    {
        case RARITY_COMMON:
            bonus_multiplier = 1;
            cost_multiplier = 1;
            break;
        case RARITY_UNCOMMON:
            bonus_multiplier = 2;
            cost_multiplier = 3;
            break;
        case RARITY_RARE:
            bonus_multiplier = 3;
            cost_multiplier = 8;
            break;
        case RARITY_VERY_RARE:
            bonus_multiplier = 5;
            cost_multiplier = 25;
            break;
        case RARITY_EPIC:
            bonus_multiplier = 8;
            cost_multiplier = 100;
            break;
        case RARITY_RELIC:
            bonus_multiplier = 12;
            cost_multiplier = 500;
            break;
        case RARITY_ARTIFACT:
            bonus_multiplier = 20;
            cost_multiplier = 2000;
            break;
    }
    
    /* Apply bonuses to armor */
    if (obj->item_type == ITEM_ARMOR)
    {
        obj->value[0] *= bonus_multiplier; /* AC vs pierce */
        obj->value[1] *= bonus_multiplier; /* AC vs bash */
        obj->value[2] *= bonus_multiplier; /* AC vs slash */
        obj->value[3] *= bonus_multiplier; /* AC vs magic */
    }
    
    /* Apply bonuses to weapons */
    if (obj->item_type == ITEM_WEAPON)
    {
        obj->value[1] = UMAX(1, obj->value[1] * bonus_multiplier / 2); /* dice number */
        obj->value[2] = UMAX(4, obj->value[2] * bonus_multiplier / 2); /* dice sides */
    }
    
    /* Apply cost multiplier */
    obj->cost *= cost_multiplier;
}

/*
 * Add special properties to weapons
 */
void add_weapon_property(OBJ_DATA *obj, int property)
{
    switch (property)
    {
        case PROP_FLAMING:
            SET_BIT(obj->value[4], WEAPON_FLAMING);
            break;
        case PROP_FROST:
            SET_BIT(obj->value[4], WEAPON_FROST);
            break;
        case PROP_VORPAL:
            SET_BIT(obj->value[4], WEAPON_VORPAL);
            break;
        case PROP_TALKING:
            SET_BIT(obj->value[4], WEAPON_TALKING);
            break;
        case PROP_TALKING_MACE:
            if (obj->value[0] == WEAPON_MACE)
                SET_BIT(obj->value[4], WEAPON_TALKING_MACE);
            break;
        case PROP_SHOCKING:
            SET_BIT(obj->value[4], WEAPON_SHOCKING);
            break;
        case PROP_PESTILENCE:
            SET_BIT(obj->value[4], WEAPON_PESTILENCE);
            break;
        case PROP_POWERLEECH:
            SET_BIT(obj->value[4], WEAPON_POWERLEECH);
            break;
        case PROP_POISONED:
            SET_BIT(obj->value[4], WEAPON_POISONED);
            break;
        case PROP_SHARP:
            SET_BIT(obj->value[4], WEAPON_SHARP);
            break;
    }
}

/*
 * Add special properties to armor
 */
void add_armor_property(OBJ_DATA *obj, int property)
{
    switch (property)
    {
        case PROP_FLAMING:
            SET_BIT(obj->extra_flags, ITEM_FLAMING);
            break;
        case PROP_FROST:
            SET_BIT(obj->extra_flags, ITEM_FROST);
            break;
        case PROP_SHOCKING:
            SET_BIT(obj->extra_flags, ITEM_SHOCKING);
            break;
        case PROP_PESTILENCE:
            SET_BIT(obj->extra_flags, ITEM_PESTILENCE);
            break;
        case PROP_POWERLEECH:
            SET_BIT(obj->extra_flags, ITEM_POWERLEECH);
            break;
        case PROP_POISONED:
            SET_BIT(obj->extra_flags, ITEM_POISONED);
            break;
    }
}

/*
 * Main armor generation function
 */
OBJ_DATA *new_gen_armor(OBJ_DATA *obj, int level)
{
    int armor_type = number_range(1, 11);
    int rarity = determine_rarity();
    
    /* Create a basic armor object using blank template */
    obj = create_object(get_obj_index(OBJ_VNUM_RARM), 0);
    obj->wear_flags = ITEM_TAKE;
    
    switch(armor_type)
    {
        case 1:  make_armor_shield(obj, level, rarity); break;
        case 2:  make_armor_head(obj, level, rarity); break;
        case 3:  make_armor_body(obj, level, rarity); break;
        case 4:  make_armor_boots(obj, level, rarity); break;
        case 5:  make_armor_gloves(obj, level, rarity); break;
        case 6:  make_armor_arms(obj, level, rarity); break;
        case 7:  make_armor_legs(obj, level, rarity); break;
        case 8:  make_armor_waist(obj, level, rarity); break;
        case 9:  make_armor_wrist(obj, level, rarity); break;
        case 10: make_armor_neck(obj, level, rarity); break;
        case 11: make_armor_finger(obj, level, rarity); break;
    }
    
    /* Apply rarity bonuses */
    apply_rarity_bonuses(obj, rarity, level);
    
    /* Add special properties for rare+ items */
    if (rarity >= RARITY_VERY_RARE)
    {
        int num_properties = 0;
        if (rarity >= RARITY_EPIC) num_properties = number_range(1, 3);
        else if (rarity >= RARITY_VERY_RARE) num_properties = number_range(1, 2);
        
        for (int i = 0; i < num_properties; i++)
        {
            int property = number_range(1, 6); /* Available armor properties */
            add_armor_property(obj, property);
        }
    }
    
    return obj;
}

/*
 * Main weapon generation function
 */
OBJ_DATA *new_gen_weapon(OBJ_DATA *obj, int level)
{
    int weapon_type = number_range(1, 15);
    int rarity = determine_rarity();
    
    /* Create a basic weapon object using blank template */
    obj = create_object(get_obj_index(OBJ_VNUM_RWEAP), 0);
    obj->wear_flags = ITEM_TAKE | ITEM_WIELD;
    
    switch(weapon_type)
    {
        case 1:  make_weapon_sword(obj, level, rarity); break;
        case 2:  make_weapon_axe(obj, level, rarity); break;
        case 3:  make_weapon_mace(obj, level, rarity); break;
        case 4:  make_weapon_dagger(obj, level, rarity); break;
        case 5:  make_weapon_spear(obj, level, rarity); break;
        case 6:  make_weapon_flail(obj, level, rarity); break;
        case 7:  make_weapon_whip(obj, level, rarity); break;
        case 8:  make_weapon_polearm(obj, level, rarity); break;
        case 9:  make_weapon_hammer(obj, level, rarity); break;
        case 10: make_weapon_longsword(obj, level, rarity); break;
        case 11: make_weapon_staff(obj, level, rarity); break;
        case 12: make_weapon_bow(obj, level, rarity); break;
        case 13: make_weapon_crossbow(obj, level, rarity); break;
        case 14: make_weapon_club(obj, level, rarity); break;
        case 15: make_weapon_scimitar(obj, level, rarity); break;
    }
    
    /* Apply rarity bonuses */
    apply_rarity_bonuses(obj, rarity, level);
    
    /* Add special properties for rare+ items */
    if (rarity >= RARITY_VERY_RARE)
    {
        int num_properties = 0;
        if (rarity >= RARITY_RELIC) num_properties = number_range(2, 5);
        else if (rarity >= RARITY_EPIC) num_properties = number_range(1, 4);
        else if (rarity >= RARITY_VERY_RARE) num_properties = number_range(1, 2);
        
        for (int i = 0; i < num_properties; i++)
        {
            int property = number_range(1, 10); /* All weapon properties */
            add_weapon_property(obj, property);
        }
    }
    
    return obj;
}

/*
 * Gold generation function
 */
OBJ_DATA *new_gen_gold(OBJ_DATA *obj, int level)
{
    MONEY gold_amount;
    int rarity = determine_rarity();
    int multiplier = 1;
    
    /* Apply rarity multiplier to gold */
    switch (rarity)
    {
        case RARITY_COMMON:     multiplier = 1; break;
        case RARITY_UNCOMMON:   multiplier = 2; break;
        case RARITY_RARE:       multiplier = 5; break;
        case RARITY_VERY_RARE:  multiplier = 15; break;
        case RARITY_EPIC:       multiplier = 50; break;
        case RARITY_RELIC:      multiplier = 200; break;
        case RARITY_ARTIFACT:   multiplier = 1000; break;
    }
    
    /* Generate gold based on level and rarity */
    gold_amount.gold = number_range(level * 2 * multiplier, level * 10 * multiplier);
    gold_amount.silver = number_range(0, level * 5 * multiplier);
    gold_amount.copper = number_range(0, level * 20 * multiplier);
    gold_amount.fract = 0;
    
    obj = create_money(&gold_amount);
    return obj;
}

/*
 * Armor creation functions
 */
void make_armor_shield(OBJ_DATA *obj, int level, int rarity)
{
    int base_ac = number_range(level * 2, level * 5);
    
    obj->item_type = ITEM_ARMOR;
    obj->wear_flags |= ITEM_WEAR_SHIELD;
    obj->value[0] = base_ac;     /* AC vs pierce */
    obj->value[1] = base_ac;     /* AC vs bash */
    obj->value[2] = base_ac;     /* AC vs slash */
    obj->value[3] = base_ac;     /* AC vs magic */
    obj->weight = number_range(5, 15);
    obj->cost = number_range(level * 10, level * 50);
    
    /* Set descriptions based on rarity */
    const char* materials[] = {"wooden", "leather", "steel", "mithril", "adamantine", "dragonhide", "divine"};
    const char* types[] = {"shield", "buckler", "tower shield", "kite shield", "round shield", "greatshield", "aegis"};
    
    int mat_idx = UMIN(rarity, 6);
    int type_idx = UMIN(rarity, 6);
    
    char name[256], short_desc[256], desc[256];
    sprintf(name, "%s %s", materials[mat_idx], types[type_idx]);
    sprintf(short_desc, "a %s %s", materials[mat_idx], types[type_idx]);
    sprintf(desc, "A %s %s lies here.", materials[mat_idx], types[type_idx]);
    
    free_string(obj->name);
    obj->name = str_dup(name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(short_desc);
    free_string(obj->description);
    obj->description = str_dup(desc);
}

void make_armor_head(OBJ_DATA *obj, int level, int rarity)
{
    int base_ac = number_range(level, level * 3);
    
    obj->item_type = ITEM_ARMOR;
    obj->wear_flags |= ITEM_WEAR_HEAD;
    obj->value[0] = base_ac;
    obj->value[1] = base_ac;
    obj->value[2] = base_ac;
    obj->value[3] = base_ac;
    obj->weight = number_range(2, 8);
    obj->cost = number_range(level * 5, level * 25);
    
    const char* materials[] = {"leather", "chain", "plate", "mithril", "adamantine", "dragonhide", "divine"};
    const char* types[] = {"cap", "helmet", "great helm", "crown", "circlet", "crown of power", "divine crown"};
    
    int mat_idx = UMIN(rarity, 6);
    int type_idx = UMIN(rarity, 6);
    
    char name[256], short_desc[256], desc[256];
    sprintf(name, "%s %s", materials[mat_idx], types[type_idx]);
    sprintf(short_desc, "a %s %s", materials[mat_idx], types[type_idx]);
    sprintf(desc, "A %s %s lies here.", materials[mat_idx], types[type_idx]);
    
    free_string(obj->name);
    obj->name = str_dup(name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(short_desc);
    free_string(obj->description);
    obj->description = str_dup(desc);
}

void make_armor_body(OBJ_DATA *obj, int level, int rarity)
{
    int base_ac = number_range(level * 2, level * 6);
    
    obj->item_type = ITEM_ARMOR;
    obj->wear_flags |= ITEM_WEAR_BODY;
    obj->value[0] = base_ac;
    obj->value[1] = base_ac;
    obj->value[2] = base_ac;
    obj->value[3] = base_ac;
    obj->weight = number_range(10, 25);
    obj->cost = number_range(level * 15, level * 75);
    
    const char* materials[] = {"leather", "chain", "plate", "mithril", "adamantine", "dragonhide", "divine"};
    const char* types[] = {"armor", "mail", "plate mail", "full plate", "battle armor", "war armor", "divine armor"};
    
    int mat_idx = UMIN(rarity, 6);
    int type_idx = UMIN(rarity, 6);
    
    char name[256], short_desc[256], desc[256];
    sprintf(name, "%s %s", materials[mat_idx], types[type_idx]);
    sprintf(short_desc, "a %s %s", materials[mat_idx], types[type_idx]);
    sprintf(desc, "A %s %s lies here.", materials[mat_idx], types[type_idx]);
    
    free_string(obj->name);
    obj->name = str_dup(name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(short_desc);
    free_string(obj->description);
    obj->description = str_dup(desc);
}

void make_armor_boots(OBJ_DATA *obj, int level, int rarity)
{
    int base_ac = number_range(level, level * 2);
    
    obj->item_type = ITEM_ARMOR;
    obj->wear_flags |= ITEM_WEAR_FEET;
    obj->value[0] = base_ac;
    obj->value[1] = base_ac;
    obj->value[2] = base_ac;
    obj->value[3] = base_ac;
    obj->weight = number_range(3, 8);
    obj->cost = number_range(level * 3, level * 15);
    
    const char* materials[] = {"leather", "chain", "plate", "mithril", "adamantine", "dragonhide", "divine"};
    const char* types[] = {"boots", "shoes", "greaves", "war boots", "battle boots", "boots of power", "divine boots"};
    
    int mat_idx = UMIN(rarity, 6);
    int type_idx = UMIN(rarity, 6);
    
    char name[256], short_desc[256], desc[256];
    sprintf(name, "%s %s", materials[mat_idx], types[type_idx]);
    sprintf(short_desc, "a pair of %s %s", materials[mat_idx], types[type_idx]);
    sprintf(desc, "A pair of %s %s lie here.", materials[mat_idx], types[type_idx]);
    
    free_string(obj->name);
    obj->name = str_dup(name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(short_desc);
    free_string(obj->description);
    obj->description = str_dup(desc);
}

void make_armor_gloves(OBJ_DATA *obj, int level, int rarity)
{
    int base_ac = number_range(level, level * 2);
    
    obj->item_type = ITEM_ARMOR;
    obj->wear_flags |= ITEM_WEAR_HANDS;
    obj->value[0] = base_ac;
    obj->value[1] = base_ac;
    obj->value[2] = base_ac;
    obj->value[3] = base_ac;
    obj->weight = number_range(1, 4);
    obj->cost = number_range(level * 2, level * 10);
    
    const char* materials[] = {"leather", "chain", "plate", "mithril", "adamantine", "dragonhide", "divine"};
    const char* types[] = {"gloves", "gauntlets", "war gloves", "battle gloves", "gloves of power", "divine gloves"};
    
    int mat_idx = UMIN(rarity, 6);
    int type_idx = UMIN(rarity, 5);
    
    char name[256], short_desc[256], desc[256];
    sprintf(name, "%s %s", materials[mat_idx], types[type_idx]);
    sprintf(short_desc, "a pair of %s %s", materials[mat_idx], types[type_idx]);
    sprintf(desc, "A pair of %s %s lie here.", materials[mat_idx], types[type_idx]);
    
    free_string(obj->name);
    obj->name = str_dup(name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(short_desc);
    free_string(obj->description);
    obj->description = str_dup(desc);
}

void make_armor_arms(OBJ_DATA *obj, int level, int rarity)
{
    int base_ac = number_range(level, level * 2);
    
    obj->item_type = ITEM_ARMOR;
    obj->wear_flags |= ITEM_WEAR_ARMS;
    obj->value[0] = base_ac;
    obj->value[1] = base_ac;
    obj->value[2] = base_ac;
    obj->value[3] = base_ac;
    obj->weight = number_range(3, 8);
    obj->cost = number_range(level * 4, level * 20);
    
    const char* materials[] = {"leather", "chain", "plate", "mithril", "adamantine", "dragonhide", "divine"};
    const char* types[] = {"bracers", "vambraces", "arm guards", "war bracers", "bracers of power", "divine bracers"};
    
    int mat_idx = UMIN(rarity, 6);
    int type_idx = UMIN(rarity, 5);
    
    char name[256], short_desc[256], desc[256];
    sprintf(name, "%s %s", materials[mat_idx], types[type_idx]);
    sprintf(short_desc, "a pair of %s %s", materials[mat_idx], types[type_idx]);
    sprintf(desc, "A pair of %s %s lie here.", materials[mat_idx], types[type_idx]);
    
    free_string(obj->name);
    obj->name = str_dup(name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(short_desc);
    free_string(obj->description);
    obj->description = str_dup(desc);
}

void make_armor_legs(OBJ_DATA *obj, int level, int rarity)
{
    int base_ac = number_range(level, level * 3);
    
    obj->item_type = ITEM_ARMOR;
    obj->wear_flags |= ITEM_WEAR_LEGS;
    obj->value[0] = base_ac;
    obj->value[1] = base_ac;
    obj->value[2] = base_ac;
    obj->value[3] = base_ac;
    obj->weight = number_range(5, 15);
    obj->cost = number_range(level * 8, level * 40);
    
    const char* materials[] = {"leather", "chain", "plate", "mithril", "adamantine", "dragonhide", "divine"};
    const char* types[] = {"leggings", "chausses", "leg guards", "war leggings", "leggings of power", "divine leggings"};
    
    int mat_idx = UMIN(rarity, 6);
    int type_idx = UMIN(rarity, 5);
    
    char name[256], short_desc[256], desc[256];
    sprintf(name, "%s %s", materials[mat_idx], types[type_idx]);
    sprintf(short_desc, "a pair of %s %s", materials[mat_idx], types[type_idx]);
    sprintf(desc, "A pair of %s %s lie here.", materials[mat_idx], types[type_idx]);
    
    free_string(obj->name);
    obj->name = str_dup(name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(short_desc);
    free_string(obj->description);
    obj->description = str_dup(desc);
}

void make_armor_waist(OBJ_DATA *obj, int level, int rarity)
{
    int base_ac = number_range(level / 2, level);
    
    obj->item_type = ITEM_ARMOR;
    obj->wear_flags |= ITEM_WEAR_WAIST;
    obj->value[0] = base_ac;
    obj->value[1] = base_ac;
    obj->value[2] = base_ac;
    obj->value[3] = base_ac;
    obj->weight = number_range(1, 3);
    obj->cost = number_range(level * 2, level * 10);
    
    const char* materials[] = {"leather", "chain", "plate", "mithril", "adamantine", "dragonhide", "divine"};
    const char* types[] = {"belt", "girdle", "war belt", "belt of power", "divine belt"};
    
    int mat_idx = UMIN(rarity, 6);
    int type_idx = UMIN(rarity, 4);
    
    char name[256], short_desc[256], desc[256];
    sprintf(name, "%s %s", materials[mat_idx], types[type_idx]);
    sprintf(short_desc, "a %s %s", materials[mat_idx], types[type_idx]);
    sprintf(desc, "A %s %s lies here.", materials[mat_idx], types[type_idx]);
    
    free_string(obj->name);
    obj->name = str_dup(name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(short_desc);
    free_string(obj->description);
    obj->description = str_dup(desc);
}

void make_armor_wrist(OBJ_DATA *obj, int level, int rarity)
{
    int base_ac = number_range(level / 2, level);
    
    obj->item_type = ITEM_ARMOR;
    obj->wear_flags |= ITEM_WEAR_WRIST;
    obj->value[0] = base_ac;
    obj->value[1] = base_ac;
    obj->value[2] = base_ac;
    obj->value[3] = base_ac;
    obj->weight = number_range(1, 2);
    obj->cost = number_range(level * 2, level * 10);
    
    const char* materials[] = {"leather", "chain", "plate", "mithril", "adamantine", "dragonhide", "divine"};
    const char* types[] = {"bracers", "wrist guards", "war bracers", "bracers of power", "divine bracers"};
    
    int mat_idx = UMIN(rarity, 6);
    int type_idx = UMIN(rarity, 4);
    
    char name[256], short_desc[256], desc[256];
    sprintf(name, "%s %s", materials[mat_idx], types[type_idx]);
    sprintf(short_desc, "a pair of %s %s", materials[mat_idx], types[type_idx]);
    sprintf(desc, "A pair of %s %s lie here.", materials[mat_idx], types[type_idx]);
    
    free_string(obj->name);
    obj->name = str_dup(name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(short_desc);
    free_string(obj->description);
    obj->description = str_dup(desc);
}

void make_armor_neck(OBJ_DATA *obj, int level, int rarity)
{
    int base_ac = number_range(level / 2, level);
    
    obj->item_type = ITEM_ARMOR;
    obj->wear_flags |= ITEM_WEAR_NECK;
    obj->value[0] = base_ac;
    obj->value[1] = base_ac;
    obj->value[2] = base_ac;
    obj->value[3] = base_ac;
    obj->weight = number_range(1, 2);
    obj->cost = number_range(level * 3, level * 15);
    
    const char* materials[] = {"leather", "chain", "plate", "mithril", "adamantine", "dragonhide", "divine"};
    const char* types[] = {"gorget", "neck guard", "war gorget", "gorget of power", "divine gorget"};
    
    int mat_idx = UMIN(rarity, 6);
    int type_idx = UMIN(rarity, 4);
    
    char name[256], short_desc[256], desc[256];
    sprintf(name, "%s %s", materials[mat_idx], types[type_idx]);
    sprintf(short_desc, "a %s %s", materials[mat_idx], types[type_idx]);
    sprintf(desc, "A %s %s lies here.", materials[mat_idx], types[type_idx]);
    
    free_string(obj->name);
    obj->name = str_dup(name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(short_desc);
    free_string(obj->description);
    obj->description = str_dup(desc);
}

void make_armor_finger(OBJ_DATA *obj, int level, int rarity)
{
    int base_ac = number_range(level / 3, level / 2);
    
    obj->item_type = ITEM_ARMOR;
    obj->wear_flags |= ITEM_WEAR_FINGER;
    obj->value[0] = base_ac;
    obj->value[1] = base_ac;
    obj->value[2] = base_ac;
    obj->value[3] = base_ac;
    obj->weight = 1;
    obj->cost = number_range(level * 5, level * 25);
    
    const char* materials[] = {"iron", "steel", "silver", "gold", "mithril", "adamantine", "divine"};
    const char* types[] = {"ring", "signet ring", "war ring", "ring of power", "divine ring"};
    
    int mat_idx = UMIN(rarity, 6);
    int type_idx = UMIN(rarity, 4);
    
    char name[256], short_desc[256], desc[256];
    sprintf(name, "%s %s", materials[mat_idx], types[type_idx]);
    sprintf(short_desc, "a %s %s", materials[mat_idx], types[type_idx]);
    sprintf(desc, "A %s %s lies here.", materials[mat_idx], types[type_idx]);
    
    free_string(obj->name);
    obj->name = str_dup(name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(short_desc);
    free_string(obj->description);
    obj->description = str_dup(desc);
}

/*
 * Weapon creation functions
 */
void make_weapon_sword(OBJ_DATA *obj, int level, int rarity)
{
    obj->item_type = ITEM_WEAPON;
    obj->value[0] = WEAPON_SWORD;
    obj->value[1] = 1;                 /* number of dice */
    obj->value[2] = 6;                 /* sides per die */
    obj->value[3] = 3;                 /* damage noun: "slash" */
    obj->weight = number_range(8, 20);
    obj->cost = number_range(level * 20, level * 100);
    
    const char* materials[] = {"iron", "steel", "fine steel", "mithril", "adamantine", "dragonbone", "divine"};
    const char* types[] = {"sword", "blade", "war sword", "battle blade", "sword of power", "divine sword"};
    
    int mat_idx = UMIN(rarity, 6);
    int type_idx = UMIN(rarity, 5);
    
    char name[256], short_desc[256], desc[256];
    sprintf(name, "%s %s", materials[mat_idx], types[type_idx]);
    sprintf(short_desc, "a %s %s", materials[mat_idx], types[type_idx]);
    sprintf(desc, "A %s %s lies here.", materials[mat_idx], types[type_idx]);
    
    free_string(obj->name);
    obj->name = str_dup(name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(short_desc);
    free_string(obj->description);
    obj->description = str_dup(desc);
}

void make_weapon_axe(OBJ_DATA *obj, int level, int rarity)
{
    obj->item_type = ITEM_WEAPON;
    obj->value[0] = WEAPON_AXE;
    obj->value[1] = 1;
    obj->value[2] = 8;
    obj->value[3] = 21;                /* damage noun: "cleave" */
    obj->weight = number_range(10, 25);
    obj->cost = number_range(level * 15, level * 80);
    
    const char* materials[] = {"iron", "steel", "fine steel", "mithril", "adamantine", "dragonbone", "divine"};
    const char* types[] = {"axe", "battle axe", "war axe", "great axe", "axe of power", "divine axe"};
    
    int mat_idx = UMIN(rarity, 6);
    int type_idx = UMIN(rarity, 5);
    
    char name[256], short_desc[256], desc[256];
    sprintf(name, "%s %s", materials[mat_idx], types[type_idx]);
    sprintf(short_desc, "a %s %s", materials[mat_idx], types[type_idx]);
    sprintf(desc, "A %s %s lies here.", materials[mat_idx], types[type_idx]);
    
    free_string(obj->name);
    obj->name = str_dup(name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(short_desc);
    free_string(obj->description);
    obj->description = str_dup(desc);
}

void make_weapon_mace(OBJ_DATA *obj, int level, int rarity)
{
    obj->item_type = ITEM_WEAPON;
    obj->value[0] = WEAPON_MACE;
    obj->value[1] = 1;
    obj->value[2] = 8;
    obj->value[3] = 27;                /* damage noun: "smash" */
    obj->weight = number_range(12, 30);
    obj->cost = number_range(level * 12, level * 60);
    
    const char* materials[] = {"iron", "steel", "fine steel", "mithril", "adamantine", "dragonbone", "divine"};
    const char* types[] = {"mace", "heavy mace", "war mace", "great mace", "mace of power", "divine mace"};
    
    int mat_idx = UMIN(rarity, 6);
    int type_idx = UMIN(rarity, 5);
    
    char name[256], short_desc[256], desc[256];
    sprintf(name, "%s %s", materials[mat_idx], types[type_idx]);
    sprintf(short_desc, "a %s %s", materials[mat_idx], types[type_idx]);
    sprintf(desc, "A %s %s lies here.", materials[mat_idx], types[type_idx]);
    
    free_string(obj->name);
    obj->name = str_dup(name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(short_desc);
    free_string(obj->description);
    obj->description = str_dup(desc);
}

void make_weapon_dagger(OBJ_DATA *obj, int level, int rarity)
{
    obj->item_type = ITEM_WEAPON;
    obj->value[0] = WEAPON_DAGGER;
    obj->value[1] = 1;
    obj->value[2] = 4;
    obj->value[3] = 11;                /* damage noun: "pierce" */
    obj->weight = number_range(2, 6);
    obj->cost = number_range(level * 8, level * 40);
    
    const char* materials[] = {"iron", "steel", "fine steel", "mithril", "adamantine", "dragonbone", "divine"};
    const char* types[] = {"dagger", "blade", "war dagger", "assassin's blade", "dagger of power", "divine dagger"};
    
    int mat_idx = UMIN(rarity, 6);
    int type_idx = UMIN(rarity, 5);
    
    char name[256], short_desc[256], desc[256];
    sprintf(name, "%s %s", materials[mat_idx], types[type_idx]);
    sprintf(short_desc, "a %s %s", materials[mat_idx], types[type_idx]);
    sprintf(desc, "A %s %s lies here.", materials[mat_idx], types[type_idx]);
    
    free_string(obj->name);
    obj->name = str_dup(name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(short_desc);
    free_string(obj->description);
    obj->description = str_dup(desc);
}

void make_weapon_spear(OBJ_DATA *obj, int level, int rarity)
{
    obj->item_type = ITEM_WEAPON;
    obj->value[0] = WEAPON_SPEAR;
    obj->value[1] = 1;
    obj->value[2] = 8;
    obj->value[3] = 33;                /* damage noun: "thrust" */
    obj->weight = number_range(8, 18);
    obj->cost = number_range(level * 15, level * 75);
    
    const char* materials[] = {"iron", "steel", "fine steel", "mithril", "adamantine", "dragonbone", "divine"};
    const char* types[] = {"spear", "lance", "war spear", "great spear", "spear of power", "divine spear"};
    
    int mat_idx = UMIN(rarity, 6);
    int type_idx = UMIN(rarity, 5);
    
    char name[256], short_desc[256], desc[256];
    sprintf(name, "%s %s", materials[mat_idx], types[type_idx]);
    sprintf(short_desc, "an %s %s", materials[mat_idx], types[type_idx]);
    sprintf(desc, "An %s %s lies here.", materials[mat_idx], types[type_idx]);
    
    free_string(obj->name);
    obj->name = str_dup(name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(short_desc);
    free_string(obj->description);
    obj->description = str_dup(desc);
}

void make_weapon_flail(OBJ_DATA *obj, int level, int rarity)
{
    obj->item_type = ITEM_WEAPON;
    obj->value[0] = WEAPON_FLAIL;
    obj->value[1] = 1;
    obj->value[2] = 8;
    obj->value[3] = 27;                /* damage noun: "smash" */
    obj->weight = number_range(12, 25);
    obj->cost = number_range(level * 18, level * 90);
    
    const char* materials[] = {"iron", "steel", "fine steel", "mithril", "adamantine", "dragonbone", "divine"};
    const char* types[] = {"flail", "war flail", "great flail", "flail of power", "divine flail"};
    
    int mat_idx = UMIN(rarity, 6);
    int type_idx = UMIN(rarity, 4);
    
    char name[256], short_desc[256], desc[256];
    sprintf(name, "%s %s", materials[mat_idx], types[type_idx]);
    sprintf(short_desc, "an %s %s", materials[mat_idx], types[type_idx]);
    sprintf(desc, "An %s %s lies here.", materials[mat_idx], types[type_idx]);
    
    free_string(obj->name);
    obj->name = str_dup(name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(short_desc);
    free_string(obj->description);
    obj->description = str_dup(desc);
}

void make_weapon_whip(OBJ_DATA *obj, int level, int rarity)
{
    obj->item_type = ITEM_WEAPON;
    obj->value[0] = WEAPON_WHIP;
    obj->value[1] = 1;
    obj->value[2] = 4;
    obj->value[3] = 4;                 /* damage noun: "whip" */
    obj->weight = number_range(2, 5);
    obj->cost = number_range(level * 10, level * 50);
    
    const char* materials[] = {"leather", "chain", "fine chain", "mithril", "adamantine", "dragonhide", "divine"};
    const char* types[] = {"whip", "war whip", "great whip", "whip of power", "divine whip"};
    
    int mat_idx = UMIN(rarity, 6);
    int type_idx = UMIN(rarity, 4);
    
    char name[256], short_desc[256], desc[256];
    sprintf(name, "%s %s", materials[mat_idx], types[type_idx]);
    sprintf(short_desc, "a %s %s", materials[mat_idx], types[type_idx]);
    sprintf(desc, "A %s %s lies here.", materials[mat_idx], types[type_idx]);
    
    free_string(obj->name);
    obj->name = str_dup(name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(short_desc);
    free_string(obj->description);
    obj->description = str_dup(desc);
}

void make_weapon_polearm(OBJ_DATA *obj, int level, int rarity)
{
    obj->item_type = ITEM_WEAPON;
    obj->value[0] = WEAPON_POLEARM;
    obj->value[1] = 1;
    obj->value[2] = 10;
    obj->value[3] = 15;                /* damage noun: "charge" */
    obj->weight = number_range(15, 35);
    obj->cost = number_range(level * 25, level * 125);
    
    const char* materials[] = {"iron", "steel", "fine steel", "mithril", "adamantine", "dragonbone", "divine"};
    const char* types[] = {"polearm", "halberd", "war polearm", "great polearm", "polearm of power", "divine polearm"};
    
    int mat_idx = UMIN(rarity, 6);
    int type_idx = UMIN(rarity, 5);
    
    char name[256], short_desc[256], desc[256];
    sprintf(name, "%s %s", materials[mat_idx], types[type_idx]);
    sprintf(short_desc, "a %s %s", materials[mat_idx], types[type_idx]);
    sprintf(desc, "A %s %s lies here.", materials[mat_idx], types[type_idx]);
    
    free_string(obj->name);
    obj->name = str_dup(name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(short_desc);
    free_string(obj->description);
    obj->description = str_dup(desc);
}

void make_weapon_hammer(OBJ_DATA *obj, int level, int rarity)
{
    obj->item_type = ITEM_WEAPON;
    obj->value[0] = WEAPON_HAMMER;
    obj->value[1] = 1;
    obj->value[2] = 8;
    obj->value[3] = 7;                 /* damage noun: "bludgeon" */
    obj->weight = number_range(12, 28);
    obj->cost = number_range(level * 15, level * 75);
    
    const char* materials[] = {"iron", "steel", "fine steel", "mithril", "adamantine", "dragonbone", "divine"};
    const char* types[] = {"hammer", "war hammer", "great hammer", "hammer of power", "divine hammer"};
    
    int mat_idx = UMIN(rarity, 6);
    int type_idx = UMIN(rarity, 4);
    
    char name[256], short_desc[256], desc[256];
    sprintf(name, "%s %s", materials[mat_idx], types[type_idx]);
    sprintf(short_desc, "a %s %s", materials[mat_idx], types[type_idx]);
    sprintf(desc, "A %s %s lies here.", materials[mat_idx], types[type_idx]);
    
    free_string(obj->name);
    obj->name = str_dup(name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(short_desc);
    free_string(obj->description);
    obj->description = str_dup(desc);
}

void make_weapon_longsword(OBJ_DATA *obj, int level, int rarity)
{
    obj->item_type = ITEM_WEAPON;
    obj->value[0] = WEAPON_LONGSWORD;
    obj->value[1] = 1;
    obj->value[2] = 8;
    obj->value[3] = 3;                 /* damage noun: "slash" */
    obj->weight = number_range(10, 22);
    obj->cost = number_range(level * 25, level * 125);
    
    const char* materials[] = {"iron", "steel", "fine steel", "mithril", "adamantine", "dragonbone", "divine"};
    const char* types[] = {"longsword", "war blade", "great sword", "longsword of power", "divine longsword"};
    
    int mat_idx = UMIN(rarity, 6);
    int type_idx = UMIN(rarity, 4);
    
    char name[256], short_desc[256], desc[256];
    sprintf(name, "%s %s", materials[mat_idx], types[type_idx]);
    sprintf(short_desc, "a %s %s", materials[mat_idx], types[type_idx]);
    sprintf(desc, "A %s %s lies here.", materials[mat_idx], types[type_idx]);
    
    free_string(obj->name);
    obj->name = str_dup(name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(short_desc);
    free_string(obj->description);
    obj->description = str_dup(desc);
}

void make_weapon_staff(OBJ_DATA *obj, int level, int rarity)
{
    obj->item_type = ITEM_WEAPON;
    obj->value[0] = WEAPON_STAFF;
    obj->value[1] = 1;
    obj->value[2] = 8;
    obj->value[3] = 7;                 /* damage noun: "bludgeon" */
    obj->weight = number_range(8, 20);
    obj->cost = number_range(level * 12, level * 60);
    
    const char* materials[] = {"oak", "ash", "yew", "mithril", "adamantine", "dragonbone", "divine"};
    const char* types[] = {"staff", "war staff", "great staff", "staff of power", "divine staff"};
    
    int mat_idx = UMIN(rarity, 6);
    int type_idx = UMIN(rarity, 4);
    
    char name[256], short_desc[256], desc[256];
    sprintf(name, "%s %s", materials[mat_idx], types[type_idx]);
    sprintf(short_desc, "an %s %s", materials[mat_idx], types[type_idx]);
    sprintf(desc, "An %s %s lies here.", materials[mat_idx], types[type_idx]);
    
    free_string(obj->name);
    obj->name = str_dup(name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(short_desc);
    free_string(obj->description);
    obj->description = str_dup(desc);
}

void make_weapon_bow(OBJ_DATA *obj, int level, int rarity)
{
    obj->item_type = ITEM_WEAPON;
    obj->value[0] = WEAPON_BOW;
    obj->value[1] = 1;
    obj->value[2] = 6;
    obj->value[3] = 11;                /* damage noun: "pierce" */
    obj->weight = number_range(3, 8);
    obj->cost = number_range(level * 15, level * 75);
    
    const char* materials[] = {"yew", "ash", "oak", "mithril", "adamantine", "dragonbone", "divine"};
    const char* types[] = {"bow", "longbow", "war bow", "great bow", "bow of power", "divine bow"};
    
    int mat_idx = UMIN(rarity, 6);
    int type_idx = UMIN(rarity, 5);
    
    char name[256], short_desc[256], desc[256];
    sprintf(name, "%s %s", materials[mat_idx], types[type_idx]);
    sprintf(short_desc, "a %s %s", materials[mat_idx], types[type_idx]);
    sprintf(desc, "A %s %s lies here.", materials[mat_idx], types[type_idx]);
    
    free_string(obj->name);
    obj->name = str_dup(name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(short_desc);
    free_string(obj->description);
    obj->description = str_dup(desc);
}

void make_weapon_crossbow(OBJ_DATA *obj, int level, int rarity)
{
    obj->item_type = ITEM_WEAPON;
    obj->value[0] = WEAPON_CROSSBOW;
    obj->value[1] = 1;
    obj->value[2] = 8;
    obj->value[3] = 11;                /* damage noun: "pierce" */
    obj->weight = number_range(8, 15);
    obj->cost = number_range(level * 20, level * 100);
    
    const char* materials[] = {"wood", "steel", "fine steel", "mithril", "adamantine", "dragonbone", "divine"};
    const char* types[] = {"crossbow", "heavy crossbow", "war crossbow", "great crossbow", "crossbow of power", "divine crossbow"};
    
    int mat_idx = UMIN(rarity, 6);
    int type_idx = UMIN(rarity, 5);
    
    char name[256], short_desc[256], desc[256];
    sprintf(name, "%s %s", materials[mat_idx], types[type_idx]);
    sprintf(short_desc, "a %s %s", materials[mat_idx], types[type_idx]);
    sprintf(desc, "A %s %s lies here.", materials[mat_idx], types[type_idx]);
    
    free_string(obj->name);
    obj->name = str_dup(name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(short_desc);
    free_string(obj->description);
    obj->description = str_dup(desc);
}

void make_weapon_club(OBJ_DATA *obj, int level, int rarity)
{
    obj->item_type = ITEM_WEAPON;
    obj->value[0] = WEAPON_CLUB;
    obj->value[1] = 1;
    obj->value[2] = 6;
    obj->value[3] = 7;                 /* damage noun: "bludgeon" */
    obj->weight = number_range(5, 12);
    obj->cost = number_range(level * 5, level * 25);
    
    const char* materials[] = {"wood", "iron", "steel", "mithril", "adamantine", "dragonbone", "divine"};
    const char* types[] = {"club", "war club", "great club", "club of power", "divine club"};
    
    int mat_idx = UMIN(rarity, 6);
    int type_idx = UMIN(rarity, 4);
    
    char name[256], short_desc[256], desc[256];
    sprintf(name, "%s %s", materials[mat_idx], types[type_idx]);
    sprintf(short_desc, "a %s %s", materials[mat_idx], types[type_idx]);
    sprintf(desc, "A %s %s lies here.", materials[mat_idx], types[type_idx]);
    
    free_string(obj->name);
    obj->name = str_dup(name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(short_desc);
    free_string(obj->description);
    obj->description = str_dup(desc);
}

void make_weapon_scimitar(OBJ_DATA *obj, int level, int rarity)
{
    obj->item_type = ITEM_WEAPON;
    obj->value[0] = WEAPON_SCIMITAR;
    obj->value[1] = 1;
    obj->value[2] = 6;
    obj->value[3] = 3;                 /* damage noun: "slash" */
    obj->weight = number_range(6, 15);
    obj->cost = number_range(level * 18, level * 90);
    
    const char* materials[] = {"iron", "steel", "fine steel", "mithril", "adamantine", "dragonbone", "divine"};
    const char* types[] = {"scimitar", "curved blade", "war scimitar", "great scimitar", "scimitar of power", "divine scimitar"};
    
    int mat_idx = UMIN(rarity, 6);
    int type_idx = UMIN(rarity, 5);
    
    char name[256], short_desc[256], desc[256];
    sprintf(name, "%s %s", materials[mat_idx], types[type_idx]);
    sprintf(short_desc, "a %s %s", materials[mat_idx], types[type_idx]);
    sprintf(desc, "A %s %s lies here.", materials[mat_idx], types[type_idx]);
    
    free_string(obj->name);
    obj->name = str_dup(name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(short_desc);
    free_string(obj->description);
    obj->description = str_dup(desc);
}