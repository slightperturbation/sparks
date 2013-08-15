#ifndef SPARK_TASK_HPP
#define SPARK_TASK_HPP

namespace spark
{
    /// An abstract base class for a unit of work.
    /// Subclasses should encapsulate the data needed to perform the work
    /// and the output to final destination asynchronously.
    /// Tasks should generally be block-free.
    class Task
    {
    public:
        virtual ~Task() {}
        virtual void execute( void ) = 0;
    };
    typedef spark::shared_ptr< Task > TaskPtr;
}


#endif
