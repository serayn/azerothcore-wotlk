/*
 * Copyright (C) 2016+     AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-GPL2
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 */

#ifndef TRINITY_FORMULAS_H
#define TRINITY_FORMULAS_H

#include "World.h"
#include "SharedDefines.h"
#include "ScriptMgr.h"
#include "Player.h"
#include "Creature.h"

namespace Trinity
{
    namespace Honor
    {
        inline float hk_honor_at_level_f(uint8 level, float multiplier = 1.0f)
        {
            bool SkipCoreCode = false; float honor = 0.0f;
            sScriptMgr->OnHonorCalculation(SkipCoreCode, honor, level, multiplier);
            if(!SkipCoreCode)  honor = multiplier * level * 1.55f;
            return honor;
        }

        inline uint32 hk_honor_at_level(uint8 level, float multiplier = 1.0f)
        {
            return uint32(ceil(hk_honor_at_level_f(level, multiplier)));
        }
    }
    namespace XP
    {
        inline uint8 GetGrayLevel(uint8 pl_level)
        {
            uint8 level;
            bool SkipCoreCode = false;
            sScriptMgr->OnGrayLevelCalculation(SkipCoreCode,level, pl_level);
            if(!SkipCoreCode){
            if (pl_level <= 5)
                level = 0;
            else if (pl_level <= 39)
                level = pl_level - 5 - pl_level / 10;
            else if (pl_level <= 59)
                level = pl_level - 1 - pl_level / 5;
            else
                level = pl_level - 9;
            }
            
            return level;
        }

        inline XPColorChar GetColorCode(uint8 pl_level, uint8 mob_level)
        {
            XPColorChar color; bool SkipCoreCode = false;
            sScriptMgr->OnColorCodeCalculation(SkipCoreCode, color, pl_level, mob_level); 
            if (!SkipCoreCode) {
                if (mob_level >= pl_level + 5)
                    color = XP_RED;
                else if (mob_level >= pl_level + 3)
                    color = XP_ORANGE;
                else if (mob_level >= pl_level - 2)
                    color = XP_YELLOW;
                else if (mob_level > GetGrayLevel(pl_level))
                    color = XP_GREEN;
                else
                    color = XP_GRAY;

            }
            return color;
        }

        inline uint8 GetZeroDifference(uint8 pl_level)
        {
            uint8 diff;
            bool SkipCoreCode = false;
            sScriptMgr->OnZeroDifferenceCalculation(SkipCoreCode, diff, pl_level); // pussywizard: optimization
            if (!SkipCoreCode) {
                if (pl_level < 8)
                    diff = 5;
                else if (pl_level < 10)
                    diff = 6;
                else if (pl_level < 12)
                    diff = 7;
                else if (pl_level < 16)
                    diff = 8;
                else if (pl_level < 20)
                    diff = 9;
                else if (pl_level < 30)
                    diff = 11;
                else if (pl_level < 40)
                    diff = 12;
                else if (pl_level < 45)
                    diff = 13;
                else if (pl_level < 50)
                    diff = 14;
                else if (pl_level < 55)
                    diff = 15;
                else if (pl_level < 60)
                    diff = 16;
                else
                    diff = 17;

            }
            return diff;
        }

        inline uint32 BaseGain(uint8 pl_level, uint8 mob_level, ContentLevels content)
        {
            uint32 baseGain;
            uint32 nBaseExp;
            bool SkipCoreCode = false;
            sScriptMgr->OnBaseGainCalculation(SkipCoreCode, baseGain, pl_level, mob_level, content); // pussywizard: optimization
            if (!SkipCoreCode) {
                switch (content)
                {
                case CONTENT_1_60:
                    nBaseExp = 45;
                    break;
                case CONTENT_61_70:
                    nBaseExp = 235;
                    break;
                case CONTENT_71_80:
                    nBaseExp = 580;
                    break;
                default:
                    sLog->outError("BaseGain: Unsupported content level %u", content);
                    nBaseExp = 45;
                    break;
                }

                if (mob_level >= pl_level)
                {
                    uint8 nLevelDiff = mob_level - pl_level;
                    if (nLevelDiff > 4)
                        nLevelDiff = 4;

                    baseGain = ((pl_level * 5 + nBaseExp) * (20 + nLevelDiff) / 10 + 1) / 2;
                }
                else
                {
                    uint8 gray_level = GetGrayLevel(pl_level);
                    if (mob_level > gray_level)
                    {
                        uint8 ZD = GetZeroDifference(pl_level);
                        baseGain = (pl_level * 5 + nBaseExp) * (ZD + mob_level - pl_level) / ZD;
                    }
                    else
                        baseGain = 0;
                }

            }
            return baseGain;
        }

        inline uint32 Gain(Player* player, Unit* u, bool isBattleGround = false)
        {
            Creature* creature = u->ToCreature();
            uint32 gain = 0;
            bool SkipCoreCode = false;
            sScriptMgr->OnGainCalculation(SkipCoreCode, gain, player, u);
            if (!SkipCoreCode)
            {
                if (!creature || (!creature->IsTotem() && !creature->IsPet() && !creature->IsCritter() &&
                    !(creature->GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_NO_XP_AT_KILL)))
                {
                    float xpMod = 1.0f;

                    gain = BaseGain(player->getLevel(), u->getLevel(), GetContentLevelsForMapAndZone(u->GetMapId(), u->GetZoneId()));

                    if (gain && creature)
                    {
                        if (creature->isElite())
                        {
                            // Elites in instances have a 2.75x XP bonus instead of the regular 2x world bonus.
                            if (u->GetMap() && u->GetMap()->IsDungeon())
                                xpMod *= 2.75f;
                            else
                                xpMod *= 2.0f;
                        }

                        // This requires TrinityCore creature_template.ExperienceModifier feature
                        // xpMod *= creature->GetCreatureTemplate()->ModExperience;
                    }

                    xpMod *= isBattleGround ? sWorld->getRate(RATE_XP_BG_KILL) : sWorld->getRate(RATE_XP_KILL);
                    gain = uint32(gain * xpMod);
                }
            }

            return gain;
        }

        inline float xp_in_group_rate(uint32 count, bool isRaid)
        {
            float rate;
            bool SkipCoreCode = false;
            sScriptMgr->OnGroupRateCalculation(SkipCoreCode, rate, count, isRaid); // pussywizard: optimization
            if (!SkipCoreCode) {
                if (isRaid)
                {
                    // FIXME: Must apply decrease modifiers depending on raid size.
                    rate = 1.0f;
                }
                else
                {
                    switch (count)
                    {
                    case 0:
                    case 1:
                    case 2:
                        rate = 1.0f;
                        break;
                    case 3:
                        rate = 1.166f;
                        break;
                    case 4:
                        rate = 1.3f;
                        break;
                    case 5:
                    default:
                        rate = 1.4f;
                    }
                }

            }
            return rate;
        }
    }
}

#endif
