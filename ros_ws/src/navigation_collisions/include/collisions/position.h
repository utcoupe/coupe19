#ifndef COLLISIONS_POSITION_H
#define COLLISIONS_POSITION_H

#include "collisions/point.h"

#include <ostream>

class Position: public Point {
public:
    constexpr Position (double x, double y, double angle) noexcept:
        Position({x, y}, angle)
    {}
    
    constexpr Position (Point pos = {0, 0}, double angle = 0.0) noexcept:
        Point(pos),
        _a(angle)
    {}
    
    constexpr Point toPoint() const noexcept {
        return {_x, _y};
    }
    
    // Operators
    constexpr bool operator== (const Position& other) const noexcept {
        return this->toPoint() == other.toPoint() && _a == other.getAngle();
    }
    
    constexpr bool operator!= (const Position& other) const noexcept{
        return !this->operator==(other);
    }

    friend std::ostream& operator<<(std::ostream& os, const Position& pos) {
        os << "(" << pos._x << "," << pos._y << ", " << pos._a <<  ")";
        return os;
    }
    
    // Getters & setters
    constexpr double getAngle() const           noexcept { return _a; }
    constexpr void setAngle(const double angle) noexcept { _a = angle; }
    
protected:
    double _a;
};

#endif // COLLISIONS_POSITION_H
