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
        "tactical",
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
		{"tactical",        {ItemIndex::WEAPON_KNIFE_TACTICAL,       "#SFUI_WPNHUD_KnifeTactical",           "weapon_knife_tactical",        "models/weapons/v_knife_tactical.mdl",          "knife_tactical"}},
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

    void run(FrameStage stage);
}