/*
 * mod-forever-shaman
 *
 * Shaman changes for 3.3.5. For now:
 *
 * - With mod-mount-scaling installed, Ghost Wolf follows its level-scaled mount speed, like a
 *   mount would. mod-mount-scaling only changes mount auras, so Ghost Wolf kept its flat 40%.
 *   Only out of combat: in combat it's the stock 40%, since a mount can't be used in combat at
 *   all.
 *
 * Ghost Wolf (2645) holds its speed on the spell itself: effect 1 raises run speed by 40%
 * (SPELL_AURA_MOD_INCREASE_SPEED). A UnitScript sets that effect once the aura is applied, and
 * again when the shaman levels up, enters or leaves combat in wolf form. No client patch.
 *
 * Released under the MIT License.
 */

#include "Config.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"

#include <algorithm>

namespace
{
    constexpr uint32 SPELL_GHOST_WOLF = 2645;

    // Riding skills, as mod-mount-scaling checks them.
    constexpr uint32 SPELL_APPRENTICE_RIDING = 33388;
    constexpr uint32 SPELL_JOURNEYMAN_RIDING = 33391;

    struct Config
    {
        bool ghostWolfSpeedEnabled = true;
        bool ghostWolfOutOfCombatOnly = true;
    };

    Config config;

    // mod-mount-scaling's ground speed settings, read from its own config file (MountScaling.*).
    // enabled is false when that module isn't installed, and Ghost Wolf keeps its stock speed.
    struct MountScalingConfig
    {
        bool enabled = false;
        float groundPerLevel = 2.5f;
        float groundMin = 20.0f;
        float groundMax = 100.0f;
        float journeymanPerLevel = 2.5f;
        float journeymanMin = 100.0f;
        float journeymanMax = 150.0f;
    };

    MountScalingConfig mountScaling;

    // mod-mount-scaling's ground formula (src/MountScaling.cpp there), so a shaman in Ghost Wolf
    // runs as fast as they would on a ground mount. 0 means no riding skill: keep the stock 40%.
    int32 GroundMountSpeed(Player* player)
    {
        if (!player->HasSpell(SPELL_APPRENTICE_RIDING))
            return 0;

        float const level = player->GetLevel();
        MountScalingConfig const& c = mountScaling;

        if (player->HasSpell(SPELL_JOURNEYMAN_RIDING))
            return int32(std::clamp(c.journeymanMin + (level - 40) * c.journeymanPerLevel, c.journeymanMin, c.journeymanMax));

        return int32(std::min(std::max(c.groundMin, level * c.groundPerLevel), c.groundMax));
    }

    // Set Ghost Wolf's run speed effect. ChangeAmount updates the speed at once.
    void ApplyGhostWolfSpeed(Player* player, Aura* aura)
    {
        if (!config.ghostWolfSpeedEnabled || !mountScaling.enabled)
            return;

        AuraEffect* effect = aura->GetEffect(EFFECT_1);
        if (!effect || effect->GetAuraType() != SPELL_AURA_MOD_INCREASE_SPEED)
            return;

        // In combat, Ghost Wolf goes back to its own speed (40%).
        int32 const speed = config.ghostWolfOutOfCombatOnly && player->IsInCombat()
            ? effect->GetSpellInfo()->Effects[EFFECT_1].CalcValue()
            : GroundMountSpeed(player);

        if (speed > 0 && speed != effect->GetAmount())
            effect->ChangeAmount(speed);
    }
}

class ForeverShamanWorldScript : public WorldScript
{
public:
    ForeverShamanWorldScript() : WorldScript("ForeverShamanWorldScript") { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        config.ghostWolfSpeedEnabled    = sConfigMgr->GetOption<bool>("ForeverShaman.GhostWolfSpeed.Enable", true);
        config.ghostWolfOutOfCombatOnly = sConfigMgr->GetOption<bool>("ForeverShaman.GhostWolfSpeed.OutOfCombatOnly", true);

        // mod-mount-scaling's own settings, with its defaults. Without that module these aren't
        // in any config file, so don't log them as missing.
        auto mountOption = [](char const* name, float def) { return sConfigMgr->GetOption<float>(name, def, false); };
        mountScaling.enabled            = sConfigMgr->GetOption<bool>("MountScaling.Enable", false, false);
        mountScaling.groundPerLevel     = mountOption("MountScaling.Ground.SpeedPerLevel", 2.5f);
        mountScaling.groundMin          = mountOption("MountScaling.Ground.MinSpeed", 20.0f);
        mountScaling.groundMax          = mountOption("MountScaling.Ground.MaxSpeed", 100.0f);
        mountScaling.journeymanPerLevel = mountOption("MountScaling.Ground.Journeyman.SpeedPerLevel", 2.5f);
        mountScaling.journeymanMin      = mountOption("MountScaling.Ground.Journeyman.MinSpeed", 100.0f);
        mountScaling.journeymanMax      = mountOption("MountScaling.Ground.Journeyman.MaxSpeed", 150.0f);
    }
};

class ForeverShamanPlayerScript : public PlayerScript
{
public:
    ForeverShamanPlayerScript() : PlayerScript("ForeverShamanPlayerScript") { }

    // A shaman who levels up, enters or leaves combat in Ghost Wolf changes speed right away.
    // Other changes (a new riding skill, a config reload) take effect the next time they shift.
    void OnPlayerLevelChanged(Player* player, uint8 /*oldLevel*/) override
    {
        Update(player);
    }

    // The core sets the combat flag before calling these, so ApplyGhostWolfSpeed sees the new
    // state.
    void OnPlayerEnterCombat(Player* player, Unit* /*enemy*/) override
    {
        Update(player);
    }

    void OnPlayerLeaveCombat(Player* player) override
    {
        Update(player);
    }

private:
    static void Update(Player* player)
    {
        if (player->getClass() != CLASS_SHAMAN)
            return;

        if (Aura* aura = player->GetAura(SPELL_GHOST_WOLF))
            ApplyGhostWolfSpeed(player, aura);
    }
};

// A UnitScript, like mod-mount-scaling's, so it runs after the aura's effects are applied.
class ForeverShamanUnitScript : public UnitScript
{
public:
    ForeverShamanUnitScript() : UnitScript("ForeverShamanUnitScript", true, { UNITHOOK_ON_AURA_APPLY }) { }

    void OnAuraApply(Unit* unit, Aura* aura) override
    {
        if (aura->GetId() != SPELL_GHOST_WOLF)
            return;

        if (Player* player = unit->ToPlayer())
            ApplyGhostWolfSpeed(player, aura);
    }
};

void AddForeverShamanScripts()
{
    new ForeverShamanWorldScript();
    new ForeverShamanPlayerScript();
    new ForeverShamanUnitScript();
}
