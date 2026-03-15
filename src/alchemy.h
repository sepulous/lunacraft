#pragma once

#include <unordered_map>

#include "block.h"

inline BlockID GetAlchemyProduct(BlockID block)
{
    static auto TRANSMUTATIONS = []()
    {
        std::unordered_map<BlockID, BlockID> map {
            {BlockID::dirt, BlockID::sand},
            {BlockID::gravel, BlockID::moon_leaf},
            {BlockID::rock, BlockID::snow},
            {BlockID::sand, BlockID::rock},
            {BlockID::blue_crystal, BlockID::moon_wood},
            {BlockID::boron_crystal, BlockID::glass},
            {BlockID::sulphur_crystal, BlockID::boron_crystal},
            {BlockID::aluminum, BlockID::titanium_ore},
            {BlockID::aluminum_ore, BlockID::molybdenum_ore},
            {BlockID::gold_ore, BlockID::silver_ore},
            {BlockID::molybdenum_ore, BlockID::gold_ore},
            {BlockID::neptunium, BlockID::moon_bark},
            {BlockID::notchium, BlockID::blue_crystal},
            {BlockID::notchium_ore, BlockID::notchium},
            {BlockID::silver_ore, BlockID::carbon},
            {BlockID::titanium, BlockID::notchium_ore},
            {BlockID::titanium_ore, BlockID::titanium},
            {BlockID::amethyst_ore, BlockID::chalchanthite},
            {BlockID::beryllium, BlockID::feldspar},
            {BlockID::calcite, BlockID::graphite},
            {BlockID::chalchanthite, BlockID::phosphate},
            {BlockID::feldspar, BlockID::amethyst_ore},
            {BlockID::granite, BlockID::quartz_ore},
            {BlockID::graphite, BlockID::beryllium},
            {BlockID::phosphate, BlockID::xenostone},
            {BlockID::quartz_ore, BlockID::zircon_ore},
            {BlockID::sulphur_ore, BlockID::granite},
            {BlockID::zircon_ore, BlockID::topsoil},
            {BlockID::light, BlockID::aluminum_ore},
            {BlockID::moon_bark, BlockID::topsoil},
            {BlockID::moon_leaf, BlockID::water},
            {BlockID::moon_wood, BlockID::calcite},
            {BlockID::xenostone, BlockID::sulphur_ore},
            {BlockID::carbon, BlockID::sulphur_ore},
            {BlockID::glass, BlockID::magnetite},
            {BlockID::magnetite, BlockID::shale_gravel},
            {BlockID::polymer, BlockID::aluminum},
            {BlockID::shale_gravel, BlockID::polymer},
            {BlockID::snow, BlockID::gravel},
        };

        return map;
    }();

    return TRANSMUTATIONS.contains(block) ? TRANSMUTATIONS.at(block) : block;
}
