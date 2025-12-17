#pragma once

#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <map>
#include "npc.h"
#include "factory.h"
#include "visitor.h"
#include "observer.h"

class GameManager {
private:
    static constexpr double MAP_WIDTH = 100.0;
    static constexpr double MAP_HEIGHT = 100.0;
    static constexpr int GAME_DURATION_SECONDS = 30;
    static constexpr int INITIAL_NPC_COUNT = 50;
    
    std::vector<std::shared_ptr<NPC>> npcs;
    mutable std::shared_mutex npcsMutex;
    
    std::thread movementThread;
    std::thread fightThread;
    std::thread renderThread;
    
    std::queue<FightTask> fightQueue;
    std::mutex fightQueueMutex;
    std::condition_variable fightQueueCV;

    std::atomic<bool> isRunning;
    std::atomic<bool> stopRequested;
    
    std::vector<std::shared_ptr<DeathObserver>> observers;
    
    std::unique_ptr<BattleVisitor> battleVisitor;
    
    std::atomic<int> fightsProcessed;
    
    static std::mutex coutMutex;
    
    void movementWorker();
    void fightWorker();
    void renderWorker();
    
    void generateInitialNPCs();
    std::string generateRandomName(const std::string& type, int index);
    
    void initializeObservers();
    void initializeVisitor();
    
public:
    GameManager();
    ~GameManager();
    
    void start();
    void stop();
    void joinAll();
    
    static void safePrint(const std::string& message);
    void printMap() const;
    void printSurvivors() const;
    
    double getMapWidth() const { return MAP_WIDTH; }
    double getMapHeight() const { return MAP_HEIGHT; }
};