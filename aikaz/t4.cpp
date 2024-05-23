#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <map>
#include <vector>

// Структура для хранения информации о городе
class City {
public:
    std::string name;         // Название города
    std::string region;      // Название региона
    int population;       // Количество жителей

    // Конструктор для удобства создания узлов
    City(const std::string& cityName, const std::string& cityRegion, int cityPopulation) :
        name(cityName), region(cityRegion), population(cityPopulation) {}
};

// Структура для узла двусвязного списка
class CityNode {
public:
    City* data;             // Указатель на данные о городе
    CityNode* prev;         // Указатель на предыдущий узел
    CityNode* next;         // Указатель на следующий узел

    // Конструктор для создания узла с данными
    CityNode(City* cityData) : data(cityData), prev(nullptr), next(nullptr) {}
};

// Класс для управления двусвязным списком городов
class CityList {
private:
    CityNode* head;         // Указатель на начало списка
    CityNode* tail;         // Указатель на конец списка

public:
    CityList() : head(nullptr), tail(nullptr) {}  // Конструктор

    // Деструктор для очистки памяти
    ~CityList() {
        clear();
    }

    // Добавление города в конец списка
    void addCity(const std::string& cityName, const std::string& cityRegion, int cityPopulation) {
        City* newCity = new City(cityName, cityRegion, cityPopulation);
        CityNode* newNode = new CityNode(newCity);

        if (tail == nullptr) {
            // Список пуст
            head = tail = newNode;
        } else {
            // Добавление в конец непустого списка
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
    }

    // Вывод списка городов на экран
    void printList() {
        CityNode* current = head;
        while (current != nullptr) {
            std::cout << "City: " << current->data->name << ", Region: " << current->data->region
                << ", Population: " << current->data->population << std::endl;
            current = current->next;
        }
    }

    // Вывод регионов в порядке убывания численности населения
    void printRegionsByPopulation() {
        std::map<std::string, int> regionPopulation;

        // Подсчет населения по регионам
        CityNode* current = head;
        while (current != nullptr) {
            regionPopulation[current->data->region] += current->data->population;
            current = current->next;
        }
        std::vector<std::pair<std::string, int>> sortedRegions(regionPopulation.begin(), regionPopulation.end());

        // Сортировка регионов по численности населения
        std::sort(sortedRegions.begin(), sortedRegions.end(),
            [](auto a, auto b) {
                return a.second > b.second;
            });
        
        // Вывод регионов
        for (const auto& region : sortedRegions) {
            std::cout << "Region: " << region.first << ", Population: " << region.second << std::endl;
        }

    }

    // Удаление городов заданного региона
    void deleteCitiesByRegion(const std::string& targetRegion) {
        CityNode* current = head;

        while (current != nullptr) {
            if (current->data->region == targetRegion) {
                // Удаление узла
                CityNode* toDelete = current;

                if (current->prev != nullptr) {
                    current->prev->next = current->next;
                } else {
                    // Удаляемый узел - head
                    head = current->next;
                }

                if (current->next != nullptr) {
                    current->next->prev = current->prev;
                } else {
                    // Удаляемый узел - tail
                    tail = current->prev;
                }

                current = current->next;  // Переход к следующему узлу перед удалением
                delete toDelete->data;    // Освобождение памяти данных города
                delete toDelete;           // Освобождение памяти узла
            } else {
                current = current->next;
            }
        }
    }

    // Очистка списка и освобождение памяти
    void clear() {
        CityNode* current = head;
        while (current != nullptr) {
            CityNode* next = current->next;
            delete current->data;
            delete current;
            current = next;
        }
        head = tail = nullptr;
    }

    // Загрузка данных из CSV файла
    void loadFromCSV(const std::string& filename) {
        std::ifstream file(filename);

        if (file.is_open()) {
            std::string line, cityName, region, populationStr;

            while (std::getline(file, line)) {
                // Разбиение строки на части по разделителю ';'
                size_t pos = 0;
                pos = line.find(';');
                cityName = line.substr(0, pos);
                line.erase(0, pos + 1);

                pos = line.find(';');
                region = line.substr(0, pos);
                line.erase(0, pos + 1);

                populationStr = line;

                // Конвертация населения в число
                int population = std::stoi(populationStr);

                // Добавление города в список
                addCity(cityName, region, population);
            }
            file.close();
            std::cout << "Data loaded from CSV file successfully.\n";
        } else {
            std::cerr << "Error: Unable to open file: " << filename << std::endl;
        }
    }
};

int main() {
    CityList cityList;
    int choice;
    std::string cityName, region, filename;
    int population;

    do {
        std::cout << "\nMenu:\n";
        std::cout << "1. Add city from keyboard\n";
        std::cout << "2. Load cities from CSV file\n";
        std::cout << "3. Print cities\n";
        std::cout << "4. Print regions by population\n";
        std::cout << "5. Delete cities by region\n";
        std::cout << "6. Clear list\n";
        std::cout << "0. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice) {
        case 1:
            std::cout << "Enter city name: ";
            std::cin.ignore(); // Очистка буфера ввода
            std::getline(std::cin, cityName);
            std::cout << "Enter region: ";
            std::getline(std::cin, region);
            std::cout << "Enter population: ";
            std::cin >> population;
            cityList.addCity(cityName, region, population);
            break;
        case 2:
            std::cout << "Enter CSV filename: ";
            std::cin.ignore();
            std::getline(std::cin, filename);
            cityList.loadFromCSV(filename);
            break;
        case 3:
            cityList.printList();
            break;
        case 4:
            cityList.printRegionsByPopulation();
            break;
        case 5:
            std::cout << "Enter region to delete cities from: ";
            std::cin.ignore();
            std::getline(std::cin, region);
            cityList.deleteCitiesByRegion(region);
            break;
        case 6:
            cityList.clear();
            std::cout << "List cleared.\n";
            break;
        case 0:
            std::cout << "Exiting...\n";
            break;
        default:
            std::cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 0);

    return 0;
}