#include "ScriptMgr.h"
#include "Configuration/Config.h"
#include "ObjectMgr.h"
#include "Chat.h"
#include "Group.h"
#include "Player.h"

//Individuale_XP-Rate
uint32 curXP;
uint32 nextLvlXP;
uint32 bonus_xp;
uint32 newXP;
uint8 level;
uint32 LevelRate;

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

        LevelRate = (uint32)atoi(args);

        if (!me)
            return false;

        me->GetSession()->SendAreaTriggerMessage("You have Updated your XP rate to %u", LevelRate);
        return true;

       
    }
};



class Individual_XP : public PlayerScript
{
public:
    Individual_XP() : PlayerScript("Individual_XP") { }

    void OnGiveXP(Player* player, uint32& amount, Unit* victim)
    { 

            curXP = player->GetUInt32Value(PLAYER_XP);
            nextLvlXP = player->GetUInt32Value(PLAYER_NEXT_LEVEL_XP);
            newXP = curXP + amount + bonus_xp;
            level = player->getLevel();
            bonus_xp = LevelRate * amount; 

            while (newXP >= nextLvlXP && level < sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
            {
                newXP -= nextLvlXP;

                if (level < sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
                    player->GiveLevel(level + 1);

                level = player->getLevel();
                nextLvlXP = player->GetUInt32Value(PLAYER_NEXT_LEVEL_XP);
            }

            player->SendLogXPGain(amount, victim, bonus_xp, false, 1.0f);

            player->SetUInt32Value(PLAYER_XP, newXP);


        }

};

class Individual_XP_conf : public WorldScript
{
public:
    Individual_XP_conf() : WorldScript("Individual_XP_conf_conf") { }

    void OnBeforeConfigLoad(bool reload) override
    {
        if (!reload) {
            std::string cfg_file = "Individual_XP_conf.conf";
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