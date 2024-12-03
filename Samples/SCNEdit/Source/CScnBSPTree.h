#ifndef CSCNBSPTREE_H_
#define CSCNBSPTREE_H_
#include "scntypes.h"
#include "util.h"
//Not even sure if this is needed.
class CScnBSPTree
{
public:
    u32 n_nodes;
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

      //  SAY("done.\n");
        return n_nodes;
    }

    ///gives node in wich point is located)
    u32 findNodePos(point3f pos) {    return findNodePos(pos.x, pos.y, pos.z); }
    u32 findNodePos(f32 x, f32 y, f32 z)
    {
        u16 idx=0;
        while (idx < n_nodes)
        {
            if (nodes[idx].plane == -1){  //it's a leaf, no more nodes
               break;
            }
            //TODO: use recursive function
            s32 eval = evalNodePos(idx,x,y,z);

            if (eval == 0)
                return -1;
                // error(true, "findNodePos: eval=0, what to do now?");
            else if (eval>0)
                idx=nodes[idx].node1;
            else if (eval<0)
                idx=nodes[idx].node2;
        }
        return idx;
    }



    ///returns 1 if pos is in front of splitting plane, -1 if behind, 0 if in plane
    s32 evalNodePos(int nodeidx, f32 x, f32 y, f32 z)
    {
        scnPlane_t * plane = &planes[nodes[nodeidx].plane];
        //dot product
        f32 prod= x * plane->a + y * plane->b + z * plane->c + plane->d;
        if (prod==0)     return 0;
        else if (prod>0) return 1;
        else             return -1;

    }


};

#endif



