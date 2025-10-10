/*
 * New D&D-Style Combat System for Icewind Legacy
 * 
 * This file implements a complete D&D 3.5/Pathfinder style combat system
 * with BAB progression, multiple attacks per round, and dual wielding.
 * 
 * Author: Claude AI Assistant
 * Date: 2024
 */

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include "merc.h"

/* BAB Categories */
typedef enum {
    BAB_HIGH,    // Barbarian, Fighter, Paladin, Ranger
    BAB_MEDIUM,  // Bard, Cleric, Druid, Monk, Rogue
    BAB_LOW      // Mage
} bab_category_t;

/* BAB values by level (from BAB.txt) */
const int bab_table[3][31] = {
    // HIGH BAB (Barbarian, Fighter, Paladin, Ranger)
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30},
    
    // MEDIUM BAB (Bard, Cleric, Druid, Monk, Rogue)
    {0, 0, 1, 2, 3, 3, 4, 5, 6, 6, 7, 8, 9, 9, 10, 11, 12, 12, 13, 14, 15, 15, 16, 17, 18, 18, 19, 20, 21, 21, 22},
    
    // LOW BAB (Mage)
    {0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15}
};

/* Multiple attack progression (when additional attacks are gained) */
const int multiple_attack_levels[3][5] = {
    // HIGH BAB: Level when each additional attack is gained
    {6, 11, 16, 21, 26},   // +6/+1, +11/+6/+1, +16/+11/+6/+1, +21/+16/+11/+6/+1, +26/+21/+16/+11/+6/+1
    
    // MEDIUM BAB: Level when each additional attack is gained  
    {8, 15, 22, 28, 30},   // +6/+1, +11/+6/+1, +16/+11/+6/+1, +21/+16/+11/+6/+1, +26/+21/+16/+11/+6/+1
    
    // LOW BAB: Level when each additional attack is gained
    {12, 20, 30, 30, 30}   // +6/+1, +10/+5, +15/+10/+5 (max 3 attacks)
};

/* Class to BAB category mapping */
const bab_category_t class_bab_category[MAX_CLASS] = {
    BAB_HIGH,   // barbarian
    BAB_MEDIUM, // bard
    BAB_MEDIUM, // cleric
    BAB_MEDIUM, // druid
    BAB_HIGH,   // fighter
    BAB_LOW,    // mage
    BAB_MEDIUM, // monk
    BAB_HIGH,   // paladin
    BAB_HIGH,   // ranger
    BAB_MEDIUM  // rogue
};

/*
 * Get the BAB category for a character's effective class
 */
bab_category_t get_bab_category(CHAR_DATA *ch)
{
    int effective_class = get_effective_class(ch);
    if (effective_class < 0 || effective_class >= MAX_CLASS)
        return BAB_LOW; // Default to low BAB for safety
    
    return class_bab_category[effective_class];
}

/*
 * Get the base attack bonus for a character
 */
int get_bab(CHAR_DATA *ch)
{
    int effective_class = get_effective_class(ch);
    int level = ch->class_levels[effective_class];
    bab_category_t category = get_bab_category(ch);
    
    if (level < 0) level = 0;
    if (level > 30) level = 30;
    
    return bab_table[category][level];
}

/*
 * Get the number of attacks per round for a character
 */
int get_attack_count(CHAR_DATA *ch)
{
    int effective_class = get_effective_class(ch);
    int level = ch->class_levels[effective_class];
    bab_category_t category = get_bab_category(ch);
    int count = 1; // Always at least 1 attack
    
    for (int i = 0; i < 5; i++) {
        if (level >= multiple_attack_levels[category][i]) {
            count++;
        } else {
            break;
        }
    }
    return count;
}

/*
 * Get the BAB for a specific attack number (0 = first attack)
 */
int get_attack_bab(CHAR_DATA *ch, int attack_num)
{
    int highest_bab = get_bab(ch);
    
    if (attack_num == 0) return highest_bab; // First attack
    
    // Calculate subsequent attack BABs (each attack is -5 BAB)
    int bab_decrement = 5;
    return highest_bab - (attack_num * bab_decrement);
}

/*
 * Get the magical bonus from a weapon (value[5])
 */
int get_weapon_magical_bonus(OBJ_DATA *weapon)
{
    if (weapon == NULL || weapon->item_type != ITEM_WEAPON)
        return 0;
    
    return weapon->value[5];
}

/*
 * Calculate new-style AC for a character
 */
int get_new_ac(CHAR_DATA *ch)
{
    int ac = 10; // Base AC
    int effective_class = get_effective_class(ch);
    
    // Add DEX modifier
    ac += stat_mod[get_curr_stat(ch, STAT_DEX)];
    
    // Check if character is a monk
    if (effective_class == class_monk) {
        // Monks get WIS bonus to AC instead of armor
        ac += stat_mod[get_curr_stat(ch, STAT_WIS)];
        // Monks get NO bonus from WEAR_BODY armor
    } else {
        // Non-monks get armor AC (only from WEAR_BODY)
        OBJ_DATA *armor = get_eq_char(ch, WEAR_BODY);
        if (armor != NULL) {
            // For now, use the existing AC system but only from body armor
            // This will need to be updated when we change the AC system
            ac += armor->value[0]; // Assuming value[0] is AC for armor
        }
    }
    
    // Add magical AC bonuses from equipment
    // This will need to be implemented when we update the equipment system
    
    return ac;
}

/*
 * Perform a single attack roll
 */
bool perform_attack_roll(CHAR_DATA *ch, CHAR_DATA *victim, int bab, int magical_bonus)
{
    int attack_roll = number_range(1, 20); // 1d20
    int str_mod = stat_mod[get_curr_stat(ch, STAT_STR)];
    int total_attack = attack_roll + bab + str_mod + magical_bonus;
    int target_ac = get_new_ac(victim);
    
    // Log the attack for debugging
    sprintf(log_buf, "NEW COMBAT: %s attacks %s: d20=%d + BAB=%d + STR=%d + Magic=%d = %d vs AC %d: %s",
            PERS(ch, ch), PERS(victim, victim),
            attack_roll, bab, str_mod, magical_bonus, total_attack, target_ac,
            total_attack >= target_ac ? "HIT" : "MISS");
    wiznet(log_buf, NULL, NULL, WIZ_COMBAT, 0, 0);
    
    return total_attack >= target_ac;
}

/*
 * Check for critical hit
 */
bool check_critical_hit(CHAR_DATA *ch, CHAR_DATA *victim, int bab, int magical_bonus)
{
    // Critical hit confirmation roll
    int confirm_roll = number_range(1, 20); // 1d20
    int str_mod = stat_mod[get_curr_stat(ch, STAT_STR)];
    int total_confirm = confirm_roll + bab + str_mod + magical_bonus;
    int target_ac = get_new_ac(victim);
    
    sprintf(log_buf, "NEW COMBAT: Critical confirmation: d20=%d + BAB=%d + STR=%d + Magic=%d = %d vs AC %d: %s",
            confirm_roll, bab, str_mod, magical_bonus, total_confirm, target_ac,
            total_confirm >= target_ac ? "CRITICAL HIT" : "NORMAL HIT");
    wiznet(log_buf, NULL, NULL, WIZ_COMBAT, 0, 0);
    
    return total_confirm >= target_ac;
}

/*
 * Calculate damage for a hit
 */
int calculate_damage(CHAR_DATA *ch, OBJ_DATA *weapon, bool is_critical)
{
    int damage = 0;
    int str_mod = stat_mod[get_curr_stat(ch, STAT_STR)];
    int magical_bonus = get_weapon_magical_bonus(weapon);
    
    if (weapon != NULL && weapon->item_type == ITEM_WEAPON) {
        // Weapon damage: dice(value[1], value[2]) + STR_mod + magical_bonus
        damage = dice(weapon->value[1], weapon->value[2]) + str_mod + magical_bonus;
    } else {
        // Unarmed damage (simplified for now)
        damage = number_range(1, 4) + str_mod; // 1d4 + STR mod
    }
    
    if (is_critical) {
        damage *= 2; // Double damage for critical hits
    }
    
    // Ensure minimum damage
    if (damage < 1) damage = 1;
    
    return damage;
}

/*
 * Perform a single attack (main hand or off-hand)
 */
void perform_single_attack(CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *weapon, int bab, bool is_offhand)
{
    if (ch == NULL || victim == NULL) return;
    
    int magical_bonus = get_weapon_magical_bonus(weapon);
    int attack_roll = number_range(1, 20);
    
    // Check for critical miss (natural 1)
    if (attack_roll == 1) {
        damage(ch, victim, 0, TYPE_HIT, DAM_BASH, TRUE);
        return;
    }
    
    // Check for critical hit (natural 20)
    bool is_critical = (attack_roll == 20);
    if (is_critical) {
        is_critical = check_critical_hit(ch, victim, bab, magical_bonus);
    }
    
    // Perform attack roll
    if (perform_attack_roll(ch, victim, bab, magical_bonus)) {
        // Hit! Calculate and apply damage
        int dam = calculate_damage(ch, weapon, is_critical);
        
        // Apply damage using existing damage function
        damage(ch, victim, dam, TYPE_HIT, DAM_BASH, TRUE);
        
        if (is_critical) {
            act("`RYou score a CRITICAL HIT!`X", ch, NULL, victim, TO_CHAR);
            act("`R$n scores a CRITICAL HIT on you!`X", ch, NULL, victim, TO_VICT);
            act("`R$n scores a CRITICAL HIT on $N!`X", ch, NULL, victim, TO_NOTVICT);
        }
    } else {
        // Miss
        damage(ch, victim, 0, TYPE_HIT, DAM_BASH, TRUE);
    }
}

/*
 * Main combat function - replaces one_hit()
 */
void new_one_hit(CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary)
{
    OBJ_DATA *main_weapon, *off_weapon;
    int attack_count, i;
    
    // Basic safety checks
    if (victim == ch || ch == NULL || victim == NULL) return;
    if (victim->position == POS_DEAD || ch->in_room != victim->in_room) return;
    
    // Get weapons
    main_weapon = get_eq_char(ch, WEAR_WIELD);
    off_weapon = get_eq_char(ch, WEAR_DUAL);
    
    // Perform main hand attacks
    attack_count = get_attack_count(ch);
    for (i = 0; i < attack_count; i++) {
        int bab = get_attack_bab(ch, i);
        perform_single_attack(ch, victim, main_weapon, bab, FALSE);
        
        // Check if victim is still alive after each attack
        if (victim->position == POS_DEAD) break;
    }
    
    // Perform off-hand attack if dual wielding
    if (off_weapon != NULL && get_skill(ch, gsn_dual) > 0) {
        int highest_bab = get_bab(ch);
        int offhand_bab = highest_bab / 2; // Half of highest BAB
        
        perform_single_attack(ch, victim, off_weapon, offhand_bab, TRUE);
    }
}

/*
 * Initialize the new combat system
 */
void init_new_combat_system(void)
{
    // This function can be used to initialize any global combat system data
    // For now, it's just a placeholder
    return;
}

/*
 * Test command to verify the new combat system works
 */
void do_test_new_combat(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int effective_class, level, bab, attack_count;
    
    if (IS_NPC(ch)) {
        send_to_char("Only players can use this command.\n\r", ch);
        return;
    }
    
    effective_class = get_effective_class(ch);
    level = ch->class_levels[effective_class];
    bab = get_bab(ch);
    attack_count = get_attack_count(ch);
    
    sprintf(buf, "`WNew Combat System Test:`X\n\r");
    send_to_char(buf, ch);
    
    sprintf(buf, "`GClass:`X %s (Level %d)\n\r", 
            class_table[effective_class].name, level);
    send_to_char(buf, ch);
    
    sprintf(buf, "`GBAB Category:`X %s\n\r", 
            get_bab_category(ch) == BAB_HIGH ? "High" :
            get_bab_category(ch) == BAB_MEDIUM ? "Medium" : "Low");
    send_to_char(buf, ch);
    
    sprintf(buf, "`GBAB:`X +%d\n\r", bab);
    send_to_char(buf, ch);
    
    sprintf(buf, "`GAttacks per round:`X %d\n\r", attack_count);
    send_to_char(buf, ch);
    
    sprintf(buf, "`GAttack sequence:`X ");
    for (int i = 0; i < attack_count; i++) {
        int attack_bab = get_attack_bab(ch, i);
        sprintf(buf + strlen(buf), "+%d", attack_bab);
        if (i < attack_count - 1) strcat(buf, "/");
    }
    strcat(buf, "\n\r");
    send_to_char(buf, ch);
    
    sprintf(buf, "`GNew AC:`X %d", get_new_ac(ch));
    if (effective_class == class_monk) {
        sprintf(buf + strlen(buf), " (10 + DEX%d + WIS%d)", 
                stat_mod[get_curr_stat(ch, STAT_DEX)],
                stat_mod[get_curr_stat(ch, STAT_WIS)]);
    }
    strcat(buf, "\n\r");
    send_to_char(buf, ch);
    
    OBJ_DATA *weapon = get_eq_char(ch, WEAR_WIELD);
    if (weapon != NULL) {
        sprintf(buf, "`GMain weapon magical bonus:`X +%d\n\r", 
                get_weapon_magical_bonus(weapon));
        send_to_char(buf, ch);
    }
    
    OBJ_DATA *off_weapon = get_eq_char(ch, WEAR_DUAL);
    if (off_weapon != NULL) {
        sprintf(buf, "`GOff-hand weapon magical bonus:`X +%d\n\r", 
                get_weapon_magical_bonus(off_weapon));
        send_to_char(buf, ch);
        
        if (get_skill(ch, gsn_dual) > 0) {
            sprintf(buf, "`GDual wield skill:`X %d%%\n\r", get_skill(ch, gsn_dual));
            send_to_char(buf, ch);
        }
    }
}
