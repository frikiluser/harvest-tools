
#ifndef HarvestIO_h
#define HarvestIO_h

#include "harvest/pb/harvest.pb.h"
#include <string>
#include <map>
#include <vector>

#include "harvest/ReferenceList.h"
#include "harvest/AnnotationList.h"
#include "harvest/PhylogenyTree.h"
#include "harvest/LcbList.h"
#include "harvest/VariantList.h"

class HarvestIO
{
public:

	HarvestIO();
	
	void loadBed(const char * file, const char * name, const char * desc);
	void loadFasta(const char * file);
	void loadGenbank(const char * file);
	bool loadHarvest(const char * file);
	void loadMFA(const char * file, bool findVariants, int * trackIndecesNew);
	void loadNewick(const char * file);
	void loadVcf(const char * file);
	void loadXmfa(const char * file, bool findVariants, int * trackIndecesNew);
	
	void writeHarvest(const char * file);
	void writeNewick(std::ostream &out) const;
	void writeSnp(std::ostream &out, bool indels = false) const;
	void writeVcf(std::ostream &out, bool indels = false) const;
	void writeXmfa(std::ostream &out, bool split = false) const;
	void writeBackbone(std::ostream &out) const;
	
	Harvest harvest;
	ReferenceList referenceList;
	AnnotationList annotationList;
	PhylogenyTree phylogenyTree;
	TrackList trackList;
	LcbList lcbList;
	VariantList variantList;
	
private:
	
	void writeNewickNode(std::ostream &out, const Harvest::Tree::Node & msg) const;
};

#endif