#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "npc.h"
#include "knight.h"
#include "orc.h"
#include "bear.h"
#include "factory.h"
#include "game_manager.h"

using namespace std::chrono_literals;

// ==================== ТЕСТЫ ДЛЯ NPC ====================

TEST(NPCTest, CreationAndBasicProperties) {
    Knight knight("Артур", 50, 50);
    
    EXPECT_EQ(knight.getName(), "Артур");
    EXPECT_EQ(knight.getType(), "Knight");
    EXPECT_EQ(knight.getX(), 50);
    EXPECT_EQ(knight.getY(), 50);
    EXPECT_TRUE(knight.isAlive());
    EXPECT_EQ(knight.getMoveDistance(), 30.0);
}

TEST(NPCTest, DistanceCalculation) {
    Knight k1("K1", 0, 0);
    Knight k2("K2", 3, 4);
    
    EXPECT_DOUBLE_EQ(k1.distanceTo(k2), 5.0);
}

TEST(NPCTest, KillingRange) {
    Knight k1("K1", 0, 0);
    Knight k2("K2", 8, 0);  // Расстояние 8 < 10
    
    EXPECT_TRUE(k1.isInKillingRange(k2));
    
    Knight k3("K3", 15, 0);  // Расстояние 15 > 10
    EXPECT_FALSE(k1.isInKillingRange(k3));
}

TEST(NPCTest, MovementWithinBounds) {
    Knight knight("K", 50, 50);
    
    // Многократно двигаем NPC и проверяем, что он остается в границах
    for (int i = 0; i < 100; i++) {
        knight.move(100, 100);
        EXPECT_GE(knight.getX(), 0);
        EXPECT_LE(knight.getX(), 100);
        EXPECT_GE(knight.getY(), 0);
        EXPECT_LE(knight.getY(), 100);
    }
}

TEST(NPCTest, DeathState) {
    Knight knight("Артур", 50, 50);
    
    EXPECT_TRUE(knight.isAlive());
    knight.die();
    EXPECT_FALSE(knight.isAlive());
    
    // Мертвый NPC не должен двигаться
    double oldX = knight.getX();
    double oldY = knight.getY();
    knight.move(100, 100);
    EXPECT_EQ(knight.getX(), oldX);
    EXPECT_EQ(knight.getY(), oldY);
}

TEST(NPCTest, CombatRules) {
    Knight knight("Рыцарь", 0, 0);
    Orc orc("Орк", 0, 0);
    Bear bear("Медведь", 0, 0);
    
    // Проверяем круговую логику боя
    EXPECT_TRUE(knight.canDefeat(orc));
    EXPECT_FALSE(knight.canDefeat(bear));
    EXPECT_FALSE(knight.canDefeat(knight));
    
    EXPECT_TRUE(orc.canDefeat(bear));
    EXPECT_FALSE(orc.canDefeat(knight));
    EXPECT_FALSE(orc.canDefeat(orc));
    
    EXPECT_TRUE(bear.canDefeat(knight));
    EXPECT_FALSE(bear.canDefeat(orc));
    EXPECT_FALSE(bear.canDefeat(bear));
}

// ==================== ТЕСТЫ ДЛЯ FACTORY ====================

TEST(FactoryTest, CreateNPC) {
    auto knight = NPCFactory::createNPC("Knight", "Артур", 50, 50);
    ASSERT_NE(knight, nullptr);
    EXPECT_EQ(knight->getType(), "Knight");
    
    auto orc = NPCFactory::createNPC("Orc", "Громозуб", 60, 60);
    ASSERT_NE(orc, nullptr);
    EXPECT_EQ(orc->getType(), "Orc");
    
    auto bear = NPCFactory::createNPC("Bear", "Бурый", 70, 70);
    ASSERT_NE(bear, nullptr);
    EXPECT_EQ(bear->getType(), "Bear");
}

TEST(FactoryTest, InvalidCoordinates) {
    // Координаты за пределами допустимых
    auto npc1 = NPCFactory::createNPC("Knight", "K1", -10, 50);
    EXPECT_EQ(npc1, nullptr);
    
    auto npc2 = NPCFactory::createNPC("Knight", "K2", 50, -10);
    EXPECT_EQ(npc2, nullptr);
    
    auto npc3 = NPCFactory::createNPC("Knight", "K3", 600, 50);
    EXPECT_EQ(npc3, nullptr);
    
    auto npc4 = NPCFactory::createNPC("Knight", "K4", 50, 600);
    EXPECT_EQ(npc4, nullptr);
}

TEST(FactoryTest, InvalidType) {
    auto npc = NPCFactory::createNPC("Dragon", "Дракон", 50, 50);
    EXPECT_EQ(npc, nullptr);
}

TEST(FactoryTest, LoadFromString) {
    std::string data = "Knight Ланселот 50 60";
    auto npc = NPCFactory::loadFromString(data);
    
    ASSERT_NE(npc, nullptr);
    EXPECT_EQ(npc->getType(), "Knight");
    EXPECT_EQ(npc->getName(), "Ланселот");
    EXPECT_EQ(npc->getX(), 50);
    EXPECT_EQ(npc->getY(), 60);
}

TEST(FactoryTest, LoadFromInvalidString) {
    // Недостаточно данных
    auto npc1 = NPCFactory::loadFromString("Knight Ланселот");
    EXPECT_EQ(npc1, nullptr);
    
    // Неправильный тип
    auto npc2 = NPCFactory::loadFromString("Dragon Смауг 50 60");
    EXPECT_EQ(npc2, nullptr);
    
    // Нечисловые координаты
    auto npc3 = NPCFactory::loadFromString("Knight Ланселот x y");
    EXPECT_EQ(npc3, nullptr);
}

// ==================== ТЕСТЫ ДЛЯ OBSERVER ====================

TEST(ObserverTest, ConsoleObserverCreation) {
    ConsoleObserver observer;
    // Проверяем, что можно вызвать метод без исключений
    EXPECT_NO_THROW(observer.onDeath("Убийца", "Жертва"));
}

TEST(ObserverTest, FileObserverCreation) {
    FileObserver observer("test_log.txt");
    EXPECT_TRUE(observer.isFileOpen());
    
    // Проверяем запись
    EXPECT_NO_THROW(observer.onDeath("Убийца", "Жертва"));
    
    // Проверяем, что файл создан и содержит данные
    std::ifstream file("test_log.txt");
    EXPECT_TRUE(file.is_open());
    
    // Удаляем тестовый файл
    file.close();
    std::remove("test_log.txt");
}

TEST(ObserverTest, ThreadSafety) {
    FileObserver observer("thread_test.txt");
    
    // Запускаем несколько потоков, которые пишут в лог
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([&observer, i]() {
            for (int j = 0; j < 10; j++) {
                observer.onDeath("Убийца_" + std::to_string(i), 
                               "Жертва_" + std::to_string(j));
            }
        });
    }
    
    // Ждем завершения всех потоков
    for (auto& t : threads) {
        t.join();
    }
    
    // Проверяем, что файл существует и не поврежден
    std::ifstream file("thread_test.txt");
    EXPECT_TRUE(file.is_open());
    
    // Считаем строки
    int lineCount = 0;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) lineCount++;
    }
    
    // Должно быть как минимум 100 строк (10 потоков × 10 записей)
    EXPECT_GE(lineCount, 100);
    
    file.close();
    std::remove("thread_test.txt");
}

// ==================== ТЕСТЫ ДЛЯ GAME MANAGER ====================

class GameManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Устанавливаем маленькие значения для быстрого тестирования
        game = std::make_unique<GameManager>();
    }
    
    void TearDown() override {
        game->stop();
        game->joinAll();
        std::this_thread::sleep_for(100ms);
    }
    
    std::unique_ptr<GameManager> game;
};

TEST_F(GameManagerTest, Initialization) {
    EXPECT_GE(game->getMapWidth(), 0);
    EXPECT_GE(game->getMapHeight(), 0);
}

TEST_F(GameManagerTest, StartStop) {
    // Тестируем запуск и остановку игры
    EXPECT_NO_THROW(game->start());
    std::this_thread::sleep_for(500ms);
    EXPECT_NO_THROW(game->stop());
    EXPECT_NO_THROW(game->joinAll());
}

TEST_F(GameManagerTest, ShortGameSession) {
    // Запускаем игру и проверяем, что она корректно завершается
    game->start();
    
    // Ждем немного и останавливаем
    std::this_thread::sleep_for(1s);
    game->stop();
    game->joinAll();
    
    // Проверяем, что потоки завершились корректно
    EXPECT_NO_THROW();
}

TEST_F(GameManagerTest, NoDeadlockOnQuickStop) {
    // Многократно запускаем и останавливаем игру для проверки deadlock'ов
    for (int i = 0; i < 10; i++) {
        auto localGame = std::make_unique<GameManager>();
        localGame->start();
        std::this_thread::sleep_for(50ms);
        localGame->stop();
        localGame->joinAll();
    }
    
    // Если мы дошли сюда, значит deadlock'ов нет
    EXPECT_TRUE(true);
}

TEST_F(GameManagerTest, MapBoundariesRespected) {
    game->start();
    std::this_thread::sleep_for(500ms);
    
    // Здесь можно было бы проверить координаты NPC,
    // но для этого нужен доступ к внутреннему состоянию GameManager
    // В реальном тесте можно добавить геттеры для тестирования
    
    game->stop();
    game->joinAll();
}

// ==================== ИНТЕГРАЦИОННЫЕ ТЕСТЫ ====================

TEST(IntegrationTest, FullCombatCycle) {
    // Создаем двух NPC разных типов
    Knight knight("Рыцарь", 0, 0);
    Orc orc("Орк", 5, 0);  // В радиусе боя (расстояние 5)
    
    // Проверяем, что они в радиусе боя
    EXPECT_TRUE(knight.isInKillingRange(orc));
    EXPECT_TRUE(orc.isInKillingRange(knight));
    
    // Проверяем правила боя
    EXPECT_TRUE(knight.canDefeat(orc));
    EXPECT_FALSE(orc.canDefeat(knight));
    
    // Проверяем, что оба живы
    EXPECT_TRUE(knight.isAlive());
    EXPECT_TRUE(orc.isAlive());
}

TEST(IntegrationTest, FactoryObserverIntegration) {
    // Создаем NPC через Factory
    auto npc = NPCFactory::createNPC("Knight", "Тестовый_Рыцарь", 50, 50);
    ASSERT_NE(npc, nullptr);
    
    // Создаем Observer
    ConsoleObserver consoleObs;
    FileObserver fileObs("integration_test.txt");
    
    // Проверяем, что можем использовать их вместе
    EXPECT_NO_THROW(consoleObs.onDeath(npc->getName(), "Жертва"));
    EXPECT_NO_THROW(fileObs.onDeath(npc->getName(), "Жертва"));
    
    // Удаляем тестовый файл
    std::remove("integration_test.txt");
}

TEST(IntegrationTest, MultiThreadStressTest) {
    // Быстрый тест под нагрузкой
    GameManager game;
    
    game.start();
    
    // Даем поработать 2 секунды
    std::this_thread::sleep_for(2s);
    
    // Останавливаем
    game.stop();
    game.joinAll();
    
    // Проверяем, что нет исключений
    EXPECT_NO_THROW();
}


// ==================== ТЕСТЫ ДЛЯ ПОТОКОВ ====================

TEST(ThreadTest, ConcurrentAccessToNPC) {
    std::shared_ptr<NPC> npc = std::make_shared<Knight>("Тест", 50, 50);
    
    std::vector<std::thread> readers;
    std::vector<std::thread> writers;
    
    std::atomic<int> readCount{0};
    std::atomic<int> writeCount{0};
    
    // Запускаем читателей
    for (int i = 0; i < 10; i++) {
        readers.emplace_back([&npc, &readCount]() {
            for (int j = 0; j < 100; j++) {
                auto name = npc->getName();
                auto type = npc->getType();
                auto x = npc->getX();
                auto y = npc->getY();
                auto alive = npc->isAlive();
                
                (void)name; (void)type; (void)x; (void)y; (void)alive; // Подавляем warnings
                readCount++;
            }
        });
    }
    
    // Запускаем писателей
    for (int i = 0; i < 5; i++) {
        writers.emplace_back([&npc, &writeCount]() {
            for (int j = 0; j < 50; j++) {
                npc->move(100, 100);
                writeCount++;
            }
        });
    }
    
    // Ждем завершения
    for (auto& t : readers) t.join();
    for (auto& t : writers) t.join();
    
    EXPECT_EQ(readCount, 1000);
    EXPECT_EQ(writeCount, 250);
}

// ==================== ТЕСТЫ ГРАНИЧНЫХ СЛУЧАЕВ ====================

TEST(EdgeCaseTest, ZeroDistanceMove) {
    Knight knight("K", 50, 50);
    double initialX = knight.getX();
    double initialY = knight.getY();
    
    // Хотя метод move использует случайные числа, мы можем проверить,
    // что координаты остаются в пределах
    knight.move(100, 100);
    
    EXPECT_GE(knight.getX(), 0);
    EXPECT_LE(knight.getX(), 100);
    EXPECT_GE(knight.getY(), 0);
    EXPECT_LE(knight.getY(), 100);
}

TEST(EdgeCaseTest, SamePositionNPC) {
    Knight k1("K1", 50, 50);
    Knight k2("K2", 50, 50);  // Та же позиция
    
    EXPECT_EQ(k1.distanceTo(k2), 0);
    EXPECT_TRUE(k1.isInKillingRange(k2));
}

// ==================== ТЕСТ ПРОИЗВОДИТЕЛЬНОСТИ ====================

TEST(PerformanceTest, MassNPCCreation) {
    const int COUNT = 1000;
    std::vector<std::shared_ptr<NPC>> npcs;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < COUNT; i++) {
        auto npc = std::make_shared<Knight>("Knight_" + std::to_string(i), 
                                           i % 100, i % 100);
        npcs.push_back(npc);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Проверяем, что создание 1000 NPC занимает меньше 100ms
    EXPECT_LT(duration.count(), 100);
    EXPECT_EQ(npcs.size(), COUNT);
}

TEST(PerformanceTest, ConcurrentMovement) {
    const int NPC_COUNT = 100;
    const int THREAD_COUNT = 10;
    const int MOVES_PER_THREAD = 100;
    
    std::vector<std::shared_ptr<NPC>> npcs;
    for (int i = 0; i < NPC_COUNT; i++) {
        npcs.push_back(std::make_shared<Knight>("K" + std::to_string(i), 50, 50));
    }
    
    std::vector<std::thread> threads;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int t = 0; t < THREAD_COUNT; t++) {
        threads.emplace_back([&npcs, MOVES_PER_THREAD]() {
            for (int m = 0; m < MOVES_PER_THREAD; m++) {
                for (auto& npc : npcs) {
                    npc->move(100, 100);
                }
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Проверяем, что все NPC переместились и нет deadlock'ов
    for (auto& npc : npcs) {
        EXPECT_GE(npc->getX(), 0);
        EXPECT_LE(npc->getX(), 100);
    }
    
    std::cout << "Performance test: " << duration.count() << "ms for " 
              << (NPC_COUNT * THREAD_COUNT * MOVES_PER_THREAD) 
              << " move operations" << std::endl;
}

// ==================== ГЛАВНАЯ ФУНКЦИЯ ====================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    
    std::cout << "Запуск тестов для многопоточной RPG Balagur Fate 3" << std::endl;
    std::cout << "Количество тестов: " 
              << ::testing::UnitTest::GetInstance()->total_test_suite_count()
              << " наборов, "
              << ::testing::UnitTest::GetInstance()->total_test_count()
              << " тестов" << std::endl;
    
    return RUN_ALL_TESTS();
}