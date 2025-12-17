#include <iostream>
#include "game_manager.h"

int main() {
    std::cout << "=== МНОГОПОТОЧНАЯ RPG BALAGUR FATE 3 ===" << std::endl;
    std::cout << "Используемые паттерны:" << std::endl;
    std::cout << "1. Factory - создание NPC разных типов" << std::endl;
    std::cout << "2. Visitor - обнаружение и обработка боев" << std::endl;
    std::cout << "3. Observer - логирование событий в консоль и файл" << std::endl;
    std::cout << "=====================================================" << std::endl;
    
    std::cout << "\nПараметры игры:" << std::endl;
    std::cout << "- Карта: 100x100 метров" << std::endl;
    std::cout << "- Начальное количество NPC: 50" << std::endl;
    std::cout << "- Длительность игры: 30 секунд" << std::endl;
    std::cout << "- Лог файл: battle_log.txt" << std::endl;
    
    try {
        GameManager game;
        
        std::cout << "\nНажмите Enter для начала игры...";
        std::cin.get();
        
        game.start();
        game.joinAll();
        
        std::cout << "\nИгра завершена. Результаты сохранены в логах." << std::endl;
        std::cout << "Нажмите Enter для выхода...";
        std::cin.get();
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}