#pragma once

#include <memory>
#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>

class NPC;
class DeathObserver;

struct FightTask {
    std::shared_ptr<NPC> attacker;
    std::shared_ptr<NPC> defender;
};

class BattleVisitor {
private:
    double range;
    std::vector<std::shared_ptr<NPC>>& npcs;
    std::vector<std::shared_ptr<DeathObserver>>& observers;
    std::mutex& fightQueueMutex;
    std::queue<FightTask>& fightQueue;
    std::condition_variable& fightQueueCV;

    void processFight(std::shared_ptr<NPC> attacker, std::shared_ptr<NPC> defender);
    void notifyObservers(const std::string& killerName, const std::string& killerType, const std::string& victimName, const std::string& victimType);
    
public:
    BattleVisitor(double r, 
                  std::vector<std::shared_ptr<NPC>>& n,
                  std::vector<std::shared_ptr<DeathObserver>>& obs,
                  std::mutex& queueMutex,
                  std::queue<FightTask>& queue,
                  std::condition_variable& cv);
    
    void visit(NPC& npc);
};