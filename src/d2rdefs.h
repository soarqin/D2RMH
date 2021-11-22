/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include <cstdint>

enum class StatId {
    /*   0 */ Strength,
    /*   1 */ Energy,
    /*   2 */ Dexterity,
    /*   3 */ Vitality,
    /*   4 */ Statpts,
    /*   5 */ Newskills,
    /*   6 */ Hitpoints,
    /*   7 */ Maxhp,
    /*   8 */ Mana,
    /*   9 */ Maxmana,
    /*  10 */ Stamina,
    /*  11 */ Maxstamina,
    /*  12 */ Level,
    /*  13 */ Experience,
    /*  14 */ Gold,
    /*  15 */ Goldbank,
    /*  16 */ ItemArmorPercent,
    /*  17 */ ItemMaxdamagePercent,
    /*  18 */ ItemMindamagePercent,
    /*  19 */ Tohit,
    /*  20 */ Toblock,
    /*  21 */ Mindamage,
    /*  22 */ Maxdamage,
    /*  23 */ SecondaryMindamage,
    /*  24 */ SecondaryMaxdamage,
    /*  25 */ Damagepercent,
    /*  26 */ Manarecovery,
    /*  27 */ Manarecoverybonus,
    /*  28 */ Staminarecoverybonus,
    /*  29 */ Lastexp,
    /*  30 */ Nextexp,
    /*  31 */ Armorclass,
    /*  32 */ ArmorclassVsMissile,
    /*  33 */ ArmorclassVsHth,
    /*  34 */ NormalDamageReduction,
    /*  35 */ MagicDamageReduction,
    /*  36 */ Damageresist,
    /*  37 */ Magicresist,
    /*  38 */ Maxmagicresist,
    /*  39 */ Fireresist,
    /*  40 */ Maxfireresist,
    /*  41 */ Lightresist,
    /*  42 */ Maxlightresist,
    /*  43 */ Coldresist,
    /*  44 */ Maxcoldresist,
    /*  45 */ Poisonresist,
    /*  46 */ Maxpoisonresist,
    /*  47 */ Damageaura,
    /*  48 */ Firemindam,
    /*  49 */ Firemaxdam,
    /*  50 */ Lightmindam,
    /*  51 */ Lightmaxdam,
    /*  52 */ Magicmindam,
    /*  53 */ Magicmaxdam,
    /*  54 */ Coldmindam,
    /*  55 */ Coldmaxdam,
    /*  56 */ Coldlength,
    /*  57 */ Poisonmindam,
    /*  58 */ Poisonmaxdam,
    /*  59 */ Poisonlength,
    /*  60 */ Lifedrainmindam,
    /*  61 */ Lifedrainmaxdam,
    /*  62 */ Manadrainmindam,
    /*  63 */ Manadrainmaxdam,
    /*  64 */ Stamdrainmindam,
    /*  65 */ Stamdrainmaxdam,
    /*  66 */ Stunlength,
    /*  67 */ Velocitypercent,
    /*  68 */ Attackrate,
    /*  69 */ OtherAnimrate,
    /*  70 */ Quantity,
    /*  71 */ Value,
    /*  72 */ Durability,
    /*  73 */ Maxdurability,
    /*  74 */ Hpregen,
    /*  75 */ ItemMaxdurabilityPercent,
    /*  76 */ ItemMaxhpPercent,
    /*  77 */ ItemMaxmanaPercent,
    /*  78 */ ItemAttackertakesdamage,
    /*  79 */ ItemGoldbonus,
    /*  80 */ ItemMagicbonus,
    /*  81 */ ItemKnockback,
    /*  82 */ ItemTimeduration,
    /*  83 */ ItemAddclassskills,
    /*  84 */ Unsentparam1,
    /*  85 */ ItemAddexperience,
    /*  86 */ ItemHealafterkill,
    /*  87 */ ItemReducedprices,
    /*  88 */ ItemDoubleherbduration,
    /*  89 */ ItemLightradius,
    /*  90 */ ItemLightcolor,
    /*  91 */ ItemReqPercent,
    /*  92 */ ItemLevelreq,
    /*  93 */ ItemFasterattackrate,
    /*  94 */ ItemLevelreqpct,
    /*  95 */ Lastblockframe,
    /*  96 */ ItemFastermovevelocity,
    /*  97 */ ItemNonclassskill,
    /*  98 */ State,
    /*  99 */ ItemFastergethitrate,
    /* 100 */ MonsterPlayercount,
    /* 101 */ SkillPoisonOverrideLength,
    /* 102 */ ItemFasterblockrate,
    /* 103 */ SkillBypassUndead,
    /* 104 */ SkillBypassDemons,
    /* 105 */ ItemFastercastrate,
    /* 106 */ SkillBypassBeasts,
    /* 107 */ ItemSingleskill,
    /* 108 */ ItemRestinpeace,
    /* 109 */ CurseResistance,
    /* 110 */ ItemPoisonlengthresist,
    /* 111 */ ItemNormaldamage,
    /* 112 */ ItemHowl,
    /* 113 */ ItemStupidity,
    /* 114 */ ItemDamagetomana,
    /* 115 */ ItemIgnoretargetac,
    /* 116 */ ItemFractionaltargetac,
    /* 117 */ ItemPreventheal,
    /* 118 */ ItemHalffreezeduration,
    /* 119 */ ItemTohitPercent,
    /* 120 */ ItemDamagetargetac,
    /* 121 */ ItemDemondamagePercent,
    /* 122 */ ItemUndeaddamagePercent,
    /* 123 */ ItemDemonTohit,
    /* 124 */ ItemUndeadTohit,
    /* 125 */ ItemThrowable,
    /* 126 */ ItemElemskill,
    /* 127 */ ItemAllskills,
    /* 128 */ ItemAttackertakeslightdamage,
    /* 129 */ IronmaidenLevel,
    /* 130 */ LifetapLevel,
    /* 131 */ ThornsPercent,
    /* 132 */ Bonearmor,
    /* 133 */ Bonearmormax,
    /* 134 */ ItemFreeze,
    /* 135 */ ItemOpenwounds,
    /* 136 */ ItemCrushingblow,
    /* 137 */ ItemKickdamage,
    /* 138 */ ItemManaafterkill,
    /* 139 */ ItemHealafterdemonkill,
    /* 140 */ ItemExtrablood,
    /* 141 */ ItemDeadlystrike,
    /* 142 */ ItemAbsorbfirePercent,
    /* 143 */ ItemAbsorbfire,
    /* 144 */ ItemAbsorblightPercent,
    /* 145 */ ItemAbsorblight,
    /* 146 */ ItemAbsorbmagicPercent,
    /* 147 */ ItemAbsorbmagic,
    /* 148 */ ItemAbsorbcoldPercent,
    /* 149 */ ItemAbsorbcold,
    /* 150 */ ItemSlow,
    /* 151 */ ItemAura,
    /* 152 */ ItemIndesctructible,
    /* 153 */ ItemCannotbefrozen,
    /* 154 */ ItemStaminadrainpct,
    /* 155 */ ItemReanimate,
    /* 156 */ ItemPierce,
    /* 157 */ ItemMagicarrow,
    /* 158 */ ItemExplosivearrow,
    /* 159 */ ItemThrowMindamage,
    /* 160 */ ItemThrowMaxdamage,
    /* 161 */ SkillHandofathena,
    /* 162 */ SkillStaminapercent,
    /* 163 */ SkillPassiveStaminapercent,
    /* 164 */ SkillConcentration,
    /* 165 */ SkillEnchant,
    /* 166 */ SkillPierce,
    /* 167 */ SkillConviction,
    /* 168 */ SkillChillingarmor,
    /* 169 */ SkillFrenzy,
    /* 170 */ SkillDecrepify,
    /* 171 */ SkillArmorPercent,
    /* 172 */ Alignment,
    /* 173 */ Target0,
    /* 174 */ Target1,
    /* 175 */ Goldlost,
    /* 176 */ ConversionLevel,
    /* 177 */ ConversionMaxhp,
    /* 178 */ UnitDooverlay,
    /* 179 */ AttackVsMontype,
    /* 180 */ DamageVsMontype,
    /* 181 */ Fade,
    /* 182 */ ArmorOverridePercent,
    /* 183 */ Unused183,
    /* 184 */ Unused184,
    /* 185 */ Unused185,
    /* 186 */ Unused186,
    /* 187 */ Unused187,
    /* 188 */ ItemAddskillTab,
    /* 189 */ Unused189,
    /* 190 */ Unused190,
    /* 191 */ Unused191,
    /* 192 */ Unused192,
    /* 193 */ Unused193,
    /* 194 */ ItemNumsockets,
    /* 195 */ ItemSkillonattack,
    /* 196 */ ItemSkillonkill,
    /* 197 */ ItemSkillondeath,
    /* 198 */ ItemSkillonhit,
    /* 199 */ ItemSkillonlevelup,
    /* 200 */ Unused200,
    /* 201 */ ItemSkillongethit,
    /* 202 */ Unused202,
    /* 203 */ Unused203,
    /* 204 */ ItemChargedSkill,
    /* 205 */ Unused205,
    /* 206 */ Unused206,
    /* 207 */ Unused207,
    /* 208 */ Unused208,
    /* 209 */ Unused209,
    /* 210 */ Unused210,
    /* 211 */ Unused211,
    /* 213 */ PassiveMasteryGethitRate,
    /* 213 */ PassiveMasteryAttackSpeed,
    /* 214 */ ItemArmorPerlevel,
    /* 215 */ ItemArmorpercentPerlevel,
    /* 216 */ ItemHpPerlevel,
    /* 217 */ ItemManaPerlevel,
    /* 218 */ ItemMaxdamagePerlevel,
    /* 219 */ ItemMaxdamagePercentPerlevel,
    /* 220 */ ItemStrengthPerlevel,
    /* 221 */ ItemDexterityPerlevel,
    /* 222 */ ItemEnergyPerlevel,
    /* 223 */ ItemVitalityPerlevel,
    /* 224 */ ItemTohitPerlevel,
    /* 225 */ ItemTohitpercentPerlevel,
    /* 226 */ ItemColdDamagemaxPerlevel,
    /* 227 */ ItemFireDamagemaxPerlevel,
    /* 228 */ ItemLtngDamagemaxPerlevel,
    /* 229 */ ItemPoisDamagemaxPerlevel,
    /* 230 */ ItemResistColdPerlevel,
    /* 231 */ ItemResistFirePerlevel,
    /* 232 */ ItemResistLtngPerlevel,
    /* 233 */ ItemResistPoisPerlevel,
    /* 234 */ ItemAbsorbColdPerlevel,
    /* 235 */ ItemAbsorbFirePerlevel,
    /* 236 */ ItemAbsorbLtngPerlevel,
    /* 237 */ ItemAbsorbPoisPerlevel,
    /* 238 */ ItemThornsPerlevel,
    /* 239 */ ItemFindGoldPerlevel,
    /* 240 */ ItemFindMagicPerlevel,
    /* 241 */ ItemRegenstaminaPerlevel,
    /* 242 */ ItemStaminaPerlevel,
    /* 243 */ ItemDamageDemonPerlevel,
    /* 244 */ ItemDamageUndeadPerlevel,
    /* 245 */ ItemTohitDemonPerlevel,
    /* 246 */ ItemTohitUndeadPerlevel,
    /* 247 */ ItemCrushingblowPerlevel,
    /* 248 */ ItemOpenwoundsPerlevel,
    /* 249 */ ItemKickDamagePerlevel,
    /* 250 */ ItemDeadlystrikePerlevel,
    /* 251 */ ItemFindGemsPerlevel,
    /* 252 */ ItemReplenishDurability,
    /* 253 */ ItemReplenishQuantity,
    /* 254 */ ItemExtraStack,
    /* 255 */ ItemFindItem,
    /* 256 */ ItemSlashDamage,
    /* 257 */ ItemSlashDamagePercent,
    /* 258 */ ItemCrushDamage,
    /* 259 */ ItemCrushDamagePercent,
    /* 260 */ ItemThrustDamage,
    /* 261 */ ItemThrustDamagePercent,
    /* 262 */ ItemAbsorbSlash,
    /* 263 */ ItemAbsorbCrush,
    /* 264 */ ItemAbsorbThrust,
    /* 265 */ ItemAbsorbSlashPercent,
    /* 266 */ ItemAbsorbCrushPercent,
    /* 267 */ ItemAbsorbThrustPercent,
    /* 268 */ ItemArmorBytime,
    /* 269 */ ItemArmorpercentBytime,
    /* 270 */ ItemHpBytime,
    /* 271 */ ItemManaBytime,
    /* 272 */ ItemMaxdamageBytime,
    /* 273 */ ItemMaxdamagePercentBytime,
    /* 274 */ ItemStrengthBytime,
    /* 275 */ ItemDexterityBytime,
    /* 276 */ ItemEnergyBytime,
    /* 277 */ ItemVitalityBytime,
    /* 278 */ ItemTohitBytime,
    /* 279 */ ItemTohitpercentBytime,
    /* 280 */ ItemColdDamagemaxBytime,
    /* 281 */ ItemFireDamagemaxBytime,
    /* 282 */ ItemLtngDamagemaxBytime,
    /* 283 */ ItemPoisDamagemaxBytime,
    /* 284 */ ItemResistColdBytime,
    /* 285 */ ItemResistFireBytime,
    /* 286 */ ItemResistLtngBytime,
    /* 287 */ ItemResistPoisBytime,
    /* 288 */ ItemAbsorbColdBytime,
    /* 289 */ ItemAbsorbFireBytime,
    /* 290 */ ItemAbsorbLtngBytime,
    /* 291 */ ItemAbsorbPoisBytime,
    /* 292 */ ItemFindGoldBytime,
    /* 293 */ ItemFindMagicBytime,
    /* 294 */ ItemRegenstaminaBytime,
    /* 295 */ ItemStaminaBytime,
    /* 296 */ ItemDamageDemonBytime,
    /* 297 */ ItemDamageUndeadBytime,
    /* 298 */ ItemTohitDemonBytime,
    /* 299 */ ItemTohitUndeadBytime,
    /* 300 */ ItemCrushingblowBytime,
    /* 301 */ ItemOpenwoundsBytime,
    /* 302 */ ItemKickDamageBytime,
    /* 303 */ ItemDeadlystrikeBytime,
    /* 304 */ ItemFindGemsBytime,
    /* 305 */ ItemPierceCold,
    /* 306 */ ItemPierceFire,
    /* 307 */ ItemPierceLtng,
    /* 308 */ ItemPiercePois,
    /* 309 */ ItemDamageVsMonster,
    /* 310 */ ItemDamagePercentVsMonster,
    /* 311 */ ItemTohitVsMonster,
    /* 312 */ ItemTohitPercentVsMonster,
    /* 313 */ ItemAcVsMonster,
    /* 314 */ ItemAcPercentVsMonster,
    /* 315 */ Firelength,
    /* 316 */ Burningmin,
    /* 317 */ Burningmax,
    /* 318 */ ProgressiveDamage,
    /* 319 */ ProgressiveSteal,
    /* 320 */ ProgressiveOther,
    /* 321 */ ProgressiveFire,
    /* 322 */ ProgressiveCold,
    /* 323 */ ProgressiveLightning,
    /* 324 */ ItemExtraCharges,
    /* 325 */ ProgressiveTohit,
    /* 326 */ PoisonCount,
    /* 327 */ DamageFramerate,
    /* 328 */ PierceIdx,
    /* 329 */ PassiveFireMastery,
    /* 330 */ PassiveLtngMastery,
    /* 331 */ PassiveColdMastery,
    /* 332 */ PassivePoisMastery,
    /* 333 */ PassiveFirePierce,
    /* 334 */ PassiveLtngPierce,
    /* 335 */ PassiveColdPierce,
    /* 336 */ PassivePoisPierce,
    /* 337 */ PassiveCriticalStrike,
    /* 338 */ PassiveDodge,
    /* 339 */ PassiveAvoid,
    /* 340 */ PassiveEvade,
    /* 341 */ PassiveWarmth,
    /* 342 */ PassiveMasteryMeleeTh,
    /* 343 */ PassiveMasteryMeleeDmg,
    /* 344 */ PassiveMasteryMeleeCrit,
    /* 345 */ PassiveMasteryThrowTh,
    /* 346 */ PassiveMasteryThrowDmg,
    /* 347 */ PassiveMasteryThrowCrit,
    /* 348 */ PassiveWeaponblock,
    /* 349 */ PassiveSummonResist,
    /* 350 */ ModifierlistSkill,
    /* 351 */ ModifierlistLevel,
    /* 352 */ LastSentHpPct,
    /* 353 */ SourceUnitType,
    /* 354 */ SourceUnitId,
    /* 355 */ Shortparam1,
    /* 356 */ Questitemdifficulty,
    /* 357 */ PassiveMagMastery,
    /* 358 */ PassiveMagPierce,
    /* 359 */ SkillCooldown,
    /* 360 */ SkillMissileDamageScale,
    TotalCount,
};

#pragma pack(push, 1)

struct DrlgRoom2 {
    uint64_t unk0[2];
    /* 0x10  DrlgRoom2 **pRoomsNear */
    uint64_t roomsNearListPtr;
    uint64_t unk1[5];
    /* 0x40  DWORD *levelDef */
    uint64_t levelPresetPtr;
    /* 0x48  DrlgRoom2 *pNext */
    uint64_t nextPtr;
    /* 0x50 */
    uint16_t roomsNear;
    uint16_t unk2;
    uint32_t roomTiles;
    /* 0x58  DrlgRoom1 *room1 */
    uint64_t room1Ptr;
    /* 0x60  DWORD dwPosX; DWORD dwPosY; DWORD dwSizeX; DWORD dwSizeY; */
    uint32_t posX, posY, sizeX, sizeY;
    /* 0x70 */
    uint32_t unk3;
    uint32_t presetType;
    /* 0x78  RoomTile *pRoomTiles */
    uint64_t roomTilesPtr;
    uint64_t unk4[2];
    /* 0x90  DrlgLevel *pLevel */
    uint64_t levelPtr;
    /* 0x98  PresetUnit *pPresetUnits */
    uint64_t presetUnitsPtr;
};

struct DrlgRoom1 {
    /* 0x00  DrlgRoom1 **pRoomsNear; */
    uint64_t roomsNearListPtr;
    uint64_t unk0[2];
    /* 0x18  DrlgRoom2 *pRoom2; */
    uint64_t room2Ptr;
    uint64_t unk1[4];
    /* 0x40 */
    uint32_t roomsNear;
    uint32_t unk2;
    /* 0x48  DrlgAct *pAct */
    uint64_t actAddr;
    uint64_t unk3[11];
    /* 0xA8  UnitAny *pUnitFirst */
    uint64_t unitFirstAddr;
    /* 0xB0  DrlgRoom1 *pNext */
    uint64_t nextPtr;
};

struct DynamicPath {
    uint16_t offsetX;
    uint16_t posX;
    uint16_t offsetY;
    uint16_t posY;
    uint32_t mapPosX;
    uint32_t mapPosY;
    uint32_t targetX;
    uint32_t targetY;
    uint32_t unk0[2];
    /* 0x20  DrlgRoom1 *pRoom1 */
    uint64_t room1Ptr;
};

struct StaticPath {
    /* DrlgRoom1 *pRoom1; */
    uint64_t room1Ptr;
    uint32_t mapPosX;
    uint32_t dwMapPosY;
    uint32_t posX;
    uint32_t posY;
};

struct DrlgAct {
    uint64_t unk0[2];
    uint32_t unk1;
    uint32_t seed;
    /* 0x18 DrlgRoom1 *room1 */
    uint64_t room1Ptr;
    uint32_t actId;
    uint32_t unk2;
    uint64_t unk3[9];
    /* DrlgMisc *misc */
    uint64_t miscPtr;
};

struct MonsterData {
    /* MonsterTxt *pMonsterTxt */
    uint64_t monsterTxtPtr;
    uint8_t components[16];
    uint16_t nameSeed;
    /*  struct MonsterFlag {
            BYTE fOther:1;  //set for some champs, uniques
            BYTE fUnique:1; //super unique
            BYTE fChamp:1;
            BYTE fBoss:1;   //unique monster ,usually boss
            BYTE fMinion:1;
            BYTE fPoss:1;   //possessed
            BYTE fGhost:1;  //ghostly
            BYTE fMulti:1;  //multishotfiring
        };
    */
    uint8_t flag;
    uint8_t lastMode;
    uint32_t duriel;
    uint8_t enchants[9];
    uint8_t unk0;
    uint16_t uniqueNo;
    uint32_t unk1;
    uint16_t unk2;
    uint16_t mercNameId;
    uint32_t unk3;
};

struct ItemData {
    uint32_t quality;
    uint32_t seedLow;
    uint32_t seedHigh;
    uint32_t ownerId;
    uint32_t fingerprint;
    uint32_t commandFlags;
    uint32_t itemFlags;
    uint64_t unk0[2];
    uint64_t actionStamp;
    uint32_t fileIndex; /* index from data files UniqueItems.txt, SetItems.txt, QualityItems.txt, LowQualityItems.txt */
    uint32_t itemLevel;
    uint16_t format;
    uint16_t rarePrefix;
    uint16_t rareSuffix;
    uint16_t autoPrefix;
    uint16_t magicPrefix[3];
    uint16_t magicSuffix[3];
    uint8_t bodyLocation; /* Id from BodyLocs.txt */
    uint8_t itemLocation; /* 0x80-ground  0xFF-equiped or on-hand  0-inv  3-cube  4-stash */
    uint16_t unk1;
    uint32_t unk2;
    uint16_t earLevel;
    uint8_t invGfxIdx;
    char playerName[16];
    uint8_t unk3[5];
    uint64_t ownerInvPtr;
    uint64_t prevItemPtr;
    uint64_t nextItemPtr;
    uint8_t unk4;
    uint8_t location; /* 0-ground 1-cube/stash/inv 2-belt 3-body 4-2nd hand */
    uint8_t unk5[6];
};

struct StatEx {
    uint16_t param;
    uint16_t stateId;
    uint32_t value;
};

struct StatList {
    /* Unit *pUnit */
    uint64_t unitPtr;
    uint32_t ownerType;
    uint32_t ownerId;
    uint32_t unk0[3];
    uint32_t flag;
    uint32_t stateNo;
    uint32_t expireFrame;
    uint32_t skillNo;
    uint32_t skillLevel;
    /* State sBaseStat */
    struct {
        /* StatEx *pStat */
        uint64_t statPtr;
        uint64_t statCount;
        uint64_t statCapacity;
        uint64_t unk2;
    } stat;
    uint64_t prevList;
    uint64_t nextList;
    uint64_t prevListEx;
    uint64_t nextListEx;
};

struct UnitInventory {
    uint32_t magic;
    uint32_t unk0;
    uint64_t ownerPtr;
    uint64_t firstItemPtr;
    uint64_t lastItemPtr;
    uint64_t invInfoPtr;
    uint64_t invInfoSize;
    uint64_t invInfoCapacity;
    uint32_t weaponId;
    uint32_t unk1;
    uint64_t cursorItemPtr;
    uint32_t ownerId;
    uint32_t filledSockets;
};

struct UnitAny {
    uint32_t unitType;
    uint32_t txtFileNo;
    uint32_t unitId;
    uint32_t mode;
    /* 0x10
    union {
        PlayerData *pPlayerData;
        MonsterData *pMonsterData;
        ObjectData *pObjectData;
        ItemData *pItemData;
        MissileData *pMissileData;
    };
     */
    uint64_t unionPtr;
    uint64_t unk0;
    /* 0x20  DrlgAct *pAct */
    uint64_t actPtr;
    uint64_t seed;
    /* 0x30 */
    uint64_t initSeed;
    /* 0x38  for Player/Monster: DynamicPath *pPath
     *       for Object:         StaticPath  *pPath */
    uint64_t pathPtr;
    /* 0x40 */
    uint32_t unk2[8];
    /* 0x60 */
    uint32_t gfxFrame;
    uint32_t frameRemain;
    /* 0x68 */
    uint32_t frameRate;
    uint32_t unk3;
    /* 0x70
    uint8_t *pGfxUnk;
    uint32_t *pGfxInfo;
     */
    uint64_t gfxUnkPtr;
    uint64_t gfxInfoPtr;
    /* 0x80 */
    uint64_t unk4;
    /* 0x88 StatList *pStats */
    uint64_t statListPtr;
    /* 0x90 UnitInventory *pInventory */
    uint64_t inventoryPtr;
    uint64_t unk5[23];
    uint64_t nextPtr;
    uint64_t roomNextPtr;
};

#pragma pack(pop)
