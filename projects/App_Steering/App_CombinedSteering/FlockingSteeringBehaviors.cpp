#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Seperation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	vector<SteeringAgent*> neighbors{};
	int nrNeighbors{};
	if (pAgent->GetBodyColor() == m_Blue)
	{
		neighbors = m_pFlock->GetBlueNeighbors();
		nrNeighbors = m_pFlock->GetNrOfBlueNeighbors();
	}
	else if (pAgent->GetBodyColor() == m_Red)
	{
		neighbors = m_pFlock->GetRedNeighbors();
		nrNeighbors = m_pFlock->GetNrOfRedNeighbors();
	}

	Elite::Vector2 fleeVector{};
	float speed{}, tempSpeed{};

	for (int index{}; index < nrNeighbors; ++index)
	{
		Elite::Vector2 temp{ pAgent->GetPosition() - neighbors[index]->GetPosition() };

		Elite::Vector2 posNeighbor{ neighbors[index]->GetPosition() };
		Elite::Vector2 posAgent{ pAgent->GetPosition() };

		tempSpeed = Elite::DistanceSquared(posNeighbor, posAgent);

		fleeVector += temp / tempSpeed;
	}

	fleeVector /= float(nrNeighbors);
	steering.LinearVelocity = fleeVector;
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); //Rescale to Max Speed


	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 }, 0.4f);
	}

	return steering;
}

//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	Elite::Vector2 target{ };

	if (pAgent->GetBodyColor() == m_Blue)
	{
		target = m_pFlock->GetAverageBlueNeighborPos(pAgent->GetBodyColor());
	}
	else if (pAgent->GetBodyColor() == m_Red)
	{
		target = m_pFlock->GetAverageRedNeighborPos(pAgent->GetBodyColor());
	}

	steering.LinearVelocity = target - pAgent->GetPosition();
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); //Rescale to Max Speed

	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawPoint(target, 5, { 1,0,1 });
		//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 }, 0.4f);
	}

	return steering;
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	Elite::Vector2 speed{};// m_pFlock->GetAverageBlueNeighborVelocity(pAgent->GetBodyColor())

	if (pAgent->GetBodyColor() == m_Blue)
	{
		speed = m_pFlock->GetAverageBlueNeighborVelocity(pAgent->GetBodyColor());
	}
	else if (pAgent->GetBodyColor() == m_Red)
	{
		speed = m_pFlock->GetAverageRedNeighborVelocity(pAgent->GetBodyColor());
	}

	steering.LinearVelocity = speed; //Rescale to Max Speed

	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 }, 0.4f);
	}

	return steering;
}