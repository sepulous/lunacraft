#pragma once

#include <vector>

#include "item.h"

struct ItemStack
{
    ItemID item = ItemID::none;
    int amount = 0;

    bool IsEmpty();
    bool ItemIsDisk();
};

class Inventory
{
    public:
        Inventory(bool is_creative = true);
        bool IsCreative();
        bool HasSpaceForItem(ItemID item);
        ItemID GetSelectedItem();
        std::vector<std::pair<ItemID, int>> GetRecipeMatch();
        int Add(ItemStack stack);

    private:
        bool _is_creative;
        bool ItemIsStackable(ItemID item);
        
    public:
        ItemStack scanner;
        ItemStack assembler_input[3][3];
        ItemStack assembler_output;
        ItemStack spacesuit[3];
        ItemStack inventory[5][10];
        ItemStack held_stack;
        int selected_hotbar_slot = 0;
};
