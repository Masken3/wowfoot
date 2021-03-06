#include "spellStrings.h"
#include <stddef.h>

const char* spellAuraName(int id) {
	switch(id) {
	case 0: return "None";
	case 1: return "Bind Sight";
	case 2: return "Mod Possess";
	case 3: return "Periodic Damage";
	case 4: return "Dummy";
	case 5: return "Mod Confuse";
	case 6: return "Mod Charm";
	case 7: return "Mod Fear";
	case 8: return "Periodic Heal";
	case 9: return "Mod Attack Speed";
	case 10: return "Mod Threat";
	case 11: return "Taunt";
	case 12: return "Stun";
	case 13: return "Mod Damage Done";
	case 14: return "Mod Damage Taken";
	case 15: return "Damage Shield";
	case 16: return "Mod Stealth";
	case 17: return "Mod Detect";
	case 18: return "Mod Invisibility";
	case 19: return "Mod Invisibility Detection";
	case 20: return "OBS Mod Intellect";
	case 21: return "OBS Mod Spirit";
	case 22: return "Mod Resistance";
	case 23: return "Periodic Trigger";
	case 24: return "Periodic Energize";
	case 25: return "Pacify";
	case 26: return "Root";
	case 27: return "Silence";
	case 28: return "Reflect Spells %";
	case 29: return "Mod Stat";
	case 30: return "Mod Skill";
	case 31: return "Mod Speed";
	case 32: return "Mod Speed Mounted";
	case 33: return "Mod Speed Slow";
	case 34: return "Mod Increase Health";
	case 35: return "Mod Increase Energy";
	case 36: return "Shapeshift";
	case 37: return "Immune Effect";
	case 38: return "Immune State";
	case 39: return "Immune School";
	case 40: return "Immune Damage";
	case 41: return "Immune Dispel Type";
	case 42: return "Proc Trigger Spell";
	case 43: return "Proc Trigger Damage";
	case 44: return "Track Creatures";
	case 45: return "Track Resources";
	case 46: return "Mod Parry Skill";
	case 47: return "Mod Parry Percent";
	case 48: return "Mod Dodge Skill";
	case 49: return "Mod Dodge Percent";
	case 50: return "Mod Block Skill";
	case 51: return "Mod Block Percent";
	case 52: return "Mod Crit Percent";
	case 53: return "Periodic Leech";
	case 54: return "Mod Hit Chance";
	case 55: return "Mod Spell Hit Chance";
	case 56: return "Transform";
	case 57: return "Mod Spell Crit Chance";
	case 58: return "Mod Speed Swim";
	case 59: return "Mod Creature Dmg Done";
	case 60: return "Pacify & Silence";
	case 61: return "Mod Scale";
	case 62: return "Periodic Health Funnel";
	case 63: return "Periodic Mana Funnel";
	case 64: return "Periodic Mana Leech";
	case 65: return "Haste - Spells";
	case 66: return "Feign Death";
	case 67: return "Disarm";
	case 68: return "Mod Stalked";
	case 69: return "School Absorb";
	case 70: return "Extra Attacks";
	case 71: return "Mod School Spell Crit Chance";
	case 72: return "Mod Power Cost";
	case 73: return "Mod School Power Cost";
	case 74: return "Reflect School Spells %";
	case 75: return "Mod Language";
	case 76: return "Far Sight";
	case 77: return "Immune Mechanic";
	case 78: return "Mounted";
	case 79: return "Mod Dmg %";
	case 80: return "Mod Stat %";
	case 81: return "Split Damage";
	case 82: return "Water Breathing";
	case 83: return "Mod Base Resistance";
	case 84: return "Mod Health Regen";
	case 85: return "Mod Power Regen";
	case 86: return "Create Death Item";
	case 87: return "Mod Dmg % Taken";
	case 88: return "Mod Health Regen Percent";
	case 89: return "Periodic Damage Percent";
	case 90: return "Mod Resist Chance";
	case 91: return "Mod Detect Range";
	case 92: return "Prevent Fleeing";
	case 93: return "Mod Uninteractible";
	case 94: return "Interrupt Regen";
	case 95: return "Ghost";
	case 96: return "Spell Magnet";
	case 97: return "Mana Shield";
	case 98: return "Mod Skill Talent";
	case 99: return "Mod Attack Power";
	case 100: return "Auras Visible";
	case 101: return "Mod Resistance %";
	case 102: return "Mod Creature Attack Power";
	case 103: return "Mod Total Threat (Fade)";
	case 104: return "Water Walk";
	case 105: return "Feather Fall";
	case 106: return "Hover";
	case 107: return "Add Flat Modifier";
	case 108: return "Add % Modifier";
	case 109: return "Add Class Target Trigger";
	case 110: return "Mod Power Regen %";
	case 111: return "Add Class Caster Hit Trigger";
	case 112: return "Override Class Scripts";
	case 113: return "Mod Ranged Dmg Taken";
	case 114: return "Mod Ranged % Dmg Taken";
	case 115: return "Mod Healing";
	case 116: return "Regen During Combat";
	case 117: return "Mod Mechanic Resistance";
	case 118: return "Mod Healing %";
	case 119: return "Share Pet Tracking";
	case 120: return "Untrackable";
	case 121: return "Empathy (Lore, whatever)";
	case 122: return "Mod Offhand Dmg %";
	case 123: return "Mod Power Cost %";
	case 124: return "Mod Ranged Attack Power";
	case 125: return "Mod Melee Dmg Taken";
	case 126: return "Mod Melee % Dmg Taken";
	case 127: return "Rngd Atk Pwr Attckr Bonus";
	case 128: return "Mod Possess Pet";
	case 129: return "Mod Speed Always";
	case 130: return "Mod Mounted Speed Always";
	case 131: return "Mod Creature Ranged Attack Power";
	case 132: return "Mod Increase Energy %";
	case 133: return "Mod Max Health %";
	case 134: return "Mod Interrupted Mana Regen";
	case 135: return "Mod Healing Done";
	case 136: return "Mod Healing Done %";
	case 137: return "Mod Total Stat %";
	case 138: return "Haste - Melee";
	case 139: return "Force Reaction";
	case 140: return "Haste - Ranged";
	case 141: return "Haste - Ranged (Ammo Only)";
	case 142: return "Mod Base Resistance %";
	case 143: return "Mod Resistance Exclusive";
	case 144: return "Safe Fall";
	case 145: return "Charisma";
	case 146: return "Persuaded";
	case 147: return "Add Creature Immunity";
	case 148: return "Retain Combo Points";
	case 149: return "Resist Pushback";
	case 150: return "Shield Block %";
	case 151: return "Track Stealthed";
	case 152: return "Mod Detected Range";
	case 153: return "Split Damage Flat";
	case 154: return "Stealth Level";
	case 155: return "Mod Water Breathing";
	case 156: return "Mod Reputation Gain";
	case 157: return "Mod Pet Damage";
	case 158: return "Mod Shield Block";
	case 159: return "No PvP Credit";
	case 160: return "Mod AoE Avoidance";
	case 161: return "Mod Health Regen in Combat";
	case 162: return "Burn Mana";
	case 163: return "Mod Crit Damage Bonus";
	case 164: return NULL;
	case 165: return "Melee Attack Power Attacker Bonus";
	case 166: return "Mod Attack Power %";
	case 167: return "Mod Ranged Attack Power %";
	case 168: return "Mod Damage Done Versus";
	case 169: return "Mod Crit % Versus";
	case 170: return "Detect Amore";
	case 171: return "Mod Speed (Not Stack)";
	case 172: return "Mod Mounted Speed (Not Stack)";
	case 173: return "Allow Champion Spells";
	case 174: return "Mod Spell Damage of Stat %";
	case 175: return "Mod Spell Healing of Stat %";
	case 176: return "Spirit of Redemption";
	case 177: return "AoE Charm";
	case 178: return "Mod Debuff Resistance";
	case 179: return "Mod Attacker Spell Crit Chance";
	case 180: return "Mod Flat Spell Damage Versus";
	case 181: return "Mod Flat Spell Crit Damage Versus";
	case 182: return "Mod Spell Resistance of Stat %";
	case 183: return "Mod Critical Threat";
	case 184: return "Mod Attacker Melee Hit Chance";
	case 185: return "Mod Attacker Ranged Hit Chance";
	case 186: return "Mod Attacker Spell Hit Chance";
	case 187: return "Mod Attacker Melee Crit Chance";
	case 188: return "Mod Attacker Ranged Crit Chance";
	case 189: return "Mod Rating";
	case 190: return "Mod Faction Reputation Gain";
	case 191: return "Use Normal Movement Speed";
	case 199: return "Mod Spell Hit Chance";
	default: return NULL;
	}
}

const char* spellEffectName(int id) {
	switch(id) {
	case 0: return "None";
	case 1: return "Instakill";
	case 2: return "School Damage";
	case 3: return "Dummy";
	case 4: return "Portal Teleport";
	case 5: return "Teleport Units";
	case 6: return "Apply Aura";
	case 7: return "Environmental Damage";
	case 8: return "Power Drain";
	case 9: return "Health Leech";
	case 10: return "Heal";
	case 11: return "Bind";
	case 12: return "Portal";
	case 13: return "Ritual Base";
	case 14: return "Ritual Specialize";
	case 15: return "Ritual Activate Portal";
	case 16: return "Quest Complete";
	case 17: return "Weapon Damage + noschool";
	case 18: return "Resurrect";
	case 19: return "Extra Attacks";
	case 20: return "Dodge";
	case 21: return "Evade";
	case 22: return "Parry";
	case 23: return "Block";
	case 24: return "Create Item";
	case 25: return "Weapon";
	case 26: return "Defense";
	case 27: return "Persistent Area Aura";
	case 28: return "Summon";
	case 29: return "Leap";
	case 30: return "Energize";
	case 31: return "Weapon % Dmg";
	case 32: return "Trigger Missile";
	case 33: return "Open Lock";
	case 34: return "Transform Item";
	case 35: return "Apply Area Aura";
	case 36: return "Learn Spell";
	case 37: return "Spell Defense";
	case 38: return "Dispel";
	case 39: return "Language";
	case 40: return "Dual Wield";
	case 41: return "Summon Wild";
	case 42: return "Summon Guardian";
	case 43: return "Summon Enemy Player";
	case 44: return "Skill Step";
	case 45: return "Add Honor";
	case 46: return "Spawn";
	case 47: return "Spell Cast UI";
	case 48: return "Stealth";
	case 49: return "Detect";
	case 50: return "Summon Object";
	case 51: return "Force Critical Hit";
	case 52: return "Guarantee Hit";
	case 53: return "Enchant Item Permanent";
	case 54: return "Enchant Item Temporary";
	case 55: return "Tame Creature";
	case 56: return "Summon Pet";
	case 57: return "Learn Pet Spell";
	case 58: return "Weapon Damage +";
	case 59: return "Open Lock Item";
	case 60: return "Proficiency";
	case 61: return "Send Event";
	case 62: return "Power Burn";
	case 63: return "Threat";
	case 64: return "Trigger Spell";
	case 65: return "Health Funnel";
	case 66: return "Power Funnel";
	case 67: return "Heal Max Health";
	case 68: return "Interrupt Cast";
	case 69: return "Distract";
	case 70: return "Pull";
	case 71: return "Pickpocket";
	case 72: return "Add Farsight";
	case 73: return "Summon Possessed";
	case 74: return "Summon Totem";
	case 75: return "Heal Mechanical";
	case 76: return "Summon Object Wild";
	case 77: return "Script Effect";
	case 78: return "Attack";
	case 79: return "Sanctuary";
	case 80: return "Add Combo Points";
	case 81: return "Create House";
	case 82: return "Bind Sight";
	case 83: return "Duel";
	case 84: return "Stuck";
	case 85: return "Summon Player";
	case 86: return "Activate Object";
	case 87: return "Summon Totem slot 1";
	case 88: return "Summon Totem slot 2";
	case 89: return "Summon Totem slot 3";
	case 90: return "Summon Totem slot 4";
	case 91: return "Threat All";
	case 92: return "Enchant Held Item";
	case 93: return "Summon Phantasm";
	case 94: return "Self Resurrect";
	case 95: return "Skinning";
	case 96: return "Charge";
	case 97: return "Summon Critter";
	case 98: return "Knock Back";
	case 99: return "Disenchant";
	case 100: return "Inebriate";
	case 101: return "Feed Pet";
	case 102: return "Dismiss Pet";
	case 103: return "Reputation";
	case 104: return "Summon Object slot 1";
	case 105: return "Summon Object slot 2";
	case 106: return "Summon Object slot 3";
	case 107: return "Summon Object slot 4";
	case 108: return "Dispel Mechanic";
	case 109: return "Summon Dead Pet";
	case 110: return "Destroy All Totems";
	case 111: return "Durability Damage";
	case 112: return "Summon Demon";
	case 113: return "Resurrect Flat";
	case 114: return "Taunt";
	case 115: return "Durability Damage";
	case 116: return "Remove Insignia";
	case 117: return "Spirit Heal";
	case 118: return "Require Skill";
	case 119: return "Apply Pet Aura";
	case 120: return "Graveyard Teleport";
	case 121: return "Normalized Weapon Damage +";

	case 123: return "Scripted Event";
	case 124: return "Player Pull";
	case 125: return "Reduce Threat %";
	case 126: return "Steal Beneficial Buff Magic";
	case 127: return "Require Ore";
	case 128: return "Apply Stat Aura";
	case 129: return "Apply Stat Aura %";

	case 133: return "Unlearn Profession Specialization";
	default: return NULL;
	}
}
