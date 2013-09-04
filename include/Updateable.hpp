#ifndef SPARK_UPDATEABLE_HPP
#define SPARK_UPDATEABLE_HPP

namespace spark 
{
    class Updateable
    {
    public:
        virtual ~Updateable() {}
        
        virtual void update( float dt ) {}
        virtual void fixedUpdate( float dt ) {}
    };
}

#endif
