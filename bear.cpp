#include "bear.h"
#include "knight.h"
#include "orc.h"

Bear::Bear(const std::string& n, double xPos, double yPos) : NPC(n, xPos, yPos, 5.0) {}

std::string Bear::getType() const {
    return "Bear";
}

bool Bear::canDefeat(const NPC& other) const {
    return other.getType() == "Knight";
}

void Bear::accept(BattleVisitor& visitor) {
    visitor.visit(*this);
}