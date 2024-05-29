#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>

using namespace std;

// Класс, описывающий студента
struct Student {
    string FIO;
    int group;
    int marks[5];
    int grant;
    string phone;

    // Конструктор по умолчанию
    Student(const string& FIO, int group, int marks[5], int grant, const string& phone) : FIO(FIO), group(group), grant(grant), phone(phone) {
        for (int i = 0; i < 5; i++) {
            this->marks[i] = marks[i];
        }
    }

    // Возвращает строковое представление студента
    std::string to_string() { 
        string result = "FIO: " + FIO + ";\t";
        result += "Group: " + std::to_string(group) + ";\t";
        result += "Marks: [";
        for (int i = 0; i < 5; i++) {
            result += std::to_string(marks[i]) + (i == 4 ? "];\t" : ", ");
        }
        result += "Grant: " + std::to_string(grant) + ";\t";
        result += "Phone: " + phone;
        int cnt = 0, sum = 0;
        for (int i = 0; i < 5; i++) {
            sum += marks[i];
            cnt++;
        }
        result += ";\tAverage marks: " + std::to_string((double)sum / cnt);
        return result;
    }

    // Инициализирует студента из строки в формате CSV
    // CSV формат: FIO;group;mark1;mark2;mark3;mark4;mark5;grant;phone
    void from_csv_string(string csv) {
        csv += ";";
        int pos = 0;
        int nextPos = 0;
        int i = 0;
        std::cout << "from_csv_string: " << csv << std::endl;
        while ((nextPos = csv.find(';', pos)) != string::npos) {
            string value = csv.substr(pos, nextPos - pos);
            switch (i) {
                case 0:
                    FIO = value;
                    break;
                case 1:
                    group = stoi(value);
                    break;
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                    marks[i - 2] = stoi(value);
                    break;
                case 7:
                    grant = stoi(value);
                    break;
                case 8:
                    phone = value;
                    break;
            }
            pos = nextPos + 1;
            i++;
        }
    }

    // Возвращает строку в формате CSV
    // CSV формат: FIO;group;mark1;mark2;mark3;mark4;mark5;grant;phone
    // Пригодится для записи в файл
    string to_csv_string() {
        string result = FIO + ";" + std::to_string(group) + ";";
        for (int i = 0; i < 5; i++) {
            result += std::to_string(marks[i]) + (i == 4 ? ";" : ";");
        }
        result += std::to_string(grant) + ";" + phone;
        return result;
    }

};

// Класс, описывающий список студентов
class List {
public:
    // Структура, описывающая узел списка
    struct Node {
        Student data;
        Node* next;
        Node* prev;

        // Конструктор узла
        Node(const Student& data, Node* next = nullptr, Node* prev = nullptr) : data(data), next(next), prev(prev) {}
    };

    // Указатели на начало и конец списка
    Node* head;
    Node* tail;

    // Конструктор по умолчанию
    List() : head(nullptr), tail(nullptr) {}

    // Деструктор
    // Освобождает память, занимаемую узлами списка
    ~List() {
        clear();
    }

    // Добавляет студента в конец списка
    void push_back(const Student& data) {
        Node* newNode = new Node(data);

        // Если список пуст, то новый узел становится и началом и концом списка
        if (head == nullptr) {
            head = tail = newNode;
        } else {
            // Иначе добавляем узел в конец списка
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
    }

    // Добавляет студента перед указанным узлом
    // Пригоится для вставки студента перед студентом с оценками ниже среднего
    void insert_before(Node* node, const Student& data) {
        Node* newNode = new Node(data);

        if (!node) {
            return;
        }

        // Если узел первый, то новый узел становится началом списка
        if (node->prev == nullptr) {
            head = newNode;
        } else {
            // Иначе вставляем узел перед указанным узлом
            node->prev->next = newNode;
            newNode->prev = node->prev;
        }
        // И не забываем про связь с указанным узлом
        newNode->next = node;
        node->prev = newNode;
    }

    // Очищает список
    void clear() {
        Node* current = head;
        while (current != nullptr) {
            Node* next = current->next;
            delete current;
            current = next;
        }
        head = tail = nullptr;
    }

    // Выводит список на экран
    void print() {
        Node* current = head;
        while (current != nullptr) {
            cout << current->data.to_string() << endl;
            current = current->next;
        }
        cout << endl;
    }
};

// Структура, описывающая данные для хранения в хеш-таблице
// Содержит указатель на узел списка и сумму и количество оценок
// Замысел в том, чтобы хранить сумму и количество оценок для каждой группы
// Это позволит быстро находить среднюю оценку по группе
// Или быстро находить студентов по группе
// При таком подходе чтобы рассчитать среднюю оценку по всем группам не нужно перебирать всех студентов
// Нужно просто пройтись по всем группам и посчитать сумму и количество оценок
// Также удобно для удаления студентов по группе
struct map_data{
    List::Node* node;
    int sum_marks;
    int count_marks;
};

const int TABLE_SIZE = 100;

// Класс, описывающий хеш-таблицу
class Map {
public:
    // Структура, описывающая узел хеш-таблицы
    struct Node {
        size_t key;
        map_data data;
        Node* next;

        // Конструктор узла
        Node(int key, const map_data& data, Node* next = nullptr) : key(key), data(data), next(next) {}
    };

    // Массив указателей на узлы
    Node* nodes[TABLE_SIZE];

    // Функция хеширования
    size_t hash(size_t key) {
        return key % TABLE_SIZE;
    }

    // Конструктор по умолчанию
    Map () {
        for (int i = 0; i < TABLE_SIZE; i++) {
            nodes[i] = nullptr;
        }
    }

    // Деструктор
    ~Map() {
        clear();
    }

    // Очищает хеш-таблицу
    void clear() {
        for (int i = 0; i < TABLE_SIZE; i++) {
            Node* current = nodes[i];
            while (current != nullptr) {
                Node* next = current->next;
                delete current;
                current = next;
            }
            nodes[i] = nullptr;
        }
    }

    // Добавляет данные в хеш-таблицу
    void insert(size_t key, const map_data& data) {
        size_t index = hash(key);

        Node* newNode = new Node(key, data);
        newNode->next = nodes[index];
        nodes[index] = newNode;
    }

    // Ищет данные в хеш-таблице по ключу
    map_data* search(size_t key) {
        size_t index = hash(key);

        Node* current = nodes[index];
        while (current != nullptr) {
            if (current->key == key) {
                return &current->data;
            }
            current = current->next;
        }
        return nullptr;
    }

    // Удаляет данные из хеш-таблицы по ключу
    void remove(size_t key) {
        size_t index = hash(key);

        Node* current = nodes[index];
        Node* prev = nullptr;
        while (current != nullptr) {
            if (current->key == key) {
                if (prev == nullptr) {
                    nodes[index] = current->next;
                } else {
                    prev->next = current->next;
                }
                delete current;
                return;
            }
            prev = current;
            current = current->next;
        }
    }
};

// Класс, описывающий список студентов
// Содержит список студентов и хеш-таблицу для быстрого доступа к студентам по группе
class StudentList {
private:
    List students;
    Map studentsMap;

public:
    // Добавляет студента в список
    void addStudent(const Student& student) {
        // Ищем данные по группе в хеш-таблице
        map_data* data = studentsMap.search(student.group);
        // Если данных нет, то создаем новые
        if (data == nullptr) {
            map_data data;
            data.node = nullptr;
            data.sum_marks = 0;
            data.count_marks = 0;
            for (int i = 0; i < 5; i++) {
                data.sum_marks += student.marks[i];
                data.count_marks++;
            }
            students.push_back(student);
            data.node = students.tail; // Не забываем указать на последний узел списка
            studentsMap.insert(student.group, data);

        } else {
            // Если данные есть, то просто добавляем оценки к сумме и увеличиваем количество оценок
            for (int i = 0; i < 5; i++) {
                data->sum_marks += student.marks[i];
                data->count_marks++;
            }
            // И добавляем студента в список перед указанным узлом
            students.insert_before(data->node, student);
            // Таким образом мы сохраняем порядок студентов в списке, так, чтобы студенты с одной группой были вместе
        }
    }

    // Выводит список студентов на экран
    void print() {
        students.print();
    }

    // Выводит студентов по группе на экран
    void printGroup(int group) {
        map_data* data = studentsMap.search(group);
        if (data != nullptr) {
            List::Node* current = data->node;
            while (current != nullptr && current->data.group == group) {
                cout << current->data.to_string() << endl;
                current = current->prev;
            }
        }
    }

    // Выводит средние оценки по группам на экран
    void printAverageMarks() {
        for (int i = 0; i < TABLE_SIZE; i++) {
            Map::Node* current = studentsMap.nodes[i];
            while (current != nullptr) {
                cout << "Group: " << current->key << ";\t";
                cout << "Average marks: " << (double)current->data.sum_marks / current->data.count_marks << endl;
                current = current->next;
            }
        }
    }

    // Очищает список студентов
    void clear() {
        students.clear();
        studentsMap.clear();
    }

    // Инициализирует список студентов из строки в формате CSV
    void from_csv_string(const string& csv) {
        int pos = 0;
        int nextPos = 0;
        while ((nextPos = csv.find('\n', pos)) != string::npos) {
            string value = csv.substr(pos, nextPos - pos);
            int marks[5]={};
            Student student("", 0, marks, 0, "");
            student.from_csv_string(value);
            addStudent(student);
            pos = nextPos + 1;
        }
    }

    // Возвращает строку в формате CSV
    string to_csv_string() {
        string result = "";
        List::Node* current = students.head;
        while (current != nullptr) {
            result += current->data.to_csv_string() + "\n";
            current = current->next;
        }
        return result;
    }

    // Сохраняет список студентов в файл
    void save(const string& path) {
        ofstream file(path);
        file << to_csv_string();
        file.close();
    }

    // Загружает список студентов из файла
    void load(const string& path) {
        ifstream file(path);
        string data;
        // перейдём в конец файла
        file.seekg(0, ios::end);
        // зарезервируем память
        data.reserve(file.tellg());
        // вернёмся в начало файла
        file.seekg(0, ios::beg);
        // считаем данные из файла в строку
        data.assign((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        // инициализируем список студентов из строки
        from_csv_string(data);
        // закрываем файл
        file.close();
    }

    // Удаляет студента по ФИО
    void removeStudent(const string& FIO) {
        List::Node* current = students.head;
        while (current != nullptr) {
            if (current->data.FIO == FIO) {
                map_data* data = studentsMap.search(current->data.group);
                for (int i = 0; i < 5; i++) {
                    data->sum_marks -= current->data.marks[i];
                    data->count_marks--;
                }
                if (current->prev == nullptr) {
                    students.head = current->next;
                } else {
                    current->prev->next = current->next;
                }
                if (current->next == nullptr) {
                    students.tail = current->prev;
                } else {
                    current->next->prev = current->prev;
                }
                // Если удаляемый студент был указателем на последний узел группы, то указатель нужно сдвинуть на предыдущий узел
                if (data->node == current) {
                    data->node = current->prev;
                    // Если удаляемый студент был последним в группе, то удаляем данные по группе из хеш-таблицы
                    if (data->node == nullptr || data->node->data.group != current->data.group) {
                        studentsMap.remove(current->data.group);
                    }
                }
                delete current;
                break;
            }
            current = current->next;
        }
    }

    // Удаляет студентов по группе
    void removeGroup(int group) {
        map_data* data = studentsMap.search(group);
        if (data != nullptr) {
            List::Node* current = data->node;
            while (current != nullptr && current->data.group == group) {
                List::Node* next = current->prev;
                if (current->prev == nullptr) {
                    students.head = current->next;
                } else {
                    current->prev->next = current->next;
                }
                if (current->next == nullptr) {
                    students.tail = current->prev;
                } else {
                    current->next->prev = current->prev;
                }
                delete current;
                current = next;
            }
            // Удаляем данные по группе из хеш-таблицы
            studentsMap.remove(group);
        }
    }

    // Выводит студентов с оценками ниже среднего по алфавиту
    void resolveTask(){
        int count_marks = 0;
        double sum_marks = 0;
        for (int i = 0; i < TABLE_SIZE; i++) {
            Map::Node* current = studentsMap.nodes[i];
            while (current != nullptr) {
                sum_marks += current->data.sum_marks;
                count_marks += current->data.count_marks;
                current = current->next;
            }
        }
        double average = sum_marks / count_marks; // Средняя оценка по всем студентам
        // Стоит отметить, что это среднее - среднее среди всех оценок всех студентов

        // Создаем список студентов с оценками ниже среднего
        List students_below_average;
        List::Node* current = students.head;
        while (current != nullptr) {
            double student_average = 0;
            for (int i = 0; i < 5; i++) {
                student_average += current->data.marks[i];
            }
            student_average /= 5;
            if (student_average < average) { // Если средняя оценка студента ниже средней оценки по всем студентам
                List::Node* c = students_below_average.head; // Ищем место для вставки студента в список
                while (c != nullptr && c->data.FIO < current->data.FIO) { // Оператор < определен для строк, так что сравнение происходит по алфавиту
                    c = c->next;
                }
                if (c == nullptr) {
                    students_below_average.push_back(current->data);
                } else {
                    students_below_average.insert_before(c, current->data);
                }
            }
            current = current->next; // Переходим к следующему студенту
        }
        students_below_average.print(); // Выводим студентов на экран

    }
};

// Функция для генерации случайных студентов
void generate_random_students(StudentList& list, int count = 20) {
    for (int i = 0; i < count; i++) {
        int marks[5];
        for (int j = 0; j < 5; j++) {
            marks[j] = rand() % 5 + 1;
        }
        std::string FIO = "";
        for (int j = 0; j < 5; j++) {
            FIO += (char)(rand() % (3) + 'a'); // Простенькие имена, чтобы было видно, что студенты по алфавиту
        }
        list.addStudent(Student(FIO, rand() % 5 + 1, marks, rand() % 1000, "8-800-555-35-35"));
    }
}

int main() {
    StudentList list;

    do {
        std::cout << "Program menu:" << std::endl;
        std::cout << "1. Add student" << std::endl;
        std::cout << "2. Print all students" << std::endl;
        std::cout << "3. Print students by group" << std::endl;
        std::cout << "4. Print average marks by group" << std::endl;
        std::cout << "5. Save to file" << std::endl;
        std::cout << "6. Load from file" << std::endl;
        std::cout << "7. Remove student" << std::endl;
        std::cout << "8. Remove group" << std::endl;
        std::cout << "9. Alphabeticly output students with marks below average" << std::endl;
        std::cout << "10. Generate random students" << std::endl;
        std::cout << "11. Exit" << std::endl;

        int choice;
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice) {
            case 1: {
                std::string FIO;
                int group;
                int marks[5];
                int grant;
                std::string phone;

                std::cout << "Enter FIO: ";
                std::cin >> FIO;
                std::cout << "Enter group: ";
                std::cin >> group;
                std::cout << "Enter marks: ";
                for (int i = 0; i < 5; i++) {
                    std::cin >> marks[i];
                }
                std::cout << "Enter grant: ";
                std::cin >> grant;
                std::cout << "Enter phone: ";
                std::cin >> phone;

                list.addStudent(Student(FIO, group, marks, grant, phone));
                break;
            }
            case 2: {
                list.print();
                break;
            }
            case 3: {
                int group;
                std::cout << "Enter group: ";
                std::cin >> group;
                list.printGroup(group);
                break;
            }
            case 4: {
                list.printAverageMarks();
                break;
            }
            case 5: {
                std::string path;
                std::cout << "Enter path: ";
                std::cin >> path;
                list.save(path);
                break;
            }
            case 6: {
                std::string path;
                std::cout << "Enter path: ";
                std::cin >> path;
                list.load(path);
                break;
            }
            case 7: {
                std::string FIO;
                std::cout << "Enter FIO: ";
                std::cin >> FIO;
                list.removeStudent(FIO);
                break;
            }
            case 8: {
                int group;
                std::cout << "Enter group: ";
                std::cin >> group;
                list.removeGroup(group);
                break;
            }
            case 9: {
                list.resolveTask();
                break;
            }
            case 10: {
                int count;
                std::cout << "Enter count: ";
                std::cin >> count;
                generate_random_students(list, count);
                break;
            }
            case 11: {
                return 0;
            }
            default: {
                std::cout << "Invalid choice" << std::endl;
                break;
            }
        }
    } while (true);

    return 0;
}

