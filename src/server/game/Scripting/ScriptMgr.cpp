/*
 * Copyright (C) 2016+     AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-GPL2
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 */

#include "ScriptMgr.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "DBCStores.h"
#include "ObjectMgr.h"
#include "OutdoorPvPMgr.h"
#include "ScriptLoader.h"
#include "ScriptSystem.h"
#include "Transport.h"
#include "Vehicle.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "GossipDef.h"
#include "CreatureAI.h"
#include "Player.h"
#include "WorldPacket.h"
#include "Chat.h"
#include "AuctionHouseMgr.h"
#include "Mail.h"
#ifdef ELUNA
#include "LuaEngine.h"
#include "ElunaUtility.h"
#endif

// Specialize for each script type class like so:
template class ScriptRegistry<SpellScriptLoader>;
template class ScriptRegistry<ServerScript>;
template class ScriptRegistry<WorldScript>;
template class ScriptRegistry<FormulaScript>;
template class ScriptRegistry<WorldMapScript>;
template class ScriptRegistry<InstanceMapScript>;
template class ScriptRegistry<BattlegroundMapScript>;
template class ScriptRegistry<ItemScript>;
template class ScriptRegistry<CreatureScript>;
template class ScriptRegistry<GameObjectScript>;
template class ScriptRegistry<AreaTriggerScript>;
template class ScriptRegistry<BattlegroundScript>;
template class ScriptRegistry<OutdoorPvPScript>;
template class ScriptRegistry<CommandScript>;
template class ScriptRegistry<WeatherScript>;
template class ScriptRegistry<AuctionHouseScript>;
template class ScriptRegistry<ConditionScript>;
template class ScriptRegistry<VehicleScript>;
template class ScriptRegistry<DynamicObjectScript>;
template class ScriptRegistry<TransportScript>;
template class ScriptRegistry<AchievementCriteriaScript>;
template class ScriptRegistry<PlayerScript>;
template class ScriptRegistry<GuildScript>;
template class ScriptRegistry<GroupScript>;
template class ScriptRegistry<GlobalScript>;
template class ScriptRegistry<UnitScript>;
template class ScriptRegistry<AllCreatureScript>;
template class ScriptRegistry<AllMapScript>;
template class ScriptRegistry<MovementHandlerScript>;
template class ScriptRegistry<BGScript>;

#include "ScriptMgrMacros.h"

// This is the global static registry of scripts.
/*template<class TScript>
class ScriptRegistry
{
    public:

        typedef std::map<uint32, TScript*> ScriptMap;
        typedef typename ScriptMap::iterator ScriptMapIterator;

        // The actual list of scripts. This will be accessed concurrently, so it must not be modified
        // after server startup.
        static ScriptMap ScriptPointerList;

        static void AddScript(TScript* const script)
        {
            ASSERT(script);

            // See if the script is using the same memory as another script. If this happens, it means that
            // someone forgot to allocate new memory for a script.
            for (ScriptMapIterator it = ScriptPointerList.begin(); it != ScriptPointerList.end(); ++it)
            {
                if (it->second == script)
                {
                    sLog->outError("Script '%s' has same memory pointer as '%s'.",
                        script->GetName().c_str(), it->second->GetName().c_str());

                    return;
                }
            }

            if (script->IsDatabaseBound())
            {
                // Get an ID for the script. An ID only exists if it's a script that is assigned in the database
                // through a script name (or similar).
                uint32 id = sObjectMgr->GetScriptId(script->GetName().c_str());
                if (id)
                {
                    // Try to find an existing script.
                    bool existing = false;
                    for (ScriptMapIterator it = ScriptPointerList.begin(); it != ScriptPointerList.end(); ++it)
                    {
                        // If the script names match...
                        if (it->second->GetName() == script->GetName())
                        {
                            // ... It exists.
                            existing = true;
                            break;
                        }
                    }

                    // If the script isn't assigned -> assign it!
                    if (!existing)
                    {
                        ScriptPointerList[id] = script;
                        sScriptMgr->IncrementScriptCount();
                    }
                    else
                    {
                        // If the script is already assigned -> delete it!
                        sLog->outError("Script '%s' already assigned with the same script name, so the script can't work.",
                            script->GetName().c_str());

                        ASSERT(false); // Error that should be fixed ASAP.
                    }
                }
                else
                {
                    // The script uses a script name from database, but isn't assigned to anything.
                    if (script->GetName().find("example") == std::string::npos && script->GetName().find("Smart") == std::string::npos)
                        sLog->outErrorDb("Script named '%s' does not have a script name assigned in database.",
                            script->GetName().c_str());
                }
            }
            else
            {
                // We're dealing with a code-only script; just add it.
                ScriptPointerList[_scriptIdCounter++] = script;
                sScriptMgr->IncrementScriptCount();
            }
        }

        // Gets a script by its ID (assigned by ObjectMgr).
        static TScript* GetScriptById(uint32 id)
        {
            ScriptMapIterator it = ScriptPointerList.find(id);
            if (it != ScriptPointerList.end())
                return it->second;

            return NULL;
        }

    private:

        // Counter used for code-only scripts.
        static uint32 _scriptIdCounter;
};*/

ScriptMgr::ScriptMgr()
    : _scriptCount(0), _scheduledScripts(0)
{

}

ScriptMgr::~ScriptMgr()
{
}

void ScriptMgr::Initialize()
{
    AddScripts();
    sLog->outString("Loading C++ scripts");
}

void ScriptMgr::Unload()
{
    #define SCR_CLEAR(T) \
        for (SCR_REG_ITR(T) itr = SCR_REG_LST(T).begin(); itr != SCR_REG_LST(T).end(); ++itr) \
            delete itr->second; \
        SCR_REG_LST(T).clear();

    // Clear scripts for every script type.
    SCR_CLEAR(SpellScriptLoader);
    SCR_CLEAR(ServerScript);
    SCR_CLEAR(WorldScript);
    SCR_CLEAR(FormulaScript);
    SCR_CLEAR(WorldMapScript);
    SCR_CLEAR(InstanceMapScript);
    SCR_CLEAR(BattlegroundMapScript);
    SCR_CLEAR(ItemScript);
    SCR_CLEAR(CreatureScript);
    SCR_CLEAR(GameObjectScript);
    SCR_CLEAR(AreaTriggerScript);
    SCR_CLEAR(BattlegroundScript);
    SCR_CLEAR(OutdoorPvPScript);
    SCR_CLEAR(CommandScript);
    SCR_CLEAR(WeatherScript);
    SCR_CLEAR(AuctionHouseScript);
    SCR_CLEAR(ConditionScript);
    SCR_CLEAR(VehicleScript);
    SCR_CLEAR(DynamicObjectScript);
    SCR_CLEAR(TransportScript);
    SCR_CLEAR(AchievementCriteriaScript);
    SCR_CLEAR(PlayerScript);
    SCR_CLEAR(GuildScript);
    SCR_CLEAR(GroupScript);
    SCR_CLEAR(GlobalScript);
    SCR_CLEAR(ModuleScript);
    SCR_CLEAR(BGScript);
    SCR_CLEAR(MailScript);
    SCR_CLEAR(ChannelScript);
    SCR_CLEAR(SocialScript);
    #undef SCR_CLEAR
}

void ScriptMgr::LoadDatabase()
{
    uint32 oldMSTime = getMSTime();

    sScriptSystemMgr->LoadScriptWaypoints();

    // Add all scripts that must be loaded after db/maps
    ScriptRegistry<WorldMapScript>::AddALScripts();
    ScriptRegistry<BattlegroundMapScript>::AddALScripts();
    ScriptRegistry<InstanceMapScript>::AddALScripts();
    ScriptRegistry<SpellScriptLoader>::AddALScripts();
    ScriptRegistry<ItemScript>::AddALScripts();
    ScriptRegistry<CreatureScript>::AddALScripts();
    ScriptRegistry<GameObjectScript>::AddALScripts();
    ScriptRegistry<AreaTriggerScript>::AddALScripts();
    ScriptRegistry<BattlegroundScript>::AddALScripts();
    ScriptRegistry<OutdoorPvPScript>::AddALScripts();
    ScriptRegistry<WeatherScript>::AddALScripts();
    ScriptRegistry<ConditionScript>::AddALScripts();
    ScriptRegistry<TransportScript>::AddALScripts();
    ScriptRegistry<AchievementCriteriaScript>::AddALScripts();

    FillSpellSummary();

    CheckIfScriptsInDatabaseExist();

    sLog->outString(">> Loaded %u C++ scripts in %u ms", GetScriptCount(), GetMSTimeDiffToNow(oldMSTime));
    sLog->outString();
}

struct TSpellSummary
{
    uint8 Targets;                                          // set of enum SelectTarget
    uint8 Effects;                                          // set of enum SelectEffect
} *SpellSummary;

void ScriptMgr::CheckIfScriptsInDatabaseExist()
{
    ObjectMgr::ScriptNameContainer& sn = sObjectMgr->GetScriptNames();
    for (ObjectMgr::ScriptNameContainer::iterator itr = sn.begin(); itr != sn.end(); ++itr)
        if (uint32 sid = sObjectMgr->GetScriptId((*itr).c_str()))
        {
            if (!ScriptRegistry<SpellScriptLoader>::GetScriptById(sid) &&
                !ScriptRegistry<ServerScript>::GetScriptById(sid) &&
                !ScriptRegistry<WorldScript>::GetScriptById(sid) &&
                !ScriptRegistry<FormulaScript>::GetScriptById(sid) &&
                !ScriptRegistry<WorldMapScript>::GetScriptById(sid) &&
                !ScriptRegistry<InstanceMapScript>::GetScriptById(sid) &&
                !ScriptRegistry<BattlegroundMapScript>::GetScriptById(sid) &&
                !ScriptRegistry<ItemScript>::GetScriptById(sid) &&
                !ScriptRegistry<CreatureScript>::GetScriptById(sid) &&
                !ScriptRegistry<GameObjectScript>::GetScriptById(sid) &&
                !ScriptRegistry<AreaTriggerScript>::GetScriptById(sid) &&
                !ScriptRegistry<BattlegroundScript>::GetScriptById(sid) &&
                !ScriptRegistry<OutdoorPvPScript>::GetScriptById(sid) &&
                !ScriptRegistry<CommandScript>::GetScriptById(sid) &&
                !ScriptRegistry<WeatherScript>::GetScriptById(sid) &&
                !ScriptRegistry<AuctionHouseScript>::GetScriptById(sid) &&
                !ScriptRegistry<ConditionScript>::GetScriptById(sid) &&
                !ScriptRegistry<VehicleScript>::GetScriptById(sid) &&
                !ScriptRegistry<DynamicObjectScript>::GetScriptById(sid) &&
                !ScriptRegistry<TransportScript>::GetScriptById(sid) &&
                !ScriptRegistry<AchievementCriteriaScript>::GetScriptById(sid) &&
                !ScriptRegistry<PlayerScript>::GetScriptById(sid) &&
                !ScriptRegistry<GuildScript>::GetScriptById(sid) &&
                !ScriptRegistry<BGScript>::GetScriptById(sid) &&
                !ScriptRegistry<GroupScript>::GetScriptById(sid))
                sLog->outErrorDb("Script named '%s' is assigned in database, but has no code!", (*itr).c_str());
        }
}

void ScriptMgr::FillSpellSummary()
{
    SpellSummary = new TSpellSummary[sSpellMgr->GetSpellInfoStoreSize()];

    SpellInfo const* pTempSpell;

    for (uint32 i = 0; i < sSpellMgr->GetSpellInfoStoreSize(); ++i)
    {
        SpellSummary[i].Effects = 0;
        SpellSummary[i].Targets = 0;

        pTempSpell = sSpellMgr->GetSpellInfo(i);
        // This spell doesn't exist.
        if (!pTempSpell)
            continue;

        for (uint32 j = 0; j < MAX_SPELL_EFFECTS; ++j)
        {
            // Spell targets me.
            if (pTempSpell->Effects[j].TargetA.GetTarget() == TARGET_UNIT_CASTER)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_SELF-1);

            // Spell targets a single enemy.
            if (pTempSpell->Effects[j].TargetA.GetTarget() == TARGET_UNIT_TARGET_ENEMY ||
                pTempSpell->Effects[j].TargetA.GetTarget() == TARGET_DEST_TARGET_ENEMY)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_SINGLE_ENEMY-1);

            // Spell targets AoE at enemy.
            if (pTempSpell->Effects[j].TargetA.GetTarget() == TARGET_UNIT_SRC_AREA_ENEMY ||
                pTempSpell->Effects[j].TargetA.GetTarget() == TARGET_UNIT_DEST_AREA_ENEMY ||
                pTempSpell->Effects[j].TargetA.GetTarget() == TARGET_SRC_CASTER ||
                pTempSpell->Effects[j].TargetA.GetTarget() == TARGET_DEST_DYNOBJ_ENEMY)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_AOE_ENEMY-1);

            // Spell targets an enemy.
            if (pTempSpell->Effects[j].TargetA.GetTarget() == TARGET_UNIT_TARGET_ENEMY ||
                pTempSpell->Effects[j].TargetA.GetTarget() == TARGET_DEST_TARGET_ENEMY ||
                pTempSpell->Effects[j].TargetA.GetTarget() == TARGET_UNIT_SRC_AREA_ENEMY ||
                pTempSpell->Effects[j].TargetA.GetTarget() == TARGET_UNIT_DEST_AREA_ENEMY ||
                pTempSpell->Effects[j].TargetA.GetTarget() == TARGET_SRC_CASTER ||
                pTempSpell->Effects[j].TargetA.GetTarget() == TARGET_DEST_DYNOBJ_ENEMY)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_ANY_ENEMY-1);

            // Spell targets a single friend (or me).
            if (pTempSpell->Effects[j].TargetA.GetTarget() == TARGET_UNIT_CASTER ||
                pTempSpell->Effects[j].TargetA.GetTarget() == TARGET_UNIT_TARGET_ALLY ||
                pTempSpell->Effects[j].TargetA.GetTarget() == TARGET_UNIT_TARGET_PARTY)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_SINGLE_FRIEND-1);

            // Spell targets AoE friends.
            if (pTempSpell->Effects[j].TargetA.GetTarget() == TARGET_UNIT_CASTER_AREA_PARTY ||
                pTempSpell->Effects[j].TargetA.GetTarget() == TARGET_UNIT_LASTTARGET_AREA_PARTY ||
                pTempSpell->Effects[j].TargetA.GetTarget() == TARGET_SRC_CASTER)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_AOE_FRIEND-1);

            // Spell targets any friend (or me).
            if (pTempSpell->Effects[j].TargetA.GetTarget() == TARGET_UNIT_CASTER ||
                pTempSpell->Effects[j].TargetA.GetTarget() == TARGET_UNIT_TARGET_ALLY ||
                pTempSpell->Effects[j].TargetA.GetTarget() == TARGET_UNIT_TARGET_PARTY ||
                pTempSpell->Effects[j].TargetA.GetTarget() == TARGET_UNIT_CASTER_AREA_PARTY ||
                pTempSpell->Effects[j].TargetA.GetTarget() == TARGET_UNIT_LASTTARGET_AREA_PARTY ||
                pTempSpell->Effects[j].TargetA.GetTarget() == TARGET_SRC_CASTER)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_ANY_FRIEND-1);

            // Make sure that this spell includes a damage effect.
            if (pTempSpell->Effects[j].Effect == SPELL_EFFECT_SCHOOL_DAMAGE ||
                pTempSpell->Effects[j].Effect == SPELL_EFFECT_INSTAKILL ||
                pTempSpell->Effects[j].Effect == SPELL_EFFECT_ENVIRONMENTAL_DAMAGE ||
                pTempSpell->Effects[j].Effect == SPELL_EFFECT_HEALTH_LEECH)
                SpellSummary[i].Effects |= 1 << (SELECT_EFFECT_DAMAGE-1);

            // Make sure that this spell includes a healing effect (or an apply aura with a periodic heal).
            if (pTempSpell->Effects[j].Effect == SPELL_EFFECT_HEAL ||
                pTempSpell->Effects[j].Effect == SPELL_EFFECT_HEAL_MAX_HEALTH ||
                pTempSpell->Effects[j].Effect == SPELL_EFFECT_HEAL_MECHANICAL ||
                (pTempSpell->Effects[j].Effect == SPELL_EFFECT_APPLY_AURA  && pTempSpell->Effects[j].ApplyAuraName == 8))
                SpellSummary[i].Effects |= 1 << (SELECT_EFFECT_HEALING-1);

            // Make sure that this spell applies an aura.
            if (pTempSpell->Effects[j].Effect == SPELL_EFFECT_APPLY_AURA)
                SpellSummary[i].Effects |= 1 << (SELECT_EFFECT_AURA-1);
        }
    }
}

void ScriptMgr::CreateSpellScripts(uint32 spellId, std::list<SpellScript*>& scriptVector)
{
    SpellScriptsBounds bounds = sObjectMgr->GetSpellScriptsBounds(spellId);

    for (SpellScriptsContainer::iterator itr = bounds.first; itr != bounds.second; ++itr)
    {
        SpellScriptLoader* tmpscript = ScriptRegistry<SpellScriptLoader>::GetScriptById(itr->second);
        if (!tmpscript)
            continue;

        SpellScript* script = tmpscript->GetSpellScript();

        if (!script)
            continue;

        script->_Init(&tmpscript->GetName(), spellId);

        scriptVector.push_back(script);
    }
}

void ScriptMgr::CreateAuraScripts(uint32 spellId, std::list<AuraScript*>& scriptVector)
{
    SpellScriptsBounds bounds = sObjectMgr->GetSpellScriptsBounds(spellId);

    for (SpellScriptsContainer::iterator itr = bounds.first; itr != bounds.second; ++itr)
    {
        SpellScriptLoader* tmpscript = ScriptRegistry<SpellScriptLoader>::GetScriptById(itr->second);
        if (!tmpscript)
            continue;

        AuraScript* script = tmpscript->GetAuraScript();

        if (!script)
            continue;

        script->_Init(&tmpscript->GetName(), spellId);

        scriptVector.push_back(script);
    }
}

void ScriptMgr::CreateSpellScriptLoaders(uint32 spellId, std::vector<std::pair<SpellScriptLoader*, SpellScriptsContainer::iterator> >& scriptVector)
{
    SpellScriptsBounds bounds = sObjectMgr->GetSpellScriptsBounds(spellId);
    scriptVector.reserve(std::distance(bounds.first, bounds.second));

    for (SpellScriptsContainer::iterator itr = bounds.first; itr != bounds.second; ++itr)
    {
        SpellScriptLoader* tmpscript = ScriptRegistry<SpellScriptLoader>::GetScriptById(itr->second);
        if (!tmpscript)
            continue;

        scriptVector.push_back(std::make_pair(tmpscript, itr));
    }
}

void ScriptMgr::OnNetworkStart()
{
    FOREACH_SCRIPT(ServerScript)->OnNetworkStart();
}

void ScriptMgr::OnNetworkStop()
{
    FOREACH_SCRIPT(ServerScript)->OnNetworkStop();
}

void ScriptMgr::OnSocketOpen(WorldSocket* socket)
{
    ASSERT(socket);

    FOREACH_SCRIPT(ServerScript)->OnSocketOpen(socket);
}

void ScriptMgr::OnSocketClose(WorldSocket* socket, bool wasNew)
{
    ASSERT(socket);

    FOREACH_SCRIPT(ServerScript)->OnSocketClose(socket, wasNew);
}

void ScriptMgr::OnPacketReceive(WorldSession* session, WorldPacket const& packet)
{
    if (SCR_REG_LST(ServerScript).empty())
        return;

    WorldPacket copy(packet);
    FOREACH_SCRIPT(ServerScript)->OnPacketReceive(session, copy);
}

void ScriptMgr::OnPacketSend(WorldSession* session, WorldPacket const& packet)
{
    ASSERT(session);

    if (SCR_REG_LST(ServerScript).empty())
        return;

    WorldPacket copy(packet);
    FOREACH_SCRIPT(ServerScript)->OnPacketSend(session, copy);
}


void ScriptMgr::OnOpenStateChange(bool open)
{
#ifdef ELUNA
    sEluna->OnOpenStateChange(open);
#endif
    FOREACH_SCRIPT(WorldScript)->OnOpenStateChange(open);
}

void ScriptMgr::OnBeforeConfigLoad(bool reload)
{
#ifdef ELUNA
    sEluna->OnConfigLoad(reload, true);
#endif
    FOREACH_SCRIPT(WorldScript)->OnBeforeConfigLoad(reload);
}

void ScriptMgr::OnAfterConfigLoad(bool reload)
{
#ifdef ELUNA
    sEluna->OnConfigLoad(reload, false);
#endif
    FOREACH_SCRIPT(WorldScript)->OnAfterConfigLoad(reload);
}

void ScriptMgr::OnMotdChange(std::string& newMotd)
{
    FOREACH_SCRIPT(WorldScript)->OnMotdChange(newMotd);
}

void ScriptMgr::OnShutdownInitiate(ShutdownExitCode code, ShutdownMask mask)
{
#ifdef ELUNA
    sEluna->OnShutdownInitiate(code, mask);
#endif
    FOREACH_SCRIPT(WorldScript)->OnShutdownInitiate(code, mask);
}

void ScriptMgr::OnShutdownCancel()
{
#ifdef ELUNA
    sEluna->OnShutdownCancel();
#endif
    FOREACH_SCRIPT(WorldScript)->OnShutdownCancel();
}

void ScriptMgr::OnWorldUpdate(uint32 diff)
{
#ifdef ELUNA
    sEluna->OnWorldUpdate(diff);
#endif
    FOREACH_SCRIPT(WorldScript)->OnUpdate(diff);
}

void ScriptMgr::OnHonorCalculation(bool &SkipCoreCode, float& honor, uint8 level, float multiplier)
{
    FOREACH_SCRIPT(FormulaScript)->OnHonorCalculation(SkipCoreCode, honor, level, multiplier);
}

void ScriptMgr::OnGrayLevelCalculation(bool &SkipCoreCode, uint8& grayLevel, uint8 playerLevel)
{
    FOREACH_SCRIPT(FormulaScript)->OnGrayLevelCalculation(SkipCoreCode, grayLevel, playerLevel);
}

void ScriptMgr::OnColorCodeCalculation(bool &SkipCoreCode, XPColorChar& color, uint8 playerLevel, uint8 mobLevel)
{
    FOREACH_SCRIPT(FormulaScript)->OnColorCodeCalculation(SkipCoreCode, color, playerLevel, mobLevel);
}

void ScriptMgr::OnZeroDifferenceCalculation(bool &SkipCoreCode, uint8& diff, uint8 playerLevel)
{
    FOREACH_SCRIPT(FormulaScript)->OnZeroDifferenceCalculation(SkipCoreCode, diff, playerLevel);
}

void ScriptMgr::OnBaseGainCalculation(bool &SkipCoreCode, uint32& gain, uint8 playerLevel, uint8 mobLevel, ContentLevels content)
{
    FOREACH_SCRIPT(FormulaScript)->OnBaseGainCalculation(SkipCoreCode, gain, playerLevel, mobLevel, content);
}

void ScriptMgr::OnGainCalculation(bool &SkipCoreCode, uint32& gain, Player* player, Unit* unit)
{
    ASSERT(player);
    ASSERT(unit);

    FOREACH_SCRIPT(FormulaScript)->OnGainCalculation(SkipCoreCode, gain, player, unit);
}

void ScriptMgr::OnGroupRateCalculation(bool &SkipCoreCode, float& rate, uint32 count, bool isRaid)
{
    FOREACH_SCRIPT(FormulaScript)->OnGroupRateCalculation(SkipCoreCode, rate, count, isRaid);
}

void ScriptMgr::OnTalentCalculation(bool &SkipCoreCode, Player const * player, uint32 & result, uint32 m_questRewardTalentCount)
{
    FOREACH_SCRIPT(FormulaScript)->OnTalentCalculation(SkipCoreCode, player, result, m_questRewardTalentCount);
}

void ScriptMgr::OnGlyphInit(bool &SkipCoreCode,uint32 level, uint32 & result)
{
    FOREACH_SCRIPT(FormulaScript)->OnGlyphInit(SkipCoreCode, level, result);
}

void ScriptMgr::OnStatToAttackPowerCalculation(bool &SkipCoreCode, Player const * player, float level, float & val2, bool ranged)
{
    FOREACH_SCRIPT(FormulaScript)->OnStatToAttackPowerCalculation(SkipCoreCode,  player, level,  val2, ranged);
}

void ScriptMgr::OnSpellBaseDamageBonusDone(bool &SkipCoreCode, Player const * player, int32 & DoneAdvertisedBenefit)
{
    FOREACH_SCRIPT(FormulaScript)->OnSpellBaseDamageBonusDone(SkipCoreCode, player, DoneAdvertisedBenefit);
}

void ScriptMgr::OnSpellBaseHealingBonusDone(bool &SkipCoreCode, Player const * player, int32 & DoneAdvertisedBenefit)
{
    FOREACH_SCRIPT(FormulaScript)->OnSpellBaseHealingBonusDone(SkipCoreCode, player, DoneAdvertisedBenefit);
}

void ScriptMgr::OnUpdateResistance(bool &SkipCoreCode, Player const * player, uint32 school,float & value)
{
    FOREACH_SCRIPT(FormulaScript)->OnUpdateResistance(SkipCoreCode, player, school, value);
}

void ScriptMgr::OnUpdateArmor(bool &SkipCoreCode, Player const * player, float & value)
{
    FOREACH_SCRIPT(FormulaScript)->OnUpdateArmor(SkipCoreCode, player, value);
}

void ScriptMgr::OnDefaultUnitMeleeSkill(bool &SkipCoreCode, Unit const * me, Unit const * target, uint32 & result)
{
    FOREACH_SCRIPT(FormulaScript)->OnDefaultUnitMeleeSkill(SkipCoreCode, me, target, result);
}

void ScriptMgr::OnManaRestore(bool &SkipCoreCode, Player const * player, float& addvalue, int32& m_regenTimer)
{
    FOREACH_SCRIPT(FormulaScript)->OnManaRestore(SkipCoreCode, player, addvalue, m_regenTimer);
}

void ScriptMgr::OnHealthRestore(bool &SkipCoreCode, Player * player, float& addvalue, uint32 m_baseHealthRegen)
{
    FOREACH_SCRIPT(FormulaScript)->OnHealthRestore(SkipCoreCode, player, addvalue,  m_baseHealthRegen);
}

void ScriptMgr::OnCanRollForItemInLFG(bool &SkipCoreCode, Player const * player, InventoryResult &RETURN_CODE, ItemTemplate const* proto)
{
    FOREACH_SCRIPT(FormulaScript)->OnCanRollForItemInLFG(SkipCoreCode, player, RETURN_CODE, proto);
}

void ScriptMgr::OnQuestXPValue(bool &SkipCoreCode, Player *player, uint32 &xp, int32 Level, uint32 RewardXPId)
{
    FOREACH_SCRIPT(FormulaScript)->OnQuestXPValue(SkipCoreCode, player,  xp, Level, RewardXPId);
}

void ScriptMgr::OnSpellAddUnitTarget(bool& SkipCoreCode, Unit * target, uint32 & targetLevelRange)
{
    FOREACH_SCRIPT(FormulaScript)->OnSpellAddUnitTarget(SkipCoreCode, target, targetLevelRange);
}

void ScriptMgr::OnGetAuraRankForLevel(bool & SkipCoreCode, uint8 level, uint32& appropriateLevel)
{
    FOREACH_SCRIPT(FormulaScript)->OnGetAuraRankForLevel( SkipCoreCode, level, appropriateLevel);
}

void ScriptMgr::OnEffectApplyGlyph(bool& SkipCoreCode, uint32 m_glyphIndex,uint8& minLevel)
{
    FOREACH_SCRIPT(FormulaScript)->OnEffectApplyGlyph(SkipCoreCode, m_glyphIndex, minLevel);
}
void ScriptMgr::OnGLANCINGCalculation(bool & SkipCoreCode, int32 leveldiff,float reducePercent)
{
    FOREACH_SCRIPT(FormulaScript)->OnGLANCINGCalculation(SkipCoreCode, leveldiff, reducePercent);
}
void ScriptMgr::OnCreatureDazePlayer(bool& SkipCoreCode, CalcDamageInfo* damageInfo, Unit* attacker, Unit* victim)
{
    FOREACH_SCRIPT(FormulaScript)->OnCreatureDazePlayer(SkipCoreCode, damageInfo, attacker, victim);
}
void ScriptMgr::OnArmorLevelPenaltyCalculation(bool& SkipCoreCode, Unit const* attacker, Unit const* victim, SpellInfo const * spellInfo, uint8 attackerLevel, float& armor, float& tmpvalue)
{
    FOREACH_SCRIPT(FormulaScript)->OnArmorLevelPenaltyCalculation(SkipCoreCode, attacker, victim, spellInfo, attackerLevel, armor, tmpvalue);
}
void ScriptMgr::OnResistChanceCalculation(bool& SkipCoreCode, Unit const* victim, float& resistanceConstant)
{
    FOREACH_SCRIPT(FormulaScript)->OnResistChanceCalculation(SkipCoreCode, victim, resistanceConstant);
}
void ScriptMgr::OnCrushingCalculation(bool& SkipCoreCode, Unit const* attacker, Unit const* victim, int32 victimDefenseSkill, int32  victimMaxSkillValueForLevel, int32  attackerMaxSkillValueForLevel,int32 roll, int32& tmp, int32& sum, MeleeHitOutcome& RETURN_CODE)
{
    FOREACH_SCRIPT(FormulaScript)->OnCrushingCalculation(SkipCoreCode, attacker, victim, victimDefenseSkill, victimMaxSkillValueForLevel, attackerMaxSkillValueForLevel,roll, tmp, sum, RETURN_CODE);
}
void ScriptMgr::OnCalculateLevelPenalty(bool& SkipCoreCode, SpellInfo const* spellProto,Unit const* me,float& result)
{
    FOREACH_SCRIPT(FormulaScript)->OnCalculateLevelPenalty(SkipCoreCode, spellProto,me, result);
}
void ScriptMgr::OnMeleeSpellSkillCheck(bool& SkipCoreCode, Unit const* me, WeaponAttackType attType, Unit* victim, SpellInfo const* spell, int32& skillDiff)
{
    FOREACH_SCRIPT(FormulaScript)->OnMeleeSpellSkillCheck(SkipCoreCode, me, attType, victim, spell, skillDiff);
}
void ScriptMgr::OnDefaultUnitDefenseSkill(bool& SkipCoreCode,Unit const* me, Unit const* target,uint32& result)
{
    FOREACH_SCRIPT(FormulaScript)->OnDefaultUnitDefenseSkill(SkipCoreCode, me, target, result);
}
void ScriptMgr::OnAggroRangeLevelCalculation(bool& SkipCoreCode, Creature const* me, Unit const* target, float& aggroRadius)
{
    FOREACH_SCRIPT(FormulaScript)->OnAggroRangeLevelCalculation(SkipCoreCode, me, target, aggroRadius);
}
void ScriptMgr::OnStealthDetectLevelCalculate(bool& SkipCoreCode, WorldObject const* me, WorldObject const* obj , int32& detectionValue)
{
    FOREACH_SCRIPT(FormulaScript)->OnStealthDetectLevelCalculate(SkipCoreCode, me,obj, detectionValue);
}
void ScriptMgr::OnUpdateCombatSkillLevelCalculate(bool& SkipCoreCode,Player const* me, WeaponAttackType attType,bool defence,uint8 plevel, uint8 graylevel, uint8 moblevel, uint8& lvldif, uint32& skilldif)
{
    FOREACH_SCRIPT(FormulaScript)->OnUpdateCombatSkillLevelCalculate(SkipCoreCode, me, attType, defence, plevel, graylevel, moblevel, lvldif, skilldif);
}
void ScriptMgr::OnAgainstGLANCINGCalculation(bool& SkipCoreCode, Unit const* attacker, Unit const* victim, WeaponAttackType attType, int32 victimDefenseSkill, int32  attackerWeaponSkill, int32  attackerMaxSkillValueForLevel, int32 roll, int32& tmp, int32& sum, MeleeHitOutcome& RETURN_CODE)
{
    FOREACH_SCRIPT(FormulaScript)->OnAgainstGLANCINGCalculation(SkipCoreCode, attacker, victim,attType, victimDefenseSkill, attackerWeaponSkill, attackerMaxSkillValueForLevel, roll, tmp, sum, RETURN_CODE);
}
void ScriptMgr::OnMagicSpellHitLevelCalculate(bool& SkipCoreCode, Unit const* me, Unit * victim, SpellInfo const* spell, int32& lchance, int32& leveldif)
{
    FOREACH_SCRIPT(FormulaScript)->OnMagicSpellHitLevelCalculate(SkipCoreCode, me, victim, spell,lchance, leveldif);
}
void ScriptMgr::OnUpdateBlockPercentage(bool& SkipCoreCode,Player const* player,float& value)
{
    FOREACH_SCRIPT(FormulaScript)->OnUpdateBlockPercentage(SkipCoreCode, player, value);
}
void ScriptMgr::OnUpdateCritPercentage(bool& SkipCoreCode, Player* player, WeaponAttackType attType,float& value)
{
    FOREACH_SCRIPT(FormulaScript)->OnUpdateCritPercentage(SkipCoreCode, player, attType,value);
}
void ScriptMgr::OnGetMissPercentageFromDefense(bool& SkipCoreCode, Player const* player,float& result)
{
    FOREACH_SCRIPT(FormulaScript)->OnGetMissPercentageFromDefense(SkipCoreCode, player, result);
}
void ScriptMgr::OnUpdateParryPercentage(bool& SkipCoreCode, Player const* player, float& m_realParry, float& value)
{
    FOREACH_SCRIPT(FormulaScript)->OnUpdateParryPercentage(SkipCoreCode, player, m_realParry, value);
}
void ScriptMgr::OnUpdateDodgePercentage(bool& SkipCoreCode, Player * player, float& m_realDodge, float&  value)
{
    FOREACH_SCRIPT(FormulaScript)->OnUpdateDodgePercentage(SkipCoreCode, player, m_realDodge, value);
}
void ScriptMgr::OnMeleeMissOutcomeAgainstAboutMiss(bool& SkipCoreCode,Unit const* me, Unit const* victim, WeaponAttackType attType, float& miss_chance)
{
    FOREACH_SCRIPT(FormulaScript)->OnMeleeMissOutcomeAgainstAboutMiss(SkipCoreCode, me, victim, attType, miss_chance);
}
void ScriptMgr::OnSpellBlockCalculation(bool& SkipCoreCode, Unit* me, Unit const* victim, WeaponAttackType attackType, float& blockChance)
{
    FOREACH_SCRIPT(FormulaScript)->OnSpellBlockCalculation(SkipCoreCode, me, victim, attackType, blockChance);
}
void ScriptMgr::OnGetUnitCriticalChanceAboutLevel(bool& SkipCoreCode, Unit const* me, Unit const * victim, float& crit)
{
    FOREACH_SCRIPT(FormulaScript)->OnGetUnitCriticalChanceAboutLevel(SkipCoreCode, me, victim, crit);
}
void ScriptMgr::OnGetWeaponSkillValue(bool& SkipCoreCode, WeaponAttackType attType, Unit const* me, Unit const * target, uint32& value)
{
    FOREACH_SCRIPT(FormulaScript)->OnGetWeaponSkillValue(SkipCoreCode, attType, me, target, value);
}
void ScriptMgr::OnSpellDamageClassRanged(bool& SkipCoreCode, Unit const* me, Unit const* caster, float& crit_chance)
{
    FOREACH_SCRIPT(FormulaScript)->OnSpellDamageClassRanged(SkipCoreCode, me, caster, crit_chance);
}

void ScriptMgr::OnBuildPlayerLevelInfo(bool& SkipCoreCode, uint8 race, uint8 _class, uint8 level, PlayerLevelInfo* info)
{
    FOREACH_SCRIPT(FormulaScript)->OnBuildPlayerLevelInfo(SkipCoreCode, race, _class, level, info);
}

void ScriptMgr::UpdatePotionCooldown(bool& SkipCoreCode, Player* me)
{
    FOREACH_SCRIPT(FormulaScript)->UpdatePotionCooldown(SkipCoreCode, me);
}

void ScriptMgr::OnInitTaxiNodesForLevel(bool& SkipCoreCode, uint32 race, uint32 chrClass, uint8 level, PlayerTaxi* me)
{
    FOREACH_SCRIPT(FormulaScript)->OnInitTaxiNodesForLevel(SkipCoreCode, race, chrClass, level, me);
}
void ScriptMgr::OnCalculateMinMaxDamage(bool& SkipCoreCode, Player* me, WeaponAttackType attType, bool normalized, bool addTotalPct, float& minDamage, float& maxDamage)
{
    FOREACH_SCRIPT(FormulaScript)->OnCalculateMinMaxDamage( SkipCoreCode, me, attType,  normalized,  addTotalPct,  minDamage,  maxDamage);
}

void ScriptMgr::OnGetCreatureType(bool& SkipCoreCode, Unit const* me, uint32& result)
{
    FOREACH_SCRIPT(FormulaScript)->OnGetCreatureType(SkipCoreCode, me, result);
}

void ScriptMgr::OnHandleModStateImmunityMask(AuraEffect const* me, Unit* target, std::list <AuraType>& aura_immunity_list, uint32& mechanic_immunity_list,int32 miscVal, bool apply)
{
    FOREACH_SCRIPT(FormulaScript)->OnHandleModStateImmunityMask(me, target, aura_immunity_list, mechanic_immunity_list, miscVal, apply);
}

void ScriptMgr::OnCanUseItem(bool& SkipCoreCode, Player const* me, Item* pItem, ItemTemplate const* pProto, bool not_loading, InventoryResult& RETURN_CODE)
{
    FOREACH_SCRIPT(FormulaScript)->OnCanUseItem(SkipCoreCode,me, pItem, pProto, not_loading, RETURN_CODE);
}
void ScriptMgr::OnCanStartAttack(bool& SkipCoreCode, Creature const* me, Unit const* who, float m_CombatDistance,bool& RETURN_CODE)
{
    FOREACH_SCRIPT(FormulaScript)->OnCanStartAttack(SkipCoreCode, me, who, m_CombatDistance, RETURN_CODE);
}
void ScriptMgr::OnCallAssistance(bool& SkipCoreCode, Creature* me, bool m_AlreadyCallAssistance, EventProcessor& m_Events)
{
    FOREACH_SCRIPT(FormulaScript)->OnCallAssistance(SkipCoreCode, me, m_AlreadyCallAssistance, m_Events);
}
void ScriptMgr::OnDurabilityLoss(bool& SkipCoreCode, Player* me, Item* item, double percent, uint32& pDurabilityLoss)
{
    FOREACH_SCRIPT(FormulaScript)->OnDurabilityLoss(SkipCoreCode, me, item, percent, pDurabilityLoss);
}

void ScriptMgr::OnUpdateCraftSkill(bool& SkipCoreCode, Player* me, uint32 spelllevel, uint32 SkillId, uint32 craft_skill_gain, bool& result)
{
    FOREACH_SCRIPT(FormulaScript)->OnUpdateCraftSkill(SkipCoreCode, me, spelllevel, SkillId, craft_skill_gain, result);
}

void ScriptMgr::OnUpdateCombatSkills(bool& SkipCoreCode,Player* me, uint32 spelllevel, bool defence, WeaponAttackType attType, uint32 chance, bool& result)
{
    FOREACH_SCRIPT(FormulaScript)->OnUpdateCombatSkills(SkipCoreCode, me, spelllevel, defence, attType, chance, result);
}
#define SCR_MAP_BGN(M, V, I, E, C, T) \
    if (V->GetEntry() && V->GetEntry()->T()) \
    { \
        FOR_SCRIPTS(M, I, E) \
        { \
            MapEntry const* C = I->second->GetEntry(); \
            if (!C) \
                continue; \
            if (C->MapID == V->GetId()) \
            {

#define SCR_MAP_END \
                return; \
            } \
        } \
    }

void ScriptMgr::OnCreateMap(Map* map)
{
    ASSERT(map);

#ifdef ELUNA
    sEluna->OnCreate(map);
#endif

    SCR_MAP_BGN(WorldMapScript, map, itr, end, entry, IsWorldMap);
        itr->second->OnCreate(map);
    SCR_MAP_END;

    SCR_MAP_BGN(InstanceMapScript, map, itr, end, entry, IsDungeon);
        itr->second->OnCreate((InstanceMap*)map);
    SCR_MAP_END;

    SCR_MAP_BGN(BattlegroundMapScript, map, itr, end, entry, IsBattleground);
        itr->second->OnCreate((BattlegroundMap*)map);
    SCR_MAP_END;
}

void ScriptMgr::OnDestroyMap(Map* map)
{
    ASSERT(map);

#ifdef ELUNA
    sEluna->OnDestroy(map);
#endif

    SCR_MAP_BGN(WorldMapScript, map, itr, end, entry, IsWorldMap);
        itr->second->OnDestroy(map);
    SCR_MAP_END;

    SCR_MAP_BGN(InstanceMapScript, map, itr, end, entry, IsDungeon);
        itr->second->OnDestroy((InstanceMap*)map);
    SCR_MAP_END;

    SCR_MAP_BGN(BattlegroundMapScript, map, itr, end, entry, IsBattleground);
        itr->second->OnDestroy((BattlegroundMap*)map);
    SCR_MAP_END;
}

void ScriptMgr::OnLoadGridMap(Map* map, GridMap* gmap, uint32 gx, uint32 gy)
{
    ASSERT(map);
    ASSERT(gmap);

    SCR_MAP_BGN(WorldMapScript, map, itr, end, entry, IsWorldMap);
        itr->second->OnLoadGridMap(map, gmap, gx, gy);
    SCR_MAP_END;

    SCR_MAP_BGN(InstanceMapScript, map, itr, end, entry, IsDungeon);
        itr->second->OnLoadGridMap((InstanceMap*)map, gmap, gx, gy);
    SCR_MAP_END;

    SCR_MAP_BGN(BattlegroundMapScript, map, itr, end, entry, IsBattleground);
        itr->second->OnLoadGridMap((BattlegroundMap*)map, gmap, gx, gy);
    SCR_MAP_END;
}

void ScriptMgr::OnUnloadGridMap(Map* map, GridMap* gmap, uint32 gx, uint32 gy)
{
    ASSERT(map);
    ASSERT(gmap);

    SCR_MAP_BGN(WorldMapScript, map, itr, end, entry, IsWorldMap);
        itr->second->OnUnloadGridMap(map, gmap, gx, gy);
    SCR_MAP_END;

    SCR_MAP_BGN(InstanceMapScript, map, itr, end, entry, IsDungeon);
        itr->second->OnUnloadGridMap((InstanceMap*)map, gmap, gx, gy);
    SCR_MAP_END;

    SCR_MAP_BGN(BattlegroundMapScript, map, itr, end, entry, IsBattleground);
        itr->second->OnUnloadGridMap((BattlegroundMap*)map, gmap, gx, gy);
    SCR_MAP_END;
}

void ScriptMgr::OnPlayerEnterMap(Map* map, Player* player)
{
    ASSERT(map);
    ASSERT(player);

#ifdef ELUNA
    sEluna->OnMapChanged(player);
    sEluna->OnPlayerEnter(map, player);
#endif

    FOREACH_SCRIPT(AllMapScript)->OnPlayerEnterAll(map, player);

    FOREACH_SCRIPT(PlayerScript)->OnMapChanged(player);

    SCR_MAP_BGN(WorldMapScript, map, itr, end, entry, IsWorldMap);
        itr->second->OnPlayerEnter(map, player);
    SCR_MAP_END;

    SCR_MAP_BGN(InstanceMapScript, map, itr, end, entry, IsDungeon);
        itr->second->OnPlayerEnter((InstanceMap*)map, player);
    SCR_MAP_END;

    SCR_MAP_BGN(BattlegroundMapScript, map, itr, end, entry, IsBattleground);
        itr->second->OnPlayerEnter((BattlegroundMap*)map, player);
    SCR_MAP_END;
}

void ScriptMgr::OnPlayerLeaveMap(Map* map, Player* player)
{
    ASSERT(map);
    ASSERT(player);

#ifdef ELUNA
    sEluna->OnPlayerLeave(map, player);
#endif

    FOREACH_SCRIPT(AllMapScript)->OnPlayerLeaveAll(map, player);
    
    SCR_MAP_BGN(WorldMapScript, map, itr, end, entry, IsWorldMap);
        itr->second->OnPlayerLeave(map, player);
    SCR_MAP_END;

    SCR_MAP_BGN(InstanceMapScript, map, itr, end, entry, IsDungeon);
        itr->second->OnPlayerLeave((InstanceMap*)map, player);
    SCR_MAP_END;

    SCR_MAP_BGN(BattlegroundMapScript, map, itr, end, entry, IsBattleground);
        itr->second->OnPlayerLeave((BattlegroundMap*)map, player);
    SCR_MAP_END;
}

void ScriptMgr::OnMapUpdate(Map* map, uint32 diff)
{
    ASSERT(map);

#ifdef ELUNA
    sEluna->OnUpdate(map, diff);
#endif

    SCR_MAP_BGN(WorldMapScript, map, itr, end, entry, IsWorldMap);
        itr->second->OnUpdate(map, diff);
    SCR_MAP_END;

    SCR_MAP_BGN(InstanceMapScript, map, itr, end, entry, IsDungeon);
        itr->second->OnUpdate((InstanceMap*)map, diff);
    SCR_MAP_END;

    SCR_MAP_BGN(BattlegroundMapScript, map, itr, end, entry, IsBattleground);
        itr->second->OnUpdate((BattlegroundMap*)map, diff);
    SCR_MAP_END;
}

#undef SCR_MAP_BGN
#undef SCR_MAP_END

InstanceScript* ScriptMgr::CreateInstanceScript(InstanceMap* map)
{
    ASSERT(map);

    GET_SCRIPT_RET(InstanceMapScript, map->GetScriptId(), tmpscript, NULL);
    return tmpscript->GetInstanceScript(map);
}

bool ScriptMgr::OnQuestAccept(Player* player, Item* item, Quest const* quest)
{
    ASSERT(player);
    ASSERT(item);
    ASSERT(quest);

#ifdef ELUNA
    if (sEluna->OnQuestAccept(player, item, quest))
        return false;
#endif

    GET_SCRIPT_RET(ItemScript, item->GetScriptId(), tmpscript, false);
    player->PlayerTalkClass->ClearMenus();
    return tmpscript->OnQuestAccept(player, item, quest);
}

bool ScriptMgr::OnItemUse(Player* player, Item* item, SpellCastTargets const& targets)
{
    ASSERT(player);
    ASSERT(item);

#ifdef ELUNA
    if (!sEluna->OnUse(player, item, targets))
        return true;
#endif

    GET_SCRIPT_RET(ItemScript, item->GetScriptId(), tmpscript, false);
    return tmpscript->OnUse(player, item, targets);
}

bool ScriptMgr::OnItemExpire(Player* player, ItemTemplate const* proto)
{
    ASSERT(player);
    ASSERT(proto);

#ifdef ELUNA
    if (sEluna->OnExpire(player, proto))
        return false;
#endif

    GET_SCRIPT_RET(ItemScript, proto->ScriptId, tmpscript, false);
    return tmpscript->OnExpire(player, proto);
}

bool ScriptMgr::OnItemRemove(Player * player, Item * item)
{
    ASSERT(player);
    ASSERT(item);
#ifdef ELUNA
    if (sEluna->OnRemove(player, item))
        return false;
#endif
    GET_SCRIPT_RET(ItemScript, item->GetScriptId(), tmpscript, false);
    return tmpscript->OnRemove(player, item);

}

void ScriptMgr::OnGossipSelect(Player* player, Item* item, uint32 sender, uint32 action)
{
    ASSERT(player);
    ASSERT(item);
#ifdef ELUNA
    sEluna->HandleGossipSelectOption(player, item, sender, action, "");
#endif
    GET_SCRIPT(ItemScript, item->GetScriptId(), tmpscript);
    tmpscript->OnGossipSelect(player, item, sender, action);
}

void ScriptMgr::OnGossipSelectCode(Player* player, Item* item, uint32 sender, uint32 action, const char* code)
{
    ASSERT(player);
    ASSERT(item);
#ifdef ELUNA
    sEluna->HandleGossipSelectOption(player, item, sender, action, code);
#endif
    GET_SCRIPT(ItemScript, item->GetScriptId(), tmpscript);
    tmpscript->OnGossipSelectCode(player, item, sender, action, code);
}

void ScriptMgr::OnGossipSelect(Player* player, uint32 menu_id, uint32 sender, uint32 action)
{
#ifdef ELUNA
    sEluna->HandleGossipSelectOption(player, menu_id, sender, action, "");
#endif
    FOREACH_SCRIPT(PlayerScript)->OnGossipSelect(player, menu_id, sender, action);
}

void ScriptMgr::OnGossipSelectCode(Player* player, uint32 menu_id, uint32 sender, uint32 action, const char* code)
{
#ifdef ELUNA
    sEluna->HandleGossipSelectOption(player, menu_id, sender, action, code);
#endif
    FOREACH_SCRIPT(PlayerScript)->OnGossipSelectCode(player, menu_id, sender, action, code);
}

bool ScriptMgr::OnGossipHello(Player* player, Creature* creature)
{
    ASSERT(player);
    ASSERT(creature);
#ifdef ELUNA
    if (sEluna->OnGossipHello(player, creature))
        return true;
#endif
    GET_SCRIPT_RET(CreatureScript, creature->GetScriptId(), tmpscript, false);
    player->PlayerTalkClass->ClearMenus();
    return tmpscript->OnGossipHello(player, creature);
}

bool ScriptMgr::OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    ASSERT(player);
    ASSERT(creature);
#ifdef ELUNA
    if (sEluna->OnGossipSelect(player, creature, sender, action))
        return true;
#endif
    GET_SCRIPT_RET(CreatureScript, creature->GetScriptId(), tmpscript, false);
    return tmpscript->OnGossipSelect(player, creature, sender, action);
}

bool ScriptMgr::OnGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, const char* code)
{
    ASSERT(player);
    ASSERT(creature);
    ASSERT(code);
#ifdef ELUNA
    if (sEluna->OnGossipSelectCode(player, creature, sender, action, code))
        return true;
#endif
    GET_SCRIPT_RET(CreatureScript, creature->GetScriptId(), tmpscript, false);
    return tmpscript->OnGossipSelectCode(player, creature, sender, action, code);
}

bool ScriptMgr::OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
{
    ASSERT(player);
    ASSERT(creature);
    ASSERT(quest);

    GET_SCRIPT_RET(CreatureScript, creature->GetScriptId(), tmpscript, false);
    player->PlayerTalkClass->ClearMenus();
    return tmpscript->OnQuestAccept(player, creature, quest);
}

bool ScriptMgr::OnQuestSelect(Player* player, Creature* creature, Quest const* quest)
{
    ASSERT(player);
    ASSERT(creature);
    ASSERT(quest);

    GET_SCRIPT_RET(CreatureScript, creature->GetScriptId(), tmpscript, false);
    player->PlayerTalkClass->ClearMenus();
    return tmpscript->OnQuestSelect(player, creature, quest);
}

bool ScriptMgr::OnQuestComplete(Player* player, Creature* creature, Quest const* quest)
{
    ASSERT(player);
    ASSERT(creature);
    ASSERT(quest);

    GET_SCRIPT_RET(CreatureScript, creature->GetScriptId(), tmpscript, false);
    player->PlayerTalkClass->ClearMenus();
    return tmpscript->OnQuestComplete(player, creature, quest);
}

bool ScriptMgr::OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 opt)
{
    ASSERT(player);
    ASSERT(creature);
    ASSERT(quest);
#ifdef ELUNA
    if (sEluna->OnQuestReward(player, creature, quest, opt))
    {
        player->PlayerTalkClass->ClearMenus();
        return false;
    }
#endif
    GET_SCRIPT_RET(CreatureScript, creature->GetScriptId(), tmpscript, false);
    player->PlayerTalkClass->ClearMenus();
    return tmpscript->OnQuestReward(player, creature, quest, opt);
}

uint32 ScriptMgr::GetDialogStatus(Player* player, Creature* creature)
{
    ASSERT(player);
    ASSERT(creature);

    GET_SCRIPT_RET(CreatureScript, creature->GetScriptId(), tmpscript, DIALOG_STATUS_SCRIPTED_NO_STATUS);
    player->PlayerTalkClass->ClearMenus();
    return tmpscript->GetDialogStatus(player, creature);
}

CreatureAI* ScriptMgr::GetCreatureAI(Creature* creature)
{
    ASSERT(creature);

#ifdef ELUNA
    if (CreatureAI* luaAI = sEluna->GetAI(creature))
        return luaAI;
#endif

    GET_SCRIPT_RET(CreatureScript, creature->GetScriptId(), tmpscript, NULL);
    return tmpscript->GetAI(creature);
}

void ScriptMgr::OnCreatureUpdate(Creature* creature, uint32 diff)
{
    ASSERT(creature);

    FOREACH_SCRIPT(AllCreatureScript)->OnAllCreatureUpdate(creature, diff);

    GET_SCRIPT(CreatureScript, creature->GetScriptId(), tmpscript);
    tmpscript->OnUpdate(creature, diff);
}

bool ScriptMgr::OnGossipHello(Player* player, GameObject* go)
{
    ASSERT(player);
    ASSERT(go);
#ifdef ELUNA
    if (sEluna->OnGossipHello(player, go))
        return true;
    if (sEluna->OnGameObjectUse(player, go))
        return true;
#endif
    GET_SCRIPT_RET(GameObjectScript, go->GetScriptId(), tmpscript, false);
    player->PlayerTalkClass->ClearMenus();
    return tmpscript->OnGossipHello(player, go);
}

bool ScriptMgr::OnGossipSelect(Player* player, GameObject* go, uint32 sender, uint32 action)
{
    ASSERT(player);
    ASSERT(go);
#ifdef ELUNA
    if (sEluna->OnGossipSelect(player, go, sender, action))
        return true;
#endif
    GET_SCRIPT_RET(GameObjectScript, go->GetScriptId(), tmpscript, false);
    return tmpscript->OnGossipSelect(player, go, sender, action);
}

bool ScriptMgr::OnGossipSelectCode(Player* player, GameObject* go, uint32 sender, uint32 action, const char* code)
{
    ASSERT(player);
    ASSERT(go);
    ASSERT(code);
#ifdef ELUNA
    if (sEluna->OnGossipSelectCode(player, go, sender, action, code))
        return true;
#endif
    GET_SCRIPT_RET(GameObjectScript, go->GetScriptId(), tmpscript, false);
    return tmpscript->OnGossipSelectCode(player, go, sender, action, code);
}

bool ScriptMgr::OnQuestAccept(Player* player, GameObject* go, Quest const* quest)
{
    ASSERT(player);
    ASSERT(go);
    ASSERT(quest);

    GET_SCRIPT_RET(GameObjectScript, go->GetScriptId(), tmpscript, false);
    player->PlayerTalkClass->ClearMenus();
    return tmpscript->OnQuestAccept(player, go, quest);
}

bool ScriptMgr::OnQuestReward(Player* player, GameObject* go, Quest const* quest, uint32 opt)
{
    ASSERT(player);
    ASSERT(go);
    ASSERT(quest);
#ifdef ELUNA
    if (sEluna->OnQuestAccept(player, go, quest))
        return false;
#endif
#ifdef ELUNA
    if (sEluna->OnQuestReward(player, go, quest, opt))
        return false;
#endif
    GET_SCRIPT_RET(GameObjectScript, go->GetScriptId(), tmpscript, false);
    player->PlayerTalkClass->ClearMenus();
    return tmpscript->OnQuestReward(player, go, quest, opt);
}

uint32 ScriptMgr::GetDialogStatus(Player* player, GameObject* go)
{
    ASSERT(player);
    ASSERT(go);

    GET_SCRIPT_RET(GameObjectScript, go->GetScriptId(), tmpscript, DIALOG_STATUS_SCRIPTED_NO_STATUS);
    player->PlayerTalkClass->ClearMenus();
    return tmpscript->GetDialogStatus(player, go);
}

void ScriptMgr::OnGameObjectDestroyed(GameObject* go, Player* player)
{
    ASSERT(go);

    GET_SCRIPT(GameObjectScript, go->GetScriptId(), tmpscript);
    tmpscript->OnDestroyed(go, player);
}

void ScriptMgr::OnGameObjectDamaged(GameObject* go, Player* player)
{
    ASSERT(go);

    GET_SCRIPT(GameObjectScript, go->GetScriptId(), tmpscript);
    tmpscript->OnDamaged(go, player);
}

void ScriptMgr::OnGameObjectLootStateChanged(GameObject* go, uint32 state, Unit* unit)
{
    ASSERT(go);

    GET_SCRIPT(GameObjectScript, go->GetScriptId(), tmpscript);
    tmpscript->OnLootStateChanged(go, state, unit);
}

void ScriptMgr::OnGameObjectStateChanged(GameObject* go, uint32 state)
{
    ASSERT(go);

    GET_SCRIPT(GameObjectScript, go->GetScriptId(), tmpscript);
    tmpscript->OnGameObjectStateChanged(go, state);
}

void ScriptMgr::OnGameObjectUpdate(GameObject* go, uint32 diff)
{
    ASSERT(go);

#ifdef ELUNA
    sEluna->UpdateAI(go, diff);
#endif

    GET_SCRIPT(GameObjectScript, go->GetScriptId(), tmpscript);
    tmpscript->OnUpdate(go, diff);
}

GameObjectAI* ScriptMgr::GetGameObjectAI(GameObject* go)
{
    ASSERT(go);

#ifdef ELUNA
    sEluna->OnSpawn(go);
#endif

    GET_SCRIPT_RET(GameObjectScript, go->GetScriptId(), tmpscript, NULL);
    return tmpscript->GetAI(go);
}

bool ScriptMgr::OnAreaTrigger(Player* player, AreaTrigger const* trigger)
{
    ASSERT(player);
    ASSERT(trigger);
#ifdef ELUNA
    if (sEluna->OnAreaTrigger(player, trigger))
        return false;
#endif
    GET_SCRIPT_RET(AreaTriggerScript, sObjectMgr->GetAreaTriggerScriptId(trigger->entry), tmpscript, false);
    return tmpscript->OnTrigger(player, trigger);
}

Battleground* ScriptMgr::CreateBattleground(BattlegroundTypeId /*typeId*/)
{
    // TODO: Implement script-side battlegrounds.
    ASSERT(false);
    return NULL;
}

OutdoorPvP* ScriptMgr::CreateOutdoorPvP(OutdoorPvPData const* data)
{
    ASSERT(data);

    GET_SCRIPT_RET(OutdoorPvPScript, data->ScriptId, tmpscript, NULL);
    return tmpscript->GetOutdoorPvP();
}

std::vector<ChatCommand> ScriptMgr::GetChatCommands()
{
    std::vector<ChatCommand> table;

    FOR_SCRIPTS_RET(CommandScript, itr, end, table)
    {
        std::vector<ChatCommand> cmds = itr->second->GetCommands();
        table.insert(table.end(), cmds.begin(), cmds.end());
    }

    // Sort commands in alphabetical order
    std::sort(table.begin(), table.end(), [](const ChatCommand& a, const ChatCommand&b)
    {
        return strcmp(a.Name, b.Name) < 0;
    });

    return table;
}

void ScriptMgr::OnWeatherChange(Weather* weather, WeatherState state, float grade)
{
    ASSERT(weather);

#ifdef ELUNA
    sEluna->OnChange(weather, weather->GetZone(), state, grade);
#endif

    GET_SCRIPT(WeatherScript, weather->GetScriptId(), tmpscript);
    tmpscript->OnChange(weather, state, grade);
}

void ScriptMgr::OnWeatherUpdate(Weather* weather, uint32 diff)
{
    ASSERT(weather);

    GET_SCRIPT(WeatherScript, weather->GetScriptId(), tmpscript);
    tmpscript->OnUpdate(weather, diff);
}

void ScriptMgr::OnAuctionAdd(AuctionHouseObject* ah, AuctionEntry* entry)
{
    ASSERT(ah);
    ASSERT(entry);

#ifdef ELUNA
    sEluna->OnAdd(ah, entry);
#endif

    FOREACH_SCRIPT(AuctionHouseScript)->OnAuctionAdd(ah, entry);
}

void ScriptMgr::OnAuctionRemove(AuctionHouseObject* ah, AuctionEntry* entry)
{
    ASSERT(ah);
    ASSERT(entry);

#ifdef ELUNA
    sEluna->OnRemove(ah, entry);
#endif

    FOREACH_SCRIPT(AuctionHouseScript)->OnAuctionRemove(ah, entry);
}

void ScriptMgr::OnAuctionSuccessful(AuctionHouseObject* ah, AuctionEntry* entry)
{
    ASSERT(ah);
    ASSERT(entry);

#ifdef ELUNA
    sEluna->OnSuccessful(ah, entry);
#endif

    FOREACH_SCRIPT(AuctionHouseScript)->OnAuctionSuccessful(ah, entry);
}

void ScriptMgr::OnAuctionExpire(AuctionHouseObject* ah, AuctionEntry* entry)
{
    ASSERT(ah);
    ASSERT(entry);

#ifdef ELUNA
    sEluna->OnExpire(ah, entry);
#endif

    FOREACH_SCRIPT(AuctionHouseScript)->OnAuctionExpire(ah, entry);
}

void ScriptMgr::OnSendAuctionSuccessfulMail(bool& SkipCoreCode, AuctionHouseMgr* me, AuctionEntry* auction, SQLTransaction& trans, Player* owner, uint64 owner_guid, uint32 owner_accId)
{
    FOREACH_SCRIPT(AuctionHouseScript)->OnSendAuctionSuccessfulMail(SkipCoreCode, me, auction, trans, owner, owner_guid, owner_accId);
}

void ScriptMgr::OnSendAuctionExpiredMail(bool& SkipCoreCode, AuctionHouseMgr* me, AuctionEntry* auction, SQLTransaction& trans, Player* owner, uint64 owner_guid, uint32 owner_accId, Item* pItem)
{
    FOREACH_SCRIPT(AuctionHouseScript)->OnSendAuctionExpiredMail(SkipCoreCode, me, auction, trans, owner, owner_guid, owner_accId, pItem);
}
void ScriptMgr::OnSendAuctionOutbiddedMail(bool& SkipCoreCode, AuctionHouseMgr* me, AuctionEntry* auction, SQLTransaction& trans, uint32 newPrice, Player* newBidder, uint64 oldBidder_guid, Player* oldBidder)
{
    FOREACH_SCRIPT(AuctionHouseScript)->OnSendAuctionOutbiddedMail(SkipCoreCode, me, auction, trans, newPrice, newBidder, oldBidder_guid, oldBidder);
}

void ScriptMgr::OnAuctionHouseUpdate(AuctionHouseMgr* me)
{
    FOREACH_SCRIPT(AuctionHouseScript)->OnAuctionHouseUpdate(me);
}
bool ScriptMgr::OnConditionCheck(Condition* condition, ConditionSourceInfo& sourceInfo)
{
    ASSERT(condition);

    GET_SCRIPT_RET(ConditionScript, condition->ScriptId, tmpscript, true);
    return tmpscript->OnConditionCheck(condition, sourceInfo);
}

void ScriptMgr::OnInstall(Vehicle* veh)
{
    ASSERT(veh);
    ASSERT(veh->GetBase()->GetTypeId() == TYPEID_UNIT);

#ifdef ELUNA
    sEluna->OnInstall(veh);
#endif

    GET_SCRIPT(VehicleScript, veh->GetBase()->ToCreature()->GetScriptId(), tmpscript);
    tmpscript->OnInstall(veh);
}

void ScriptMgr::OnUninstall(Vehicle* veh)
{
    ASSERT(veh);
    ASSERT(veh->GetBase()->GetTypeId() == TYPEID_UNIT);

#ifdef ELUNA
    sEluna->OnUninstall(veh);
#endif

    GET_SCRIPT(VehicleScript, veh->GetBase()->ToCreature()->GetScriptId(), tmpscript);
    tmpscript->OnUninstall(veh);
}

void ScriptMgr::OnReset(Vehicle* veh)
{
    ASSERT(veh);
    ASSERT(veh->GetBase()->GetTypeId() == TYPEID_UNIT);

    GET_SCRIPT(VehicleScript, veh->GetBase()->ToCreature()->GetScriptId(), tmpscript);
    tmpscript->OnReset(veh);
}

void ScriptMgr::OnInstallAccessory(Vehicle* veh, Creature* accessory)
{
    ASSERT(veh);
    ASSERT(veh->GetBase()->GetTypeId() == TYPEID_UNIT);
    ASSERT(accessory);

#ifdef ELUNA
    sEluna->OnInstallAccessory(veh, accessory);
#endif

    GET_SCRIPT(VehicleScript, veh->GetBase()->ToCreature()->GetScriptId(), tmpscript);
    tmpscript->OnInstallAccessory(veh, accessory);
}

void ScriptMgr::OnAddPassenger(Vehicle* veh, Unit* passenger, int8 seatId)
{
    ASSERT(veh);
    ASSERT(veh->GetBase()->GetTypeId() == TYPEID_UNIT);
    ASSERT(passenger);

#ifdef ELUNA
    sEluna->OnAddPassenger(veh, passenger, seatId);
#endif

    GET_SCRIPT(VehicleScript, veh->GetBase()->ToCreature()->GetScriptId(), tmpscript);
    tmpscript->OnAddPassenger(veh, passenger, seatId);
}

void ScriptMgr::OnRemovePassenger(Vehicle* veh, Unit* passenger)
{
    ASSERT(veh);
    ASSERT(veh->GetBase()->GetTypeId() == TYPEID_UNIT);
    ASSERT(passenger);

#ifdef ELUNA
    sEluna->OnRemovePassenger(veh, passenger);
#endif

    GET_SCRIPT(VehicleScript, veh->GetBase()->ToCreature()->GetScriptId(), tmpscript);
    tmpscript->OnRemovePassenger(veh, passenger);
}

void ScriptMgr::OnDynamicObjectUpdate(DynamicObject* dynobj, uint32 diff)
{
    ASSERT(dynobj);

    FOR_SCRIPTS(DynamicObjectScript, itr, end)
        itr->second->OnUpdate(dynobj, diff);
}

void ScriptMgr::OnAddPassenger(Transport* transport, Player* player)
{
    ASSERT(transport);
    ASSERT(player);

    GET_SCRIPT(TransportScript, transport->GetScriptId(), tmpscript);
    tmpscript->OnAddPassenger(transport, player);
}

void ScriptMgr::OnAddCreaturePassenger(Transport* transport, Creature* creature)
{
    ASSERT(transport);
    ASSERT(creature);

    GET_SCRIPT(TransportScript, transport->GetScriptId(), tmpscript);
    tmpscript->OnAddCreaturePassenger(transport, creature);
}

void ScriptMgr::OnRemovePassenger(Transport* transport, Player* player)
{
    ASSERT(transport);
    ASSERT(player);

    GET_SCRIPT(TransportScript, transport->GetScriptId(), tmpscript);
    tmpscript->OnRemovePassenger(transport, player);
}

void ScriptMgr::OnTransportUpdate(Transport* transport, uint32 diff)
{
    ASSERT(transport);

    GET_SCRIPT(TransportScript, transport->GetScriptId(), tmpscript);
    tmpscript->OnUpdate(transport, diff);
}

void ScriptMgr::OnRelocate(Transport* transport, uint32 waypointId, uint32 mapId, float x, float y, float z)
{
    GET_SCRIPT(TransportScript, transport->GetScriptId(), tmpscript);
    tmpscript->OnRelocate(transport, waypointId, mapId, x, y, z);
}

void ScriptMgr::OnStartup()
{
#ifdef ELUNA
    sEluna->OnStartup();
#endif
    FOREACH_SCRIPT(WorldScript)->OnStartup();
}

void ScriptMgr::OnShutdown()
{
#ifdef ELUNA
    sEluna->OnShutdown();
#endif
    FOREACH_SCRIPT(WorldScript)->OnShutdown();
}

bool ScriptMgr::OnCriteriaCheck(uint32 scriptId, Player* source, Unit* target, uint32 criteria_id)
{
    ASSERT(source);
    // target can be NULL.

    GET_SCRIPT_RET(AchievementCriteriaScript, scriptId, tmpscript, false);
    return tmpscript->OnCheck(source, target, criteria_id);
}

// Player
void ScriptMgr::OnPlayerReleasedGhost(Player* player)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerReleasedGhost(player);
}

void ScriptMgr::OnPVPKill(Player* killer, Player* killed)
{
#ifdef ELUNA
    sEluna->OnPVPKill(killer, killed);
#endif
    FOREACH_SCRIPT(PlayerScript)->OnPVPKill(killer, killed);
}

void ScriptMgr::OnCreatureKill(Player* killer, Creature* killed)
{
#ifdef ELUNA
    sEluna->OnCreatureKill(killer, killed);
#endif
    FOREACH_SCRIPT(PlayerScript)->OnCreatureKill(killer, killed);
}

void ScriptMgr::OnCreatureKilledByPet(Player* petOwner, Creature* killed)
{
    FOREACH_SCRIPT(PlayerScript)->OnCreatureKilledByPet(petOwner, killed);
}

void ScriptMgr::OnPlayerKilledByCreature(Creature* killer, Player* killed)
{
#ifdef ELUNA
    sEluna->OnPlayerKilledByCreature(killer, killed);
#endif
    FOREACH_SCRIPT(PlayerScript)->OnPlayerKilledByCreature(killer, killed);
}

void ScriptMgr::OnPlayerLevelChanged(Player* player, uint8 oldLevel)
{
#ifdef ELUNA
    sEluna->OnLevelChanged(player, oldLevel);
#endif
    FOREACH_SCRIPT(PlayerScript)->OnLevelChanged(player, oldLevel);
}

void ScriptMgr::OnPlayerFreeTalentPointsChanged(Player* player, uint32 points)
{
#ifdef ELUNA
    sEluna->OnFreeTalentPointsChanged(player, points);
#endif
    FOREACH_SCRIPT(PlayerScript)->OnFreeTalentPointsChanged(player, points);
}

void ScriptMgr::OnPlayerTalentsReset(Player* player, bool noCost)
{
#ifdef ELUNA
    sEluna->OnTalentsReset(player, noCost);
#endif
    FOREACH_SCRIPT(PlayerScript)->OnTalentsReset(player, noCost);
}

void ScriptMgr::OnPlayerMoneyChanged(Player* player, int32& amount)
{
#ifdef ELUNA
    sEluna->OnMoneyChanged(player, amount);
#endif
    FOREACH_SCRIPT(PlayerScript)->OnMoneyChanged(player, amount);
}

void ScriptMgr::OnGivePlayerXP(Player* player, uint32& amount, Unit* victim)
{
#ifdef ELUNA
    sEluna->OnGiveXP(player, amount, victim);
#endif
    FOREACH_SCRIPT(PlayerScript)->OnGiveXP(player, amount, victim);
}

void ScriptMgr::OnPlayerReputationChange(Player* player, uint32 factionID, int32& standing, bool incremental)
{
#ifdef ELUNA
    sEluna->OnReputationChange(player, factionID, standing, incremental);
#endif
    FOREACH_SCRIPT(PlayerScript)->OnReputationChange(player, factionID, standing, incremental);
}

void ScriptMgr::OnPlayerDuelRequest(Player* target, Player* challenger)
{
#ifdef ELUNA
    sEluna->OnDuelRequest(target, challenger);
#endif
    FOREACH_SCRIPT(PlayerScript)->OnDuelRequest(target, challenger);
}

void ScriptMgr::OnPlayerDuelStart(Player* player1, Player* player2)
{
#ifdef ELUNA
    sEluna->OnDuelStart(player1, player2);
#endif
    FOREACH_SCRIPT(PlayerScript)->OnDuelStart(player1, player2);
}

void ScriptMgr::OnPlayerDuelEnd(Player* winner, Player* loser, DuelCompleteType type)
{
#ifdef ELUNA
    sEluna->OnDuelEnd(winner, loser, type);
#endif
    FOREACH_SCRIPT(PlayerScript)->OnDuelEnd(winner, loser, type);
}

void ScriptMgr::OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg)
{
    FOREACH_SCRIPT(PlayerScript)->OnChat(player, type, lang, msg);
}

void ScriptMgr::OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Player* receiver)
{
    FOREACH_SCRIPT(PlayerScript)->OnChat(player, type, lang, msg, receiver);
}

void ScriptMgr::OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Group* group, bool& sendit)
{
    FOREACH_SCRIPT(PlayerScript)->OnChat(player, type, lang, msg, group, sendit);
}

void ScriptMgr::OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Guild* guild, bool& sendit)
{
    FOREACH_SCRIPT(PlayerScript)->OnChat(player, type, lang, msg, guild, sendit);
}

void ScriptMgr::OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg, Channel* channel)
{
    FOREACH_SCRIPT(PlayerScript)->OnChat(player, type, lang, msg, channel);
}

void ScriptMgr::OnPlayerEmote(Player* player, uint32 emote)
{
#ifdef ELUNA
    sEluna->OnEmote(player, emote);
#endif
    FOREACH_SCRIPT(PlayerScript)->OnEmote(player, emote);
}

void ScriptMgr::OnPlayerTextEmote(Player* player, uint32 textEmote, uint32 emoteNum, uint64 guid)
{
#ifdef ELUNA
    sEluna->OnTextEmote(player, textEmote, emoteNum, guid);
#endif
    FOREACH_SCRIPT(PlayerScript)->OnTextEmote(player, textEmote, emoteNum, guid);
}

void ScriptMgr::OnPlayerSpellCast(Player* player, Spell* spell, bool skipCheck, bool SkipOtherCode)
{
#ifdef ELUNA
    sEluna->OnSpellCast(player, spell, skipCheck);
#endif
    FOREACH_SCRIPT(PlayerScript)->OnSpellCast(player, spell, skipCheck, SkipOtherCode);
}

void ScriptMgr::OnBeforePlayerUpdate(Player* player, uint32 p_time)
{
    FOREACH_SCRIPT(PlayerScript)->OnBeforeUpdate(player, p_time);
}

void ScriptMgr::OnPlayerLogin(Player* player)
{
#ifdef ELUNA
    sEluna->OnLogin(player);
#endif
    FOREACH_SCRIPT(PlayerScript)->OnLogin(player);
}

void ScriptMgr::OnPlayerLoadFromDB(Player* player)
{
    FOREACH_SCRIPT(PlayerScript)->OnLoadFromDB(player);
}

void ScriptMgr::OnPlayerLogout(Player* player)
{
#ifdef ELUNA
    sEluna->OnLogout(player);
#endif
    FOREACH_SCRIPT(PlayerScript)->OnLogout(player);
}

void ScriptMgr::OnPlayerCreate(Player* player)
{
#ifdef ELUNA
    sEluna->OnCreate(player);
#endif
    FOREACH_SCRIPT(PlayerScript)->OnCreate(player);
}

void ScriptMgr::OnPlayerSave(Player * player)
{
#ifdef ELUNA
    sEluna->OnSave(player);
#endif
    FOREACH_SCRIPT(PlayerScript)->OnSave(player);
}

void ScriptMgr::OnPlayerDelete(uint64 guid)
{
#ifdef ELUNA
    sEluna->OnDelete(GUID_LOPART(guid));
#endif
    FOREACH_SCRIPT(PlayerScript)->OnDelete(guid);
}

void ScriptMgr::OnPlayerBindToInstance(Player* player, Difficulty difficulty, uint32 mapid, bool permanent)
{
#ifdef ELUNA
    sEluna->OnBindToInstance(player, difficulty, mapid, permanent);
#endif
    FOREACH_SCRIPT(PlayerScript)->OnBindToInstance(player, difficulty, mapid, permanent);
}

void ScriptMgr::OnPlayerUpdateZone(Player* player, uint32 newZone, uint32 newArea)
{
#ifdef ELUNA
    sEluna->OnUpdateZone(player, newZone, newArea);
#endif
    FOREACH_SCRIPT(PlayerScript)->OnUpdateZone(player, newZone, newArea);
}

void ScriptMgr::OnPlayerUpdateArea(Player* player, uint32 oldArea, uint32 newArea)
{
    FOREACH_SCRIPT(PlayerScript)->OnUpdateArea(player, oldArea, newArea);
}

bool ScriptMgr::OnBeforePlayerTeleport(Player* player, uint32 mapid, float x, float y, float z, float orientation, uint32 options, Unit *target)
{
    bool ret=true;
    FOR_SCRIPTS_RET(PlayerScript, itr, end, ret) // return true by default if not scripts
        if (!itr->second->OnBeforeTeleport(player, mapid, x, y, z, orientation, options, target))
            ret=false; // we change ret value only when scripts return false

    return ret;
}

void ScriptMgr::OnPlayerUpdateFaction(Player* player)
{
    FOREACH_SCRIPT(PlayerScript)->OnUpdateFaction(player);
}

void ScriptMgr::OnPlayerAddToBattleground(Player* player, Battleground *bg)
{
    FOREACH_SCRIPT(PlayerScript)->OnAddToBattleground(player, bg);
}

void ScriptMgr::OnPlayerRemoveFromBattleground(Player* player, Battleground* bg)
{
    FOREACH_SCRIPT(PlayerScript)->OnRemoveFromBattleground(player, bg);
}

void ScriptMgr::OnAchievementComplete(Player* player, AchievementEntry const* achievement)
{
    FOREACH_SCRIPT(PlayerScript)->OnAchiComplete(player, achievement);
}

void ScriptMgr::OnCriteriaProgress(Player* player, AchievementCriteriaEntry const* criteria)
{
    FOREACH_SCRIPT(PlayerScript)->OnCriteriaProgress(player, criteria);
}

void ScriptMgr::OnAchievementSave(SQLTransaction& trans, Player* player, uint16 achiId, CompletedAchievementData achiData)
{
    FOREACH_SCRIPT(PlayerScript)->OnAchiSave(trans, player, achiId, achiData);
}

void ScriptMgr::OnCriteriaSave(SQLTransaction& trans, Player* player, uint16 critId, CriteriaProgress criteriaData)
{
    FOREACH_SCRIPT(PlayerScript)->OnCriteriaSave(trans, player, critId, criteriaData);
}

void ScriptMgr::OnPlayerBeingCharmed(Player* player, Unit* charmer, uint32 oldFactionId, uint32 newFactionId)
{
    FOREACH_SCRIPT(PlayerScript)->OnBeingCharmed(player, charmer, oldFactionId, newFactionId);
}

void ScriptMgr::OnAfterPlayerSetVisibleItemSlot(Player* player, uint8 slot, Item *item)
{
    FOREACH_SCRIPT(PlayerScript)->OnAfterSetVisibleItemSlot(player, slot,item);
}

void ScriptMgr::OnAfterPlayerMoveItemFromInventory(Player* player, Item* it, uint8 bag, uint8 slot, bool update)
{
    FOREACH_SCRIPT(PlayerScript)->OnAfterMoveItemFromInventory(player, it, bag, slot, update);
}

void ScriptMgr::OnEquip(Player* player, Item* it, uint8 bag, uint8 slot, bool update)
{
    FOREACH_SCRIPT(PlayerScript)->OnEquip(player, it, bag, slot, update);
}

void ScriptMgr::OnPlayerJoinBG(Player* player)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerJoinBG(player);
}

void ScriptMgr::OnPlayerJoinArena(Player* player)
{
    FOREACH_SCRIPT(PlayerScript)->OnPlayerJoinArena(player);
}

void ScriptMgr::OnLootItem(Player* player, Item* item, uint32 count, uint64 lootguid)
{
    FOREACH_SCRIPT(PlayerScript)->OnLootItem(player, item, count, lootguid);
}

void ScriptMgr::OnCreateItem(Player* player, Item* item, uint32 count)
{
    FOREACH_SCRIPT(PlayerScript)->OnCreateItem(player, item, count);
}

void ScriptMgr::OnQuestRewardItem(Player* player, Item* item, uint32 count)
{
    FOREACH_SCRIPT(PlayerScript)->OnQuestRewardItem(player, item, count);
}

void ScriptMgr::OnFirstLogin(Player* player)
{
#ifdef ELUNA
    sEluna->OnFirstLogin(player);
#endif
    FOREACH_SCRIPT(PlayerScript)->OnFirstLogin(player);
}

// Guild
void ScriptMgr::OnGuildAddMember(Guild* guild, Player* player, uint8& plRank)
{
#ifdef ELUNA
    sEluna->OnAddMember(guild, player, plRank);
#endif
    FOREACH_SCRIPT(GuildScript)->OnAddMember(guild, player, plRank);
}

void ScriptMgr::OnGuildRemoveMember(Guild* guild, Player* player, bool isDisbanding, bool isKicked)
{
#ifdef ELUNA
    sEluna->OnRemoveMember(guild, player, isDisbanding);
#endif
    FOREACH_SCRIPT(GuildScript)->OnRemoveMember(guild, player, isDisbanding, isKicked);
}

void ScriptMgr::OnGuildMOTDChanged(Guild* guild, const std::string& newMotd)
{
#ifdef ELUNA
    sEluna->OnMOTDChanged(guild, newMotd);
#endif
    FOREACH_SCRIPT(GuildScript)->OnMOTDChanged(guild, newMotd);
}

void ScriptMgr::OnGuildInfoChanged(Guild* guild, const std::string& newInfo)
{
#ifdef ELUNA
    sEluna->OnInfoChanged(guild, newInfo);
#endif
    FOREACH_SCRIPT(GuildScript)->OnInfoChanged(guild, newInfo);
}

void ScriptMgr::OnGuildCreate(Guild* guild, Player* leader, const std::string& name)
{
#ifdef ELUNA
    sEluna->OnCreate(guild, leader, name);
#endif
    FOREACH_SCRIPT(GuildScript)->OnCreate(guild, leader, name);
}

void ScriptMgr::OnGuildDisband(Guild* guild)
{
#ifdef ELUNA
    sEluna->OnDisband(guild);
#endif
    FOREACH_SCRIPT(GuildScript)->OnDisband(guild);
}

void ScriptMgr::OnGuildMemberWitdrawMoney(Guild* guild, Player* player, uint32 &amount, bool isRepair)
{
#ifdef ELUNA
    sEluna->OnMemberWitdrawMoney(guild, player, amount, isRepair);
#endif
    FOREACH_SCRIPT(GuildScript)->OnMemberWitdrawMoney(guild, player, amount, isRepair);
}

void ScriptMgr::OnGuildMemberDepositMoney(Guild* guild, Player* player, uint32 &amount)
{
#ifdef ELUNA
    sEluna->OnMemberDepositMoney(guild, player, amount);
#endif
    FOREACH_SCRIPT(GuildScript)->OnMemberDepositMoney(guild, player, amount);
}

void ScriptMgr::OnGuildItemMove(Guild* guild, Player* player, Item* pItem, bool isSrcBank, uint8 srcContainer, uint8 srcSlotId,
            bool isDestBank, uint8 destContainer, uint8 destSlotId)
{
#ifdef ELUNA
    sEluna->OnItemMove(guild, player, pItem, isSrcBank, srcContainer, srcSlotId, isDestBank, destContainer, destSlotId);
#endif
    FOREACH_SCRIPT(GuildScript)->OnItemMove(guild, player, pItem, isSrcBank, srcContainer, srcSlotId, isDestBank, destContainer, destSlotId);
}

void ScriptMgr::OnGuildEvent(Guild* guild, uint8 eventType, uint32 playerGuid1, uint32 playerGuid2, uint8 newRank)
{
#ifdef ELUNA
    sEluna->OnEvent(guild, eventType, playerGuid1, playerGuid2, newRank);
#endif
    FOREACH_SCRIPT(GuildScript)->OnEvent(guild, eventType, playerGuid1, playerGuid2, newRank);
}

void ScriptMgr::OnGuildBankEvent(Guild* guild, uint8 eventType, uint8 tabId, uint32 playerGuid, uint32 itemOrMoney, uint16 itemStackCount, uint8 destTabId)
{
#ifdef ELUNA
    sEluna->OnBankEvent(guild, eventType, tabId, playerGuid, itemOrMoney, itemStackCount, destTabId);
#endif
    FOREACH_SCRIPT(GuildScript)->OnBankEvent(guild, eventType, tabId, playerGuid, itemOrMoney, itemStackCount, destTabId);
}

void ScriptMgr::OnBoradcastToGuild(bool& SkipCoreCode, const Guild* me, WorldSession* session, bool officerOnly, std::string const&  msg, uint32 language)
{
FOREACH_SCRIPT(GuildScript)->OnBoradcastToGuild(SkipCoreCode, me, session, officerOnly, msg, language);
}

void ScriptMgr::OnBroadcastPacketToRank(bool& SkipCoreCode, const Guild* me, WorldPacket* packet, uint8 rankId)
{
    FOREACH_SCRIPT(GuildScript)->OnBroadcastPacketToRank(SkipCoreCode, me, packet, rankId);
}

void ScriptMgr::OnBroadcastPacket(bool& SkipCoreCode, const Guild* me, WorldPacket* packet)
{
    FOREACH_SCRIPT(GuildScript)->OnBroadcastPacket(SkipCoreCode, me, packet);
}
// Group
void ScriptMgr::OnGroupAddMember(Group* group, uint64 guid)
{
    ASSERT(group);
#ifdef ELUNA
        sEluna->OnAddMember(group, guid);
#endif
    FOREACH_SCRIPT(GroupScript)->OnAddMember(group, guid);
}

void ScriptMgr::OnGroupInviteMember(Group* group, uint64 guid)
{
    ASSERT(group);
#ifdef ELUNA
    sEluna->OnInviteMember(group, guid);
#endif
    FOREACH_SCRIPT(GroupScript)->OnInviteMember(group, guid);
}

void ScriptMgr::OnGroupRemoveMember(Group* group, uint64 guid, RemoveMethod method, uint64 kicker, const char* reason)
{
    ASSERT(group);
#ifdef ELUNA
    sEluna->OnRemoveMember(group, guid, method);
#endif
    FOREACH_SCRIPT(GroupScript)->OnRemoveMember(group, guid, method, kicker, reason);
}

void ScriptMgr::OnGroupChangeLeader(Group* group, uint64 newLeaderGuid, uint64 oldLeaderGuid)
{
    ASSERT(group);
#ifdef ELUNA
    sEluna->OnChangeLeader(group, newLeaderGuid, oldLeaderGuid);
#endif
    FOREACH_SCRIPT(GroupScript)->OnChangeLeader(group, newLeaderGuid, oldLeaderGuid);
}

void ScriptMgr::OnGroupDisband(Group* group)
{
    ASSERT(group);
#ifdef ELUNA
    sEluna->OnDisband(group);
#endif
    FOREACH_SCRIPT(GroupScript)->OnDisband(group);
}

void ScriptMgr::OnGroupBroadcastPacket(bool& SkipCoreCode, Group* me, WorldPacket* packet,bool& ignorePlayersInBGRaid, int& group, uint64& ignore)
{
    FOREACH_SCRIPT(GroupScript)->OnGroupBroadcastPacket(SkipCoreCode, me, packet, ignorePlayersInBGRaid, group, ignore);
}

void ScriptMgr::OnBroadcastReadyCheck(bool& SkipCoreCode, Group* me, WorldPacket* packet)
{
    FOREACH_SCRIPT(GroupScript)->OnBroadcastReadyCheck(SkipCoreCode, me, packet);
}
void ScriptMgr::OnOfflineReadyCheck(bool& SkipCoreCode,Group* me, Group::MemberSlotList& m_memberSlots)
{
    FOREACH_SCRIPT(GroupScript)->OnOfflineReadyCheck(SkipCoreCode, me, m_memberSlots);
}
void ScriptMgr::OnGlobalItemDelFromDB(SQLTransaction& trans, uint32 itemGuid)
{
    ASSERT(trans);
    ASSERT(itemGuid);

    FOREACH_SCRIPT(GlobalScript)->OnItemDelFromDB(trans,itemGuid);
}

void ScriptMgr::OnGlobalMirrorImageDisplayItem(const Item *item, uint32 &display)
{
    FOREACH_SCRIPT(GlobalScript)->OnMirrorImageDisplayItem(item,display);
}

void ScriptMgr::OnBeforeUpdateArenaPoints(ArenaTeam* at, std::map<uint32, uint32> &ap)
{
    FOREACH_SCRIPT(GlobalScript)->OnBeforeUpdateArenaPoints(at,ap);
}

void ScriptMgr::OnAfterRefCount(Player const* player, Loot& loot, bool canRate, uint16 lootMode, LootStoreItem* LootStoreItem, uint32 &maxcount, LootStore const& store)
{
    FOREACH_SCRIPT(GlobalScript)->OnAfterRefCount(player, LootStoreItem, loot, canRate, lootMode, maxcount, store);
}

void ScriptMgr::OnBeforeDropAddItem(Player const* player, Loot& loot, bool canRate, uint16 lootMode, LootStoreItem* LootStoreItem, LootStore const& store)
{
    FOREACH_SCRIPT(GlobalScript)->OnBeforeDropAddItem(player, loot, canRate, lootMode, LootStoreItem, store);
}

void ScriptMgr::OnItemRoll(Player const* player, LootStoreItem const* LootStoreItem, float &chance, Loot& loot, LootStore const& store) {
    FOREACH_SCRIPT(GlobalScript)->OnItemRoll(player, LootStoreItem,  chance, loot, store);
}

void ScriptMgr::OnInitializeLockedDungeons(Player* player, uint8& level, uint32& lockData)
{
    FOREACH_SCRIPT(GlobalScript)->OnInitializeLockedDungeons(player, level, lockData);
}

void ScriptMgr::OnAfterInitializeLockedDungeons(Player* player)
{
    FOREACH_SCRIPT(GlobalScript)->OnAfterInitializeLockedDungeons(player);
}

void ScriptMgr::OnAfterUpdateEncounterState(Map* map, EncounterCreditType type, uint32 creditEntry, Unit* source, Difficulty difficulty_fixed, DungeonEncounterList const* encounters, uint32 dungeonCompleted, bool updated) 
{
    FOREACH_SCRIPT(GlobalScript)->OnAfterUpdateEncounterState(map, type, creditEntry, source, difficulty_fixed, encounters, dungeonCompleted, updated);
}

uint32 ScriptMgr::DealDamage(Unit* AttackerUnit, Unit *pVictim, uint32 damage, DamageEffectType damagetype)
{
    FOR_SCRIPTS_RET(UnitScript, itr, end, damage)
        damage = itr->second->DealDamage(AttackerUnit, pVictim, damage, damagetype);
    return damage;
}
void ScriptMgr::Creature_SelectLevel(const CreatureTemplate *cinfo, Creature* creature)
{
    FOREACH_SCRIPT(AllCreatureScript)->Creature_SelectLevel(cinfo, creature);
}
void ScriptMgr::OnHeal(Unit* healer, Unit* reciever, uint32& gain)
{
    FOREACH_SCRIPT(UnitScript)->OnHeal(healer, reciever, gain);
}

void ScriptMgr::OnDamage(Unit* attacker, Unit* victim, uint32& damage)
{
    FOREACH_SCRIPT(UnitScript)->OnDamage(attacker, victim, damage);
}

void ScriptMgr::ModifyPeriodicDamageAurasTick(Unit* target, Unit* attacker, uint32& damage)
{
    FOREACH_SCRIPT(UnitScript)->ModifyPeriodicDamageAurasTick(target, attacker, damage);
}

void ScriptMgr::ModifyMeleeDamage(Unit* target, Unit* attacker, uint32& damage)
{
    FOREACH_SCRIPT(UnitScript)->ModifyMeleeDamage(target, attacker, damage);
}

void ScriptMgr::ModifySpellDamageTaken(Unit* target, Unit* attacker, int32& damage)
{
    FOREACH_SCRIPT(UnitScript)->ModifySpellDamageTaken(target, attacker, damage);
}

void ScriptMgr::ModifyHealRecieved(Unit* target, Unit* attacker, uint32& damage)
{
    FOREACH_SCRIPT(UnitScript)->ModifyHealRecieved(target, attacker, damage);
}

void ScriptMgr::OnBeforeRollMeleeOutcomeAgainst(const Unit* attacker, const Unit* victim, WeaponAttackType attType, int32 &attackerMaxSkillValueForLevel, int32 &victimMaxSkillValueForLevel, int32 &attackerWeaponSkill, int32 &victimDefenseSkill, int32 &crit_chance, int32 &miss_chance, int32 &dodge_chance, int32 &parry_chance, int32 &block_chance)
{
    FOREACH_SCRIPT(UnitScript)->OnBeforeRollMeleeOutcomeAgainst(attacker, victim, attType, attackerMaxSkillValueForLevel, victimMaxSkillValueForLevel, attackerWeaponSkill, victimDefenseSkill, crit_chance, miss_chance, dodge_chance, parry_chance, block_chance);
}

void ScriptMgr::OnPlayerMove(Player* player, MovementInfo movementInfo, uint32 opcode)
{
    FOREACH_SCRIPT(MovementHandlerScript)->OnPlayerMove(player, movementInfo, opcode);
}

void ScriptMgr::OnBeforeBuyItemFromVendor(Player* player, uint64 vendorguid, uint32 vendorslot, uint32 &item, uint8 count, uint8 bag, uint8 slot)
{
    FOREACH_SCRIPT(PlayerScript)->OnBeforeBuyItemFromVendor(player, vendorguid, vendorslot, item, count, bag, slot);
}

void ScriptMgr::OnAfterStoreOrEquipNewItem(Player* player, uint32 vendorslot, uint32 &item, uint8 count, uint8 bag, uint8 slot, ItemTemplate const* pProto, Creature* pVendor, VendorItem const* crItem, bool bStore) 
{
    FOREACH_SCRIPT(PlayerScript)->OnAfterStoreOrEquipNewItem(player, vendorslot, item, count, bag, slot, pProto, pVendor, crItem, bStore);
}


void ScriptMgr::OnAfterUpdateMaxPower(Player* player, Powers& power, float& value)
{
    FOREACH_SCRIPT(PlayerScript)->OnAfterUpdateMaxPower(player, power, value);
}

void ScriptMgr::OnAfterUpdateMaxHealth(Player* player, float& value)
{
    FOREACH_SCRIPT(PlayerScript)->OnAfterUpdateMaxHealth(player, value);
}

void ScriptMgr::OnBeforeUpdateAttackPowerAndDamage(Player* player, float& level, float& val2, bool ranged)
{
    FOREACH_SCRIPT(PlayerScript)->OnBeforeUpdateAttackPowerAndDamage(player, level, val2, ranged);
}

void ScriptMgr::OnAfterUpdateAttackPowerAndDamage(Player* player, float& level, float& base_attPower, float& attPowerMod, float& attPowerMultiplier, bool ranged)
{
    FOREACH_SCRIPT(PlayerScript)->OnAfterUpdateAttackPowerAndDamage(player, level, base_attPower, attPowerMod, attPowerMultiplier, ranged);
}

void ScriptMgr::OnBeforeInitTalentForLevel(Player* player, uint8& level, uint32& talentPointsForLevel)
{
    FOREACH_SCRIPT(PlayerScript)->OnBeforeInitTalentForLevel(player, level, talentPointsForLevel);
}

void ScriptMgr::OnAfterArenaRatingCalculation(bool &ScriptUse, Battleground *const bg, int32 &winnerMatchmakerChange, int32 &loserMatchmakerChange, int32 &winnerChange, int32 &loserChange)
{
    FOREACH_SCRIPT(FormulaScript)->OnAfterArenaRatingCalculation(ScriptUse, bg, winnerMatchmakerChange, loserMatchmakerChange, winnerChange, loserChange);
}

// BGScript
void ScriptMgr::OnBattlegroundStart(Battleground* bg)
{
    FOREACH_SCRIPT(BGScript)->OnBattlegroundStart(bg);
}

void ScriptMgr::OnBattlegroundEndReward(Battleground* bg, Player* player, TeamId winnerTeamId)
{
    FOREACH_SCRIPT(BGScript)->OnBattlegroundEndReward(bg, player, winnerTeamId);
}

void ScriptMgr::OnBattlegroundUpdate(Battleground* bg, uint32 diff)
{
    FOREACH_SCRIPT(BGScript)->OnBattlegroundUpdate(bg, diff);
}

void ScriptMgr::OnBattlegroundAddPlayer(Battleground* bg, Player* player)
{
    FOREACH_SCRIPT(BGScript)->OnBattlegroundAddPlayer(bg, player);
}

void ScriptMgr::OnApplyingNonSSDItemStatsBonus(bool &SkipCoreCode, Player* player, ItemTemplate const* proto, uint8 slot, uint8 i, uint32& statType, int32 &value, uint32 &statsCount)
{
    FOREACH_SCRIPT(PlayerScript)->OnApplyingNonSSDItemStatsBonus(SkipCoreCode, player , proto, slot ,i, statType, value, statsCount);
}

void ScriptMgr::OnApplyingItemBeforeArmorAndResistance(bool &SkipCoreCode, Player* player, ItemTemplate const* proto,uint8 slot, uint32 &armor,int32 &holy_res, int32 &fire_res, int32 &nature_res, int32 &frost_res, int32 &shadow_res, int32 &arcane_res)
{
    FOREACH_SCRIPT(PlayerScript)->OnApplyingItemBeforeArmorAndResistance(SkipCoreCode, player, proto, slot, armor, holy_res, fire_res, nature_res, frost_res, shadow_res, arcane_res);
}

void ScriptMgr::OnBeforeApplyingWeaponDamage(bool &SkipCoreCode, Player* player, ItemTemplate const* proto, uint8 slot, float &damage)
{
    FOREACH_SCRIPT(PlayerScript)->OnBeforeApplyingWeaponDamage(SkipCoreCode, player, proto, slot, damage);
}
void ScriptMgr::OnSendMailTo(bool& SkipCoreCode, MailDraft* me, SQLTransaction& trans, MailReceiver const& receiver, MailSender const& sender, MailCheckMask checked, uint32 deliver_delay, uint32 custom_expiration, Player* pReceiver, Player* pSender, uint32 mailId, MailItemMap& m_items, uint32& m_money, uint32& m_COD)
{
    FOREACH_SCRIPT(MailScript)->OnSendMailTo(SkipCoreCode, me, trans, receiver, sender, checked, deliver_delay, custom_expiration, pReceiver, pSender, mailId, m_items, m_money, m_COD);
}
void ScriptMgr::OnSendToAll(bool& SkipCoreCode, Channel* me, WorldPacket* data, uint64& guid)
{
    FOREACH_SCRIPT(ChannelScript)->OnSendToAll(SkipCoreCode, me, data, guid);
}

void ScriptMgr::OnSendToAllButOne(bool& SkipCoreCode, Channel* me, WorldPacket* data, uint64& who)
{
    FOREACH_SCRIPT(ChannelScript)->OnSendToAllButOne(SkipCoreCode, me, data, who);
}

void ScriptMgr::OnSendToOne(bool& SkipCoreCode, Channel* me, WorldPacket* data, uint64& who)
{
    FOREACH_SCRIPT(ChannelScript)->OnSendToOne(SkipCoreCode, me, data, who);
}

void ScriptMgr::OnSendToAllWatching(bool& SkipCoreCode, Channel* me, WorldPacket* data)
{
    FOREACH_SCRIPT(ChannelScript)->OnSendToAllWatching(SkipCoreCode, me, data);
}

void ScriptMgr::OnHandleJoinChannel(bool& SkipCoreCode, WorldSession* me, WorldPacket& recvPacket)
{
    FOREACH_SCRIPT(ChannelScript)->OnHandleJoinChannel(SkipCoreCode, me, recvPacket);
}

void ScriptMgr::OnHandleContactListOpcode(bool& SkipCoreCode, WorldSession* me, WorldPacket& recv_data, Player* player)
{
    FOREACH_SCRIPT(SocialScript)->OnHandleContactListOpcode(SkipCoreCode, me, recv_data, player);
}

void ScriptMgr::OnHandleWhoOpcode(bool& SkipCoreCode, WorldSession* me, WorldPacket& recvData, time_t timeWhoCommandAllowed, Player* player)
{
    FOREACH_SCRIPT(SocialScript)->OnHandleWhoOpcode(SkipCoreCode, me, recvData, timeWhoCommandAllowed, player);
}

void ScriptMgr::OnBroadcastToFriendListers(bool& SkipCoreCode, SocialMgr* me, Player* player, WorldPacket* packet, SocialMap& m_socialMap)
{
    FOREACH_SCRIPT(SocialScript)->OnBroadcastToFriendListers(SkipCoreCode, me, player, packet, m_socialMap);
}

void ScriptMgr::OnSendSocialList(bool& SkipCoreCode, PlayerSocial* me,Player* player, PlayerSocialMap& m_playerSocialMap)
{
    FOREACH_SCRIPT(SocialScript)->OnSendSocialList(SkipCoreCode, me, player, m_playerSocialMap);
}
AllMapScript::AllMapScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<AllMapScript>::AddScript(this);
}

AllCreatureScript::AllCreatureScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<AllCreatureScript>::AddScript(this);
}

UnitScript::UnitScript(const char* name, bool addToScripts)
    : ScriptObject(name)
{
    if (addToScripts)
        ScriptRegistry<UnitScript>::AddScript(this);
}

MovementHandlerScript::MovementHandlerScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<MovementHandlerScript>::AddScript(this);
}

SpellScriptLoader::SpellScriptLoader(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<SpellScriptLoader>::AddScript(this);
}

ServerScript::ServerScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<ServerScript>::AddScript(this);
}

WorldScript::WorldScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<WorldScript>::AddScript(this);
}

FormulaScript::FormulaScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<FormulaScript>::AddScript(this);
}

WorldMapScript::WorldMapScript(const char* name, uint32 mapId)
    : ScriptObject(name), MapScript<Map>(mapId)
{
    ScriptRegistry<WorldMapScript>::AddScript(this);
}

InstanceMapScript::InstanceMapScript(const char* name, uint32 mapId)
    : ScriptObject(name), MapScript<InstanceMap>(mapId)
{
    ScriptRegistry<InstanceMapScript>::AddScript(this);
}

BattlegroundMapScript::BattlegroundMapScript(const char* name, uint32 mapId)
    : ScriptObject(name), MapScript<BattlegroundMap>(mapId)
{
    ScriptRegistry<BattlegroundMapScript>::AddScript(this);
}

ItemScript::ItemScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<ItemScript>::AddScript(this);
}

CreatureScript::CreatureScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<CreatureScript>::AddScript(this);
}

GameObjectScript::GameObjectScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<GameObjectScript>::AddScript(this);
}

AreaTriggerScript::AreaTriggerScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<AreaTriggerScript>::AddScript(this);
}

BattlegroundScript::BattlegroundScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<BattlegroundScript>::AddScript(this);
}

OutdoorPvPScript::OutdoorPvPScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<OutdoorPvPScript>::AddScript(this);
}

CommandScript::CommandScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<CommandScript>::AddScript(this);
}

WeatherScript::WeatherScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<WeatherScript>::AddScript(this);
}

AuctionHouseScript::AuctionHouseScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<AuctionHouseScript>::AddScript(this);
}

ConditionScript::ConditionScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<ConditionScript>::AddScript(this);
}

VehicleScript::VehicleScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<VehicleScript>::AddScript(this);
}

DynamicObjectScript::DynamicObjectScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<DynamicObjectScript>::AddScript(this);
}

TransportScript::TransportScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<TransportScript>::AddScript(this);
}

AchievementCriteriaScript::AchievementCriteriaScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<AchievementCriteriaScript>::AddScript(this);
}

PlayerScript::PlayerScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<PlayerScript>::AddScript(this);
}

GuildScript::GuildScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<GuildScript>::AddScript(this);
}

GroupScript::GroupScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<GroupScript>::AddScript(this);
}

GlobalScript::GlobalScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<GlobalScript>::AddScript(this);
}

BGScript::BGScript(char const* name)
    : ScriptObject(name)
{
    ScriptRegistry<BGScript>::AddScript(this);
}

ModuleScript::ModuleScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<ModuleScript>::AddScript(this);
}

MailScript::MailScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<MailScript>::AddScript(this);
}

ChannelScript::ChannelScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<ChannelScript>::AddScript(this);
}
SocialScript::SocialScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<SocialScript>::AddScript(this);
}
