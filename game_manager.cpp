#include "game_manager.h"
#include <iostream>
#include <chrono>
#include <random>
#include <iomanip>
#include <algorithm>

using namespace std::chrono_literals;

std::mutex GameManager::coutMutex;

void GameManager::initializeVisitor() {
    battleVisitor = std::make_unique<BattleVisitor>(
        10.0,
        npcs,
        observers,
        fightQueueMutex,
        fightQueue,
        fightQueueCV
    );
    
    safePrint("BattleVisitor инициализирован с дистанцией боя 10м");
}

void GameManager::initializeObservers() {
    auto consoleObs = std::make_shared<ConsoleObserver>();
    auto fileObs = std::make_shared<FileObserver>("battle_log.txt");
    
    observers.push_back(consoleObs);
    observers.push_back(fileObs);
    
    safePrint("Observer'ы инициализированы: ConsoleObserver, FileObserver (battle_log.txt)");
}


GameManager::GameManager() : isRunning(false), stopRequested(false), fightsProcessed(0) {
    generateInitialNPCs();
    
    initializeObservers();
    initializeVisitor();
}
GameManager::~GameManager() {
    stop();
    joinAll();
}

void GameManager::generateInitialNPCs() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> posDist(1.0, MAP_WIDTH - 1.0);
    std::uniform_int_distribution<> typeDist(0, 2);
    
    std::vector<std::string> types = {"Knight", "Orc", "Bear"};
    std::map<std::string, int> typeCount;
    
    for (int i = 0; i < INITIAL_NPC_COUNT; i++) {
        int typeIndex = typeDist(gen);
        std::string type = types[typeIndex];
        
        typeCount[type]++;
        std::string name = generateRandomName(type, typeCount[type]);
        
        double x = posDist(gen);
        double y = posDist(gen);
        
        auto npc = NPCFactory::createNPC(type, name, x, y);
        if (npc) {
            npcs.push_back(std::shared_ptr<NPC>(std::move(npc)));
        }
    }
    
    safePrint("Сгенерировано " + std::to_string(npcs.size()) + " NPC");
    safePrint("Распределение: " + 
              std::to_string(typeCount["Knight"]) + " рыцарей, " +
              std::to_string(typeCount["Orc"]) + " орков, " +
              std::to_string(typeCount["Bear"]) + " медведей");
}

std::string GameManager::generateRandomName(const std::string& type, int index) {
    if (type == "Knight") return "Рыцарь_" + std::to_string(index);
    if (type == "Orc") return "Орк_" + std::to_string(index);
    if (type == "Bear") return "Медведь_" + std::to_string(index);
    return "NPC_" + std::to_string(index);
}

void GameManager::start() {
    if (isRunning) return;
    
    isRunning = true;
    stopRequested = false;
    
    movementThread = std::thread(&GameManager::movementWorker, this);
    fightThread = std::thread(&GameManager::fightWorker, this);
    renderThread = std::thread(&GameManager::renderWorker, this);
    
    safePrint("Игра началась! Длительность: " + 
              std::to_string(GAME_DURATION_SECONDS) + " секунд");
}

void GameManager::stop() {
    stopRequested = true;
    isRunning = false;
    fightQueueCV.notify_all();
}

void GameManager::joinAll() {
    if (movementThread.joinable()) movementThread.join();
    if (fightThread.joinable()) fightThread.join();
    if (renderThread.joinable()) renderThread.join();
}

void GameManager::movementWorker() {
    safePrint("Поток движения запущен");
    while (!stopRequested) {
        {
            std::unique_lock lock(npcsMutex);
            for (auto& npc : npcs) {
                if (npc->isAlive()) {
                    npc->move(MAP_WIDTH, MAP_HEIGHT);
                }
            }
        }
        
        if (battleVisitor) {
            std::shared_lock lock(npcsMutex);
            for (auto& npc : npcs) {
                if (npc->isAlive()) {
                    battleVisitor->visit(*npc);
                }
            }
        }
        
        std::this_thread::sleep_for(100ms);
    }
    
    safePrint("Поток движения остановлен");
}

void GameManager::fightWorker() {
    safePrint("Поток боев запущен");
    
    while (!stopRequested || !fightQueue.empty()) {
        FightTask task;
        {
            std::unique_lock lock(fightQueueMutex);
            if (fightQueue.empty()) {
                fightQueueCV.wait_for(lock, 500ms, [this]() { return !fightQueue.empty() || stopRequested; });
                
                if (fightQueue.empty() && stopRequested) break;
                if (fightQueue.empty()) continue;
            }
            
            task = fightQueue.front();
            fightQueue.pop();
        }
        
        if (!task.attacker->isAlive() || !task.defender->isAlive()) {
            continue;
        }
        
        if (!task.attacker->isInKillingRange(*task.defender)) {
            continue;
        }
        
        {
            std::unique_lock lock(npcsMutex);
            task.attacker->fight(*task.defender);
            task.defender->fight(*task.attacker);
        }
        
        fightsProcessed++;
    }
    
    safePrint("Поток боев остановлен. Обработано боев: " + std::to_string(fightsProcessed));
}

void GameManager::renderWorker() {
    safePrint("Поток отрисовки запущен");
    
    auto startTime = std::chrono::steady_clock::now();
    while (!stopRequested) {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
        
        if (elapsed >= GAME_DURATION_SECONDS) {
            stop();
            break;
        }

        printMap();
        
        if (elapsed % 5 == 0) {
            int aliveCount = 0;
            {
                std::shared_lock lock(npcsMutex);
                for (const auto& npc : npcs) {
                    if (npc->isAlive()) aliveCount++;
                }
            }
            
            safePrint("Время: " + std::to_string(elapsed) + 
                     "с, Выжило: " + std::to_string(aliveCount) +
                     ", Боев: " + std::to_string(fightsProcessed));
        }
        
        std::this_thread::sleep_for(1s);
    }
    
    printMap();
    printSurvivors();
    safePrint("Игра завершена!");
}

void GameManager::safePrint(const std::string& message) {
    std::lock_guard lock(coutMutex);
    std::cout << "[Игра] " << message << std::endl;
}

void GameManager::printMap() const {
    const int CELL_SIZE = 10;
    const int COLS = static_cast<int>(MAP_WIDTH / CELL_SIZE);
    const int ROWS = static_cast<int>(MAP_HEIGHT / CELL_SIZE);
    
    std::lock_guard lock(coutMutex);
    
    std::cout << "\n=== КАРТА ПОДЗЕМЕЛЬЯ ===" << std::endl;
    std::cout << "Масштаб: 1 клетка = " << CELL_SIZE << "x" << CELL_SIZE << " метров" << std::endl;
    
    std::vector<std::vector<int>> grid(ROWS, std::vector<int>(COLS, 0));
    
    {
        std::shared_lock lock(npcsMutex);
        
        for (const auto& npc : npcs) {
            if (npc->isAlive()) {
                int col = static_cast<int>(npc->getX() / CELL_SIZE);
                int row = static_cast<int>(npc->getY() / CELL_SIZE);
                
                if (col >= 0 && col < COLS && row >= 0 && row < ROWS) {
                    grid[row][col]++;
                }
            }
        }
    }
    
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            if (grid[row][col] == 0) {
                std::cout << ". ";
            } else if (grid[row][col] < 10) {
                std::cout << grid[row][col] << " ";
            } else {
                std::cout << "* ";
            }
        }
        std::cout << std::endl;
    }
    
    std::cout << "\nЛегенда: . - пусто, цифра - количество NPC, * - много NPC" << std::endl;
}

void GameManager::printSurvivors() const {
    std::lock_guard lock(coutMutex);
    
    std::cout << "\n=== ВЫЖИВШИЕ NPC ===" << std::endl;
    
    std::map<std::string, int> survivorsByType;
    std::vector<std::shared_ptr<NPC>> aliveNPCs;
    
    {
        std::shared_lock lock(npcsMutex);
        
        for (const auto& npc : npcs) {
            if (npc->isAlive()) {
                survivorsByType[npc->getType()]++;
                aliveNPCs.push_back(npc);
            }
        }
    }
    
    if (aliveNPCs.empty()) {
        std::cout << "Никто не выжил!" << std::endl;
        return;
    }
    
    std::cout << "Всего выжило: " << aliveNPCs.size() << std::endl;
    std::cout << "По типам: " 
              << survivorsByType["Knight"] << " рыцарей, "
              << survivorsByType["Orc"] << " орков, "
              << survivorsByType["Bear"] << " медведей" << std::endl;
    
    std::cout << "\nСписок выживших:" << std::endl;
    for (const auto& npc : aliveNPCs) {
        std::cout << "- " << npc->getName() << " (" << npc->getType()
                  << ") в (" << std::fixed << std::setprecision(1)
                  << npc->getX() << ", " << npc->getY() << ")" << std::endl;
    }
}

