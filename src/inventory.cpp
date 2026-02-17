
#include <iostream>

#include "inventory.h"

Inventory::Inventory(bool is_creative)
{
    if (is_creative) // Default creative loadout
    {
        inventory[4][0] = {ItemID::aluminum, 1};
        inventory[4][1] = {ItemID::aluminum_ore, 1};
        inventory[4][2] = {ItemID::amethyst_ore, 1};
        inventory[4][3] = {ItemID::beryllium, 1};
        inventory[4][4] = {ItemID::blue_crystal, 1};
        inventory[4][5] = {ItemID::boron_crystal, 1};
        inventory[4][6] = {ItemID::sulphur_crystal, 1};
        inventory[4][7] = {ItemID::calcite, 1};
        inventory[4][8] = {ItemID::carbon, 1};
        inventory[4][9] = {ItemID::chalchanthite, 1};

        inventory[3][0] = {ItemID::dirt, 1};
        inventory[3][1] = {ItemID::feldspar, 1};
        inventory[3][2] = {ItemID::glass, 1};
        inventory[3][3] = {ItemID::gold_ore, 1};
        inventory[3][4] = {ItemID::granite, 1};
        inventory[3][5] = {ItemID::graphite, 1};
        inventory[3][6] = {ItemID::gravel, 1};
        inventory[3][7] = {ItemID::light, 1};
        inventory[3][8] = {ItemID::magnetite, 1};
        inventory[3][9] = {ItemID::molybdenum_ore, 1};

        inventory[2][0] = {ItemID::moon_bark, 1};
        inventory[2][1] = {ItemID::moon_leaf, 1};
        inventory[2][2] = {ItemID::moon_wood, 1};
        inventory[2][3] = {ItemID::neptunium, 1};
        inventory[2][4] = {ItemID::notchium, 1};
        inventory[2][5] = {ItemID::notchium_ore, 1};
        inventory[2][6] = {ItemID::phosphate, 1};
        inventory[2][7] = {ItemID::polymer, 1};
        inventory[2][8] = {ItemID::quartz_ore, 1};
        inventory[2][9] = {ItemID::rock, 1};

        inventory[1][0] = {ItemID::sand, 1};
        inventory[1][1] = {ItemID::zircon_ore, 1};
        inventory[1][2] = {ItemID::silver_ore, 1};
        inventory[1][3] = {ItemID::shale_gravel, 1};
        inventory[1][4] = {ItemID::titanium_ore, 1};
        inventory[1][5] = {ItemID::titanium, 1};
        inventory[1][6] = {ItemID::minilight, 1};

        inventory[0][0] = {ItemID::drill_t3, 1};
        inventory[0][1] = {ItemID::slug_pistol_t3, 1};
        inventory[0][2] = {ItemID::medkit, 1};
        inventory[0][3] = {ItemID::disk, 1};
        inventory[0][4] = {ItemID::chronobooster, 1};
        inventory[0][5] = {ItemID::chronowinder, 1};
        inventory[0][6] = {ItemID::camera, 1};
    }
    else // Default explore loadout
    {
        inventory[0][0] = {ItemID::drill_t1, 1};
        inventory[0][2] = {ItemID::slug_pistol_t1, 1};
        inventory[0][3] = {ItemID::medkit, 1};
        inventory[0][4] = {ItemID::disk, 1};
        inventory[0][9] = {ItemID::camera, 1};
    }

    _is_creative = is_creative;
}

bool Inventory::HasSpaceForItem(ItemID item)
{
    for (int row = 0; row < 5; row++)
    {
        for (int col = 0; col < 10; col++)
        {
            ItemStack slot = inventory[row][col];
            if (slot.item == ItemID::none || slot.amount == 0 || (slot.item == item && ItemIsStackable(item)))
                return true;
        }
    }

    return false;
}

ItemID Inventory::GetSelectedItem()
{
    return inventory[0][selected_hotbar_slot].item;
}

bool Inventory::ItemIsStackable(ItemID item)
{
    return item < ItemID::camera;
}

bool Inventory::IsCreative()
{
    return _is_creative;
}
