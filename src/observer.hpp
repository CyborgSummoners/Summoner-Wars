#ifndef OBSERVER_HPP
#define OBSERVER_HPP

namespace sum 
{

template<class T>
class Observer
{

public:

	virtual void update(T &message)=0;

};

}

#endif