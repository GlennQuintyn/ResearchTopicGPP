//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_Flocking.h"
#include "../SteeringAgent.h"
#include "Flock.h"

App_Flocking::App_Flocking()
{
	m_MouseTargetLclick.Position = { m_TrimWorldSize / 5.f, m_TrimWorldSize / 2.f };
	m_MouseTargetRclick.Position = { m_TrimWorldSize * 4.f / 5.f, m_TrimWorldSize / 2.f };
}

//Destructor
App_Flocking::~App_Flocking()
{

}

//Functions
void App_Flocking::Start()
{
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom((m_TrimWorldSize / 2) + 5.0f);//plus 5 to zoom out just a bit more than the worldsize box
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(m_TrimWorldSize * .75f, m_TrimWorldSize * .5f));//center the worldbox in the center incl the ui box
}

void App_Flocking::Update(float deltaTime)
{
	//INPUT
	if (INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eLeft) && m_VisualizeMouseTarget)
	{
		auto const mouseData = INPUTMANAGER->GetMouseData(InputType::eMouseButton, InputMouseButton::eLeft);
		m_MouseTargetLclick.Position = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y) });
	}

	if (INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eMiddle) && m_VisualizeMouseTarget)
	{
		auto const mouseData = INPUTMANAGER->GetMouseData(InputType::eMouseButton, InputMouseButton::eMiddle);
		m_MouseTargetRclick.Position = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y) });
	}

	m_Flock.UpdateAndRenderUI();
	m_Flock.Update(deltaTime, m_TrimWorldSize, m_MouseTargetLclick, m_MouseTargetRclick);
}

void App_Flocking::Render(float deltaTime) const
{
	m_Flock.Render(deltaTime);

	std::vector<Elite::Vector2> points =
	{
		{ 0,m_TrimWorldSize },
		{ m_TrimWorldSize,m_TrimWorldSize },
		{ m_TrimWorldSize,0 },
		{0,0 }
	};
	DEBUGRENDERER2D->DrawPolygon(&points[0], 4, { 1,0,0,1 }, 0.4f);

	//Render Target
	if (m_VisualizeMouseTarget)
		DEBUGRENDERER2D->DrawSolidCircle(m_MouseTargetLclick.Position, 0.3f, { 0.f,0.f }, { 1.f,0.f,0.f }, -0.8f);
}
