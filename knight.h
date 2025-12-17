#pragma once
#include "npc.h"
#include "visitor.h"

class Knight : public NPC {
public:
    Knight(const std::string& n, double xPos, double yPos);
    
    std::string getType() const override;
    bool canDefeat(const NPC& other) const override;
    
    void accept(BattleVisitor& visitor) override;
};