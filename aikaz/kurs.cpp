#include <iostream>
#include <string>

using namespace std;

struct Student {
    string FIO;
    int group;
    int marks[5];
    int grant;
    string phone;

    Student(const string& FIO, int group, int marks[5], int grant, const string& phone) : FIO(FIO), group(group), grant(grant), phone(phone) {
        for (int i = 0; i < 5; i++) {
            this->marks[i] = marks[i];
        }
    }

    string to_string() {
        string result = "FIO: " + FIO + ";\t";
        result += "Group: " + std::to_string(group) + ";\t";
        result += "Marks: []";
        for (int i = 0; i < 5; i++) {
            result += std::to_string(marks[i]) + (i == 4 ? "" : ", ");
        }
        result += "];\t";
        result += "Grant: " + std::to_string(grant) + ";\t";
        result += "Phone: " + phone + "\t";
        return result;
    }

    void from_csv_string(const string& csv) {
        int pos = 0;
        int nextPos = 0;
        int i = 0;
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
                    marks[0] = stoi(value);
                    break;
                case 3:
                    marks[1] = stoi(value);
                    break;
                case 4:
                    marks[2] = stoi(value);
                    break;
                case 5:
                    marks[3] = stoi(value);
                    break;
                case 6:
                    marks[4] = stoi(value);
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

    string to_csv_string() {
        string result = FIO + ";" + std::to_string(group) + ";";
        for (int i = 0; i < 5; i++) {
            result += std::to_string(marks[i]) + (i == 4 ? ";" : ";");
        }
        result += std::to_string(grant) + ";" + phone;
        return result;
    }

};

class List {
public:
    struct Node {
        Student data;
        Node* next;
        Node* prev;

        Node(const Student& data, Node* next = nullptr, Node* prev = nullptr) : data(data), next(next), prev(prev) {}
    };

    Node* head;
    Node* tail;

    List() : head(nullptr), tail(nullptr) {}

    ~List() {
        clear();
    }

    void push_back(const Student& data) {
        Node* newNode = new Node(data);

        if (head == nullptr) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
    }

    void clear() {
        Node* current = head;
        while (current != nullptr) {
            Node* next = current->next;
            delete current;
            current = next;
        }
        head = tail = nullptr;
    }

    void print() {
        Node* current = head;
        while (current != nullptr) {
            cout << current->data.to_string() << endl;
            current = current->next;
        }
        cout << endl;
    }
};

struct map_data{
    List::Node* node;
    int sum_marks;
    int count_marks;
};

const int TABLE_SIZE = 100;

class Map {
public:
    struct Node {
        size_t key;
        map_data data;
        Node* next;

        Node(int key, const map_data& data, Node* next = nullptr) : key(key), data(data), next(next) {}
    };

    Node* nodes[TABLE_SIZE];

    size_t hash(size_t key) {
        return key % TABLE_SIZE;
    }

    Map () {
        for (int i = 0; i < TABLE_SIZE; i++) {
            nodes[i] = nullptr;
        }
    }

    ~Map() {
        clear();
    }

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

    void insert(size_t key, const map_data& data) {
        size_t index = hash(key);

        Node* newNode = new Node(key, data);
        newNode->next = nodes[index];
        nodes[index] = newNode;
    }

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

};

class StudentList {
private:
    List students;
    Map studentsMap;

public:
    void addStudent(const Student& student) {
        students.push_back(student);
        map_data data;
        data.node = students.tail;
        data.sum_marks = 0;
        data.count_marks = 0;
        for (int i = 0; i < 5; i++) {
            data.sum_marks += student.marks[i];
            data.count_marks++;
        }
        studentsMap.insert(students.tail->data.group, data);
    }

    void print() {
        students.print();
    }

    void printGroup(int group) {
        map_data* data = studentsMap.search(group);
        if (data != nullptr) {
            cout << data->node->data.to_string() << endl;
        }
    }

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

    void clear() {
        students.clear();
        studentsMap.clear();
    }

    void from_csv_string(const string& csv) {
        int pos = 0;
        int nextPos = 0;
        while ((nextPos = csv.find('\n', pos)) != string::npos) {
            string value = csv.substr(pos, nextPos - pos);
            Student student("", 0, new int[5]{0, 0, 0, 0, 0}, 0, "");
            student.from_csv_string(value);
            addStudent(student);
            pos = nextPos + 1;
        }
    }

    string to_csv_string() {
        string result = "";
        List::Node* current = students.head;
        while (current != nullptr) {
            result += current->data.to_csv_string() + "\n";
            current = current->next;
        }
        return result;
    }
};