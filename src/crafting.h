#pragma once

#include <vector>

#include "item.h"

inline std::vector<std::vector<std::pair<ItemID, int>>> GetCraftingRecipes()
{
    static auto recipes = []()
    {
        std::vector<std::vector<std::pair<ItemID, int>>> _recipes =
        {
            {
                {ItemID::energy_orb, 1},
                {ItemID::none, 0},          {ItemID::power_crystal, 1}, {ItemID::none, 0},
                {ItemID::power_crystal, 1}, {ItemID::neptunium, 1},     {ItemID::power_crystal, 1},
                {ItemID::none, 0},          {ItemID::power_crystal, 1}, {ItemID::none, 0}
            },
            {
                {ItemID::power_crystal, 1},
                {ItemID::boron_crystal, 1},   {ItemID::sulphur_crystal, 1}, {ItemID::boron_crystal, 1},
                {ItemID::sulphur_crystal, 1}, {ItemID::boron_crystal, 1},   {ItemID::sulphur_crystal, 1},
                {ItemID::boron_crystal, 1},   {ItemID::sulphur_crystal, 1}, {ItemID::boron_crystal, 1}
            },
            {
                {ItemID::battery, 1},
                {ItemID::aluminum, 1}, {ItemID::aluminum, 1}, {ItemID::aluminum, 1},
                {ItemID::light, 1},    {ItemID::light, 1},    {ItemID::light, 1},
                {ItemID::aluminum, 1}, {ItemID::aluminum, 1}, {ItemID::aluminum, 1}
            },
            {
                {ItemID::beacon, 1},
                {ItemID::aluminum, 1}, {ItemID::chronobooster, 1},   {ItemID::aluminum, 1},
                {ItemID::aluminum, 1}, {ItemID::power_crystal, 1}, {ItemID::aluminum, 1},
                {ItemID::aluminum, 1}, {ItemID::mechanism, 1},     {ItemID::aluminum, 1}
            },
            {
                {ItemID::mechanism, 1},
                {ItemID::aluminum, 1}, {ItemID::polymer, 1},  {ItemID::aluminum, 1},
                {ItemID::polymer, 1},  {ItemID::aluminum, 1}, {ItemID::polymer, 1},
                {ItemID::aluminum, 1}, {ItemID::polymer, 1},  {ItemID::aluminum, 1}
            },
            {
                {ItemID::drill_t1, 1},
                {ItemID::mechanism, 1}, {ItemID::none, 0},     {ItemID::none, 0},
                {ItemID::battery, 1},   {ItemID::aluminum, 1}, {ItemID::aluminum, 1},
                {ItemID::mechanism, 1}, {ItemID::none, 0},     {ItemID::none, 0}
            },
            {
                {ItemID::drill_t2, 1},
                {ItemID::mechanism, 1},     {ItemID::none, 0},     {ItemID::none, 0},
                {ItemID::power_crystal, 1}, {ItemID::titanium, 1}, {ItemID::titanium, 1},
                {ItemID::mechanism, 1},     {ItemID::none, 0},     {ItemID::none, 0}
            },
            {
                {ItemID::drill_t3, 1},
                {ItemID::mechanism, 1},  {ItemID::none, 0},     {ItemID::none, 0},
                {ItemID::energy_orb, 1}, {ItemID::notchium, 1}, {ItemID::notchium, 1},
                {ItemID::mechanism, 1},  {ItemID::none, 0},     {ItemID::none, 0}
            },
            {
                {ItemID::slug_pistol_t1, 1},
                {ItemID::battery, 1}, {ItemID::magnet, 1}, {ItemID::aluminum, 1},
                {ItemID::aluminum, 1}
            },
            {
                {ItemID::slug_pistol_t2, 1},
                {ItemID::power_crystal, 1}, {ItemID::magnet, 1}, {ItemID::titanium, 1},
                {ItemID::titanium, 1}
            },
            
            {
                {ItemID::slug_pistol_t3, 1},
                {ItemID::energy_orb, 1}, {ItemID::magnet, 1}, {ItemID::notchium, 1},
                {ItemID::notchium, 1}
            },
            
            {
                {ItemID::jetpack_t1, 1},
                {ItemID::aluminum, 1}, {ItemID::aluminum, 1}, {ItemID::aluminum, 1},
                {ItemID::battery, 1},  {ItemID::battery, 1},  {ItemID::battery, 1},
                {ItemID::aluminum, 1}, {ItemID::none, 0},     {ItemID::aluminum, 1}
            },
            
            {
                {ItemID::jetpack_t2, 1},
                {ItemID::titanium, 1},      {ItemID::titanium, 1},      {ItemID::titanium, 1},
                {ItemID::power_crystal, 1}, {ItemID::power_crystal, 1}, {ItemID::power_crystal, 1},
                {ItemID::titanium, 1},      {ItemID::none, 0},          {ItemID::titanium, 1}
            },
            
            {
                {ItemID::jetpack_t3, 1},
                {ItemID::notchium, 1},   {ItemID::notchium, 1},   {ItemID::notchium, 1},
                {ItemID::energy_orb, 1}, {ItemID::energy_orb, 1}, {ItemID::energy_orb, 1},
                {ItemID::notchium, 1},   {ItemID::none, 0},       {ItemID::notchium, 1}
            },
            
            {
                {ItemID::camera, 1},
                {ItemID::none, 0},   {ItemID::energy_orb, 1}, {ItemID::none, 0},
                {ItemID::carbon, 1}, {ItemID::mechanism, 1}, {ItemID::carbon, 1},
                {ItemID::carbon, 1}, {ItemID::mechanism, 1}, {ItemID::carbon, 1}
            },
            
            {
                {ItemID::medkit, 1},
                {ItemID::aluminum, 1}, {ItemID::mechanism, 1}, {ItemID::aluminum, 1},
                {ItemID::aluminum, 1}, {ItemID::biogel, 6},    {ItemID::aluminum, 1},
                {ItemID::aluminum, 1}, {ItemID::aluminum, 1},  {ItemID::aluminum, 1}
            },
            
            {
                {ItemID::turret_t1, 1},
                {ItemID::none, 0},     {ItemID::slug_pistol_t1, 1}, {ItemID::none, 0},
                {ItemID::titanium, 1}, {ItemID::battery, 1},        {ItemID::titanium, 1},
                {ItemID::titanium, 1}, {ItemID::battery, 1},        {ItemID::titanium, 1}
            },
            
            {
                {ItemID::turret_t2, 1},
                {ItemID::none, 0},    {ItemID::slug_pistol_t2, 1},   {ItemID::none, 0},
                {ItemID::polymer, 1}, {ItemID::power_crystal, 1},    {ItemID::polymer, 1},
                {ItemID::polymer, 1}, {ItemID::power_crystal, 1},    {ItemID::polymer, 1}
            },
            
            {
                {ItemID::turret_t3, 1},
                {ItemID::none, 0},     {ItemID::slug_pistol_t3, 1}, {ItemID::none, 0},
                {ItemID::notchium, 1}, {ItemID::energy_orb, 1},     {ItemID::notchium, 1},
                {ItemID::notchium, 1}, {ItemID::energy_orb, 1},     {ItemID::notchium, 1}
            },
            
            {
                {ItemID::chronobooster, 1},
                {ItemID::none, 0},      {ItemID::xenostone, 1}, {ItemID::none, 0},
                {ItemID::xenostone, 1}, {ItemID::mechanism, 1}, {ItemID::xenostone, 1},
                {ItemID::none, 0},      {ItemID::xenostone, 1}, {ItemID::none, 0}
            },
            
            {
                {ItemID::chronowinder, 1},
                {ItemID::none, 0},         {ItemID::amethyst_ore, 8}, {ItemID::none, 0},
                {ItemID::amethyst_ore, 8}, {ItemID::mechanism, 1},    {ItemID::amethyst_ore, 8},
                {ItemID::none, 0},         {ItemID::amethyst_ore, 8}, {ItemID::none, 0}
            },
            
            {
                {ItemID::minilight, 4},
                {ItemID::light, 1}, {ItemID::adhesive, 1}
            },
            
            {
                {ItemID::granite, 1},
                {ItemID::sulphur_ore, 1}, {ItemID::feldspar, 1}
            },
            
            {
                {ItemID::granite, 1},
                {ItemID::feldspar, 1}, {ItemID::sulphur_ore, 1}
            },
            
            {
                {ItemID::beryllium, 1},
                {ItemID::sulphur_ore, 1}, {ItemID::chalchanthite, 1}
            },
            
            {
                {ItemID::beryllium, 1},
                {ItemID::chalchanthite, 1}, {ItemID::sulphur_ore, 1}
            },
            
            {
                {ItemID::amethyst_ore, 1},
                {ItemID::chalchanthite, 1}, {ItemID::feldspar, 1}
            },
            
            {
                {ItemID::amethyst_ore, 1},
                {ItemID::feldspar, 1}, {ItemID::chalchanthite, 1}
            },
            
            {
                {ItemID::rock, 1},
                {ItemID::calcite, 1}, {ItemID::graphite, 1}
            },
            
            {
                {ItemID::rock, 1},
                {ItemID::graphite, 1}, {ItemID::calcite, 1}
            },
            
            {
                {ItemID::graphite, 1},
                {ItemID::moon_bark, 1}, {ItemID::rock, 1}
            },
            
            {
                {ItemID::graphite, 1},
                {ItemID::rock, 1}, {ItemID::moon_bark, 1}
            },
            
            {
                {ItemID::calcite, 1},
                {ItemID::moon_bark, 1}, {ItemID::sand, 1}
            },
            
            {
                {ItemID::calcite, 1},
                {ItemID::sand, 1}, {ItemID::moon_bark, 1}
            },
            
            {
                {ItemID::biogel, 10},
                {ItemID::xenostone, 1}
            },
            
            {
                {ItemID::xenostone, 1},
                {ItemID::biogel, 10}
            },
            
            {
                {ItemID::sand, 2},
                {ItemID::dirt, 1}, {ItemID::water, 1}
            },
            
            {
                {ItemID::sand, 2},
                {ItemID::water, 1}, {ItemID::dirt, 1}
            },
            
            {
                {ItemID::adhesive, 3},
                {ItemID::biogel, 1}, {ItemID::water, 1}
            },
            
            {
                {ItemID::adhesive, 3},
                {ItemID::water, 1}, {ItemID::biogel, 1}
            },
            
            {
                {ItemID::magnet, 1},
                {ItemID::magnetite, 4}
            },
            
            {
                {ItemID::polymer, 1},
                {ItemID::shale_gravel, 4}
            },
            
            {
                {ItemID::gold, 1},
                {ItemID::gold_ore, 4}
            },
            
            {
                {ItemID::aluminum, 1},
                {ItemID::aluminum_ore, 4}
            },
            
            {
                {ItemID::titanium, 1},
                {ItemID::titanium_ore, 4}
            },
            
            {
                {ItemID::notchium, 1},
                {ItemID::notchium_ore, 4}
            },
            {
                {ItemID::biogel, 1},
                {ItemID::moon_leaf, 8}
            },
            {
                {ItemID::neptunium, 1},
                {ItemID::blue_crystal, 16}
            }
        };

        return _recipes;
    }();

    return recipes;
};
