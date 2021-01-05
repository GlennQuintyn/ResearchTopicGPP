#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\App_Steering\SteeringAgent.h"

// --- Cell ---
// ------------
#pragma region Cell
Cell::Cell(float left, float bottom, float width, float height)
{
	boundingBox.bottomLeft = { left, bottom };
	boundingBox.width = width;
	boundingBox.height = height;
}

std::vector<Elite::Vector2> Cell::GetRectPoints() const
{
	auto left = boundingBox.bottomLeft.x;
	auto bottom = boundingBox.bottomLeft.y;
	auto width = boundingBox.width;
	auto height = boundingBox.height;

	std::vector<Elite::Vector2> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}
#pragma endregion

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(float width, float height, int rows, int cols, int maxEntities)
	: m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows)
	, m_NrOfCols(cols)
	, m_Neighbors(maxEntities)//make neighbors memory pool
	, m_NrOfNeighbors(0)
	, m_CellWidth{ width / cols }
	, m_CellHeight{ height / rows }
{

	//make  rows * cols objects and put them into the vec
	for (int i = 0; i < rows * cols; i++)
	{
		Cell tempCellObject{ (i % m_NrOfCols) * m_CellWidth ,(i / m_NrOfRows) * m_CellHeight,m_CellWidth,m_CellHeight };
		m_Cells.push_back(tempCellObject);
	}
}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	int index{ PositionToIndex(agent->GetPosition()) };
	m_Cells[index].agents.push_back(agent);

}

void CellSpace::UpdateAgentCell(SteeringAgent* agent, const Elite::Vector2& oldPos)
{
	int newIndex{ PositionToIndex(agent->GetPosition()) };
	int oldIndex{ PositionToIndex(oldPos) };

	if (newIndex != oldIndex)
	{
		//int temp1 = m_Cells[oldIndex].agents.size();
		//int temp2 = m_Cells[newIndex].agents.size();
		m_Cells[oldIndex].agents.remove(agent);
		m_Cells[newIndex].agents.push_back(agent);
		//temp1 = m_Cells[oldIndex].agents.size();
		//temp2 = m_Cells[newIndex].agents.size();

		//std::cout << temp1 << "\n";
		//std::cout << temp2 << "\n";

	}


}

void CellSpace::RegisterNeighbors(const Elite::Vector2& pos, float queryRadius, bool debugRender, bool first)
{
	std::vector<int> cellIndexVec{};
	Elite::Rect hoodBox{ GetRectFromPosRadius(pos, queryRadius) };
	m_NrOfNeighbors = 0;

	//check each cell if they overlap with the hoodbox and if so put their index into the vec
	for (uint32_t index{}; index < m_Cells.size(); ++index)
	{
		if (Elite::IsOverlapping(hoodBox, m_Cells[index].boundingBox))
		{
			cellIndexVec.push_back(index);
		}
	}

	//goes through the index vec to check if they are in hood radius, if so add them to the neighbours vec
	for (size_t index = 0; index < cellIndexVec.size(); index++)
	{
		std::list<Cell* >::iterator it;
		//for (it = m_Cells[cellIndexVec[index]].agents.begin(); it != m_Cells[cellIndexVec[index]].agents.end(); ++it) 
		for (const auto& agent : m_Cells[cellIndexVec[index]].agents)
		{
			if (Elite::DistanceSquared(pos, agent->GetPosition()) <= queryRadius * queryRadius &&
				(pos != agent->GetPosition()))
			{
				m_Neighbors[m_NrOfNeighbors] = agent;
				++m_NrOfNeighbors;
			}
		}


		//std::copy(m_Cells[cellIndexVec[index]].agents.begin(), m_Cells[cellIndexVec[index]].agents.end(), &m_Neighbors[m_NrOfNeighbors]);
		//m_NrOfNeighbors += m_Cells[cellIndexVec[index]].agents.size();
	}

	//debug rendering
	if (debugRender && first)
	{
		//draw neighborhood box
		DEBUGRENDERER2D->DrawDirection({ hoodBox.bottomLeft.x , hoodBox.bottomLeft.y }, { 1,0 }, hoodBox.width, { 1,1,1 }, .4f);
		DEBUGRENDERER2D->DrawDirection({ hoodBox.bottomLeft.x , hoodBox.bottomLeft.y }, { 0,1 }, hoodBox.height, { 1,1,1 }, .4f);
		DEBUGRENDERER2D->DrawDirection({ hoodBox.bottomLeft.x + hoodBox.width , hoodBox.bottomLeft.y + hoodBox.height }, { -1,0 }, hoodBox.width, { 1,1,1 }, .4f);
		DEBUGRENDERER2D->DrawDirection({ hoodBox.bottomLeft.x + hoodBox.width , hoodBox.bottomLeft.y + hoodBox.height }, { 0,-1 }, hoodBox.height, { 1,1,1 }, .4f);

		//render the squares
		for (size_t index = 0; index < cellIndexVec.size(); index++)
		{
			DEBUGRENDERER2D->DrawDirection({ m_Cells[cellIndexVec[index]].boundingBox.bottomLeft.x , m_Cells[cellIndexVec[index]].boundingBox.bottomLeft.y },
				{ 1,0 }, m_Cells[cellIndexVec[index]].boundingBox.width, { 1,1,0 }, .1f);
			DEBUGRENDERER2D->DrawDirection({ m_Cells[cellIndexVec[index]].boundingBox.bottomLeft.x , m_Cells[cellIndexVec[index]].boundingBox.bottomLeft.y },
				{ 0,1 }, m_Cells[cellIndexVec[index]].boundingBox.height, { 1,1,0 }, .1f);
			DEBUGRENDERER2D->DrawDirection({ m_Cells[cellIndexVec[index]].boundingBox.bottomLeft.x + m_Cells[cellIndexVec[index]].boundingBox.width ,
				m_Cells[cellIndexVec[index]].boundingBox.bottomLeft.y + m_Cells[cellIndexVec[index]].boundingBox.height }, { -1,0 }, m_Cells[cellIndexVec[index]].boundingBox.width, { 1,1,0 }, .1f);
			DEBUGRENDERER2D->DrawDirection({ m_Cells[cellIndexVec[index]].boundingBox.bottomLeft.x + m_Cells[cellIndexVec[index]].boundingBox.width ,
				m_Cells[cellIndexVec[index]].boundingBox.bottomLeft.y + m_Cells[cellIndexVec[index]].boundingBox.height }, { 0,-1 }, m_Cells[cellIndexVec[index]].boundingBox.height, { 1,1,0 }, .1f);
		}

	}
}

void CellSpace::RenderCells() const
{

	float tempheight{  };
	float tempwidth{ };
	int index{};
	for (int r = 0; r < m_NrOfRows; r++)
	{
		for (int c = 0; c < m_NrOfCols; c++)
		{
			if (r == 0)
			{
				//vertical debug lines
				DEBUGRENDERER2D->DrawDirection({ tempwidth + m_CellWidth,0 }, { 0,1 }, m_SpaceHeight, { .7f,.7f,.7f }, .9f);
			}

			DEBUGRENDERER2D->DrawString({ (c * m_CellWidth) + (m_CellWidth / 4) ,(r * m_CellHeight) + (m_CellHeight / 2) },
				std::to_string(m_Cells[c + (r * m_NrOfRows)].agents.size()).c_str());

			tempwidth += m_CellWidth;
			index++;
		}
		//horizontal debug lines
		DEBUGRENDERER2D->DrawDirection({ 0,tempheight + m_CellHeight }, { 1,0 }, m_SpaceWidth, { .7f,.7f,.7f }, .9f);
		tempheight += m_CellHeight;
	}



}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{
	int index{};
	Elite::Vector2 tempPos{ pos };
	if (pos.x >= m_SpaceWidth) { tempPos.x = m_SpaceWidth - 1; }
	if (pos.x < 0.f) { tempPos.x = 0.f; }
	if (pos.y >= m_SpaceHeight) { tempPos.y = m_SpaceHeight - 1; }
	if (pos.y < 0.f) { tempPos.y = 0.f; }

	index = ((int(tempPos.x / m_CellWidth)) % m_NrOfCols) + ((int(tempPos.y / m_CellHeight)) * m_NrOfRows);

	//(i % m_NrOfCols)* m_CellWidth, (i / m_NrOfRows)* m_CellHeight

	return index;
}

Elite::Rect CellSpace::GetRectFromPosRadius(const Elite::Vector2& centerPoint, float queryRadius) const
{
	//bottom left of the rectangle is the center position of the circle - radius
	Elite::Rect rect{ {centerPoint.x - queryRadius,centerPoint.y - queryRadius},queryRadius * 2,queryRadius * 2 };
	return rect;
}
