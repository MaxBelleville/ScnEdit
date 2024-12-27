#ifndef CSCNBSPTREE_H_
#define CSCNBSPTREE_H_
#include "scntypes.h"
#include "util.h"
//Not even sure if this is needed.
class CScnBSPTree
{
public:
	u32 n_nodes;
	u32 n_cells=0;
	scnRawCell_t* rawcells;
	scnNode_t * nodes;
	scnPlane_t ** planes_ad;    //address to pointer to data, planes pointer is set only later
	scnPlane_t * planes; //also needs pointer to planes given from parent cscnsolid

	///constructor. Needs number of nodes
	CScnBSPTree(u32 n)
	{
		//because, at the time this is called, planes in CScnsolid hasn't been set
		//solid->plane point to nowhere, so we have to use a pointer to a pointer
		n_nodes=n;
		if (n>0)
			nodes = new scnNode_t[n];
		else
			nodes=0;
	}

	///destructor
	~CScnBSPTree()
	{
		if (nodes)
			delete[] nodes;
	}


	int loadNodes(std::ifstream * file)
	{
		if (n_nodes <= 0) return -1;

		read_generic(nodes, file,n_nodes*sizeof(scnNode_t));

	  //  os::Printer::log("done.\n");
		return n_nodes;
	}

	u16 findNodePos(core::vector3df pos, u16 nodeindx=0)
	{

		if (nodes[nodeindx].plane == -1) {  //it's a leaf, no more nodes

			return nodeindx;
		}
		else {
			s32 eval= evalNodePos(nodeindx, pos);
			

		  
			if (eval >= 0)
				nodeindx = findNodePos(pos, nodes[nodeindx].node1);
			else
				nodeindx = findNodePos(pos, nodes[nodeindx].node2);

			return nodeindx;
		}
	}
 

	///returns 1 if pos is in front of splitting plane, -1 if behind, 0 if in plane
	s32 evalNodePos(int nodeidx, core::vector3df pos)
	{
		scnPlane_t * plane = &planes[nodes[nodeidx].plane];
		//dot product
		f32 prod= pos.X * plane->a + pos.Y * plane->b + pos.Z * plane->c + plane->d;
		if (prod==0)     return 0;
		else if (prod>0) return 1;
		else             return -1;

	}


};

#endif



