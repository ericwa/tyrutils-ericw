/*
    Copyright (C) 1996-1997  Id Software, Inc.
    Copyright (C) 1997       Greg Lewis

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

    See file, 'COPYING', for details.
*/

#ifndef QBSP_MAP_HH
#define QBSP_MAP_HH

typedef struct epair_s {
    struct epair_s *next;
    char *key;
    char *value;
} epair_t;

typedef struct mapface_s {
    qbsp_plane_t plane;
    vec3_t planepts[3];
    std::string texname;
    int texinfo;
    int linenum;
    
    bool set_planepts(const vec3_t *pts);
} mapface_t;

enum class brushformat_t {
    NORMAL, BRUSH_PRIMITIVES
};
    
class mapbrush_t {
public:
    int firstface;
    int numfaces;
    brushformat_t format;
    
    mapbrush_t() : firstface(0), numfaces(0), format(brushformat_t::NORMAL) {}
    const mapface_t &face(int i) const;
} ;

struct lumpdata {
    int count;
    int index;
    void *data;
};

typedef struct mapentity_s {
    vec3_t origin;

    int firstmapbrush;
    int nummapbrushes;
    
    // Temporary lists used to build `brushes` in the correct order.
    brush_t *solid, *sky, *detail, *detail_illusionary, *detail_fence, *liquid;
    
    epair_t *epairs;
    vec3_t mins, maxs;
    brush_t *brushes;           /* NULL terminated list */
    int numbrushes;
    struct lumpdata lumps[BSPX_LUMPS];
    
    const mapbrush_t &mapbrush(int i) const;
} mapentity_t;

typedef struct mapdata_s {
    /* Arrays of actual items */
    std::vector<mapface_t> faces;
    std::vector<mapbrush_t> brushes;
    std::vector<mapentity_t> entities;
    std::vector<qbsp_plane_t> planes;
    std::vector<miptex_t> miptex;
    std::vector<mtexinfo_t> mtexinfos;
    
    /* quick lookup for texinfo */
    std::map<mtexinfo_t, int> mtexinfo_lookup;
    
    /* map from plane hash code to list of indicies in `planes` vector */
    std::unordered_map<int, std::vector<int>> planehash;
    
    /* Number of items currently used */
    int numfaces() const { return faces.size(); };
    int numbrushes() const { return brushes.size(); };
    int numentities() const { return entities.size(); };
    int numplanes() const { return planes.size(); };
    int nummiptex() const { return miptex.size(); };
    int numtexinfo() const { return static_cast<int>(mtexinfos.size()); };

    /* Totals for BSP data items -> TODO: move to a bspdata struct? */
    int cTotal[BSPX_LUMPS];

    /* Misc other global state for the compile process */
    int fillmark;       /* For marking leaves while outside filling */
    bool leakfile;      /* Flag once we've written a leak (.por/.pts) file */
    
    // helpers
    std::string texinfoTextureName(int texinfo) const {
        int mt = mtexinfos.at(texinfo).miptex;
        return miptex.at(mt);
    }
} mapdata_t;

extern mapdata_t map;
extern mapentity_t *pWorldEnt();

void EnsureTexturesLoaded();
bool IsWorldBrushEntity(const mapentity_t *entity);
void LoadMapFile(void);
void ConvertMapFile(void);

int FindMiptex(const char *name);
int FindTexinfo(mtexinfo_t *texinfo, uint64_t flags); //FIXME: Make this take const texinfo
int FindTexinfoEnt(mtexinfo_t *texinfo, mapentity_t *entity); //FIXME: Make this take const texinfo

void PrintEntity(const mapentity_t *entity);
const char *ValueForKey(const mapentity_t *entity, const char *key);
void SetKeyValue(mapentity_t *entity, const char *key, const char *value);
void GetVectorForKey(const mapentity_t *entity, const char *szKey, vec3_t vec);

void WriteEntitiesToString(void);

void FixRotateOrigin(mapentity_t *entity);

/* Create BSP brushes from map brushes in src and save into dst */
void Brush_LoadEntity(mapentity_t *dst, const mapentity_t *src,
                      const int hullnum);

/* Builds the dst->brushes list. Call after Brush_LoadEntity. */
void Entity_SortBrushes(mapentity_t *dst);


surface_t *CSGFaces(const mapentity_t *entity);
int PortalizeWorld(const mapentity_t *entity, node_t *headnode, const int hullnum);
void TJunc(const mapentity_t *entity, node_t *headnode);
node_t *SolidBSP(const mapentity_t *entity, surface_t *surfhead, bool midsplit);
int MakeFaceEdges(mapentity_t *entity, node_t *headnode);
void ExportClipNodes(mapentity_t *entity, node_t *headnode, const int hullnum);
void ExportDrawNodes(mapentity_t *entity, node_t *headnode, int firstface);

struct bspxbrushes_s
{
        byte *lumpinfo;
        size_t lumpsize;
        size_t lumpmaxsize;
};
void BSPX_Brushes_Finalize(struct bspxbrushes_s *ctx);
void BSPX_Brushes_Init(struct bspxbrushes_s *ctx);
void BSPX_Brushes_AddModel(struct bspxbrushes_s *ctx, int modelnum, brush_t *brushes);

void ExportObj(const surface_t *surfaces);

#endif
