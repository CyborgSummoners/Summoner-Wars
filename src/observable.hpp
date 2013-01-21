#ifndef OBSERVABLE_HPP
#define OBSERVABLE_HPP

#include "include.hpp"
#include "observer.hpp"

namespace sum
{

template<class T>
class Observable
{

public:

	void addObs(Observer<T> *observer)
	{
		observers.push_back(observer);
	}

	void clearObs()
	{
		observers.clear();
	}

	void updateAll(const T &message)
	{
		for(size_t i=0;i<observers.size();++i)
		{
			observers[i]->update(message);
		}
	}

private:

	std::vector<Observer<T>*> observers;

};

}

#endif
