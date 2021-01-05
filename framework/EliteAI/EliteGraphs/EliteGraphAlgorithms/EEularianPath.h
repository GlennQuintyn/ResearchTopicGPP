#pragma once
#include <stack>

namespace Elite
{
	enum class Eulerianity
	{
		notEulerian,
		semiEulerian,
		eulerian,
	};

	template <class T_NodeType, class T_ConnectionType>
	class EulerianPath
	{
	public:

		EulerianPath(IGraph<T_NodeType, T_ConnectionType>* pGraph);

		Eulerianity IsEulerian() const;
		vector<T_NodeType*> FindPath(Eulerianity& eulerianity) const;

	private:
		void VisitAllNodesDFS(int startIdx, vector<bool>& visited) const;
		bool IsConnected() const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
	};

	template<class T_NodeType, class T_ConnectionType>
	inline EulerianPath<T_NodeType, T_ConnectionType>::EulerianPath(IGraph<T_NodeType, T_ConnectionType>* pGraph)
		: m_pGraph(pGraph)
	{
	}

	template<class T_NodeType, class T_ConnectionType>
	inline Eulerianity EulerianPath<T_NodeType, T_ConnectionType>::IsEulerian() const
	{
		// If the graph is not connected, there can be no Eulerian Trail
		if (IsConnected() == false)
		{
			return Eulerianity::notEulerian;
		}

		// Count nodes with odd degree 
		int nrOfNodes = m_pGraph->GetNrOfNodes();
		int nrOfActiveNodes = m_pGraph->GetNrOfActiveNodes();
		int oddCount{};
		for (int i = 0; i < nrOfNodes; i++)
		{
			if (m_pGraph->IsNodeValid(i) && (m_pGraph->GetNodeConnections(i).size()) & 1)//same as modulo %2
			{
				oddCount++;
			}
		}

		// A connected graph with more than 2 nodes with an odd degree (an odd amount of connections) is not Eulerian
		if (oddCount > 2)
		{
			return Eulerianity::notEulerian;
		}
		// A connected graph with exactly 2 nodes with an odd degree is Semi-Eulerian (an Euler trail can be made, but only starting and ending in these 2 nodes)
		else if (oddCount == 2 && nrOfActiveNodes != 2)
		{
			return Eulerianity::semiEulerian;
		}
		// A connected graph with no odd nodes is Eulerian
		return Eulerianity::eulerian;

	}

	template<class T_NodeType, class T_ConnectionType>
	inline vector<T_NodeType*> EulerianPath<T_NodeType, T_ConnectionType>::FindPath(Eulerianity& eulerianity) const
	{
		// Get a copy of the graph because this algorithm involves removing edges
		auto graphCopy = m_pGraph->Clone();
		int nrOfNodes = graphCopy->GetNrOfNodes();
		int nrOfActiveNodes = graphCopy->GetNrOfActiveNodes();

		auto path = vector<T_NodeType*>();
		vector<int> stackEulerien{};

		///
		///  algorithm...
		/// 
		switch (eulerianity)
		{
		case Elite::Eulerianity::notEulerian:
			return path;//return empty path
			break;
		case Elite::Eulerianity::semiEulerian:
			break;


		case Elite::Eulerianity::eulerian:
			// find a valid starting node that has connections
			int index = invalid_node_index;
			for (int i = 0; i < nrOfNodes; i++)
			{
				if (graphCopy->IsNodeValid(i))
				{
					if (graphCopy->GetNodeConnections(i).size() != 0)
					{
						index = i;
						break;
					}
					else
					{
						return path;//return empty path because there is no valid node to start on
					}
				}
			}

			bool done{};
			while (!done)
			{

				// check for next possible connection
				bool alone{};

				while (!alone)
				{
					alone = true;
					for (T_ConnectionType* connection : graphCopy->GetNodeConnections(index))
					{
						if (graphCopy->IsNodeValid(connection->GetTo()))
						{
							stackEulerien.push_back(index);
							index = connection->GetTo();
							graphCopy->RemoveConnection(connection);

							alone = false;
							break;
						}
					}


				}
				//REMOVE CONNECTION

				if (alone)
				{
					path.push_back(graphCopy->GetNode(index));
					index = stackEulerien.back();
					stackEulerien.pop_back();
					if (stackEulerien.size() == 0)
					{
						done = true;
					}
				}
			}




			break;
		}
		//check if all vertices hace an even degree or atleast 2 having odd degree otherwise no path
		//int nrOfNodes = m_pGraph->GetNrOfNodes();
		//int evenCount{}, oddCount{};
		//for (size_t i = 0; i < nrOfNodes; i++)
		//{
		//	if (m_pGraph->IsNodeValid(i) && (m_pGraph->GetNodeConnections(i).size() == 2))//checking if uneven en then inv 
		//	{
		//		evenCount++;
		//	}
		//	else if (m_pGraph->IsNodeValid(i) && (m_pGraph->GetNodeConnections(i).size()) & 1)//checking if uneven
		//	{
		//		oddCount++;
		//	}
		//}

		//if (evenCount == nrOfActiveNodes)
		//{

		//}
		//else if()
		//{

		//}


		eulerianity;




		return path;
	}

	template<class T_NodeType, class T_ConnectionType>
	inline void EulerianPath<T_NodeType, T_ConnectionType>::VisitAllNodesDFS(int startIdx, vector<bool>& visited) const
	{
		// mark the visited node
		visited[startIdx] = true;

		// recursively visit any valid connected nodes that were not visited before
		for (T_ConnectionType* connection : m_pGraph->GetNodeConnections(startIdx))
		{
			if (m_pGraph->IsNodeValid(connection->GetTo()) && !visited[connection->GetTo()])
			{
				VisitAllNodesDFS(connection->GetTo(), visited);
			}
		}
	}

	template<class T_NodeType, class T_ConnectionType>
	inline bool EulerianPath<T_NodeType, T_ConnectionType>::IsConnected() const
	{
		int nrOfNodes = m_pGraph->GetNrOfNodes();
		vector<bool> visited(nrOfNodes, false);

		// find a valid starting node that has connections
		int connectionIndex = invalid_node_index;
		for (int i = 0; i < nrOfNodes; i++)
		{
			if (m_pGraph->IsNodeValid(i))
			{
				if (m_pGraph->GetNodeConnections(i).size() != 0)
				{
					connectionIndex = i;
					break;
				}
				else
				{
					return false;
				}
			}
		}

		// if no valid node could be found, return false
		if (connectionIndex == invalid_node_index)
		{
			return false;
		}

		// start a depth-first-search traversal from a node that has connections
		VisitAllNodesDFS(connectionIndex, visited);

		// if a node was never visited, this graph is not connected
		for (int i = 0; i < nrOfNodes; i++)
		{
			if (m_pGraph->IsNodeValid(i) && visited[i] == false)
			{
				return false;
			}
		}
		return true;
	}
}