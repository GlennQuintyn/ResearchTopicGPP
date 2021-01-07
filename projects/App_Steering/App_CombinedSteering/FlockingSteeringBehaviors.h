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
	Elite::Color m_Blue{ 0, 0, 1.f };
	Elite::Color m_Red{ 1.f, 0, 0 };
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
	Elite::Color m_Blue{ 0, 0, 1.f };
	Elite::Color m_Red{ 1.f, 0, 0 };
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
	Elite::Color m_Blue{ 0, 0, 1.f };
	Elite::Color m_Red{ 1.f, 0, 0 };
};
