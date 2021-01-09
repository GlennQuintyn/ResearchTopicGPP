#include "stdafx.h"
#include "SteeringAgent.h"
#include "SteeringBehaviors.h"

void SteeringAgent::Update(float dt)
{
	if (m_pSteeringBehavior)
	{
		m_TotalTime += dt;

		if (m_TotalTime >= m_AttackDelay)
		{
			m_CanAttack = true;
		}

		auto output = m_pSteeringBehavior->CalculateSteering(dt, this);

		//if agent is dead
		if (!m_Alive)
		{
			//std::cout << "\nDEAD\n\n";
			//make agent color black
			SetBodyColor({ 0,0,0 });

			//if dead then set velocity to 0
			output.LinearVelocity = {};
		}


		//Linear Movement
		//***************
		auto linVel = GetLinearVelocity();
		auto steeringForce = output.LinearVelocity - linVel;
		auto acceleration = steeringForce / GetMass();

		if (m_RenderBehavior)
		{
			DEBUGRENDERER2D->DrawDirection(GetPosition(), acceleration, acceleration.Magnitude(), { 0, 1, 1 ,0.5f }, 0.40f);
			DEBUGRENDERER2D->DrawDirection(GetPosition(), linVel, linVel.Magnitude(), { 1, 0, 1 ,0.5f }, 0.40f);
		}
		SetLinearVelocity(linVel + (acceleration * dt));

		//Angular Movement
		//****************
		if (m_AutoOrient)
		{
			auto desiredOrientation = Elite::GetOrientationFromVelocity(GetLinearVelocity());
			SetRotation(desiredOrientation);
		}
		else
		{
			if (output.AngularVelocity > m_MaxAngularSpeed)
				output.AngularVelocity = m_MaxAngularSpeed;
			SetAngularVelocity(output.AngularVelocity);
		}
	}
}

void SteeringAgent::Render(float dt)
{
	//if agent is dead
	if (!m_Alive)
	{
		//std::cout << "\nDEAD\n\n";
		//make agent color black
		SetBodyColor({ 0,0,0 });
	}

	//Use Default Agent Rendering
	BaseAgent::Render(dt);
}

bool SteeringAgent::Attack()
{
	if (m_CanAttack)
	{
		m_CanAttack = false;
		m_TotalTime = 0;
		return true;
	}
	return false;
}

void SteeringAgent::TakeDamage(float damage)
{
	m_Health -= damage;
	if (m_Health <= 0)
	{
		m_Alive = false;
	}
}
