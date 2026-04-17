#ifndef CBSPTREE_H_
#define CBSPTREE_H_
#include "scntypes.h"

//Not even sure if this is needed.
class CBSPTree
{
public:
	core::array<node_t*> nodes;

	///destructor
	inline ~CBSPTree()
	{
		nodes.clear();
	}

	inline u16 findNodePos(core::array<plane_t*> planes, core::vector3df pos, u16 nodeindx = 0)
	{
;
		if (nodes[nodeindx]->plane == -1)   //it's a leaf, no more nodes
			return nodeindx;
		else {
			s32 eval = evalNodePos(planes, nodeindx, pos);
			nodeindx = (eval >= 0) ? findNodePos(planes, pos, nodes[nodeindx]->node1) :
				findNodePos(planes, pos, nodes[nodeindx]->node2);
			return nodeindx;
		}
	}

	///returns 1 if pos is in front of splitting plane, -1 if behind, 0 if in plane
	inline s32 evalNodePos(core::array<plane_t*> planes, int nodeidx, core::vector3df pos)
	{
		plane_t* plane = planes[nodes[nodeidx]->plane];
		//dot product
		f32 prod = pos.X * plane->a + pos.Y * plane->b + pos.Z * plane->c + plane->d;
		f32 epsilon = 0.001f; // Small margin of error

		if (prod > epsilon) return 1;       // Clearly Front
		else if (prod < -epsilon) return -1; // Clearly Back
		else return 0;                     // "On" the plane
	}
};

#endif
