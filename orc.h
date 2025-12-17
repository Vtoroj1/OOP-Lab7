#pragma once
#include "npc.h"
#include "visitor.h"

class Orc : public NPC {
public:
    Orc(const std::string& n, double xPos, double yPos);
    
    std::string getType() const override;
    bool canDefeat(const NPC& other) const override;
    
    void accept(BattleVisitor& visitor) override;
};