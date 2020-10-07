/* date = May 22nd 2020 11:50 am */

#ifndef _CLASS_H
#define _CLASS_H

const char *Classes[] = 
{
    "Barbarian", "Bard", "Cleric", "Druid", 
    "Fighter", "Monk", "Paladin", "Ranger",
    "Rogue", "Sorcerer", "Wizard"
};

enum GameClass : u8
{
    CLASS_INVALID,
    
    CLASS_BARBARIAN,
    CLASS_BARD,
    CLASS_CLERIC,
    CLASS_DRUID,
    CLASS_FIGHTER,
    CLASS_MONK,
    CLASS_PALADIN,
    CLASS_RANGER,
    CLASS_ROGUE,
    CLASS_SORCERER,
    CLASS_WIZARD,
    
    CLASS_COUNT
};

s32 ClassBAB[CLASS_COUNT][4][20] = 
{
    {
        {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20}, 
        {0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5}
    },
    
    {
        {0, 1, 2, 3, 3, 4, 5, 6, 6, 7, 8, 9, 9, 10, 11, 12, 12, 13, 14, 15}, 
        {0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 4, 4, 5, 6, 7, 7, 8, 9, 10}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    
    {
        {0, 1, 2, 3, 3, 4, 5, 6, 6, 7, 8, 9, 9, 10, 11, 12, 12, 13, 14, 15}, 
        {0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 4, 4, 5, 6, 7, 7, 8, 9, 10}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    
    {
        {0, 1, 2, 3, 3, 4, 5, 6, 6, 7, 8, 9, 9, 10, 11, 12, 12, 13, 14, 15}, 
        {0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 4, 4, 5, 6, 7, 7, 8, 9, 10}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    
    {
        {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20}, 
        {0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5}
    },
    
    {
        {0, 1, 2, 3, 3, 4, 5, 6, 6, 7, 8, 9, 9, 10, 11, 12, 12, 13, 14, 15}, 
        {0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 4, 4, 5, 6, 7, 7, 8, 9, 10}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    
    {
        {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20}, 
        {0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5}
    },
    
    {
        {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20}, 
        {0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5}
    },
    
    {
        {0, 1, 2, 3, 3, 4, 5, 6, 6, 7, 8, 9, 9, 10, 11, 12, 12, 13, 14, 15}, 
        {0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 4, 4, 5, 6, 7, 7, 8, 9, 10}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    
    {
        {0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} 
    },
    
    {
        {0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} 
    }
};

const char *ClassBABString[CLASS_COUNT][4][20] =
{
    {
        {"+1", "+2", "+3", "+4", "+5", "+6", "+7", "+8", "+9", "+10",
            "+11", "+12", "+13", "+14", "+15", "+16", "+17", "+18", "+19", "+20"}, 
        {"+0", "+0", "+0", "+0", "+0", "+1", "+2", "+3", "+4", "+5",
            "+6", "+7", "+8", "+9", "+10", "+11", "+12", "+13", "+14", "+15"}, 
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",
            "+1", "+2", "+3", "+4", "+5", "+6", "+7", "+8", "+9", "+10"}, 
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",
            "+0", "+0", "+0", "+0", "+0", "+1", "+2", "+3", "+4", "+5"}
    },
    
    {
        {"+0", "+1", "+2", "+3", "+3", "+4", "+5", "+6", "+6", "+7", "+8",
            "+9", "+9", "+10", "+11", "+12", "+12", "+13", "+14", "+15"}, 
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+1", "+1", "+2",
            "+3", "+4", "+4", "+5", "+6", "+7", "+7", "+8", "+9", "+10",}, 
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",
            "+0", "+0", "+0", "+0", "+0", "+1", "+2", "+3", "+4", "+5",},
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",
            "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",}
    },
    
    {
        
        {"+0", "+1", "+2", "+3", "+3", "+4", "+5", "+6", "+6", "+7", "+8",
            "+9", "+9", "+10", "+11", "+12", "+12", "+13", "+14", "+15"}, 
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+1", "+1", "+2",
            "+3", "+4", "+4", "+5", "+6", "+7", "+7", "+8", "+9", "+10",}, 
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",
            "+0", "+0", "+0", "+0", "+0", "+1", "+2", "+3", "+4", "+5",},
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",
            "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",}
    },
    
    {
        
        {"+0", "+1", "+2", "+3", "+3", "+4", "+5", "+6", "+6", "+7", "+8",
            "+9", "+9", "+10", "+11", "+12", "+12", "+13", "+14", "+15"}, 
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+1", "+1", "+2",
            "+3", "+4", "+4", "+5", "+6", "+7", "+7", "+8", "+9", "+10",}, 
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",
            "+0", "+0", "+0", "+0", "+0", "+1", "+2", "+3", "+4", "+5",},
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",
            "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",}
    },
    
    {
        {"+1", "+2", "+3", "+4", "+5", "+6", "+7", "+8", "+9", "+10",
            "+11", "+12", "+13", "+14", "+15", "+16", "+17", "+18", "+19", "+20"}, 
        {"+0", "+0", "+0", "+0", "+0", "+1", "+2", "+3", "+4", "+5",
            "+6", "+7", "+8", "+9", "+10", "+11", "+12", "+13", "+14", "+15"}, 
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",
            "+1", "+2", "+3", "+4", "+5", "+6", "+7", "+8", "+9", "+10"}, 
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",
            "+0", "+0", "+0", "+0", "+0", "+1", "+2", "+3", "+4", "+5"}
    },
    
    {
        {"+0", "+1", "+2", "+3", "+3", "+4", "+5", "+6", "+6", "+7", "+8",
            "+9", "+9", "+10", "+11", "+12", "+12", "+13", "+14", "+15"}, 
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+1", "+1", "+2",
            "+3", "+4", "+4", "+5", "+6", "+7", "+7", "+8", "+9", "+10",}, 
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",
            "+0", "+0", "+0", "+0", "+0", "+1", "+2", "+3", "+4", "+5",},
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",
            "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",}
    },
    
    {
        {"+1", "+2", "+3", "+4", "+5", "+6", "+7", "+8", "+9", "+10",
            "+11", "+12", "+13", "+14", "+15", "+16", "+17", "+18", "+19", "+20"}, 
        {"+0", "+0", "+0", "+0", "+0", "+1", "+2", "+3", "+4", "+5",
            "+6", "+7", "+8", "+9", "+10", "+11", "+12", "+13", "+14", "+15"}, 
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",
            "+1", "+2", "+3", "+4", "+5", "+6", "+7", "+8", "+9", "+10"}, 
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",
            "+0", "+0", "+0", "+0", "+0", "+1", "+2", "+3", "+4", "+5"}
    },
    
    {
        {"+1", "+2", "+3", "+4", "+5", "+6", "+7", "+8", "+9", "+10",
            "+11", "+12", "+13", "+14", "+15", "+16", "+17", "+18", "+19", "+20"}, 
        {"+0", "+0", "+0", "+0", "+0", "+1", "+2", "+3", "+4", "+5",
            "+6", "+7", "+8", "+9", "+10", "+11", "+12", "+13", "+14", "+15"}, 
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",
            "+1", "+2", "+3", "+4", "+5", "+6", "+7", "+8", "+9", "+10"}, 
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",
            "+0", "+0", "+0", "+0", "+0", "+1", "+2", "+3", "+4", "+5"}
    },
    
    {
        {"+0", "+1", "+2", "+3", "+3", "+4", "+5", "+6", "+6", "+7", "+8",
            "+9", "+9", "+10", "+11", "+12", "+12", "+13", "+14", "+15"}, 
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+1", "+1", "+2",
            "+3", "+4", "+4", "+5", "+6", "+7", "+7", "+8", "+9", "+10",}, 
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",
            "+0", "+0", "+0", "+0", "+0", "+1", "+2", "+3", "+4", "+5",},
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",
            "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",}
    },
    
    {
        {"+0", "+1", "+1", "+2", "+2", "+3", "+3", "+4", "+4", "+5",
            "+5", "+6", "+6", "+7", "+7", "+8", "+8", "+9", "+9", "+10",},
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",
            "+0", "+1", "+1", "+2", "+2", "+3", "+3", "+4", "+4", "+5",},
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",
            "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",},
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",
            "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",} 
    },
    
    {
        {"+0", "+1", "+1", "+2", "+2", "+3", "+3", "+4", "+4", "+5",
            "+5", "+6", "+6", "+7", "+7", "+8", "+8", "+9", "+9", "+10",},
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",
            "+0", "+1", "+1", "+2", "+2", "+3", "+3", "+4", "+4", "+5",},
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",
            "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",},
        {"+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",
            "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0", "+0",} 
    }
};

s32 ClassSavingThrows[CLASS_COUNT][3][20] = 
{
    {
        {2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12}, 
        {0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6}, 
        {0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6}
    },
    
    {
        {0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6}, 
        {2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12}, 
        {2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12}
    },
    
    {
        {2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12}, 
        {0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6}, 
        {2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12}
    },
    
    {
        {2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12}, 
        {0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6}, 
        {2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12}
    },
    
    {
        {2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12}, 
        {2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12}, 
        {2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12}
    },
    
    {
        {2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12}, 
        {0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6}, 
        {2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12}
    },
    
    {
        {2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12}, 
        {2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12}, 
        {0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6}
    },
    
    {
        {0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6}, 
        {2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12}, 
        {0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6}
    },
    
    {
        {0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6}, 
        {0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6},
        {2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12}
    },
    
    {
        {0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6}, 
        {0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6},
        {2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12}
    }
};

const char *ClassSavingThrowsString[CLASS_COUNT][3][20] = 
{
    {
        {"+2", "+3", "+3", "+4", "+4", "+5", "+5", "+6", "+6", "+7", 
            "+7", "+8", "+8", "+9", "+9", "+10", "+10", "+11", "+11", "+12"}, 
        {"0", "0", "+1", "+1", "+1", "+2", "+2", "+2", "+3", "+3", 
            "+3", "+4", "+4", "+4", "+5", "+5", "+5", "+6", "+6", "+6"}, 
        {"0", "0", "+1", "+1", "+1", "+2", "+2", "+2", "+3", "+3", 
            "+3", "+4", "+4", "+4", "+5", "+5", "+5", "+6", "+6", "+6"}
    },
    
    {
        {"0", "0", "+1", "+1", "+1", "+2", "+2", "+2", "+3", "+3", 
            "+3", "+4", "+4", "+4", "+5", "+5", "+5", "+6", "+6", "+6"}, 
        {"+2", "+3", "+3", "+4", "+4", "+5", "+5", "+6", "+6", "+7", 
            "+7", "+8", "+8", "+9", "+9", "+10", "+10", "+11", "+11", "+12"}, 
        {"+2", "+3", "+3", "+4", "+4", "+5", "+5", "+6", "+6", "+7", 
            "+7", "+8", "+8", "+9", "+9", "+10", "+10", "+11", "+11", "+12"}
    },
    
    {
        {"+2", "+3", "+3", "+4", "+4", "+5", "+5", "+6", "+6", "+7", 
            "+7", "+8", "+8", "+9", "+9", "+10", "+10", "+11", "+11", "+12"}, 
        {"0", "0", "+1", "+1", "+1", "+2", "+2", "+2", "+3", "+3", 
            "+3", "+4", "+4", "+4", "+5", "+5", "+5", "+6", "+6", "+6"}, 
        {"+2", "+3", "+3", "+4", "+4", "+5", "+5", "+6", "+6", "+7", 
            "+7", "+8", "+8", "+9", "+9", "+10", "+10", "+11", "+11", "+12"}
    },
    
    {
        {"+2", "+3", "+3", "+4", "+4", "+5", "+5", "+6", "+6", "+7", 
            "+7", "+8", "+8", "+9", "+9", "+10", "+10", "+11", "+11", "+12"}, 
        {"0", "0", "+1", "+1", "+1", "+2", "+2", "+2", "+3", "+3", 
            "+3", "+4", "+4", "+4", "+5", "+5", "+5", "+6", "+6", "+6"}, 
        {"+2", "+3", "+3", "+4", "+4", "+5", "+5", "+6", "+6", "+7", 
            "+7", "+8", "+8", "+9", "+9", "+10", "+10", "+11", "+11", "+12"}
    },
    
    {
        {"+2", "+3", "+3", "+4", "+4", "+5", "+5", "+6", "+6", "+7", 
            "+7", "+8", "+8", "+9", "+9", "+10", "+10", "+11", "+11", "+12"}, 
        {"+2", "+3", "+3", "+4", "+4", "+5", "+5", "+6", "+6", "+7", 
            "+7", "+8", "+8", "+9", "+9", "+10", "+10", "+11", "+11", "+12"}, 
        {"+2", "+3", "+3", "+4", "+4", "+5", "+5", "+6", "+6", "+7", 
            "+7", "+8", "+8", "+9", "+9", "+10", "+10", "+11", "+11", "+12"}
    },
    
    {
        {"+2", "+3", "+3", "+4", "+4", "+5", "+5", "+6", "+6", "+7", 
            "+7", "+8", "+8", "+9", "+9", "+10", "+10", "+11", "+11", "+12"}, 
        {"0", "0", "+1", "+1", "+1", "+2", "+2", "+2", "+3", "+3", 
            "+3", "+4", "+4", "+4", "+5", "+5", "+5", "+6", "+6", "+6"}, 
        {"+2", "+3", "+3", "+4", "+4", "+5", "+5", "+6", "+6", "+7", 
            "+7", "+8", "+8", "+9", "+9", "+10", "+10", "+11", "+11", "+12"}
    },
    
    {
        {"+2", "+3", "+3", "+4", "+4", "+5", "+5", "+6", "+6", "+7", 
            "+7", "+8", "+8", "+9", "+9", "+10", "+10", "+11", "+11", "+12"}, 
        {"+2", "+3", "+3", "+4", "+4", "+5", "+5", "+6", "+6", "+7", 
            "+7", "+8", "+8", "+9", "+9", "+10", "+10", "+11", "+11", "+12"}, 
        {"0", "0", "+1", "+1", "+1", "+2", "+2", "+2", "+3", "+3", 
            "+3", "+4", "+4", "+4", "+5", "+5", "+5", "+6", "+6", "+6"}
    },
    
    {
        {"0", "0", "+1", "+1", "+1", "+2", "+2", "+2", "+3", "+3", 
            "+3", "+4", "+4", "+4", "+5", "+5", "+5", "+6", "+6", "+6"}, 
        {"+2", "+3", "+3", "+4", "+4", "+5", "+5", "+6", "+6", "+7", 
            "+7", "+8", "+8", "+9", "+9", "+10", "+10", "+11", "+11", "+12"}, 
        {"0", "0", "+1", "+1", "+1", "+2", "+2", "+2", "+3", "+3", 
            "+3", "+4", "+4", "+4", "+5", "+5", "+5", "+6", "+6", "+6"}
    },
    
    {
        {"0", "0", "+1", "+1", "+1", "+2", "+2", "+2", "+3", "+3", 
            "+3", "+4", "+4", "+4", "+5", "+5", "+5", "+6", "+6", "+6"}, 
        {"0", "0", "+1", "+1", "+1", "+2", "+2", "+2", "+3", "+3", 
            "+3", "+4", "+4", "+4", "+5", "+5", "+5", "+6", "+6", "+6"},
        {"+2", "+3", "+3", "+4", "+4", "+5", "+5", "+6", "+6", "+7", 
            "+7", "+8", "+8", "+9", "+9", "+10", "+10", "+11", "+11", "+12"}
    },
    
    {
        {"0", "0", "+1", "+1", "+1", "+2", "+2", "+2", "+3", "+3", 
            "+3", "+4", "+4", "+4", "+5", "+5", "+5", "+6", "+6", "+6"}, 
        {"0", "0", "+1", "+1", "+1", "+2", "+2", "+2", "+3", "+3", 
            "+3", "+4", "+4", "+4", "+5", "+5", "+5", "+6", "+6", "+6"},
        {"+2", "+3", "+3", "+4", "+4", "+5", "+5", "+6", "+6", "+7", 
            "+7", "+8", "+8", "+9", "+9", "+10", "+10", "+11", "+11", "+12"}
    }
};


#endif //_CLASS_H
