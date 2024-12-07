#if false

struct vertProp_t
{
	u32 vertidxidx;
	u32 surf_vertidx;
	bool bShared;
	core::array<u32> sharesWith;
};

class CSelector
{
private:
	


	CScn* scn;
	core::array<scene::SMesh*> scnMeshes;
	u32 totalSize;
	core::array <core::array<u32>>      redSurfs;
	core::array <core::array<u32>>      blueSurfs;
	core::array<i_point3f>                sharedVerts;
	core::array<i_point3f>                selVerts;
	core::stringw log;



	void clearRedSurfs();
	void clearBlueSurfs();
	video::ITexture* redtex;
	video::ITexture* bluetex;
	std::unordered_map<int,video::ITexture*> prevtexRed;
	std::unordered_map<int,video::ITexture*> prevtexBlue;
	std::unordered_map<int, video::E_MATERIAL_TYPE> prevtype;

	core::array<u32>  calcSharedSurfaces(u32,u32);
	core::array<u32>  calcSharedSurfaces(u32,core::array<u32>&);
	
   
	void makeSurfRed(u32,u32); void makeSurfRed(u32,core::array<u32>);
	void makeSurfBlue(u32,u32); void makeSurfBlue(u32,core::array<u32>);
	void makeSelectedVerts(u32);
	void remRedSurf(u32,u32);
	void remBlueSurf(u32,u32);
	void clearSelectedVerts();

public:
	enum E_PAINT_TYPE { RED, BLUE, CLEAR_RED, CLEAR_BLUE };
	CSelector(CScn*, core::array<scene::SMesh*>, u32, IrrlichtDevice*);
	~CSelector();

	core::array<u32>& getRedSurfs(u32 meshIndx) { return redSurfs[meshIndx]; }

	core::array<u32>& getBlueSurfs(u32 meshIndx) { return blueSurfs[meshIndx]; }

	core::array <i_point3f> getVerts() { return selVerts; }
	i_point3f getVert(u32 vIndx) { return selVerts[vIndx]; }
	void setVert(u32 vIndx, i_point3f vert) { selVerts[vIndx]= vert; }

	core::array <i_point3f> getSharedVerts() { return sharedVerts; }
	i_point3f getSharedVert(u32 vIndx) { return sharedVerts[vIndx]; }
	void setSharedVert(u32 vIndx, i_point3f vert) { sharedVerts[vIndx] = vert; }
	void clearVerts() {
		sharedVerts.set_used(0);
		selVerts.set_used(0);
	}
	void selectSurf(u32 meshIndx, s32 i, bool bAppend);
 
	void paint(u32, u32, E_PAINT_TYPE); void paint(u32, core::array<u32>, E_PAINT_TYPE);
	core::array<vertProp_t> getSurfVertProps(u32 meshIndx,u32 si);
	void clear();
	core::stringw getLog() {
		return log;
	}

};
#endif