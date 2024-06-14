#include <maze_utils.hpp>
#include <random>


namespace utils{

int r()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<int> dis(0, 100000);
    return dis(gen);
}

}  // namespace utils