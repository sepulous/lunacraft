
#include <unordered_map>

#include "item.h"

bool ItemIsSprite(ItemID item)
{
    return item == ItemID::mechanism
        || item == ItemID::battery
        || item == ItemID::energy_orb
        || item == ItemID::power_crystal
        || item == ItemID::adhesive
        || item == ItemID::minilight
        || item == ItemID::magnet
        || item == ItemID::biogel
        || item == ItemID::gold
        || item == ItemID::camera
        || item == ItemID::disk
        || item == ItemID::disk1
        || item == ItemID::disk2
        || item == ItemID::disk3
        || item == ItemID::disk4
        || item == ItemID::disk5
        || item == ItemID::disk6
        || item == ItemID::disk7
        || item == ItemID::disk8
        || item == ItemID::disk9
        || item == ItemID::disk10
        || item == ItemID::disk11
        || item == ItemID::disk12
        || item == ItemID::disk13
        || item == ItemID::disk14
        || item == ItemID::disk15
        || item == ItemID::disk16
        || item == ItemID::disk17
        || item == ItemID::disk18
        || item == ItemID::jetpack_t1
        || item == ItemID::jetpack_t2
        || item == ItemID::jetpack_t3
        || item == ItemID::medkit
        || item == ItemID::chronobooster
        || item == ItemID::chronowinder;
}

std::string GetItemFile(ItemID item)
{
    static auto ITEM_FILE_MAP = []()
    {
        std::unordered_map<ItemID, std::string> map {
            {ItemID::none, "none.png"},
            {ItemID::aluminum, "aluminum.png"},
            {ItemID::aluminum_ore, "aluminum_ore.png"},
            {ItemID::amethyst_ore, "amethyst_ore.png"},
            {ItemID::beryllium, "beryllium.png"},
            {ItemID::blue_crystal, "blue_crystal.png"},
            {ItemID::boron_crystal, "boron_crystal.png"},
            {ItemID::calcite, "calcite.png"},
            {ItemID::carbon, "carbon.png"},
            {ItemID::chalchanthite, "chalchanthite.png"},
            {ItemID::dirt, "dirt.png"},
            {ItemID::feldspar, "feldspar.png"},
            {ItemID::glass, "glass.png"},
            {ItemID::gold_ore, "gold_ore.png"},
            {ItemID::granite, "granite.png"},
            {ItemID::graphite, "graphite.png"},
            {ItemID::gravel, "gravel.png"},
            {ItemID::light, "light.png"},
            {ItemID::magnetite, "magnetite.png"},
            {ItemID::molybdenum_ore, "molybdenum_ore.png"},
            {ItemID::moon_bark, "moon_bark.png"},
            {ItemID::moon_leaf, "moon_leaf.png"},
            {ItemID::moon_wood, "moon_wood.png"},
            {ItemID::neptunium, "neptunium.png"},
            {ItemID::notchium, "notchium.png"},
            {ItemID::notchium_ore, "notchium_ore.png"},
            {ItemID::phosphate, "phosphate.png"},
            {ItemID::polymer, "polymer.png"},
            {ItemID::quartz_ore, "quartz_ore.png"},
            {ItemID::rock, "rock.png"},
            {ItemID::sand, "sand.png"},
            {ItemID::shale_gravel, "shale_gravel.png"},
            {ItemID::silver_ore, "silver_ore.png"},
            {ItemID::snow, "snow.png"},
            {ItemID::sulphur_crystal, "sulphur_crystal.png"},
            {ItemID::sulphur_ore, "sulphur_ore.png"},
            {ItemID::titanium, "titanium.png"},
            {ItemID::titanium_ore, "titanium_ore.png"},
            {ItemID::topsoil, "topsoil.png"},
            {ItemID::water, "water.png"},
            {ItemID::xenostone, "xenostone.png"},
            {ItemID::zircon_ore, "zircon_ore.png"},
            {ItemID::beacon, "beacon.png"},
            {ItemID::mechanism, "mechanism.png"},
            {ItemID::battery, "battery.png"},
            {ItemID::energy_orb, "energy_orb.png"},
            {ItemID::power_crystal, "power_crystal.png"},
            {ItemID::adhesive, "adhesive.png"},
            {ItemID::minilight, "minilight.png"},
            {ItemID::magnet, "magnet.png"},
            {ItemID::biogel, "biogel.png"},
            {ItemID::gold, "gold.png"},
            {ItemID::turret_t1, "turret_t1.png"},
            {ItemID::turret_t2, "turret_t2.png"},
            {ItemID::turret_t3, "turret_t3.png"},
            {ItemID::green_mob_egg, "green_mob_egg.png"},
            {ItemID::brown_mob_egg, "brown_mob_egg.png"},
            {ItemID::space_giraffe_egg, "space_giraffe_egg.png"},
            {ItemID::blue_i_egg, "blue_i_egg.png"},
            {ItemID::astronaut_egg, "astronaut_egg.png"},
            {ItemID::camera, "camera.png"},
            {ItemID::disk, "disk.png"},
            {ItemID::disk1, "disk.png"},
            {ItemID::disk2, "disk.png"},
            {ItemID::disk3, "disk.png"},
            {ItemID::disk4, "disk.png"},
            {ItemID::disk5, "disk.png"},
            {ItemID::disk6, "disk.png"},
            {ItemID::disk7, "disk.png"},
            {ItemID::disk8, "disk.png"},
            {ItemID::disk9, "disk.png"},
            {ItemID::disk10, "disk.png"},
            {ItemID::disk11, "disk.png"},
            {ItemID::disk12, "disk.png"},
            {ItemID::disk13, "disk.png"},
            {ItemID::disk14, "disk.png"},
            {ItemID::disk15, "disk.png"},
            {ItemID::disk16, "disk.png"},
            {ItemID::disk17, "disk.png"},
            {ItemID::disk18, "disk.png"},
            {ItemID::drill_t1, "drill_t1.png"},
            {ItemID::drill_t2, "drill_t2.png"},
            {ItemID::drill_t3, "drill_t3.png"},
            {ItemID::jetpack_t1, "jetpack_t1.png"},
            {ItemID::jetpack_t2, "jetpack_t2.png"},
            {ItemID::jetpack_t3, "jetpack_t3.png"},
            {ItemID::medkit, "medkit.png"},
            {ItemID::slug_pistol_t1, "slug_pistol_t1.png"},
            {ItemID::slug_pistol_t2, "slug_pistol_t2.png"},
            {ItemID::slug_pistol_t3, "slug_pistol_t3.png"},
            {ItemID::chronobooster, "chronobooster.png"},
            {ItemID::chronowinder, "chronowinder.png"}
        };

        return map;
    }();

    return ITEM_FILE_MAP[item];
}

std::unordered_map<ScannerDataType, std::string> GetItemScannerData(ItemID item)
{
    static auto SCANNER_DATA = []() {
        std::unordered_map<ItemID, std::unordered_map<ScannerDataType, std::string>> map {
            {
                ItemID::adhesive,
                {
                    {ScannerDataType::TYPE, "Adhesive"},
                    {ScannerDataType::COMPOSITION, ""},
                    {ScannerDataType::VALUE, ""}
                }
            },
            {
                ItemID::aluminum,
                {
                    {ScannerDataType::TYPE, "Metal"},
                    {ScannerDataType::COMPOSITION, "Aluminum\n(Refined)"},
                    {ScannerDataType::VALUE, "9.3 kC"}
                }
            },
            {
                ItemID::aluminum_ore,
                {
                    {ScannerDataType::TYPE, "Metal"},
                    {ScannerDataType::COMPOSITION, "Aluminum\n(Impure)"},
                    {ScannerDataType::VALUE, "4.6 kC"}
                }
            },
            {
                ItemID::amethyst_ore,
                {
                    {ScannerDataType::TYPE, "Mineral"},
                    {ScannerDataType::COMPOSITION, "Amethyst Ore"},
                    {ScannerDataType::VALUE, "0.8 kC"}
                }
            },
            {
                ItemID::beryllium,
                {
                    {ScannerDataType::TYPE, "Mineral"},
                    {ScannerDataType::COMPOSITION, "Beryllium Ore"},
                    {ScannerDataType::VALUE, "6.0 kC"}
                }
            },
            {
                ItemID::blue_crystal,
                {
                    {ScannerDataType::TYPE, "Crystal"},
                    {ScannerDataType::COMPOSITION, "Aluminum Oxide"},
                    {ScannerDataType::VALUE, "38.0 kC"}
                }
            },
            {
                ItemID::boron_crystal,
                {
                    {ScannerDataType::TYPE, "Crystal"},
                    {ScannerDataType::COMPOSITION, "Boron Rubelite"},
                    {ScannerDataType::VALUE, "108.3 kC"}
                }
            },
            {
                ItemID::calcite,
                {
                    {ScannerDataType::TYPE, "Mineral"},
                    {ScannerDataType::COMPOSITION, "Calcite Ore"},
                    {ScannerDataType::VALUE, "0.4 kC"}
                }
            },
            {
                ItemID::carbon,
                {
                    {ScannerDataType::TYPE, "Carbon\nAllotrope"},
                    {ScannerDataType::COMPOSITION, "Carbon"},
                    {ScannerDataType::VALUE, "0.2 kC"}
                }
            },
            {
                ItemID::chalchanthite,
                {
                    {ScannerDataType::TYPE, "Mineral"},
                    {ScannerDataType::COMPOSITION, "Chalcanthite"},
                    {ScannerDataType::VALUE, "4.8 kC"}
                }
            },
            {
                ItemID::dirt,
                {
                    {ScannerDataType::TYPE, "Exolunar\nsoil"},
                    {ScannerDataType::COMPOSITION, "Disintegrated\nbasaltic rock"},
                    {ScannerDataType::VALUE, "0.0 kC"}
                }
            },
            {
                ItemID::feldspar,
                {
                    {ScannerDataType::TYPE, "Mineral"},
                    {ScannerDataType::COMPOSITION, "Quartz/Feldspar"},
                    {ScannerDataType::VALUE, "0.2 kC"}
                }
            },
            {
                ItemID::glass,
                {
                    {ScannerDataType::TYPE, "\nHyperglass"},
                    {ScannerDataType::COMPOSITION, "Synthetic\nSilicate"},
                    {ScannerDataType::VALUE, "18.0 kC"}
                }
            },
            {
                ItemID::gold_ore,
                {
                    {ScannerDataType::TYPE, "Transition\nMetal"},
                    {ScannerDataType::COMPOSITION, "Elemental Gold"},
                    {ScannerDataType::VALUE, "81.9 kC"}
                }
            },
            {
                ItemID::granite,
                {
                    {ScannerDataType::TYPE, "Mineral"},
                    {ScannerDataType::COMPOSITION, "Pink Granite"},
                    {ScannerDataType::VALUE, "0.8 kC"}
                }
            },
            {
                ItemID::graphite,
                {
                    {ScannerDataType::TYPE, "Mineral"},
                    {ScannerDataType::COMPOSITION, "Graphite"},
                    {ScannerDataType::VALUE, "1.0 kC"}
                }
            },
            {
                ItemID::gravel,
                {
                    {ScannerDataType::TYPE, "Exolunar\ngravel"},
                    {ScannerDataType::COMPOSITION, "Disintegrated\nbasaltic rock"},
                    {ScannerDataType::VALUE, "0.0 kC"}
                }
            },
            {
                ItemID::light,
                {
                    {ScannerDataType::TYPE, "Light\nSource"},
                    {ScannerDataType::COMPOSITION, "Luminous\norganic material"},
                    {ScannerDataType::VALUE, "4.5 kC"}
                }
            },
            {
                ItemID::magnetite,
                {
                    {ScannerDataType::TYPE, "Ore"},
                    {ScannerDataType::COMPOSITION, "Magnetite"},
                    {ScannerDataType::VALUE, "8.1 kC"}
                }
            },
            {
                ItemID::molybdenum_ore,
                {
                    {ScannerDataType::TYPE, "Transition\nMetal"},
                    {ScannerDataType::COMPOSITION, "Elemental\nMolybdenum"},
                    {ScannerDataType::VALUE, "11.3 kC"}
                }
            },
            {
                ItemID::moon_bark,
                {
                    {ScannerDataType::TYPE, "\nXenobiological"},
                    {ScannerDataType::COMPOSITION, "Fibrous\nsilicates"},
                    {ScannerDataType::VALUE, "???"}
                }
            },
            {
                ItemID::moon_leaf,
                {
                    {ScannerDataType::TYPE, "\nXenobiological"},
                    {ScannerDataType::COMPOSITION, "Photosynthetic\norganic material"},
                    {ScannerDataType::VALUE, "0.3 kC"}
                }
            },
            {
                ItemID::moon_wood,
                {
                    {ScannerDataType::TYPE, "\nXenobiological"},
                    {ScannerDataType::COMPOSITION, "Fibrous\nsilicates"},
                    {ScannerDataType::VALUE, "4.0 kC"}
                }
            },
            {
                ItemID::neptunium,
                {
                    {ScannerDataType::TYPE, "\nRadioactive"},
                    {ScannerDataType::COMPOSITION, "Neptunium"},
                    {ScannerDataType::VALUE, "\n15000.0 kc"}
                }
            },
            {
                ItemID::notchium,
                {
                    {ScannerDataType::TYPE, "Unknown\nMetal"},
                    {ScannerDataType::COMPOSITION, "Notchium\n(Refined)"},
                    {ScannerDataType::VALUE, "???"}
                }
            },
            {
                ItemID::notchium_ore,
                {
                    {ScannerDataType::TYPE, "Unknown\nMetal"},
                    {ScannerDataType::COMPOSITION, "Notchium"},
                    {ScannerDataType::VALUE, "???"}
                }
            },
            {
                ItemID::phosphate,
                {
                    {ScannerDataType::TYPE, "Mineral"},
                    {ScannerDataType::COMPOSITION, "Hydrous\nPhosphate of\nCopper, Aluminum"},
                    {ScannerDataType::VALUE, "6.6 kC"}
                }
            },
            {
                ItemID::polymer,
                {
                    {ScannerDataType::TYPE, "Security\nStructure"},
                    {ScannerDataType::COMPOSITION, "Polymer"},
                    {ScannerDataType::VALUE, "150.0 kc"}
                }
            },
            {
                ItemID::quartz_ore,
                {
                    {ScannerDataType::TYPE, "Mineral"},
                    {ScannerDataType::COMPOSITION, "Quartz Ore"},
                    {ScannerDataType::VALUE, "1.2 kC"}
                }
            },
            {
                ItemID::rock,
                {
                    {ScannerDataType::TYPE, "Mineral"},
                    {ScannerDataType::COMPOSITION, "Basaltic Rock"},
                    {ScannerDataType::VALUE, "0.0 kC"}
                }
            },
            {
                ItemID::sand,
                {
                    {ScannerDataType::TYPE, "Exolunar\nsand"},
                    {ScannerDataType::COMPOSITION, "Silicate mix"},
                    {ScannerDataType::VALUE, "0.0 kC"}
                }
            },
            {
                ItemID::shale_gravel,
                {
                    {ScannerDataType::TYPE, "Ore"},
                    {ScannerDataType::COMPOSITION, "Petrochem\nShale"},
                    {ScannerDataType::VALUE, "40.0 kC"}
                }
            },
            {
                ItemID::silver_ore,
                {
                    {ScannerDataType::TYPE, "Transition\nMetal"},
                    {ScannerDataType::COMPOSITION, "Elemental\nSilver"},
                    {ScannerDataType::VALUE, "52.0 kC"}
                }
            },
            {
                ItemID::snow,
                {
                    {ScannerDataType::TYPE, "Snow"},
                    {ScannerDataType::COMPOSITION, "Crystalline\nhydrogen\nfluoride"},
                    {ScannerDataType::VALUE, "???"}
                }
            },
            {
                ItemID::sulphur_crystal,
                {
                    {ScannerDataType::TYPE, "Crystal"},
                    {ScannerDataType::COMPOSITION, "Elemental\nSulphur"},
                    {ScannerDataType::VALUE, "38.8 kC"}
                }
            },
            {
                ItemID::sulphur_ore,
                {
                    {ScannerDataType::TYPE, "Mineral"},
                    {ScannerDataType::COMPOSITION, "Sulphur Ore"},
                    {ScannerDataType::VALUE, "1.3 kC"}
                }
            },
            {
                ItemID::titanium,
                {
                    {ScannerDataType::TYPE, "Transition\nMetal"},
                    {ScannerDataType::COMPOSITION, "Titanium\n(Refined)"},
                    {ScannerDataType::VALUE, "40.6 kC"}
                }
            },
            {
                ItemID::titanium_ore,
                {
                    {ScannerDataType::TYPE, "Transition\nMetal"},
                    {ScannerDataType::COMPOSITION, "Titanium"},
                    {ScannerDataType::VALUE, "20.3 kC"}
                }
            },
            {
                ItemID::water,
                {
                    {ScannerDataType::TYPE, "Exolunar\nIce"},
                    {ScannerDataType::COMPOSITION, "Hydrogen\nFluoride"},
                    {ScannerDataType::VALUE, "0.0 kC"}
                }
            },
            {
                ItemID::xenostone,
                {
                    {ScannerDataType::TYPE, "\nXenobiological"},
                    {ScannerDataType::COMPOSITION, "Crystalline\nsilicates"},
                    {ScannerDataType::VALUE, "2.1 kC"}
                }
            },
            {
                ItemID::zircon_ore,
                {
                    {ScannerDataType::TYPE, "Mineral"},
                    {ScannerDataType::COMPOSITION, "Zircon Ore"},
                    {ScannerDataType::VALUE, "3.3 kc"}
                }
            },
            {
                ItemID::beacon,
                {
                    {ScannerDataType::TYPE, "Beacon"},
                    {ScannerDataType::COMPOSITION, "Quantum\nTransmitter"},
                    {ScannerDataType::VALUE, "480.0 kC"}
                }
            },
            {
                ItemID::disk,
                {
                    {ScannerDataType::TYPE, "Data"},
                    {ScannerDataType::COMPOSITION, "Split items with\nright click."},
                    {ScannerDataType::VALUE, ""}
                }
            },
            {
                ItemID::disk1,
                {
                    {ScannerDataType::TYPE, "Data"},
                    {ScannerDataType::COMPOSITION, "Encase biology\nand machine to\nheal."},
                    {ScannerDataType::VALUE, ""}
                }
            },
            {
                ItemID::disk2,
                {
                    {ScannerDataType::TYPE, "Data"},
                    {ScannerDataType::COMPOSITION, "Aluminum\nbatteries can be\nmade from\nglowplants."},
                    {ScannerDataType::VALUE, ""}
                }
            },
            {
                ItemID::disk3,
                {
                    {ScannerDataType::TYPE, "Data"},
                    {ScannerDataType::COMPOSITION, "The slug pistol\nuses a magnet\nand needs\nenergy."},
                    {ScannerDataType::VALUE, ""}
                }
            },
            {
                ItemID::disk4,
                {
                    {ScannerDataType::TYPE, "Data"},
                    {ScannerDataType::COMPOSITION, "Jetpacks use a\nlot of energy\nand need a\nstrong case."},
                    {ScannerDataType::VALUE, ""}
                }
            },
            {
                ItemID::disk5,
                {
                    {ScannerDataType::TYPE, "Data"},
                    {ScannerDataType::COMPOSITION, "The best\nmachines are\nmade from the\nbest aluminum."},
                    {ScannerDataType::VALUE, ""}
                }
            },
            {
                ItemID::disk6,
                {
                    {ScannerDataType::TYPE, "Data"},
                    {ScannerDataType::COMPOSITION, "Crystals have\nan exotic\nenergy in them."},
                    {ScannerDataType::VALUE, ""}
                }
            },
            {
                ItemID::disk7,
                {
                    {ScannerDataType::TYPE, "Data"},
                    {ScannerDataType::COMPOSITION, "My boss\'s slug\npistol uses an\nenergy orb."},
                    {ScannerDataType::VALUE, ""}
                }
            },
            {
                ItemID::disk8,
                {
                    {ScannerDataType::TYPE, "Data"},
                    {ScannerDataType::COMPOSITION, "Refine minerals\nto build useful\nitems."},
                    {ScannerDataType::VALUE, ""}
                }
            },
            {
                ItemID::disk9,
                {
                    {ScannerDataType::TYPE, "Data"},
                    {ScannerDataType::COMPOSITION, "Pistols are made\nin a pistol-shape."},
                    {ScannerDataType::VALUE, ""}
                }
            },
            {
                ItemID::disk10,
                {
                    {ScannerDataType::TYPE, "Data"},
                    {ScannerDataType::COMPOSITION, "Turrets are\npowered, armored,\nand armed."},
                    {ScannerDataType::VALUE, ""}
                }
            },
            {
                ItemID::disk11,
                {
                    {ScannerDataType::TYPE, "Data"},
                    {ScannerDataType::COMPOSITION, "Drills use two\nmachines, a power\nsource, and a\nmetal bit."},
                    {ScannerDataType::VALUE, ""}
                }
            },
            {
                ItemID::disk12,
                {
                    {ScannerDataType::TYPE, "Data"},
                    {ScannerDataType::COMPOSITION, "Energy orbs\nrequire neptunium."},
                    {ScannerDataType::VALUE, ""}
                }
            },
            {
                ItemID::disk13,
                {
                    {ScannerDataType::TYPE, "Data"},
                    {ScannerDataType::COMPOSITION, "Neptunium comes\nfrom the rarest\ncrystal."},
                    {ScannerDataType::VALUE, ""}
                }
            },
            {
                ItemID::disk14,
                {
                    {ScannerDataType::TYPE, "Data"},
                    {ScannerDataType::COMPOSITION, "Combine alien\nbio and machines\nto manipulate\ntime."},
                    {ScannerDataType::VALUE, ""}
                }
            },
            {
                ItemID::disk15,
                {
                    {ScannerDataType::TYPE, "Data"},
                    {ScannerDataType::COMPOSITION, "Jetpacks use\nthree energy\nunits."},
                    {ScannerDataType::VALUE, ""}
                }
            },
            {
                ItemID::disk16,
                {
                    {ScannerDataType::TYPE, "Data"},
                    {ScannerDataType::COMPOSITION, "Quality metal is\nneeded for pistol\nstocks and\nbarrels."},
                    {ScannerDataType::VALUE, ""}
                }
            },
            {
                ItemID::disk17,
                {
                    {ScannerDataType::TYPE, "Data"},
                    {ScannerDataType::COMPOSITION, "Dilute biogel to\nmake post-it\nlights."},
                    {ScannerDataType::VALUE, ""}
                }
            },
            {
                ItemID::disk18,
                {
                    {ScannerDataType::TYPE, "Data"},
                    {ScannerDataType::COMPOSITION, "Barren moon?\nDistill wood for\nxenobiologicals."},
                    {ScannerDataType::VALUE, ""}
                }
            },
            {
                ItemID::drill_t1,
                {
                    {ScannerDataType::TYPE, "Drill"},
                    {ScannerDataType::COMPOSITION, "Electromechanical"},
                    {ScannerDataType::VALUE, "120.0 kC"}
                }
            },
            {
                ItemID::drill_t2,
                {
                    {ScannerDataType::TYPE, "Drill II"},
                    {ScannerDataType::COMPOSITION, "Electromechanical"},
                    {ScannerDataType::VALUE, "240.0 kC"}
                }
            },
            {
                ItemID::drill_t3,
                {
                    {ScannerDataType::TYPE, "Drill III"},
                    {ScannerDataType::COMPOSITION, "Electromechanical"},
                    {ScannerDataType::VALUE, "960.0 kC"}
                }
            },
            {
                ItemID::jetpack_t1,
                {
                    {ScannerDataType::TYPE, "Jetpack I"},
                    {ScannerDataType::COMPOSITION, "Microfusion Cell"},
                    {ScannerDataType::VALUE, "250.0 kC"}
                }
            },
            {
                ItemID::jetpack_t2,
                {
                    {ScannerDataType::TYPE, "Jetpack II"},
                    {ScannerDataType::COMPOSITION, "Multifusion Cell"},
                    {ScannerDataType::VALUE, "500.0 kC"}
                }
            },
            {
                ItemID::jetpack_t3,
                {
                    {ScannerDataType::TYPE, "Jetpack\nIII"},
                    {ScannerDataType::COMPOSITION, "Alien Technology"},
                    {ScannerDataType::VALUE, "3050.0 kC"}
                }
            },
            {
                ItemID::mechanism,
                {
                    {ScannerDataType::TYPE, "Mechanism"},
                    {ScannerDataType::COMPOSITION, ""},
                    {ScannerDataType::VALUE, ""}
                }
            },
            {
                ItemID::medkit,
                {
                    {ScannerDataType::TYPE, "Medkit"},
                    {ScannerDataType::COMPOSITION, "Therapeutic\nNanos"},
                    {ScannerDataType::VALUE, "300.0 kC"}
                }
            },
            {
                ItemID::slug_pistol_t1,
                {
                    {ScannerDataType::TYPE, "Slug\nPistol"},
                    {ScannerDataType::COMPOSITION, "Electromechanical"},
                    {ScannerDataType::VALUE, "199.9 kC"}
                }
            },
            {
                ItemID::slug_pistol_t2,
                {
                    {ScannerDataType::TYPE, "Slug\nPistol II"},
                    {ScannerDataType::COMPOSITION, "Electromagnetic"},
                    {ScannerDataType::VALUE, "400.0 kC"}
                }
            },
            {
                ItemID::slug_pistol_t3,
                {
                    {ScannerDataType::TYPE, "Slug\nPistol III"},
                    {ScannerDataType::COMPOSITION, "Gravitic\nMechanism"},
                    {ScannerDataType::VALUE, "1400.0 kC"}
                }
            },
            {
                ItemID::turret_t1,
                {
                    {ScannerDataType::TYPE, "Security\nRobot"},
                    {ScannerDataType::COMPOSITION, "Polymer"},
                    {ScannerDataType::VALUE, "150.0 kC"}
                }
            },
            {
                ItemID::turret_t2,
                {
                    {ScannerDataType::TYPE, "Security\nRobot"},
                    {ScannerDataType::COMPOSITION, "Polymer"},
                    {ScannerDataType::VALUE, "450.0 kC"}
                }
            },
            {
                ItemID::turret_t3,
                {
                    {ScannerDataType::TYPE, "Security\nRobot"},
                    {ScannerDataType::COMPOSITION, "Polymer"},
                    {ScannerDataType::VALUE, "900.0 kC"}
                }
            },
            {
                ItemID::chronobooster,
                {
                    {ScannerDataType::TYPE, "Alien\ndevice"},
                    {ScannerDataType::COMPOSITION, "Unknown"},
                    {ScannerDataType::VALUE, "???"}
                }
            },
            {
                ItemID::chronowinder,
                {
                    {ScannerDataType::TYPE, "Alien\ndevice"},
                    {ScannerDataType::COMPOSITION, "Unknown"},
                    {ScannerDataType::VALUE, "???"}
                }
            },
            {
                ItemID::battery,
                {
                    {ScannerDataType::TYPE, "Energy\nSource"},
                    {ScannerDataType::COMPOSITION, "Electrochemical\nCells"},
                    {ScannerDataType::VALUE, "90.5 kC"}
                }
            },
            {
                ItemID::energy_orb,
                {
                    {ScannerDataType::TYPE, "Energy\nSource"},
                    {ScannerDataType::COMPOSITION, "Alien\nTechnology"},
                    {ScannerDataType::VALUE, "???"}
                }
            },
            {
                ItemID::power_crystal,
                {
                    {ScannerDataType::TYPE, "Energy\nSource"},
                    {ScannerDataType::COMPOSITION, "Alien\nTechnology"},
                    {ScannerDataType::VALUE, "12050.0 kC"}
                }
            },
            {
                ItemID::minilight,
                {
                    {ScannerDataType::TYPE, "Electrical\nLight"},
                    {ScannerDataType::COMPOSITION, "Mixed"},
                    {ScannerDataType::VALUE, "10.0 kC"}
                }
            },
            {
                ItemID::magnet,
                {
                    {ScannerDataType::TYPE, "Magnet"},
                    {ScannerDataType::COMPOSITION, "Iron Oxide/Barium\nCarbonate"},
                    {ScannerDataType::VALUE, "50.0 kC"}
                }
            },
            {
                ItemID::biogel,
                {
                    {ScannerDataType::TYPE, "\nXenobiological"},
                    {ScannerDataType::COMPOSITION, "Biochemical\nSilicates"},
                    {ScannerDataType::VALUE, "60.0 kC"}
                }
            },
            {
                ItemID::gold,
                {
                    {ScannerDataType::TYPE, "Currency"},
                    {ScannerDataType::COMPOSITION, "Gold"},
                    {ScannerDataType::VALUE, "1000.0 kC"}
                }
            },
        };

        return map;
    }();

    if (SCANNER_DATA.contains(item))
        return SCANNER_DATA[item];
    else
        return std::unordered_map<ScannerDataType, std::string>{};
}
