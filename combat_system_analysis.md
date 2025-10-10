# D&D-Style Combat System Implementation Plan

## Current Combat System Analysis

### Current Hit/Miss Mechanics (fight.c:one_hit)

**Current THAC0 System:**
- Uses `thac0_00` and `thac0_99` from class table
- Interpolates between these values based on level
- Applies skill-based modifiers (weapon skill/100)
- Uses hitroll bonuses from equipment
- Final calculation: `diceroll < thac0 - victim_ac` = miss

**Current AC System:**
- Four separate AC types: PIERCE, BASH, SLASH, EXOTIC
- Base AC starts at 100 (naked)
- Equipment reduces AC (better AC = lower number)
- DEX bonus applied via `dex_app[stat].defensive`
- Current formula: `GET_AC(ch,type) = armor[type] + dex_bonus`

**Current Damage System:**
- Weapon damage: `dice(value[1], value[2]) + value[5]` * skill/100
- Unarmed: `number_range(1 + 4*skill/100, 2*level/3*skill/100)`
- NPCs use hardcoded damage based on level
- Enhanced damage skill adds bonus damage
- Various special attack multipliers (backstab, assassinate)

### Current Class Structure

**Classes and BAB Categories:**
- **High BAB:** Barbarian, Fighter, Paladin, Ranger
- **Medium BAB:** Bard, Cleric, Druid, Monk, Rogue  
- **Low BAB:** Mage

**Current Class Table Structure:**
```c
struct class_type {
    char *name;           // "barbarian", "fighter", etc.
    char *who_name;       // "Brb", "Ftr", etc.
    char *plural_name;    // "barbarians", "fighters", etc.
    int attr_prime;       // Primary stat (STAT_STR, etc.)
    int first_weapon;     // Starting weapon vnum
    int max_guild[2];     // Guild limits
    int max_skill;        // Max skill percentage
    int thac0_00;         // THAC0 at level 1
    int thac0_99;         // THAC0 at level 20
    int hp_min;           // Min HP per level
    int hp_max;           // Max HP per level
    int flags;            // Class flags
    char *base_group;     // Base skill group
    char *default_group;  // Default skill group
    int *class_value;     // Pointer to class constant
    int tier;             // TIER_ONE or TIER_TWO
    int max_align;        // Max alignment
    int min_align;        // Min alignment
    int max_ethos;        // Max ethos
    int min_ethos;        // Min ethos
    int base_class;       // BASE_FIGHTER, BASE_MAGE, etc.
};
```

### Current Stat Modifier System

**Stat Mod Table (const.c:2140):**
```c
const int stat_mod[26] = {
    -999,  /*  0 - invalid */
    -4,    /*  1 */
    -4,    /*  2 */
    -3,    /*  3 */
    -3,    /*  4 */
    -2,    /*  5 */
    -2,    /*  6 */
    -1,    /*  7 */
    -1,    /*  8 */
    0,     /*  9 */
    0,     /* 10 */
    0,     /* 11 */
    1,     /* 12 */
    1,     /* 13 */
    2,     /* 14 */
    2,     /* 15 */
    3,     /* 16 */
    3,     /* 17 */
    4,     /* 18 */
    4,     /* 19 */
    5,     /* 20 */
    5,     /* 21 */
    6,     /* 22 */
    6,     /* 23 */
    7,     /* 24 */
    7      /* 25 */
};
```

### Current Weapon Value System

**Weapon Values:**
- `value[0]`: Weapon type (sword, mace, etc.)
- `value[1]`: Number of dice
- `value[2]`: Die type
- `value[3]`: Damage type (pierce, bash, slash)
- `value[4]`: Unused
- `value[5]`: Damage bonus

## New D&D-Style Combat System Design

### BAB Table Implementation

**BAB Categories:**
```c
typedef enum {
    BAB_HIGH,    // Barbarian, Fighter, Paladin, Ranger
    BAB_MEDIUM,  // Bard, Cleric, Druid, Monk, Rogue
    BAB_LOW      // Mage
} bab_category_t;

// BAB values by level (from BAB.txt)
const int bab_table[3][31] = {
    // HIGH BAB (Barbarian, Fighter, Paladin, Ranger)
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30},
    
    // MEDIUM BAB (Bard, Cleric, Druid, Monk, Rogue)
    {0, 0, 1, 2, 3, 3, 4, 5, 6, 6, 7, 8, 9, 9, 10, 11, 12, 12, 13, 14, 15, 15, 16, 17, 18, 18, 19, 20, 21, 21, 22},
    
    // LOW BAB (Mage)
    {0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15}
};

// Multiple attack progression (when additional attacks are gained)
const int multiple_attack_levels[3][5] = {
    // HIGH BAB: Level when each additional attack is gained
    {6, 11, 16, 21, 26},   // +6/+1, +11/+6/+1, +16/+11/+6/+1, +21/+16/+11/+6/+1, +26/+21/+16/+11/+6/+1
    
    // MEDIUM BAB: Level when each additional attack is gained  
    {8, 15, 22, 28, 30},   // +6/+1, +11/+6/+1, +16/+11/+6/+1, +21/+16/+11/+6/+1, +26/+21/+16/+11/+6/+1
    
    // LOW BAB: Level when each additional attack is gained
    {12, 20, 30, 30, 30}   // +6/+1, +10/+5, +15/+10/+5 (max 3 attacks)
};
```

### New Hit/Miss Mechanics

**Attack Roll Formula:**
```
Attack Roll = 1d20 + BAB + STR_mod + magical_bonus
Hit if: Attack Roll >= Target's AC
Critical Miss: Natural 1 (always misses)
Critical Hit: Natural 20 (requires confirmation roll)
```

**Multiple Attacks Per Round:**
- High BAB classes get multiple attacks at certain levels
- Level 6: +6/+1 (2 attacks)
- Level 11: +11/+6/+1 (3 attacks) 
- Level 16: +16/+11/+6/+1 (4 attacks)
- Level 21: +21/+16/+11/+6/+1 (5 attacks)
- Each attack is rolled independently with its own BAB
- Each attack can critical hit on natural 20

**Critical Hit Confirmation:**
```
Confirmation Roll = 1d20 + BAB + STR_mod + magical_bonus
If Confirmation Roll >= Target's AC: Critical Hit (double damage)
If Confirmation Roll < Target's AC: Normal Hit
```

**Dual Wielding:**
- Requires dual wielding skill
- Off-hand attack: 1d20 + (highest_BAB/2) + STR_mod + magical_bonus
- Off-hand gets only 1 attack per round (cannot be split)
- Off-hand can also critical hit on natural 20

**Attack Sequence Examples:**

*Level 20 Fighter (High BAB, Dual Wielding):*
- Main Hand Attack 1: 1d20 + 20 + STR_mod + magical_bonus
- Main Hand Attack 2: 1d20 + 15 + STR_mod + magical_bonus  
- Main Hand Attack 3: 1d20 + 10 + STR_mod + magical_bonus
- Main Hand Attack 4: 1d20 + 5 + STR_mod + magical_bonus
- Off-Hand Attack: 1d20 + 10 + STR_mod + magical_bonus (20/2 = 10)

*Level 15 Rogue (Medium BAB, Single Weapon):*
- Main Hand Attack 1: 1d20 + 11 + STR_mod + magical_bonus
- Main Hand Attack 2: 1d20 + 6 + STR_mod + magical_bonus

*Level 10 Mage (Low BAB, Single Weapon):*
- Main Hand Attack 1: 1d20 + 5 + STR_mod + magical_bonus

### New AC System

**AC Formula:**
```
AC = 10 + DEX_mod + armor_AC + other_bonuses
```

**AC Sources:**
- Base AC: 10 (all characters)
- DEX modifier: From stat_mod table
- Armor AC: Only from WEAR_BODY equipment (except monks)
- Other bonuses: Magical items (necklaces, bracers, etc.)

**Special Monk AC:**
```
Monk AC = 10 + DEX_mod + WIS_mod
```
- Monks get NO bonus from WEAR_BODY armor
- Monks get WIS modifier added to AC instead
- Example: Monk with 16 DEX (+3) and 18 WIS (+4) = AC 17

### New Damage System

**Damage Formula:**
```
Normal Damage = weapon_dice + STR_mod + magical_bonus
Critical Damage = (weapon_dice + STR_mod + magical_bonus) * 2
```

**Weapon Values (New):**
- `value[0]`: Weapon type (unchanged)
- `value[1]`: Number of dice (unchanged)
- `value[2]`: Die type (unchanged)
- `value[3]`: Damage type (unchanged)
- `value[4]`: Unused
- `value[5]`: Magical bonus (0-5, was damage bonus)

## Implementation Plan

### Phase 1: Data Structure Changes

1. **Add BAB Category to Class Table:**
   ```c
   // Add to class_type struct in merc.h
   bab_category_t bab_category;
   ```

2. **Update Class Definitions:**
   ```c
   // In const.c, add bab_category to each class
   {
       "barbarian", "Brb", "barbarians", STAT_STR, OBJ_VNUM_SCHOOL_SWORD,
       { 0, 0 }, 75, 100, 10, 1, 12, CLASS_NOFLAGS,
       "barbarian basics", "barbarian default", &class_barbarian,
       TIER_TWO, 1000, -1000, 1000, -1000, BASE_FIGHTER,
       BAB_HIGH  // NEW
   },
   ```

3. **Create BAB Lookup Functions:**
   ```c
   int get_bab(CHAR_DATA *ch) {
       int effective_class = get_effective_class(ch);
       int level = ch->class_levels[effective_class];
       bab_category_t category = class_table[effective_class].bab_category;
       return bab_table[category][level];
   }
   
   int get_attack_count(CHAR_DATA *ch) {
       int effective_class = get_effective_class(ch);
       int level = ch->class_levels[effective_class];
       bab_category_t category = class_table[effective_class].bab_category;
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
   
   int get_attack_bab(CHAR_DATA *ch, int attack_num) {
       int highest_bab = get_bab(ch);
       int effective_class = get_effective_class(ch);
       int level = ch->class_levels[effective_class];
       bab_category_t category = class_table[effective_class].bab_category;
       
       if (attack_num == 0) return highest_bab; // First attack
       
       // Calculate subsequent attack BABs
       int bab_decrement = 5; // Each attack is -5 BAB
       return highest_bab - (attack_num * bab_decrement);
   }
   ```

### Phase 2: Combat Function Replacement

1. **Replace one_hit() Function:**
   - Remove THAC0 calculations
   - Implement 1d20 + BAB + STR_mod + magical_bonus
   - Add critical hit/miss logic
   - Add critical confirmation roll
   - **NEW:** Implement multiple attacks per round
   - **NEW:** Add dual wielding support with off-hand attacks

2. **Update AC Calculation:**
   - Change base AC from 100 to 10
   - Only use WEAR_BODY for armor AC
   - Apply DEX modifier from stat_mod table
   - Keep magical item bonuses

3. **Update Damage Calculation:**
   - Remove skill-based damage scaling
   - Use weapon_dice + STR_mod + weapon_bonus
   - Implement critical damage doubling

### Phase 3: Multiclass Support

1. **Highest Level Class Logic:**
   - Use existing `get_effective_class()` function
   - This already returns the class with highest level
   - BAB will be based on this class's category and level

2. **Example Multiclass Scenarios:**
   - Fighter 5 / Mage 3: Uses Fighter BAB (High, level 5) = +5
   - Rogue 8 / Cleric 4: Uses Rogue BAB (Medium, level 8) = +6/+1
   - Barbarian 10 / Bard 6: Uses Barbarian BAB (High, level 10) = +10/+5

### Phase 4: Equipment Changes

1. **Armor AC Values:**
   - Set all non-body armor AC to 0 in OLC
   - Only WEAR_BODY equipment provides AC
   - Magical items can still provide AC bonuses

2. **Weapon Value[5] Change:**
   - Change from damage bonus to magical bonus (0-5)
   - Update all existing weapons in area files
   - Update OLC help text
   - Magical bonus applies to both attack and damage rolls

### Phase 5: Testing and Validation

1. **Combat Testing:**
   - Test hit/miss rates at various levels
   - Verify critical hit/miss mechanics
   - Test multiclass BAB calculations
   - Validate AC calculations

2. **Balance Testing:**
   - Compare damage output vs. current system
   - Test various class combinations
   - Verify equipment bonuses work correctly

## Files to Modify

### Core Combat Files:
- `fight.c` - Main combat logic
- `const.c` - Class table and BAB table
- `merc.h` - Data structure definitions

### Supporting Files:
- `handler.c` - AC calculation updates
- `act_info.c` - Display updates for new AC system
- `olc_obj.c` - OLC updates for weapon value[5] change

### Area Files:
- All `.are` files - Update weapon value[5] from damage to attack bonus
- All armor objects - Set non-body armor AC to 0

## Migration Strategy

1. **Backup Current System:**
   - Save current combat functions
   - Document current balance

2. **Gradual Implementation:**
   - Implement new system alongside old
   - Add debug commands to compare results
   - Switch over when validated

3. **Data Migration:**
   - Script to update weapon value[5] in area files (damage bonus → magical bonus)
   - Script to reset armor AC values
   - Update player character data if needed
   - Builder guidelines for magical item creation (total +5 budget)

## Magical Item Creation Guidelines

**Magical Bonus Budget System:**
- Total magical "budget" for any item: +5
- `value[5]` represents the magical bonus (0-5)
- Additional magical properties reduce the available bonus

**Examples:**
- **+5 Longsword:** `value[5] = 5` (no other properties)
- **Flaming +4 Longsword:** `value[5] = 4` + flaming property (total +5 budget)
- **Flaming +2 STR +3 Longsword:** `value[5] = 3` + flaming + +2 STR (total +5 budget)

**Builder Guidelines:**
- Pure magical bonus: `value[5] = 1-5`
- Magical bonus + 1 property: `value[5] = 1-4`
- Magical bonus + 2 properties: `value[5] = 1-3`
- And so on...

This system encourages thoughtful magical item design while maintaining balance.

## Expected Benefits

1. **D&D Authenticity:**
   - True to D&D 3.5/Pathfinder mechanics
   - Familiar to D&D players
   - Clear progression system

2. **Balance Improvements:**
   - More predictable hit rates
   - Better class differentiation
   - Clearer equipment value
   - Controlled magical item power

3. **Multiclass Support:**
   - Proper BAB progression
   - Encourages diverse builds
   - Maintains class identity

4. **Builder-Friendly:**
   - Clear magical item creation rules
   - Balanced power progression
   - Encourages creative item design

5. **Combat Depth:**
   - Multiple attacks per round for high-level characters
   - Dual wielding provides additional tactical options
   - Each attack can potentially critical hit
   - Dramatic high-level combat with multiple critical possibilities

## Risk Mitigation

1. **Compatibility:**
   - Keep old system as fallback
   - Extensive testing before switch
   - Gradual rollout

2. **Balance:**
   - Monitor combat effectiveness
   - Adjust BAB table if needed
   - Fine-tune AC values

3. **Player Impact:**
   - Clear communication of changes
   - Provide migration assistance
   - Maintain character effectiveness

This implementation plan provides a comprehensive roadmap for converting the current THAC0-based system to a modern D&D-style d20 system while maintaining compatibility with the existing multiclass system and equipment structure.

---

## Armor System Overhaul

### New ITEM_ARMOR Value Structure

**Current Armor Values (to be replaced):**
- `value[0]`: AC bonus (to be changed)
- `value[1]`: Unused
- `value[2]`: Unused  
- `value[3]`: Unused
- `value[4]`: Unused
- `value[5]`: Unused

**New Armor Values:**
- `value[0]`: Armor type (1=light, 2=medium, 3=heavy)
- `value[1]`: Armor bonus (0-8, added to AC)
- `value[2]`: Max DEX bonus (limits DEX modifier to AC/rolls)
- `value[3]`: Spell failure chance (percentage)
- `value[4]`: Unused
- `value[5]`: Unused

### Armor Type Definitions

**Light Armor (value[0] = 1):**
- Examples: Padded, Leather, Studded Leather
- Low AC bonus, high max DEX, low spell failure
- Good for spellcasters and agile characters

**Medium Armor (value[0] = 2):**
- Examples: Chain Shirt, Scale Mail, Breastplate
- Moderate AC bonus, moderate max DEX, moderate spell failure
- Balanced protection and mobility

**Heavy Armor (value[0] = 3):**
- Examples: Chain Mail, Splint Mail, Full Plate
- High AC bonus, low max DEX, high spell failure
- Maximum protection but restricts movement

### Armor Examples

**Padded Armor:**
- `value[0] = 1` (Light)
- `value[1] = 1` (+1 AC bonus)
- `value[2] = 8` (+8 max DEX)
- `value[3] = 5` (5% spell failure)

**Chain Shirt:**
- `value[0] = 2` (Medium)
- `value[1] = 4` (+4 AC bonus)
- `value[2] = 4` (+4 max DEX)
- `value[3] = 20` (20% spell failure)

**Full Plate:**
- `value[0] = 3` (Heavy)
- `value[1] = 8` (+8 AC bonus)
- `value[2] = 1` (+1 max DEX)
- `value[3] = 35` (35% spell failure)

### DEX Limiting System

**How Max DEX Works:**
- Character's DEX modifier is limited by armor's max DEX bonus
- Example: Character with 25 DEX (+7 modifier) wearing Full Plate (+1 max DEX)
- Result: Only +1 DEX modifier applies to AC and attack rolls
- Remaining +6 DEX modifier is ignored while wearing this armor

**AC Calculation with DEX Limits:**
```
Effective DEX = min(character_DEX_mod, armor_max_DEX)
AC = 10 + Effective DEX + armor_bonus + other_bonuses
```

### Spell Failure System

**Spell Failure Rules:**
- **Clerics and Paladins**: No spell failure from any armor
- **Bards**: Spell failure only applies to medium and heavy armor
- **All Other Classes**: Spell failure applies to all armor types

**Spell Failure Calculation:**
```
if (class == CLERIC || class == PALADIN) {
    spell_failure = 0;
} else if (class == BARD && armor_type == LIGHT) {
    spell_failure = 0;
} else {
    spell_failure = armor_value[3];
}
```

**Spell Failure Examples:**
- **Cleric in Full Plate**: 0% spell failure (immune)
- **Bard in Padded**: 0% spell failure (light armor)
- **Bard in Full Plate**: 35% spell failure (heavy armor)
- **Mage in Padded**: 5% spell failure (not immune)

### Monk AC Exception

**Monk AC Calculation:**
```
Monk AC = 10 + DEX_mod + WIS_mod
```

**Monk Armor Rules:**
- Monks get NO bonus from WEAR_BODY armor
- Monks get WIS modifier added to AC instead of armor bonus
- Monks have NO DEX limit (can use full DEX modifier)
- Example: Monk with 16 DEX (+3) and 18 WIS (+4) = AC 17

### Implementation Requirements

**Files to Modify:**
- `handler.c` - Update AC calculation functions
- `olc_obj.c` - Add armor value editing support
- `act_info.c` - Update AC display in score/who commands
- All area files - Update existing armor objects

**OLC Commands for Armor:**
```
> oedit <armor>
> v0 2          # Set armor type (1=light, 2=medium, 3=heavy)
> v1 4          # Set AC bonus (0-8)
> v2 4          # Set max DEX bonus (0-8)
> v3 20         # Set spell failure % (0-100)
```

**Migration Strategy:**
1. Update all existing armor objects in area files
2. Set non-body armor AC to 0 (only WEAR_BODY provides AC)
3. Add new armor value structure to OLC
4. Update AC calculation functions
5. Test with various class/armor combinations

This armor system overhaul brings authentic D&D mechanics to the combat system, making armor choices more strategic and meaningful while maintaining balance between protection and mobility.

---

## Turn-Based Combat System

### Overview
Implementation of a tactical turn-based combat system that transforms real-time combat into strategic, tabletop D&D-style encounters. Players get time to make tactical decisions, use healing items, and plan their actions.

### Combat Initiation
- **Command**: `kill <target>` initiates turn-based combat
- **State**: Characters enter special "in turn-based combat" state
- **Restriction**: Only combat commands available during this state

### Initiative System

**Initiative Roll Formula:**
```
PC Initiative = 1d20 + DEX_stat_mod
NPC Initiative = 1d20 + DEX_stat_mod
```

**Turn Order Determination:**
1. All participants roll initiative
2. Highest roll goes first, second highest goes second, etc.
3. **Tie-Breaking**: If same roll AND same DEX_mod, both reroll until different result
4. **Tie Resolution**: Winner of reroll goes in original slot, loser goes in next lower slot

**Example Initiative:**
```
Combat Participants: PC1 (Fighter), PC2 (Mage), NPC1 (Orc), NPC2 (Goblin)

Initiative Rolls:
- PC1: 1d20 + 3 = 18
- PC2: 1d20 + 1 = 12  
- NPC1: 1d20 + 0 = 15
- NPC2: 1d20 + 2 = 20

Turn Order: NPC2 (20) → PC1 (18) → NPC1 (15) → PC2 (12)
```

### Turn-Based Command System

**Main Combat Menu:**
```
[A]ttack - Melee attack (all attacks in one turn)
[S]kill - Use a skill
[C]ast - Cast a spell
[I]tem - Use an item
[E]nd - End turn
[F]lee - Flee from combat
```

**Command Details:**

**A - Attack:**
- Executes ALL attacks for the round (main hand + off-hand + multiple attacks)
- Uses `new_one_hit()` function with full BAB progression
- Ends turn, opponent gets their turn, back to main menu

**S - Skill:**
- Prompts for skill input or 'cancel'
- Validates skill availability and arguments
- Executes skill if valid, ends turn
- 'cancel' returns to main menu

**C - Cast:**
- Prompts for 'cast <spell>' or 'cancel'
- Validates spell availability and arguments
- Executes spell if valid, ends turn
- 'cancel' returns to main menu

**I - Item:**
- Allows only: quaff, zap, recite, brandish, drink, eat, cancel
- Useful for healing potions, scrolls, wands during combat
- Executes item use, ends turn
- 'cancel' returns to main menu

**E - End:**
- Skip turn, opponent gets their turn
- Back to main menu for next turn

**F - Flee:**
- Activates `do_flee` function
- Success: Exit combat, lose XP for fleeing
- Failure: End turn, opponent gets their turn, back to main menu

### Group Combat

**Multiple Participants:**
- All PCs and NPCs roll initiative
- Turn order determined by initiative rolls
- Each participant gets full turn before next participant

**Combat Flow:**
1. Player 1 turn (all attacks/skills/spells/items)
2. Player 2 turn (all attacks/skills/spells/items)
3. NPC 1 turn (automatic - attacks or special abilities)
4. NPC 2 turn (automatic - attacks or special abilities)
5. Back to Player 1 for next round

**NPC Behavior:**
- NPCs act automatically during their turns
- Use melee attacks unless they have special abilities
- Special NPCs (from `special.c`) use their special functions
- NPCs with offensive flags (OFF_BASH, etc.) use those abilities

### Combat End Conditions

**Combat Ends When:**
- All enemies are dead
- All PCs are dead
- All PCs successfully flee
- Manual combat termination (admin command)

**Post-Combat:**
- Experience, money, and loot awarded
- Characters return to normal command state
- Combat state flags cleared

### Implementation Structure

**Combat State Management:**
```c
typedef struct {
    CHAR_DATA **participants;    // Array of all combat participants
    int *initiative_rolls;       // Their initiative rolls
    int participant_count;       // Number of participants
    int current_turn;           // Index of whose turn it is
    bool combat_active;         // Is combat still ongoing?
} turn_combat_state_t;
```

**Key Functions:**
- `initiate_turn_combat()` - Start turn-based combat
- `roll_initiative()` - Calculate initiative for each participant
- `sort_by_initiative()` - Order participants by initiative
- `handle_turn_combat_command()` - Process combat commands
- `advance_turn()` - Move to next participant's turn
- `execute_npc_turn()` - Handle NPC automatic actions
- `end_turn_combat()` - Clean up and award rewards

### Integration with New Combat System

**Combat Mechanics:**
- **[A]ttack** uses `new_one_hit()` for all attacks in one turn
- **BAB progression** and **multiple attacks** work identically
- **Critical hits/misses** function the same way
- **Dual wielding** works the same way
- **Armor system** with DEX limits and spell failure works the same way

**Timing Changes:**
- Combat becomes turn-based instead of real-time
- Players get time to make tactical decisions
- Healing items become valuable during combat
- Strategic planning becomes important

### Benefits

**Tactical Depth:**
- Time to use healing potions, scrolls, wands during combat
- Strategic decision-making for each action
- Planning ahead for multiple turns

**D&D Authenticity:**
- Matches tabletop D&D pacing and decision-making
- Initiative-based turn order like tabletop
- Tactical combat similar to D&D sessions

**Combat Balance:**
- Reduced PC health makes tactical decisions crucial
- Healing items become valuable combat tools
- Combat duration matches tabletop D&D expectations

### Files to Modify

**Core Files:**
- `fight.c` - Add turn-based combat state management
- `interp.c` - Add combat command interception
- `merc.h` - Add combat state flags and structures
- `new_combat.c` - Integrate with turn-based system

**Supporting Files:**
- `act_move.c` - Update flee functionality
- `act_obj.c` - Add item use during combat
- `act_skills.c` - Add skill use during combat
- `magic.c` - Add spell casting during combat

This turn-based system transforms combat from real-time action to strategic, tabletop D&D-style encounters while maintaining all the new combat mechanics and armor system features.
