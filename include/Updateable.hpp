#ifndef SPARK_UPDATEABLE_HPP
#define SPARK_UPDATEABLE_HPP

namespace spark 
{
    class Updateable
    {
    public:
        virtual ~Updateable() {}
        virtual void update( float dt ) = 0;
        virtual void fixedUpdate( float dt ) = 0;
    };
}

#endif
