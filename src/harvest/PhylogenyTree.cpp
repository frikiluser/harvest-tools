// Copyright © 2014, Battelle National Biodefense Institute (BNBI);
// all rights reserved. Authored by: Brian Ondov, Todd Treangen, and
// Adam Phillippy
//
// See the LICENSE.txt file included with this software for license information.

#include "PhylogenyTree.h"
#include <fstream>

using namespace::std;

PhylogenyTree::PhylogenyTree()
{
	root = 0;
	mult = 1.0;
}

PhylogenyTree::~PhylogenyTree()
{
	if ( root )
	{
		delete root;
	}
}

void PhylogenyTree::clear()
{
	leaves.clear();
	mult = 1;
	
	if ( root )
	{
		delete root;
		root = 0;
	}
}

const PhylogenyTreeNode * PhylogenyTree::getLca(int track1, int track2) const
{
	const PhylogenyTreeNode * node1;
	const PhylogenyTreeNode * node2;
	
	for ( int i = 0; i < leaves.size(); i++ )
	{
		if ( leaves[i]->getTrackId() == track1 )
		{
			node1 = leaves[i];
		}
		
		if ( leaves[i]->getTrackId() == track2 )
		{
			node2 = leaves[i];
		}
	}
	
	while ( node1 && node1->getAncestors() > node2->getAncestors() )
	{
		node1 = node1->getParent();
	}
	
	while ( node2 && node2->getAncestors() > node1->getAncestors() )
	{
		node2 = node2->getParent();
	}
	
	while ( node1 && node2 && node1 != node2 )
	{
		node1 = node1->getParent();
		node2 = node2->getParent();
	}
	
	if ( ! node1 || ! node2 )
	{
		cout << "ERROR: could not get LCA for tracks " << track1 << " and " << track2 << "." << endl;
		exit(1);
	}
	
	return node1;
}

void PhylogenyTree::getLeafIds(vector<int> & ids) const
{
	ids.resize(0);
	root->getLeafIds(ids);
}

void PhylogenyTree::init()
{
	int leaf = 0;
	
	nodeCount = 0;
	root->initialize(nodeCount, leaf);
	leaves.resize(0);
	root->getLeaves(leaves);
}

void PhylogenyTree::initFromCapnp(const capnp::Harvest::Reader & harvestReader)
{
	if ( root )
	{
		delete root;
	}
	
	int leaf = 0;
	auto treeReader = harvestReader.getTree();
	root = new PhylogenyTreeNode(treeReader.getRoot());
	
	if ( treeReader.getMultiplier() )
	{
		mult = treeReader.getMultiplier();
	}
	else
	{
		mult = 1.0;
	}
	
	init();
}

void PhylogenyTree::initFromNewick(const char * file, TrackList * trackList)
{
	if ( root )
	{
		delete root;
	}
	
	ifstream in(file);
	char * line = new char[1 << 20];
	
	bool useNames = trackList->getTrackCount() == 0;
	
	in.getline(line, (1 << 20) - 1, ';');
	char * token = line;
	
	try
	{
		root = new PhylogenyTreeNode(token, trackList, useNames);
	}
	catch ( const TrackList::TrackNotFoundException & e )
	{
		root = 0;
		throw;
	}
	
	delete [] line;
	in.close();
	init();
}


void PhylogenyTree::initFromProtocolBuffer(const Harvest::Tree & msg)
{
	if ( root )
	{
		delete root;
	}
	
	int leaf = 0;
	root = new PhylogenyTreeNode(msg.root());
	
	if ( msg.has_multiplier() )
	{
		mult = msg.multiplier();
	}
	else
	{
		mult = 1.0;
	}
	
	init();
}


float PhylogenyTree::leafDistance(int leaf1, int leaf2) const
{
	const PhylogenyTreeNode * node1 = leaves[leaf1];
	const PhylogenyTreeNode * node2 = leaves[leaf2];
	
	float distance = 0;
	
	while ( node1 != node2 )
	{
		if ( node1->getDepth() > node2->getDepth() )
		{
			distance += node1->getDistance();
			node1 = node1->getParent();
		}
		else if ( node2->getDepth() > node1->getDepth() )
		{
			distance += node2->getDistance();
			node2 = node2->getParent();
		}
		else
		{
			distance += node1->getDistance();
			distance += node2->getDistance();
			node1 = node1->getParent();
			node2 = node2->getParent();
		}
	}
	
	return distance;
}

void PhylogenyTree::midpointReroot()
{
	// lower triangular matrix of pairwise distances between leaves
	//
	int leavesCount = leaves.size();
	float ** distance = new float*[leavesCount - 1];
	
	for ( int i = 0; i < leavesCount - 1; i++ )
	{
		distance[i] = new float[i + 1];
		memset(distance[i], 0, sizeof(float) * (i + 1));
	}
	
	root->getPairwiseDistances(distance, leavesCount);
	
	float max = 0;
	int maxLeaf1;
	int maxLeaf2;
	
	for ( int i = 0; i < leavesCount - 1; i++ )
	{
		for ( int j = 0; j < i + 1; j++ )
		{
			if ( distance[i][j] > max )
			{
				max = distance[i][j];
				maxLeaf1 = i + 1;
				maxLeaf2 = j;
			}
		}
	}
	
	float midDistance = distance[maxLeaf1 - 1][maxLeaf2] / 2;
	
	for ( int i = 0; i < leavesCount - 1; i++ )
	{
		delete [] distance[i];
	}
	
	delete [] distance;
	
	const PhylogenyTreeNode * node;
	
	if ( leaves[maxLeaf1]->getDepth() > leaves[maxLeaf2]->getDepth() )
	{
		node = leaves[maxLeaf1];
	}
	else
	{
		node = leaves[maxLeaf2];
	}
	
	float depth = 0;
	
	while ( depth + node->getDistance() < midDistance && node->getParent() )
	{
		depth += node->getDistance();
		node = node->getParent();
	}
	
	if ( node != root )
	{
		reroot(node, midDistance - depth);
	}
}

void PhylogenyTree::setOutgroup(const PhylogenyTreeNode * node)
{
	reroot(node, node->getParent() == root ? (root->getChild(0)->getDistance() + root->getChild(1)->getDistance()) / 2 : node->getDistance() / 2, true);
}

void PhylogenyTree::setTrackIndeces(int * trackIndecesNew)
{
	for ( int i = 0; i < leaves.size(); i++ )
	{
		leaves[i]->setTrackId(trackIndecesNew[leaves[i]->getTrackId()]);
	}
}

void PhylogenyTree::reroot(const PhylogenyTreeNode * rootNew, float distance, bool reorder)
{
	int leaf = 0;
	nodeCount = 0;
	
	if ( rootNew->getParent() == root )
	{
		PhylogenyTreeNode * rootNewMutable;
		PhylogenyTreeNode * sibling;
		
		if ( root->getChild(0) == rootNew )
		{
			rootNewMutable = root->getChild(0);
			sibling = root->getChild(1);
		}
		else
		{
			sibling = root->getChild(0);
			rootNewMutable = root->getChild(1);
			
			if ( reorder )
			{
				root->swapSiblings();
			}
		}
		
		sibling->setParent(root, rootNew->getDistance() + sibling->getDistance() - distance);
		rootNewMutable->setParent(root, distance);
	}
	else
	{
		root = const_cast<PhylogenyTreeNode *>(rootNew)->bisectEdge(distance);
	}
	
	root->initialize(nodeCount, leaf);
	leaves.resize(0);
	root->getLeaves(leaves);
	//root->setAlignDist(root->getDistanceMax(), 0);
}

void PhylogenyTree::writeToCapnp(capnp::Harvest::Builder & harvestBuilder) const
{
	auto treeBuilder = harvestBuilder.initTree();
	treeBuilder.setMultiplier(mult);
	auto rootBuilder = treeBuilder.initRoot();
	root->writeToCapnp(rootBuilder);
}

void PhylogenyTree::writeToNewick(std::ostream &out, const TrackList & trackList, bool useMult) const
{
	root->writeToNewick(out, trackList, useMult ? mult : 1);
	out << ";\n";
}

void PhylogenyTree::writeToProtocolBuffer(Harvest * msg) const
{
	Harvest::Tree * msgTree = msg->mutable_tree();
	//save multiplier value to protobuf
	msgTree->set_multiplier(mult);
	root->writeToProtocolBuffer(msgTree->mutable_root());
}
