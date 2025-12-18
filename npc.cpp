#include "npc.h"
#include <cmath>
#include <algorithm>

std::random_device NPC::rd;
std::mt19937 NPC::gen(NPC::rd());
std::uniform_int_distribution<> NPC::dice(1, 6);

NPC::NPC(const std::string& n, double xPos, double yPos, double moveDist) : name(n), x(xPos), y(yPos), alive(true), moveDistance(moveDist) {}

std::string NPC::getName() const {
    return name;
}

double NPC::getX() const {
    return x;
}

double NPC::getY() const {
    return y;
}

bool NPC::isAlive() const {
    return alive;
}

double NPC::getMoveDistance() const {
    return moveDistance;
}

void NPC::move(double maxX, double maxY) {
    if (!alive) return;
    
    std::uniform_real_distribution<> dirDist(0.0, 2.0 * M_PI);
    std::uniform_real_distribution<> distDist(0.0, moveDistance);
    
    double direction = dirDist(gen);
    double distance = distDist(gen);
    
    double newX = x + distance * cos(direction);
    double newY = y + distance * sin(direction);
    
    newX = std::max(0.0, std::min(newX, maxX - 1));
    newY = std::max(0.0, std::min(newY, maxY - 1));
    
    x = newX;
    y = newY;
}

double NPC::distanceTo(const NPC& other) const {
    return sqrt(pow(x - other.x, 2) + pow(y - other.y, 2));
}

bool NPC::isInKillingRange(const NPC& other) const {
    return distanceTo(other) <= 10.0;
}

void NPC::fight(NPC& other) {
    if (!alive || !other.alive) return;
    
    int attackPower = rollDice();
    int defensePower = other.rollDice();
    
    if (canDefeat(other) && attackPower > defensePower) {
        other.die();
    }
}

void NPC::die() {
    alive = false;
}

int NPC::rollDice() {
    return dice(gen);
}