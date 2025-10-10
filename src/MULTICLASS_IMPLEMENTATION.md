# Multiclass System Implementation - Complete

## Overview
Successfully implemented a comprehensive multiclass system for the MUD codebase with player-initiated leveling, experience penalties, and full OLC support for manual creation.

## Core System Features

### Character Progression
- Characters select primary class at creation
- Can add up to 2 additional classes (3 total classes maximum)
- Total character level capped at 20
- Experience formula: `N*(N-1)*500` base XP with 20% penalty per extra class
- Player-initiated leveling via `levelup` command with `PLR_NOEXP` flag
- Multiclass menu system for class selection during advancement

### Experience System
- Base XP for level N: `N * (N-1) * 500`
- Multiclass penalty: +20% XP per extra class
- Example: Level 3 fighter (6000 XP) vs 1 fighter/1 rogue/1 barbarian (8400 XP)

## Data Structures Added

### CHAR_DATA Fields
```c
int multiclass_count;           // Number of classes (1-3)
int primary_class;              // Original class from creation
int secondary_class;            // Second class (0 if none)
int tertiary_class;             // Third class (0 if none)
int class_levels[MAX_CLASS];    // Individual class levels
```

### PC_DATA Fields
```c
bool pending_class_choice;      // Flag for menu interaction
```

## Key Commands & Functions

### New Commands
- `levelup` - Triggers multiclass selection menu
- `multiclass <number>` - Handles class choice input

### Updated Commands
- `do_skills` - Shows all available skills from all classes
- `do_spells` - Shows all available spells from all classes  
- `do_practice` - Allows practicing skills from all classes
- `do_study` - Enforces class restrictions on learning spells
- `show_score` - Displays multiclass info with short class names
- `do_who` - Shows multiclass status

## New Item Type - Manuals

### Manual System
- Added `ITEM_MANUAL` (49) for learning non-magic skills
- Full OLC support for creating manuals
- `do_study` command supports manuals with class restrictions
- Manuals teach skills (not spells) and respect class availability

### OLC Manual Creation
```
> oedit create
> type manual
> name a manual of axe fighting
> v0 1          # Required level
> v1 axe        # Primary skill
> v2 none       # Secondary skill
> v3 none       # Tertiary skill
> save
```

## Files Modified

### Core Files
- **merc.h** - Added multiclass fields and function prototypes
- **multiclass.c** - New file with all multiclass logic
- **update.c** - Modified `gain_exp` and `advance_level`
- **skills.c** - Updated `exp_per_level` and `do_skills`
- **save.c** - Added multiclass save/load with backward compatibility

### Display Files
- **act_info.c** - Updated `do_who`, `do_score`, `do_practice`
- **act_obj.c** - Added manual support to `do_study`
- **comm.c** - Fixed `%X` prompt for multiclass experience

### OLC Files
- **tables.c** - Added manual to `item_types`
- **olc_obj.c** - Added manual OLC support with skill validation
- **db.c** - Fixed manual loading in `Read_object`

## Helper Functions

### Core Functions
- `get_total_level()` - Calculates total character level
- `add_class()` - Adds new class to character
- `show_multiclass_choices()` - Displays class selection menu
- `handle_multiclass_levelup()` - Processes class choice

### Skill Functions
- `is_skill_available_to_multiclass()` - Checks if skill available to any class
- `get_multiclass_skill_level()` - Gets minimum skill level requirement
- `multiclass_meets_skill_level()` - Checks if class level meets requirement
- `get_multiclass_skill_rating()` - Gets skill rating from appropriate class

### Utility Functions
- `recalculate_multiclass_count()` - Updates multiclass count from class_levels
- `get_multiclass_display()` - Formats multiclass display for commands
- `get_multiclass_exp_requirement()` - Calculates XP with penalties

## Experience Calculation

### Base Formula
```c
int get_base_exp_for_level(int level) {
    if (level <= 1) return 0;
    if (level > 20) return 999999;
    return level * (level - 1) * 500;
}
```

### Multiclass Penalty
```c
int get_multiclass_exp_requirement(CHAR_DATA *ch, int target_level) {
    int base_exp = get_base_exp_for_level(target_level);
    int penalty = (ch->multiclass_count - 1) * 20;  // 20% per extra class
    return base_exp + (base_exp * penalty / 100);
}
```

## Leveling Process

1. Character gains enough XP to level up
2. `PLR_NOEXP` flag is set (stops XP gain)
3. Player types `levelup` command
4. Multiclass menu displays available options
5. Player types `multiclass <number>` to choose
6. Character levels up in chosen class
7. `PLR_NOEXP` flag is removed
8. Character continues gaining XP

## Class Restrictions

### Skill/Spell Availability
- Skills/spells must be available to at least one of character's classes
- Level requirements checked against individual class levels
- `do_study` enforces class restrictions for learning from items

### Manual Learning
- Manuals can only teach skills (not spells)
- Skills must be available to character's classes
- Example: Barbarian can't learn meditation from manual

## Backward Compatibility

### Existing Characters
- Single-class characters automatically initialized with multiclass data
- `init_multiclass()` called for new characters
- `recalculate_multiclass_count()` called for existing multiclass characters
- No "no match" bug messages for missing multiclass fields

## Testing Status

### Completed Features
- ✅ Multiclass leveling system
- ✅ Experience calculation with penalties
- ✅ Skill/spell display and practice
- ✅ Class restrictions on learning
- ✅ Manual item type and OLC support
- ✅ Save/load system with backward compatibility
- ✅ Display functions (score, who, prompt)
- ✅ All compilation errors fixed

### Production Ready
The multiclass system is fully functional and production-ready with proper class restrictions, experience penalties, skill/spell availability, and complete OLC support for manual creation.

## Usage Examples

### Creating a Manual
```
> oedit create
> type manual
> name a manual of axe fighting
> short a manual of axe fighting
> long A detailed manual teaching the art of axe combat.
> v0 1
> v1 axe
> save
```

### Leveling Up
```
> levelup
Choose your class advancement:
1. Fighter (level 3)
2. Barbarian (level 2)
3. Add new class: Ranger
Your choice? Type 'multiclass <number>'
> multiclass 1
You advance to level 4 Fighter!
```

### Studying Manuals
```
> study manual
You study a manual of axe fighting.
You have learned the art of axe!
```

The system is complete and ready for use!

---

# New D&D-Style Combat System Implementation

## Overview
Implemented a complete D&D 3.5/Pathfinder-style combat system to replace the existing THAC0-based system. The new system includes Base Attack Bonus (BAB) progression, multiple attacks per round, critical hits/misses, and authentic armor mechanics.

## Core Combat Mechanics

### Base Attack Bonus (BAB) System
- **High BAB**: Barbarian, Fighter, Paladin, Ranger (+1 per level)
- **Medium BAB**: Bard, Cleric, Druid, Monk, Rogue (+3/4 per level)  
- **Low BAB**: Mage (+1/2 per level)

### Attack Rolls
- Formula: `1d20 + BAB + STR_mod + weapon_magical_bonus`
- Natural 1: Automatic miss (critical miss)
- Natural 20: Triggers critical hit confirmation roll
- Critical confirmation: `1d20 + BAB + STR_mod + magical_bonus` vs AC
- Confirmed critical: Double damage, otherwise normal hit

### Multiple Attacks
- Based on BAB progression and character level
- High BAB: 6 attacks at level 26+ (+26/+21/+16/+11/+6/+1)
- Medium BAB: 3 attacks at level 22+ (+15/+10/+5)
- Low BAB: 2 attacks at level 20+ (+10/+5)

### Dual Wielding
- Off-hand attack at 1/2 highest BAB
- Cannot be split into additional attacks
- Requires `gsn_dual` skill

## Armor System Overhaul

### ITEM_ARMOR Value Structure
- **value[0]**: Armor type (light/medium/heavy)
- **value[1]**: Armor bonus (0-8, added to AC)
- **value[2]**: Max DEX bonus (limits DEX modifier to AC/rolls)
- **value[3]**: Spell failure chance (percentage)

### Armor Examples
- **Padded Armor**: Light, +1 AC, +8 max DEX, 5% spell failure
- **Full Plate**: Heavy, +8 AC, +1 max DEX, 35% spell failure

### Spell Failure Rules
- **Clerics/Paladins**: No spell failure from armor
- **Bards**: Spell failure only for medium/heavy armor
- **Other classes**: Spell failure applies to all armor types

### AC Calculation
- **Base AC**: 10 + DEX_mod + armor_bonus
- **DEX limit**: Cannot exceed armor's max DEX bonus
- **Monks**: 10 + DEX_mod + WIS_mod (no armor bonus, no DEX limit)

## Implementation Details

### Files Created/Modified
- **new_combat.c**: Complete new combat system
- **merc.h**: Added function declarations
- **interp.c**: Added `testnewcombat` command
- **Makefile**: Added new_combat.c to build

### Key Functions
- `new_one_hit()`: Main combat entry point
- `get_bab()`: Calculates BAB from highest class level
- `get_attack_count()`: Determines number of attacks per round
- `get_new_ac()`: Calculates new-style AC with armor limits
- `perform_attack_roll()`: Handles attack rolls and criticals
- `calculate_damage()`: Computes damage with critical multipliers

### Multiclass Integration
- Uses highest class level for BAB calculation
- Maintains existing multiclass character progression
- Seamless integration with current leveling system

## Testing Status

### Completed Features
- ✅ BAB progression tables and lookup functions
- ✅ Multiple attack system with proper sequencing
- ✅ Critical hit/miss mechanics with confirmation
- ✅ Dual wielding support
- ✅ Monk AC calculation (10 + DEX + WIS)
- ✅ Test command for verification
- ✅ Clean compilation with no errors

### Ready for Integration
The new combat system is complete and ready to replace the existing THAC0 system. It can be integrated by:
1. Commenting out old combat calls in `fight.c`
2. Replacing with `new_one_hit()` calls
3. Testing in-game combat scenarios
4. Rolling back easily if needed (modular design)

## Usage Examples

### Testing Combat Stats
```
> testnewcombat
New Combat System Test:
Class: fighter (Level 7)
BAB Category: High
BAB: +7
Attacks per round: 2
Attack sequence: +7/+2
New AC: 17 (10 + DEX3 + armor4)
```

### Armor System
```
> oedit <armor>
> v0 2          # Heavy armor
> v1 8          # +8 AC bonus
> v2 1          # +1 max DEX
> v3 35         # 35% spell failure
```

The new combat system brings authentic D&D mechanics to Icewind Legacy while maintaining compatibility with the existing multiclass system.

