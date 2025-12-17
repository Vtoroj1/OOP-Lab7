#include "factory.h"
#include <iostream>

std::unique_ptr<NPC> NPCFactory::createNPC(const std::string& type, const std::string& name, double x, double y) {
    if (x <= 0 || x > 500 || y <= 0 || y > 500) {
        std::cerr << "Ошибка создания NPC: некорректные координаты " << x << ", " << y << " для " << name << std::endl;
        return nullptr;
    }
    
    if (type == "Knight") return std::make_unique<Knight>(name, x, y);
    if (type == "Orc") return std::make_unique<Orc>(name, x, y);
    if (type == "Bear") return std::make_unique<Bear>(name, x, y);
    
    std::cerr << "Ошибка создания NPC: неизвестный тип " << type << std::endl;
    return nullptr;
}

std::unique_ptr<NPC> NPCFactory::loadFromString(const std::string& data) {
    std::stringstream ss(data);
    std::string type, name;
    double x, y;
    
    if (!(ss >> type >> name >> x >> y)) {
        std::cerr << "Ошибка парсинга строки: " << data << std::endl;
        return nullptr;
    }
    
    return createNPC(type, name, x, y);
}

