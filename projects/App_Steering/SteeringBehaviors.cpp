//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "SteeringAgent.h"

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = (m_Target).Position - pAgent->GetPosition(); //Desired Velocity
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); //Rescale to Max Speed

	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 }, 0.4f);

	return steering;
}

//FLEE (base> SEEK)
//******
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	float distance{ Elite::Distance(pAgent->GetPosition(), m_Target.Position) };

	//outside range
	if (distance >= m_FleeRadius)
	{
		steering.IsValid = false;
		return steering;
	}

	steering.LinearVelocity = pAgent->GetPosition() - (m_Target).Position; //unDesired Velocity
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); //Rescale to Max Speed

	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 }, 0.4f);

	return steering;
}

//ARRIVE (base> SEEK)
//******
SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	float slowDownDistance{ 10 };

	steering.LinearVelocity = (m_Target).Position - pAgent->GetPosition(); //unUesired Velocity
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	float distance{ Elite::Distance(pAgent->GetPosition(), (m_Target).Position) };

	//std::cout << distance << "\n";

	if (distance < slowDownDistance)
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed() * (distance / (2 * slowDownDistance)); //Rescale to Max Speed
	}
	else
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); //Rescale to Max Speed
	}

	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawPoint(pAgent->GetPosition(), 5, { 1,0,0 }, 0.1f);
		DEBUGRENDERER2D->DrawPoint((m_Target).Position, 5, { 0,1,1 }, 0.1f);
	}

	return steering;
}

//FACE (base> SEEK)
//******
SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	float marginOfError{ 0.08f };
	auto targetVector{ (m_Target).Position - pAgent->GetPosition() };
	Elite::Vector2 baseOrientationVector{ cos(pAgent->GetOrientation() - Elite::ToRadians(90)), sin(pAgent->GetOrientation() - Elite::ToRadians(90)) };
	float angle{ targetVector.AngleWith(baseOrientationVector) };

	//down is 0, left up is -90, right up is 90
	pAgent->SetAutoOrient(false);
	if (angle > marginOfError)
	{
		steering.AngularVelocity = -pAgent->GetMaxAngularSpeed();
	}
	else if (angle < -marginOfError)
	{
		steering.AngularVelocity = pAgent->GetMaxAngularSpeed();
	}

	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), targetVector, 10, { 0,1,1 }, 0.5f);
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), pAgent->GetDirection(), 15, { 1,1,1 }, 0.4f);
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), { 0,-1 }, 5, { 1,0,0 }, 0.3f);
	}
	return steering;
}

//WANDER (base> SEEK)
//******
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	pAgent->SetAutoOrient(true);
	Elite::Vector2 baseOrientationVector{ cos(pAgent->GetOrientation() - Elite::ToRadians(90)), sin(pAgent->GetOrientation() - Elite::ToRadians(90)) };
	Elite::Vector2 circleCenter{ ((baseOrientationVector.GetNormalized()) * m_Offset) + pAgent->GetPosition() };

	m_TotalTime += deltaT;
	if (m_TotalTime >= m_NewPointDelay)
	{
		m_TotalTime -= m_NewPointDelay;
		float randomAngle{ Elite::randomFloat(-m_MaxAngleChange,m_MaxAngleChange) };
		float oldAngle{ atan2(m_PrevDirection.y,m_PrevDirection.x) };
		float newAngle{ oldAngle + randomAngle };
		Elite::Vector2 newRandomVect{ cos(newAngle), sin(newAngle) };
		newRandomVect = newRandomVect.GetNormalized() * m_Radius;
		m_PrevDirection = newRandomVect;
	}

	Elite::Vector2 target{ m_PrevDirection + circleCenter };

	steering.LinearVelocity = target - pAgent->GetPosition();
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(circleCenter, m_PrevDirection, m_Radius, { 1,1,1 }, 0.4f);
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), circleCenter - pAgent->GetPosition(), m_Offset, { 0,1,0 }, 0.3f);//line fromt agent to circle center
		DEBUGRENDERER2D->DrawPoint(circleCenter, 5, { 0,1,1 }, 0.1f);//circle center point
		DEBUGRENDERER2D->DrawPoint(target, 5, { 1,0,0 }, 0.1f); //new target point
		DEBUGRENDERER2D->DrawCircle(circleCenter, m_Radius, { 1,1,0 }, .8f); //circle of new points
	}
	return steering;
}

//PURSUIT
//******
SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	Elite::Vector2 target{};

	float PredictionLimiter{ .1f };
	float distance{ Elite::Distance(m_Target.Position, pAgent->GetPosition()) * PredictionLimiter };
	target = (m_Target.LinearVelocity * distance) + m_Target.Position;


	steering.LinearVelocity = target - pAgent->GetPosition();
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(m_Target.Position, m_Target.GetDirection(), m_Target.LinearVelocity.Magnitude(), { 0,1,0 }, 0.3f);//line fromt agent to circle center
		DEBUGRENDERER2D->DrawPoint(m_Target.Position, 5, { 0,1,1 }, 0.1f);//
		DEBUGRENDERER2D->DrawPoint(target, 5, { 1,0,0 }, 0.1f); //new target point
	}
	return steering;
}

//EVADE
//******
SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	Elite::Vector2 target{};
	float distance{ Elite::Distance(pAgent->GetPosition(), m_Target.Position) };

	//outside range
	if (distance >= m_EvadeRadius)
	{
		steering.IsValid = false;
		return steering;
	}

	Elite::Vector2 evadepoint{ m_Target.Position + (m_Target.LinearVelocity.GetNormalized() * m_EvadePredRange) };
	target= pAgent->GetPosition() - evadepoint;

	steering.LinearVelocity = target;
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); //Rescale to Max Speed

	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawPoint(evadepoint, 5, { 1,1,1 }, 0.1f); //target point
		//DEBUGRENDERER2D->DrawPoint(m_Target.Position, 5, { 0,1,1 }, 0.1f);//
		//DEBUGRENDERER2D->DrawPoint((m_Target.Position + m_Target.LinearVelocity), 5, { 1,0,1 }, 0.1f); //target point
		DEBUGRENDERER2D->DrawDirection(m_Target.Position, target, 1000, { 1,1,1 }, 0.1f); //target point
	}
	return steering;
	}
