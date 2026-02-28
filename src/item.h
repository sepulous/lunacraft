#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

enum class ItemID : uint8_t
{
    // Stackable items
    none,
    aluminum,
    aluminum_ore,
    amethyst_ore,
    beryllium,
    blue_crystal,
    boron_crystal,
    calcite,
    carbon,
    chalchanthite,
    dirt,
    feldspar,
    glass,
    gold_ore,
    granite,
    graphite,
    gravel,
    light,
    magnetite,
    molybdenum_ore,
    moon_bark,
    moon_leaf,
    moon_wood,
    neptunium,
    notchium,
    notchium_ore,
    phosphate,
    polymer,
    quartz_ore,
    rock,
    sand,
    shale_gravel,
    silver_ore,
    snow,
    sulphur_crystal,
    sulphur_ore,
    titanium,
    titanium_ore,
    topsoil,
    water,
    xenostone,
    zircon_ore,
    beacon,
    mechanism,
    battery,
    energy_orb,
    power_crystal,
    adhesive,
    minilight,
    magnet,
    biogel,
    gold,
    turret_t1,
    turret_t2,
    turret_t3,
    green_mob_egg,
    brown_mob_egg,
    space_giraffe_egg,
    blue_i_egg,
    astronaut_egg,

    // Non-stackable items
    camera,
    disk,
    disk1,
    disk2,
    disk3,
    disk4,
    disk5,
    disk6,
    disk7,
    disk8,
    disk9,
    disk10,
    disk11,
    disk12,
    disk13,
    disk14,
    disk15,
    disk16,
    disk17,
    disk18,
    drill_t1,
    drill_t2,
    drill_t3,
    jetpack_t1,
    jetpack_t2,
    jetpack_t3,
    medkit,
    slug_pistol_t1,
    slug_pistol_t2,
    slug_pistol_t3,
    chronobooster,
    chronowinder
};

enum class ScannerDataType {TYPE, COMPOSITION, VALUE};
struct ItemIcon
{
    unsigned char *bytes;
    int width, height, num_channels;
};

std::string GetItemFile(ItemID item);
std::unordered_map<ScannerDataType, std::string> GetItemScannerData(ItemID item);
bool ItemIsSprite(ItemID item);
bool ItemIsDisk(ItemID item);
bool ItemIsDrill(ItemID item);
bool ItemIsPistol(ItemID item);
bool ItemIsBlock(ItemID item);
ItemIcon GetItemIcon(ItemID item);
