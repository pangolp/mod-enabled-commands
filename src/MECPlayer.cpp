/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"

using namespace Acore::ChatCommands;

class MecCommandsScript : public CommandScript
{
public:
    MecCommandsScript() : CommandScript("MecCommandsScript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable commandTable =
        {
            { "itemadd", HandleItemAddCommand, SEC_PLAYER, Console::No }
        };

        return commandTable;
    }

    static bool HandleItemAddCommand(ChatHandler* handler, ItemTemplate const* itemTemplate, Optional<int32> _count)
    {
        if (!sObjectMgr->GetItemTemplate(itemTemplate->ItemId))
        {
            handler->PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, itemTemplate->ItemId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 itemId = itemTemplate->ItemId;
        int32 count = 1;

        if (_count)
        {
            count = *_count;
        }

        if (!count)
        {
            count = 1;
        }

        Player* player = handler->GetSession()->GetPlayer();

        if (count < 0)
        {
            if (player->GetSession()->GetSecurity() == SEC_PLAYER)
            {
                if (!player->HasItemCount(itemId, 0))
                {
                    handler->PSendSysMessage(LANG_REMOVEITEM_FAILURE, handler->GetNameLink(player).c_str(), itemId);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                if (!player->HasItemCount(itemId, -count))
                {
                    handler->PSendSysMessage(LANG_REMOVEITEM_ERROR, handler->GetNameLink(player).c_str(), itemId);
                    handler->SetSentErrorMessage(true);
                    return false;
                }
            }

            player->DestroyItemCount(itemId, -count, true, false);
            handler->PSendSysMessage(LANG_REMOVEITEM, itemId, -count, handler->GetNameLink(player).c_str());
            return true;
        }

        uint32 noSpaceForCount = 0;

        ItemPosCountVec dest;
        InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount);

        if (msg != EQUIP_ERR_OK)
        {
            count -= noSpaceForCount;
        }

        if (!count || dest.empty())
        {
            handler->PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Item* item = player->StoreNewItem(dest, itemId, true);

        if (count && item)
        {
            player->SendNewItem(item, count, true, false);
        }

        if (noSpaceForCount)
        {
            handler->PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);
        }

        return true;
    }
};

class MecPlayer : public PlayerScript
{
public:
    MecPlayer() : PlayerScript("MecPlayer") { }

    void OnLogin(Player* player) override
    {
        if (sConfigMgr->GetOption<bool>("MEC.Enable", true))
        {
            ChatHandler(player->GetSession()).PSendSysMessage("This server is running the |cff4CFF00Enabled commands |rmodule.");
        }
    }
};

void AddMECPlayerScripts()
{
    new MecPlayer();
    new MecCommandsScript();
}
