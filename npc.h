#pragma once
#include <string>
#include <random>

class BattleVisitor;

class NPC {
protected:
    std::string name;
    double x, y;
    bool alive;
    double moveDistance; 
    
    static std::random_device rd;
    static std::mt19937 gen;
    static std::uniform_int_distribution<> dice;
    
public:
    NPC(const std::string& n, double xPos, double yPos, double moveDist);
    virtual ~NPC() = default;

    virtual std::string getType() const = 0;
    std::string getName() const;
    double getX() const;
    double getY() const;
    bool isAlive() const;
    double getMoveDistance() const;
    
    void move(double maxX, double maxY);
    
    double distanceTo(const NPC& other) const;
    bool isInKillingRange(const NPC& other) const;
    
    virtual bool canDefeat(const NPC& other) const = 0;
    virtual void fight(NPC& other);
    
    void die();
    
    virtual void accept(BattleVisitor& visitor) = 0;
    
    static int rollDice();
};