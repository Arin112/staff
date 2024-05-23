#include <iostream>


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

int main() {
    BinarySearchTree tree;
    int value;

    std::cout << "Enter numbers to add to the tree (enter 0 to finish):\n";

    do {
        std::cin >> value;
        if (value != 0) {
            tree.insert(value);
        }
    } while (value != 0);

    std::cout << "Enter numbers to search in the tree (enter 0 to finish):\n";

    do {
        std::cin >> value;
        if (value != 0) {
            if (tree.search(value)) {
                std::cout << "Value " << value << " found in the tree.\n";
            } else {
                std::cout << "Value " << value << " not found in the tree.\n";
            }
        }
    } while (value != 0);

    return 0;
}