//
//  VelocityFieldInterface.hpp
//  sparks
//
//  Created by Brian Allen on 4/1/13.
//
//

#ifndef sparks_VelocityFieldInterface_hpp
#define sparks_VelocityFieldInterface_hpp

#include "Spark.hpp"

#include <Eigen/Dense>
#include <memory>
namespace spark
{
    class VelocityFieldInterface
    {
    public:
        virtual Eigen::Vector3f velocityAtPosition( const Eigen::Vector3f& pos ) = 0;
    };
    typedef spark::shared_ptr< VelocityFieldInterface > VelocityFieldInterfacePtr;
} // end namespace spark

#endif