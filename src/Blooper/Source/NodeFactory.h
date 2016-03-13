/*
 ==============================================================================
 
 NodeFactory.h
 Created: 4 Mar 2016 10:56:58am
 Author:  bkupe
 
 ==============================================================================
 */

#ifndef NODEFACTORY_H_INCLUDED
#define NODEFACTORY_H_INCLUDED

#include "DummyNode.h"
#include "AudioMixerNode.h"
#include "LooperNode.h"
#include "SpatNode.h"
#include "VSTNode.h"
#include "AudioInNode.h"
#include "AudioOutNode.h"

class NodeManager;

class NodeFactory
{
public:
    enum NodeType
    {
        Dummy,
        AudioMixer,
        Spat,
        Looper,
        VST,
        AudioIn,
        AudioOut
    };
    
    NodeFactory()
    {
        
    }
    
    ~NodeFactory()
    {
        
    }
    
    NodeBase * createNode(NodeManager * nodeManager,NodeType nodeType, uint32 nodeId = 0)
    {
        NodeBase * n = nullptr;
        
        switch (nodeType)
        {
            case Dummy:
                n = new DummyNode(nodeManager,nodeId);
                break;
                
            case AudioMixer:
                n = new AudioMixerNode(nodeManager,nodeId);
                break;
            case Spat:
                n = new SpatNode(nodeManager,nodeId);
                break;
            case Looper:
                n = new LooperNode(nodeManager,nodeId);
                break;
            case VST:
                n = new VSTNode(nodeManager,nodeId);
                break;
                
            case AudioIn:
                n = new AudioInNode(nodeManager,nodeId);
                break;
                
            case AudioOut:
                n = new AudioOutNode(nodeManager,nodeId);
                break;
                
            default:
                jassert(false);
                break;
        }
        
        
        return n;
    }
    
    static PopupMenu * getNodeTypesMenu(int menuIdOffset = 0)
    {
        PopupMenu * p = new PopupMenu();
        const static String nodeTypeNames[] = { "Dummy","AudioMixer","Spat","Looper","VST","AudioIn","AudioOut" };
        for (int i = 0; i < numElementsInArray(nodeTypeNames);i++)
        {
            p->addItem(menuIdOffset + i+1, nodeTypeNames[i]);
        }
        
        return p;
    }
};


#endif  // NODEFACTORY_H_INCLUDED
