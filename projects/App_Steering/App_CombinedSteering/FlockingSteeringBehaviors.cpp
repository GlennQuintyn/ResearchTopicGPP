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

	vector<SteeringAgent*> neighbors{ m_pFlock->GetNeighbors() };
	int nrNeighbors{ m_pFlock->GetNrOfNeighbors() };

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

	Elite::Vector2 target{ m_pFlock->GetAverageNeighborPos() };

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

	Elite::Vector2 speed{ m_pFlock->GetAverageNeighborVelocity() };

	steering.LinearVelocity = speed; //Rescale to Max Speed

	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 }, 0.4f);
	}

	return steering;
}