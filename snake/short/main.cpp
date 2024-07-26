#include <iostream>
#include <vector>
#include <memory>

class Block {
  public:
    using BlockID = int;
    BlockID type_;
    static BlockID next_id;

    Block(BlockID type) : type_(type) {}
};

class XY {
  public:
    int x, y;
    XY(int x, int y) : x(x), y(y) {}
    XY() : x(0), y(0) {}
    XY operator+(const XY &other) const { return XY(x + other.x, y + other.y); }
    XY operator-(const XY &other) const { return XY(x - other.x, y - other.y); }
    XY operator-() const { return XY(-x, -y); }
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

// Renderable object
class Renderable {
  public:
    virtual std::vector<std::pair<XY, char>> render() = 0;
    virtual ~Renderable() = default;
};

class CollisionObject {
  public:
    virtual bool collides(const XY &pos) = 0;
    virtual bool collides(const CollisionObject &other) = 0;
    virtual std::vector<XY> get_collision_points() = 0;
    virtual ~CollisionObject() = default;
};

class Snake : public Renderable, public CollisionObject {
  public:
    std::vector<XY> body;
    XY direction;
    bool grow;

    Snake(XY pos, XY direction) : direction(direction), grow(false) {
        body.push_back(pos);
    }

    bool set_direction(XY dir) {
        if (dir + direction != XY(0, 0)) {
            direction = dir;
            return true;
        }
        return false;
    }

    void step() {}
};

class Food : public Renderable, public CollisionObject {
  public:
    XY pos;
};

// mainly for borders, but can be used for any walls
class Walls : public Renderable, public CollisionObject {
  public:
    std::vector<XY> walls;
};


class CollosionField {
    std::vector<std::vector<Block>> arr;
    std::vector<std::weak_ptr<CollisionObject>> objects;
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

class Game {};

class ControllerBase {
    virtual XY getDirection() = 0;
    virtual ~ControllerBase() = default;
};

class ControllerKeyboard : public ControllerBase {
    XY getDirection() override { return XY(); }
};

int main() { return 0; }
