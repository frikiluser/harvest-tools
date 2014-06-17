#ifndef harvest_PhylogenyTree
#define harvest_PhylogenyTree

#include <vector>
#include <iostream>

#include "harvest/pb/harvest.pb.h"
#include "harvest/PhylogenyTreeNode.h"
#include "harvest/TrackList.h"

class PhylogenyTree
{
public:
	
	PhylogenyTree();
	~PhylogenyTree();
	
	const PhylogenyTreeNode * getLeaf(int id) const;
	void getLeafIds(std::vector<int> & ids) const;
	int getNodeCount() const;
	void initFromNewick(const char * file, TrackList * trackList);
	void initFromProtocolBuffer(const Harvest::Tree & msg);
	float leafDistance(int leaf1, int leaf2) const;
	void midpointReroot();
	void setOutgroup(const PhylogenyTreeNode * node);
	void writeToNewick(std::ostream &out, const TrackList & trackList) const;
	void writeToProtocolBuffer(Harvest * msg) const;
	
	PhylogenyTreeNode * getRoot() const;
	
private:
	
	void reroot(const PhylogenyTreeNode * rootNew, float distance, bool reorder = false);
	
	std::vector<const PhylogenyTreeNode *> leaves;
	PhylogenyTreeNode * root;
	int nodeCount;
};

inline const PhylogenyTreeNode * PhylogenyTree::getLeaf(int id) const {return leaves[id];}
inline int PhylogenyTree::getNodeCount() const {return nodeCount;}
inline PhylogenyTreeNode * PhylogenyTree::getRoot() const {return this->root;}

#endif