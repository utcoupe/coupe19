#ifndef COLLISIONS_ENGINE_VELOCITY
#define COLLISIONS_ENGINE_VELOCITY

#include "collisions/engine/velocity_check_zone.h"

#include "collisions/obstacle.h"
#include "collisions/shapes/abstract_shape.h"

#include <memory>

class Velocity {
public:
    using ObstaclePtr = std::shared_ptr<Obstacle>;
    using ShapePtr = std::shared_ptr<CollisionsShapes::AbstractShape>;
    
    constexpr Velocity(double width, double height, double velLinear = 0.0, double velAngular = 0.0) noexcept:
        velLinear_(velLinear),
        velAngular_(velAngular),
        checkZone_(width, height, CollisionLevel::LEVEL_STOP)
    {}
    
    std::vector<ShapePtr> getShapes(Position objectPos, double maxDist = -1);
    std::vector<Collision> checkCollisions(Position objectPos, std::vector<ObstaclePtr> obstacles);
    
    constexpr void setVelLinear(double velLinear) noexcept
    { velLinear_ = velLinear; }
    constexpr void setVelAngular(double velAngular) noexcept { velAngular_ = velAngular; }
        
private:
    double velLinear_, velAngular_;
    VelocityCheckZone checkZone_;
};

#endif // COLLISIONS_ENGINE_VELOCITY
