#include <iostream>
#include <string>
#include <vector>

// Размер хэш-таблицы (количество букв в латинском алфавите)
const int TABLE_SIZE = 26;

struct WordNode
{
    std::string word;
    WordNode *next;

    WordNode(const std::string &w) : word(w), next(nullptr) {}
};

class HashTable
{
private:
    WordNode *table[TABLE_SIZE];

    // Так как у нас всего размер таблицы 26, то можно использовать первую букву слова, всё равно коллизий будет много
    int hashFunction(const std::string &key)
    {
        return key.size() > 0 ? key[0] % TABLE_SIZE : 0;
    }

public:
    HashTable()
    {
        for (int i = 0; i < TABLE_SIZE; ++i)
        {
            table[i] = nullptr;
        }
    }

    ~HashTable()
    {
        for (int i = 0; i < TABLE_SIZE; ++i)
        {
            WordNode *current = table[i];
            while (current != nullptr)
            {
                WordNode *next = current->next;
                delete current;
                current = next;
            }
        }
    }

    void insert(const std::string &word)
    {
        int index = hashFunction(word);

        WordNode *newNode = new WordNode(word);

        newNode->next = table[index];
        table[index] = newNode;
    }

    bool search(const std::string &word)
    {
        int index = hashFunction(word);
        WordNode *current = table[index];

        while (current != nullptr)
        {
            if (current->word == word)
            {
                return true;
            }
            current = current->next;
        }
        return false;
    }

    void printTable()
    {
        for (int i = 0; i < TABLE_SIZE; ++i)
        {
            std::cout << i << ": ";
            WordNode *current = table[i];
            while (current != nullptr)
            {
                std::cout << current->word << " -> ";
                current = current->next;
            }
            std::cout << "nullptr" << std::endl;
        }
    }
};

int main()
{
    HashTable ht;
    std::string word;
    // Например вводим слова: apple banana cat dog elephant milk more my no not now on one orange out over people person play please put question quick quiet quite rabbit rain end
    std::cout << "Enter words to add to the hash table (enter 'end' to finish):\n";
    do
    {
        std::cin >> word;
        if (word != "end")
        {
            ht.insert(word);
        }
    } while (word != "end");

    ht.printTable();

    do
    {
        std::cout << "Enter a word to search for: ";
        std::cin >> word;

        if (ht.search(word))
        {
            std::cout << "Word found: " << word << std::endl;
        }
        else
        {
            std::cout << "Word not found: " << word << std::endl;
        }
    } while (word != "end");

    return 0;
}
