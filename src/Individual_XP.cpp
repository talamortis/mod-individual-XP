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
    uint32 XPRate = 1;
};

uint32 MaxRate;

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

    void OnGiveXP(Player* p, uint32& amount, Unit* victim)
    {
        uint32 bonus_xp = amount * p->CustomData.Get<PlayerXpRate>("Individual_XP")->XPRate ;
        uint32 curXP = p->GetUInt32Value(PLAYER_XP);
        uint32 nextLvlXP = p->GetUInt32Value(PLAYER_NEXT_LEVEL_XP);
        uint8 level = p->getLevel();
        uint32 newXP = curXP + bonus_xp - amount;

        if (p->getLevel() == sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
            return;

        while (newXP >= nextLvlXP && level < sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
        {
            newXP -= nextLvlXP;

            if (level < sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
                p->GiveLevel(level + 1);

            level = p->getLevel();
            nextLvlXP = p->GetUInt32Value(PLAYER_NEXT_LEVEL_XP);
        }

        p->SetUInt32Value(PLAYER_XP, (newXP));
        p->SendLogXPGain(bonus_xp, victim, NULL, false, 1.0f);
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
        Player* me = handler->GetSession()->GetPlayer();
        if (!*args)
            return false;

        if (!me)
            return false;

        if (me->getLevel() == sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
            return false;

        if (atoi(args) > MaxRate || (atol(args) == 0))
            return false;

        me->CustomData.Get<PlayerXpRate>("Individual_XP")->XPRate = (uint32)atol(args); //Return int from command

        me->GetSession()->SendAreaTriggerMessage("You have updated your XP rate to %u", me->CustomData.Get<PlayerXpRate>("Individual_XP")->XPRate);
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
            std::string conf_path = _CONF_DIR;
            std::string cfg_file = conf_path + "/Individual-XP.conf";

#ifdef WIN32
            cfg_file = "Individual-XP.conf";
#endif

            std::string cfg_def_file = cfg_file + ".dist";
            sConfigMgr->LoadMore(cfg_def_file.c_str());
            sConfigMgr->LoadMore(cfg_file.c_str());
            MaxRate = sConfigMgr->GetIntDefault("MaxXPRate", 10);
        }
    }
};

void AddIndividual_XPScripts()
{
    new Individual_XP();
    new Individual_XP_conf();
    new Individual_XP_command();
}