#include <iostream>
#include <string>
#include <vector>
#include <bitset>

template<size_t N>
void increment(std::bitset<N> &b) {
    for (size_t i = 0; i < N; ++i) {
        if (b[i] == 0) {
            b[i] = 1;
            break;
        } else {
            b[i] = 0;
        }
    }
}

int main(){

    //std::bitset<40> b(0);
/*
    while(true){
        std::cout << b << std::endl;
        increment(b);
    }
*/
    return 0;
}