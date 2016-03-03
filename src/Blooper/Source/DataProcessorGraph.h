/*
  ==============================================================================

    DataProcessorGraph.h
    Created: 3 Mar 2016 1:52:48pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef DATAPROCESSORGRAPH_H_INCLUDED
#define DATAPROCESSORGRAPH_H_INCLUDED

#include <JuceHeader.h>
#include "DataProcessor.h"

class DataProcessorGraph
{
	
public:
	DataProcessorGraph();
	~DataProcessorGraph();

	class  Node : public ReferenceCountedObject
	{
	public:
		
		const uint32 nodeId;
		DataProcessor * getProcessor() const noexcept { return processor; }

		DataProcessor::DataType getInputDataType(String dataName, String componentName)
		{
			return processor->getInputDataType(dataName, componentName);
		}

		DataProcessor::DataType getOutputDataType(String dataName, String componentName)
		{
			return processor->getOutputDataType(dataName, componentName);
		}
		
		//Used in parent node containing both data and audio node
		//NamedValueSet properties;

		//==============================================================================
		/** A convenient typedef for referring to a pointer to a node object. */
		typedef ReferenceCountedObjectPtr<Node> Ptr;

	private:
		//==============================================================================
		friend class DataProcessorGraph;

		const ScopedPointer<DataProcessor> processor;

		Node(uint32 nodeId, DataProcessor*) noexcept;

		void setParentGraph(DataProcessorGraph*) const;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Node)
	};

	//==============================================================================
	/** Represents a connection between two channels of two nodes in an AudioProcessorGraph.

	To create a connection, use AudioProcessorGraph::addConnection().
	*/
	struct JUCE_API  Connection
	{
	public:
		//==============================================================================
		Connection(uint32 sourceNodeId, String sourceDataName, String sourceComponentName,
			uint32 destNodeId, String destDataName, String destComponentName) noexcept;

		uint32 sourceNodeId;


		String sourceDataName;
		String sourceComponentName;

		uint32 destNodeId;

		String destDataName;
		String destComponentName;

	private:
		//==============================================================================
		JUCE_LEAK_DETECTOR(Connection)
	};


	void clear();

	int getNumNodes() const noexcept { return nodes.size(); }


	Node* getNode(const int index) const noexcept { return nodes[index]; }

	Node* getNodeForId(const uint32 nodeId) const;


	Node* addNode(DataProcessor* newProcessor, uint32 nodeId = 0);


	bool removeNode(uint32 nodeId);


	int getNumConnections() const { return connections.size(); }


	const Connection* getConnection(int index) const { return connections[index]; }


	const Connection* getConnectionBetween(uint32 sourceNodeId,
		String sourceDataName,
		String sourceComponentName,
		uint32 destNodeId,
		String destDataName,
		String destComponentName) const;


	bool isConnected(uint32 possibleSourceNodeId,
		uint32 possibleDestNodeId) const;

	bool canConnect(uint32 sourceNodeId,
		String sourceDataName,
		String sourceComponentName,
		uint32 destNodeId,
		String destDataName,
		String destComponentName) const;


	bool addConnection(uint32 sourceNodeId,
		String sourceDataName,
		String sourceComponentName,
		uint32 destNodeId,
		String destDataName,
		String destComponentName);

	void removeConnection(int index);


	bool removeConnection(uint32 sourceNodeId,
		String sourceDataName,
		String sourceComponentName,
		uint32 destNodeId,
		String destDataName,
		String destComponentName);


	bool disconnectNode(uint32 nodeId);

	private:
		ReferenceCountedArray<Node> nodes;
		OwnedArray<Connection> connections;
		uint32 lastNodeId;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DataProcessorGraph)
};


#endif  // DATAPROCESSORGRAPH_H_INCLUDED
