#include "visitor.h"
#include "npc.h"
#include "knight.h"
#include "orc.h"
#include "bear.h"
#include "observer.h"
#include <iostream>
#include <random>

BattleVisitor::BattleVisitor(double r, 
                           std::vector<std::shared_ptr<NPC>>& n,
                           std::vector<std::shared_ptr<DeathObserver>>& obs,
                           std::mutex& queueMutex,
                           std::queue<FightTask>& queue,
                           std::condition_variable& cv) : range(r), npcs(n), observers(obs), 
                           fightQueueMutex(queueMutex), fightQueue(queue), fightQueueCV(cv) {}

void BattleVisitor::visit(NPC& npc) {
    if (!npc.isAlive()) return;
    
    std::shared_ptr<NPC> npcPtr;
    for (auto& ptr : npcs) {
        if (ptr.get() == &npc) {
            npcPtr = ptr;
            break;
        }
    }
    
    if (!npcPtr) return;
    
    for (auto& other : npcs) {
        if (!other || other == npcPtr || !other->isAlive()) continue;
        
        if (npc.distanceTo(*other) <= range) {
            FightTask task;
            task.attacker = npcPtr;
            task.defender = other;
            
            {
                std::lock_guard lock(fightQueueMutex);
                fightQueue.push(task);
            }
            fightQueueCV.notify_one();
        }
    }
}

void BattleVisitor::processFight(std::shared_ptr<NPC> attacker, std::shared_ptr<NPC> defender) {
    if (!attacker->isAlive() || !defender->isAlive()) return;

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dice(1, 6);
    
    if (attacker->canDefeat(*defender)) {
        int attackPower = dice(gen);
        int defensePower = dice(gen);
        
        if (attackPower > defensePower) {
            defender->die();
            notifyObservers(attacker->getName(), attacker->getType(), defender->getName(), defender->getType());
        }
    }
    
    if (defender->isAlive() && defender->canDefeat(*attacker)) {
        int attackPower = dice(gen);
        int defensePower = dice(gen);
        
        if (attackPower > defensePower) {
            attacker->die();
            notifyObservers(defender->getName(), defender->getType(), attacker->getName(), attacker->getType());
        }
    }
}

void BattleVisitor::notifyObservers(const std::string& killerName, const std::string& killerType, const std::string& victimName, const std::string& victimType) {
    for (auto& observer : observers) {
        if (observer) {
            observer->onDeath(killerName + " (" + killerType + ")", victimName + " (" + victimType + ")");
        }
    }
}