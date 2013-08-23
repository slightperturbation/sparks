
#include "SixenseInputDevice.hpp"

#include <glm/gtc/matrix_transform.hpp>

glm::vec3
spark::SixenseInputDevice
::getPosition( void ) const
{
    return glm::vec3();
}

glm::mat4
spark::SixenseInputDevice
::getTransform( void ) const
{
    return glm::mat4();
}

bool
spark::SixenseInputDevice
::isButtonPressed( int buttonNumber ) const
{
    return false;
}


