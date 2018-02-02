#include "ScriptMgr.h"
#include "Configuration/Config.h"
#include "ObjectMgr.h"
#include "Chat.h"
#include "Group.h"
#include "Player.h"
#include "Object.h"
#include "DataMap.h"

/*
Coded by Talamortis - For Azerothcore
Thanks to Rochet for the assistance
*/

class PlayerXpRate : public DataMap::Base
{
public:
    PlayerXpRate() {}
    PlayerXpRate(uint32 XPRate) : XPRate(XPRate) {}
    uint32 XPRate = 0;
};

class Individual_XP : public PlayerScript
{
public:
    Individual_XP() : PlayerScript("Individual_XP") { }

    void OnLogin(Player* p) override
    {
        QueryResult result = CharacterDatabase.PQuery("SELECT XPRate FROM individualxp WHERE CharacterGUID = %u", p->GetGUIDLow());

        if (!result)
        {
            p->CustomData.GetDefault<PlayerXpRate>("Individual_XP")->XPRate;
            return;
        }

        Field* fields = result->Fetch();
        p->CustomData.Set("Individual_XP", new PlayerXpRate(fields[0].GetUInt32()));
    }

    void OnLogout(Player* p) override
    {
        if (PlayerXpRate* data = p->CustomData.Get<PlayerXpRate>("Individual_XP"))
            CharacterDatabase.PQuery("REPLACE INTO `individualxp` (`CharacterGUID`, `XPRate`) VALUES (%u, %u);", p->GetGUIDLow(), data->XPRate);
    }

    void OnGiveXP(Player* p, uint32& amount, Unit* victim) override
    {
        uint32 curXP = p->GetUInt32Value(PLAYER_XP);
        uint32 nextLvlXP = p->GetUInt32Value(PLAYER_NEXT_LEVEL_XP);
        uint32 bonus_xp = p->CustomData.Get<PlayerXpRate>("Individual_XP")->XPRate * amount;
        uint32 newXP = curXP + amount + bonus_xp;
        uint8 level = p->getLevel();

        while (newXP >= nextLvlXP && level < sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
        {
            newXP -= nextLvlXP;

            if (level < sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
                p->GiveLevel(level + 1);

            level = p->getLevel();
            nextLvlXP = p->GetUInt32Value(PLAYER_NEXT_LEVEL_XP);
        }

        p->SendLogXPGain(amount, victim, bonus_xp, false, 1.0f);
        p->SetUInt32Value(PLAYER_XP, newXP);
    }
};

class Individual_XP_command : public CommandScript
{
public:
    Individual_XP_command() : CommandScript("Individual_XP_command") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> IndividualXPTable =
        {
            { "SetXP", SEC_PLAYER, false, &HandleIndividualXPCommand, "" }
        };
        return IndividualXPTable;
    }

    static bool HandleIndividualXPCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;
        
        Player* me = handler->GetSession()->GetPlayer();

        // Crash before Set
        me->CustomData.Get<PlayerXpRate>("Individual_XP")->XPRate = (uint32)atoi(args); //Return int from command

        if (!me)
            return false;

        me->GetSession()->SendAreaTriggerMessage("You have Updated your XP rate to %u", me->CustomData.Get<PlayerXpRate>("Individual_XP")->XPRate);
        return true;


    }
};

class Individual_XP_conf : public WorldScript
{
public:
    Individual_XP_conf() : WorldScript("Individual_XP_conf_conf") { }

    void OnBeforeConfigLoad(bool reload) override
    {
        if (!reload) {
            std::string cfg_file = "Individual-XP.conf";
            std::string cfg_def_file = cfg_file + ".dist";

            sConfigMgr->LoadMore(cfg_def_file.c_str());

            sConfigMgr->LoadMore(cfg_file.c_str());
        }
    }
};

void AddIndividual_XPScripts()
{
    new Individual_XP();
    new Individual_XP_conf();
    new Individual_XP_command();
}