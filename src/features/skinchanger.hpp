#pragma once
#include <string>
#include <map>
#include "../interfaces.hpp"
#include "../sdk/classids.hpp"

namespace SkinChanger {
    struct Item {
        ItemIndex index;
        const char* displayName;
        const char* entityName;
        const char* modelName;
        const char* killIcon;
    };

    inline const char* knives[] = {
        "",
        "bayonet",
        "flip",
        "gut",
        "karambit",
        "m9 bayonet",
        "huntsman",
        "falchion",
        "bowie",
        "butterfly",
        "push",
        "ursus",
        "navaja",
        "stiletto",
        "talon",
        "classic",
        "ghost",
        "gold"
    };

    inline const char* weapons[] = {
        "",
        "deagle",
        "elites",
        "five-seven",
        "glock",
        "ak47",
        "aug",
        "awp",
        "famas",
        "g3sg1",
        "galil",
        "m249",
        "m4a4",
        "mac10",
        "p90",
        "ump45",
        "mp5",
        "xm1014",
        "bizon",
        "mag7",
        "negev",
        "sawedoff",
        "tec9",
        "zeus",
        "p2000",
        "mp7",
        "mp9",
        "nova",
        "p250",
        "scar20",
        "sg556",
        "scout",
        /* knives */
        "knife",
        "bayonet",
        "flip",
        "gut",
        "karambit",
        "m9 bayonet",
        "huntsman",
        "falchion",
        "bowie",
        "butterfly",
        "push",
        "ursus",
        "navaja",
        "stiletto",
        "talon",
        "classic",
        "ghost",
        "gold"
    };

    inline std::map<std::string_view, Item> nameToItemMap = {
        {"bayonet",         {ItemIndex::WEAPON_KNIFE_BAYONET,        "#SFUI_WPNHUD_KnifeBayonet",            "weapon_knife_bayonet",         "models/weapons/v_knife_bayonet.mdl",           "bayonet"}},
		{"flip",            {ItemIndex::WEAPON_KNIFE_FLIP,           "#SFUI_WPNHUD_KnifeFlip",               "weapon_knife_flip",            "models/weapons/v_knife_flip.mdl",              "knife_flip"}},
		{"gut",             {ItemIndex::WEAPON_KNIFE_GUT,            "#SFUI_WPNHUD_KnifeGut",                "weapon_knife_gut",             "models/weapons/v_knife_gut.mdl",               "knife_gut"}},
		{"karambit",        {ItemIndex::WEAPON_KNIFE_KARAMBIT,       "#SFUI_WPNHUD_KnifeKaram",              "weapon_knife_karambit",        "models/weapons/v_knife_karam.mdl",             "knife_karambit"}},
		{"m9 bayonet",      {ItemIndex::WEAPON_KNIFE_M9_BAYONET,     "#SFUI_WPNHUD_KnifeM9",                 "weapon_knife_m9_bayonet",      "models/weapons/v_knife_m9_bay.mdl",            "knife_m9_bayonet"}},
		{"huntsman",        {ItemIndex::WEAPON_KNIFE_TACTICAL,       "#SFUI_WPNHUD_KnifeTactical",           "weapon_knife_tactical",        "models/weapons/v_knife_tactical.mdl",          "knife_tactical"}},
		{"falchion",        {ItemIndex::WEAPON_KNIFE_FALCHION,       "#SFUI_WPNHUD_knife_falchion_advanced", "weapon_knife_falchion",        "models/weapons/v_knife_falchion_advanced.mdl", "knife_falchion"}},
		{"bowie",           {ItemIndex::WEAPON_KNIFE_SURVIVAL_BOWIE, "#SFUI_WPNHUD_knife_survival_bowie",    "weapon_knife_survival_bowie",  "models/weapons/v_knife_survival_bowie.mdl",    "knife_survival_bowie"}},
		{"butterfly",       {ItemIndex::WEAPON_KNIFE_BUTTERFLY,      "#SFUI_WPNHUD_Knife_Butterfly",         "weapon_knife_butterfly",       "models/weapons/v_knife_butterfly.mdl",         "knife_butterfly"}},
		{"push",            {ItemIndex::WEAPON_KNIFE_PUSH,           "#SFUI_WPNHUD_knife_push",              "weapon_knife_push",            "models/weapons/v_knife_push.mdl",              "knife_push"}},
		{"ursus",           {ItemIndex::WEAPON_KNIFE_URSUS,          "#SFUI_WPNHUD_knife_ursus",             "weapon_knife_ursus",           "models/weapons/v_knife_ursus.mdl",             "knife_ursus"}},
		{"navaja",          {ItemIndex::WEAPON_KNIFE_GYPSY_JACKKNIFE,"#SFUI_WPNHUD_knife_gypsy_jackknife",   "weapon_knife_gypsy_jackknife", "models/weapons/v_knife_gypsy_jackknife.mdl",   "knife_gypsy_jackknife"}},
		{"stiletto",        {ItemIndex::WEAPON_KNIFE_STILETTO,       "#SFUI_WPNHUD_knife_stiletto",          "weapon_knife_stiletto",        "models/weapons/v_knife_stiletto.mdl",          "knife_stiletto"}},
		{"talon",           {ItemIndex::WEAPON_KNIFE_WIDOWMAKER,     "#SFUI_WPNHUD_knife_widowmaker",        "weapon_knife_widowmaker",      "models/weapons/v_knife_widowmaker.mdl",        "knife_widowmaker"}},
		{"classic",         {ItemIndex::WEAPON_KNIFE_CSS,            "#SFUI_WPNHUD_KnifeCSS",                "weapon_knife_css",             "models/weapons/v_knife_css.mdl",               "knife_css"}},
		{"ghost",           {ItemIndex::WEAPON_KNIFE_GHOST,          "#SFUI_WPNHUD_knife_ghost",             "weapon_knife_ghost",           "models/weapons/v_knife_ghost.mdl",             "knife_ghost"}},
		{"gold",            {ItemIndex::WEAPON_KNIFEGG,              "#SFUI_WPNHUD_Knife_GG",                "weapon_knifegg",               "models/weapons/v_knife_gg.mdl",                "knifegg"}}
    };

    inline std::map<ItemIndex, const char*> itemIndexToNameMap = {
        {ItemIndex::WEAPON_DEAGLE, "deagle"},
        {ItemIndex::WEAPON_ELITE, "elites"},
        {ItemIndex::WEAPON_FIVESEVEN, "five-seven"},
        {ItemIndex::WEAPON_GLOCK, "glock"},
        {ItemIndex::WEAPON_AK47, "ak47"},
        {ItemIndex::WEAPON_AUG, "aug"},
        {ItemIndex::WEAPON_AWP, "awp"},
        {ItemIndex::WEAPON_FAMAS, "famas"},
        {ItemIndex::WEAPON_G3SG1, "g3sg1"},
        {ItemIndex::WEAPON_GALILAR, "galil"},
        {ItemIndex::WEAPON_M249, "m249"},
        {ItemIndex::WEAPON_M4A1, "m4a4"},
        {ItemIndex::WEAPON_MAC10, "mac10"},
        {ItemIndex::WEAPON_P90, "p90"},
        {ItemIndex::WEAPON_UMP45, "ump45"},
        {ItemIndex::WEAPON_MP5, "mp5"},
        {ItemIndex::WEAPON_XM1014, "xm1014"},
        {ItemIndex::WEAPON_BIZON, "bizon"},
        {ItemIndex::WEAPON_MAG7, "mag7"},
        {ItemIndex::WEAPON_NEGEV, "negev"},
        {ItemIndex::WEAPON_SAWEDOFF, "sawedoff"},
        {ItemIndex::WEAPON_TEC9, "tec9"},
        {ItemIndex::WEAPON_TASER, "zeus"},
        {ItemIndex::WEAPON_HKP2000, "p2000"},
        {ItemIndex::WEAPON_MP7, "mp7"},
        {ItemIndex::WEAPON_MP9, "mp9"},
        {ItemIndex::WEAPON_NOVA, "nova"},
        {ItemIndex::WEAPON_P250, "p250"},
        {ItemIndex::WEAPON_SCAR20, "scar20"},
        {ItemIndex::WEAPON_SG556, "sg556"},
        {ItemIndex::WEAPON_SSG08, "scout"},
        {ItemIndex::WEAPON_USP_SILENCER, "usp-s"},
        {ItemIndex::WEAPON_M4A1_SILENCER, "m4a1-s"},
        {ItemIndex::WEAPON_C4, "c4"},
        /* knives */
        {ItemIndex::WEAPON_KNIFE, "knife"},
        {ItemIndex::WEAPON_KNIFE_BAYONET, "bayonet"},
        {ItemIndex::WEAPON_KNIFE_FLIP, "flip"},
        {ItemIndex::WEAPON_KNIFE_GUT, "gut"},
        {ItemIndex::WEAPON_KNIFE_KARAMBIT, "karambit"},
        {ItemIndex::WEAPON_KNIFE_M9_BAYONET, "m9 bayonet"},
        {ItemIndex::WEAPON_KNIFE_TACTICAL, "huntsman"},
        {ItemIndex::WEAPON_KNIFE_FALCHION, "falchion"},
        {ItemIndex::WEAPON_KNIFE_SURVIVAL_BOWIE, "bowie"},
        {ItemIndex::WEAPON_KNIFE_BUTTERFLY, "butterfly"},
        {ItemIndex::WEAPON_KNIFE_PUSH, "push"},
        {ItemIndex::WEAPON_KNIFE_URSUS, "ursus"},
        {ItemIndex::WEAPON_KNIFE_GYPSY_JACKKNIFE, "navaja"},
        {ItemIndex::WEAPON_KNIFE_STILETTO, "stiletto"},
        {ItemIndex::WEAPON_KNIFE_WIDOWMAKER, "talon"},
        {ItemIndex::WEAPON_KNIFE_CSS, "classic"},
        {ItemIndex::WEAPON_KNIFE_GHOST, "ghost"},
        {ItemIndex::WEAPON_KNIFEGG, "gold"}
    };

    void run();
}