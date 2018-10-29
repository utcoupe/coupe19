#include "collisions/engine/engine.h"

#include <ros/console.h>

#include <cmath>

using namespace CollisionResolver;

std::vector<PtrObstacle> findCollisions(const std::vector<PtrShape>& robotShapes, const std::vector<PtrObstacle>& obstacleShapes) {
    std::vector<PtrObstacle> collisions;
    for (auto& robotShape: robotShapes) {
        for (auto& obstShape: obstacleShapes) {
            bool intersecting = false;
            if (robotShape->isCollidingWith(obstShape->getShape().get())) {
                collisions.push_back(obstShape);
                intersecting = true;
            }
            auto obstVelShapes = obstShape->getVelocityShapes();
            if (!intersecting && !obstVelShapes.empty()) {
                //
            }
        }
    }
    return collisions;
}
