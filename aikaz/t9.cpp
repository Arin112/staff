#include <iostream>
#include <vector>


struct Node {
    int data;
    Node* left;
    Node* right;

    Node(int value) : data(value), left(nullptr), right(nullptr) {}
};


class BinarySearchTree {
private:
    Node* root;

    
    Node* insertNode(Node* node, int value) {
        if (node == nullptr) {
            return new Node(value);
        }

        if (value < node->data) {
            node->left = insertNode(node->left, value);
        } else if (value > node->data) {
            node->right = insertNode(node->right, value);
        } else {
            return node; 
        }
        return node;
    }

    
    bool searchNode(Node* node, int value) {
        if (node == nullptr) {
            return false;
        }

        if (node->data == value) {
            return true;
        } else if (value < node->data) {
            return searchNode(node->left, value);
        } else {
            return searchNode(node->right, value);
        }
    }

    
    void deleteTree(Node* node) {
        if (node != nullptr) {
            deleteTree(node->left);
            deleteTree(node->right);
            delete node;
        }
    }

public:
    BinarySearchTree() : root(nullptr) {}

    
    ~BinarySearchTree() {
        deleteTree(root);
    }

    
    void insert(int value) {
        root = insertNode(root, value);
    }

    
    bool search(int value) {
        return searchNode(root, value);
    }
};


const int HASH_TABLE_SIZE = 10;


class FastSearchStructure {
private:
    std::vector<BinarySearchTree> hashTable;

    
    int hashFunction(int value) {
        return value % HASH_TABLE_SIZE;
    }

public:
    FastSearchStructure() : hashTable(HASH_TABLE_SIZE) {}

    
    void insert(int value) {
        int index = hashFunction(value);
        hashTable[index].insert(value);
    }

    
    bool search(int value) {
        int index = hashFunction(value);
        return hashTable[index].search(value);
    }
};

int main() {
    FastSearchStructure fastSearch;
    int value;

    std::cout << "Enter numbers to add to the structure (enter 0 to finish):\n";
    do {
        std::cin >> value;
        if (value != 0) {
            fastSearch.insert(value);
        }
    } while (value != 0);

    std::cout << "Enter numbers to search for (enter 0 to finish):\n";
    do {
        std::cin >> value;
        if (value != 0) {
            if (fastSearch.search(value)) {
                std::cout << "Value " << value << " found.\n";
            } else {
                std::cout << "Value " << value << " not found.\n";
            }
        }
    } while (value != 0);

    return 0;
}
