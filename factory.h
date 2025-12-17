#pragma once

#include <memory>
#include <string>
#include <sstream>
#include <iostream>
#include "npc.h"
#include "knight.h"
#include "orc.h"
#include "bear.h"

class NPCFactory {
public:
    static std::unique_ptr<NPC> createNPC(const std::string& type, const std::string& name, double x, double y);
    static std::unique_ptr<NPC> loadFromString(const std::string& data);
};


