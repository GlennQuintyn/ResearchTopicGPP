#pragma once
#include "../SteeringBehaviors.h"
class Flock;

//SEPARATION - FLOCKING
//*********************
class Seperation : public ISteeringBehavior
{
public:
	Seperation(Flock* pflock)
		:m_pFlock{ pflock }
	{}
	virtual ~Seperation() = default;

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	Flock* m_pFlock = nullptr;
};


//COHESION - FLOCKING
//*******************
class Cohesion : public ISteeringBehavior
{
public:
	Cohesion(Flock* pflock)
		:m_pFlock{ pflock }
	{}
	virtual ~Cohesion() = default;

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	Flock* m_pFlock = nullptr;
};


//VELOCITY MATCH - FLOCKING
//************************
class VelocityMatch : public ISteeringBehavior
{
public:
	VelocityMatch(Flock* pflock)
		:m_pFlock{ pflock }
	{}
	virtual ~VelocityMatch() = default;

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	Flock* m_pFlock = nullptr;
};
