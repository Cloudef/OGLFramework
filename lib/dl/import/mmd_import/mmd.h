#ifndef MMD_IMPORT_H
#define MMD_IMPORT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* header */
typedef struct mmd_header_t
{
   float version;
   char  name[20];
   char  comment[256];
} mmd_header;

/* texture */
typedef struct mmd_texture_t
{
   char file[20];
} mmd_texture;

/* bone */
typedef struct mmd_bone_t
{
   /* bone name */
   char name[20];

   /* type */
   uint8_t type;

   /* indices */
   unsigned short parent_bone_index;
   unsigned short tail_pos_bone_index;
   unsigned short ik_parend_bone_index;

   /* head position */
   float head_pos[3];
} mmd_bone;

/* IK */
typedef struct mmd_ik_t
{
   /* chain length */
   uint8_t  chain_length;

   /* indices */
   unsigned short bone_index;
   unsigned short target_bone_index;
   unsigned short iterations;

   /* cotrol weight */
   float cotrol_weight;

   /* child bone indices */
   unsigned short *child_bone_index;
} mmd_ik;

/* bone name */
typedef struct mmd_bone_name_t
{
   /* bone name */
   char name[50];
} mmd_bone_name;

/* Skin vertices */
typedef struct mmd_skin_vertices_t
{
   /* index */
   unsigned int index;

   /* translation */
   float translation[3];
} mmd_skin_vertices;

/* Skin */
typedef struct mmd_skin_t
{
   /* skin name */
   char name[20];

   /* vertices on this skin */
   unsigned int  num_vertices;

   /* skin type */
   uint8_t type;

   /* vertices */
   mmd_skin_vertices *vertices;
} mmd_skin;

/* all data */
typedef struct mmd_data_t
{
   /* count */
   unsigned int   num_vertices;
   unsigned int   num_indices;
   unsigned int   num_materials;
   unsigned short num_bones;
   unsigned short num_ik;
   unsigned short num_skins;
   uint8_t        num_skin_displays;
   uint8_t        num_bone_names;

   /* vertex */
   float          *vertices;
   float          *normals;
   float          *coords;

   /* index */
   unsigned short *indices;

   /* bone */
   unsigned int   *bone_indices;
   uint8_t        *bone_weight;
   uint8_t        *edge_flag;

   /* bone && ik arrays */
   mmd_bone       *bones;
   mmd_ik         *ik;
   mmd_bone_name  *bone_name;

   /* skin array */
   mmd_skin       *skin;
   unsigned int   *skin_display;

   /* material */
   float          *diffuse;
   float          *specular;
   float          *ambient;
   float          *alpha;
   float          *power;
   uint8_t        *toon;
   uint8_t        *edge;
   unsigned int   *face;
   mmd_texture    *texture;

} mmd_data;

/* allocate new mmd_data structure
 * which holds all vertices,
 * indices, materials and etc. */
mmd_data* newMMD(void);

/* frees the MMD structure */
void freeMMD( mmd_data* );

/* 1 - read header from MMD file */
int mmd_readHeader( FILE*, mmd_header* );

/* 2 - read vertex data from MMD file */
int mmd_readVertexData( FILE*, mmd_data* );

/* 3 - read index data from MMD file */
int mmd_readIndexData( FILE*, mmd_data* );

/* 4 - read material data from MMD file */
int mmd_readMaterialData( FILE*, mmd_data* );

/* 5 - read bone data from MMD file */
int mmd_readBoneData( FILE*, mmd_data* );

/* 6 - read IK data from MMD file */
int mmd_readIKData( FILE *f, mmd_data *mmd );

/* 7 - read Skin data from MMD file */
int mmd_readSkinData( FILE *f, mmd_data *mmd );

/* 8 - read Skin display data from MMD file */
int mmd_readSkinDisplayData( FILE *f, mmd_data *mmd );

/* 9 - read bone name data from MMD file */
int mmd_readBoneNameData( FILE *f, mmd_data *mmd );

/* EXAMPLE:
 *
 * FILE *f;
 * mmd_header header;
 * mmd_data   *mmd;
 *
 * unsigned int i;
 *
 * f = fopen( "HatsuneMiku.pmd", "rb" );
 * if(!f)
 *    exit( EXIT_FAILURE );
 *
 * if( readHeader( f, &header ) != 0 )
 *    exit( EXIT_FAILURE );
 *
 * // SJIS encoded, so probably garbage
 * puts( header.name );
 * puts( header.comment );
 *
 * mmd = newMMD();
 * if(!mmd)
 *    exit( EXIT_FAILURE );
 *
 * if( readVertexData( f, mmd ) != 0 )
 *    exit( EXIT_FAILURE );
 *
 * if( readIndexData( f, mmd ) != 0 )
 *    exit( EXIT_FAILURE );
 *
 * if( readMaterialData( f, mmd ) != 0 )
 *    exit( EXIT_FAILURE );
 *
 * fclose( f );
 *
 * // there are many ways you could handle storing or rendering MMD object
 * // which has many materials and only one set of texture coordinates.
 * //
 * // 1. create one VBO for whole object and one IBO for each material
 * // 2. create atlas texture and then batch everything into one VBO and IBO
 * // 3. split into small VBO's and IBO's
 * // 4. render using shaders and do material reset
 * //
 * // I haven't yet tested rendering using shaders, but blender can do this.
 * // In my tests the fastest results can be archieved with option 1,
 * // however in real use scenario, I would say option 2 is going to be faster
 * // when you have lots of stuff in screen.
 *
 * // Atlas and texture packing is quite big chunk of code so
 * // I'm not gonna make example out of that.
 * // Option 1 approach below :
 *
 * // Pack single big VBO here
 * i = 0;
 * for(; i != mmd->num_vertices; ++i)
 * {
 *    // handle vertices,
 *    // coords and normals here
 *
 *    // depending implentation
 *    // you can also handle indices
 *    // on num_indices loop
 *
 *    // mmd->vertices[ i * 3 ];
 *    // mmd->normals[  i * 3 ];
 *    // mmd->coords[   i * 2 ];
 * }
 *
 * // Split into small IBOs here
 * i = 0;
 * for(; i != mmd->num_materials; ++i)
 * {
 *    // this is texture filename
 *    // mmd->texture[i].file;
 *
 *    // split each material
 *    // into child object
 *    // and give it its own indices
 *    // but share the same VBO
 *
 *    // you could probably
 *    // avoid splitting
 *    // with using some material reset
 *    // technique and shaders
 *
 *    // it's not that costly to share
 *    // one VBO and have multiple IBO's
 *    // tho
 *
 *    num_faces = mmd->face[i];
 *    i2 = start;
 *    for(; i2 != start + num_faces; ++i2)
 *    {
 *       // loop for current material's
 *       // indices
 *       // mmd->indices[ i2 ];
 *    }
 *    start += num_faces;
 * }
 *
 * freeMMD( mmd );
 *
 */

#ifdef __cplusplus
}
#endif

#endif /* MMD_IMPORT_H */

/* EoF */
