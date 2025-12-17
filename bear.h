#pragma once
#include "npc.h"
#include "visitor.h"

class Bear : public NPC {
public:
    Bear(const std::string& n, double xPos, double yPos);
    
    std::string getType() const override;
    bool canDefeat(const NPC& other) const override;
    
    void accept(BattleVisitor& visitor) override;
};