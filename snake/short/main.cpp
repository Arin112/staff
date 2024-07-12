#include <iostream>
#include <vector>

class XY {
  public:
    int x, y;
    XY(int x, int y) : x(x), y(y) {}
    XY() : x(0), y(0) {}
    XY operator+(const XY &other) const { return XY(x + other.x, y + other.y); }
    XY operator-(const XY &other) const { return XY(x - other.x, y - other.y); }
    bool operator==(const XY &other) const {
        return x == other.x && y == other.y;
    }
    bool operator!=(const XY &other) const {
        return x != other.x || y != other.y;
    }
    XY operator*(int n) const { return XY(x * n, y * n); }
    XY operator*=(int n) {
        x *= n;
        y *= n;
        return *this;
    }
    XY operator/(int n) const { return XY(x / n, y / n); }
    XY operator/=(int n) {
        x /= n;
        y /= n;
        return *this;
    }
    XY operator%(int n) const { return XY(x % n, y % n); }
} const UP(0, -1), DOWN(0, 1), LEFT(-1, 0), RIGHT(1, 0);

class Snake {
  public:
    std::vector<XY> body;
};

class Food {
  public:
    XY pos;
};

class Field {
  public:
    enum Blocks { EMPTY, WALL, FOOD, SNAKE_HEAD, SNAKE_BODY };
    int width, height;
    std::vector<std::vector<Blocks>> arr;

    Field(int width, int height)
        : width(width), height(height),
          arr(width, std::vector<Blocks>(height, EMPTY)) {}
};

int main() {}