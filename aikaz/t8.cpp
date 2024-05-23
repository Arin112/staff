#include <iostream>


struct Node {
    int data;             
    Node* left;         
    Node* right;        

    Node(int value) : data(value), left(nullptr), right(nullptr) {} 
};


class BinarySearchTree {
public:
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

    
    void printPrefix(Node* node) {
        if (node != nullptr) {
            std::cout << node->data << " ";
            printPrefix(node->left);
            printPrefix(node->right);
        }
    }

    
    void printInfix(Node* node) {
        if (node != nullptr) {
            printInfix(node->left);
            std::cout << node->data << " ";
            printInfix(node->right);
        }
    }

    
    void printPostfix(Node* node) {
        if (node != nullptr) {
            printPostfix(node->left);
            printPostfix(node->right);
            std::cout << node->data << " ";
        }
    }

    
    void printByLayers(Node* node) {
        std::cout << "Printing tree by layers:" << std::endl;
        if (node == nullptr) {
            return;
        }

        int level = 0;
        while (printByLayers_impl(node, level)) {
            std::cout << std::endl;
            ++level;
        }
    }

    bool printByLayers_impl(Node* node, int level) {
        if (node == nullptr) {
            return false;
        }

        if (level == 0) {
            std::cout << node->data << " ";
            return true;
        }

        bool left = printByLayers_impl(node->left, level - 1);
        bool right = printByLayers_impl(node->right, level - 1);

        return left || right;
    
    }

};

int main() {
    BinarySearchTree tree;
    int value;
    // Например: 5 3 8 2 4 6 9 11 515 24 77 252 95 333 12 14 15 16 17 18 19 20 21 22 23 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 0
    std::cout << "Enter numbers to add to the tree (enter 0 to finish):\n";

    do {
        std::cin >> value;
        if (value != 0) {
            tree.insert(value);
        }
    } while (value != 0);

    do {
        std::cout << "Menu:\n";
        std::cout << "1 - Search for a value in the tree\n";
        std::cout << "2 - Print tree in prefix order\n";
        std::cout << "3 - Print tree in infix order\n";
        std::cout << "4 - Print tree in postfix order\n";
        std::cout << "5 - Print by layets - top to bottom, left to right\n";
        std::cout << "0 - Exit\n";

        std::cin >> value;

        if (value == 1) {
            do {
                std::cout << "Enter a value to search (enter 0 to finish): ";
                std::cin >> value;
                if (value != 0) {
                    if (tree.search(value)) {
                        std::cout << "Value " << value << " found in the tree.\n";
                    } else {
                        std::cout << "Value " << value << " not found in the tree.\n";
                    }
                }
            } while (value != 0);
            value = -1;
        } else if (value == 2) {
            tree.printPrefix(tree.root);
            std::cout << std::endl;
        } else if (value == 3) {
            tree.printInfix(tree.root);
            std::cout << std::endl;
        } else if (value == 4) {
            tree.printPostfix(tree.root);
            std::cout << std::endl;
        } else if (value == 5) {
            tree.printByLayers(tree.root);
        }
    } while (value != 0);


    return 0;
}