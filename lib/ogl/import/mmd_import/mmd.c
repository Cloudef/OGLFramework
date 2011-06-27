#include <stdio.h>
#include <malloc.h>
#include <string.h>

/* structs reside here */
#include "mmd.h"

/* 0 = success */
typedef enum {
   RETURN_OK = 0, RETURN_FAIL, RETURN_NOTHING
} mmd_funcReturn;

/* only needed for printing comment and name as utf-8
 * in most cases uselsess.
 *
 * would be nice to have own sjis to utf-8 decoder
 * but it might bloat the code a lot */
#if ICONV_SJIS_PMD
   #include "iconv.h"
#endif

#define MAGIC_HEADER_SIZE     3
#define MAGIC_HEADER_STRING   "Pmd"

/* for reading */
#define SIZE_BYTE    1
#define SIZE_INTEGER 4
#define SIZE_FLOAT   4
#define SIZE_SHORT   2

/* TO-DO:
 * Bone data
 * IK data
 * Skin data
 * Skin display?
 * Bone names
 * Bone display?
 */

#if ICONV_SJIS_PMD
/* conversion */
void convertSjisToUtf8( char *sjis, char *utf8, size_t sjisSize )
{
   iconv_t icd;
   char *p_src = NULL, *p_dst = NULL;
   size_t n_src, n_dst;

   icd = iconv_open("UTF-8", "Shift_JIS");
   p_src = sjis;
   p_dst = utf8;
   n_src = sjisSize;
   n_dst = sjisSize;

   iconv(icd, &p_src, &n_src, &p_dst, &n_dst);
   iconv_close(icd);
}
#endif

/* Read PMD header */
int mmd_readHeader( FILE *f, mmd_header *header )
{
   char  sjis_name[20];
   char  sjis_comment[256];

   /* first read header
    * even that our import wrapper does this already,
    * we want to be sure about this since this code may be reused
    * and that we know if the import wrapper screwed up */

   char MAGIC_HEADER[ MAGIC_HEADER_SIZE ];
   if(!fread( &MAGIC_HEADER, SIZE_BYTE, MAGIC_HEADER_SIZE, f ))
      return( RETURN_FAIL );
   MAGIC_HEADER[ MAGIC_HEADER_SIZE ] = '\0';

   if( strcmp( MAGIC_HEADER, MAGIC_HEADER_STRING ) != 0)
      return( RETURN_FAIL );

   /* FLOAT: version */
   if(!fread( (void*)(&header->version), SIZE_FLOAT, 1, f ))
         return( RETURN_FAIL );

   /* SHIFT-JIS STRING: name */
   if(!fread( &sjis_name, SIZE_BYTE, 20, f ))
      return( RETURN_FAIL );

   /* SHIFT-JIS STRING: comment */
   if(!fread( &sjis_comment, SIZE_BYTE, 256, f))
      return( RETURN_FAIL );

#if ICONV_SJIS_PMD
   convertSjisToUtf8( sjis_name,    header->name,    20 );
   convertSjisToUtf8( sjis_comment, header->comment, 256 );
#else
   sprintf( header->name,    sjis_name );
   sprintf( header->comment, sjis_comment );
#endif

   return( RETURN_OK );
}

/* Read vertex data */
int mmd_readVertexData( FILE *f, mmd_data *mmd )
{
   unsigned int i;

   /* UNSIGNED INT: vertex count */
   if(!fread( (void*)(&mmd->num_vertices), SIZE_INTEGER, 1, f ))
      return( RETURN_FAIL );

   /* vertices */
   mmd->vertices     = malloc( mmd->num_vertices * 3 * sizeof(float) );
   if(!mmd->vertices)
      return( RETURN_FAIL );

   /* normals */
   mmd->normals      = malloc( mmd->num_vertices * 3 * sizeof(float) );
   if(!mmd->normals)
      return( RETURN_FAIL );

   /* coords */
   mmd->coords       = malloc( mmd->num_vertices * 2 * sizeof(float) );
   if(!mmd->coords)
      return( RETURN_FAIL );

   /* bone indices */
   mmd->bone_indices = malloc( mmd->num_vertices * sizeof(unsigned int) );
   if(!mmd->bone_indices)
      return( RETURN_FAIL );

   /* bone weights */
   mmd->bone_weight  = malloc( mmd->num_vertices * sizeof(uint8_t) );
   if(!mmd->bone_weight)
      return( RETURN_FAIL );

   /* some edge flag */
   mmd->edge_flag    = malloc( mmd->num_vertices * sizeof(uint8_t) );
   if(!mmd->edge_flag)
      return( RETURN_FAIL );

   i = 0;
   for(; i != mmd->num_vertices; ++i)
   {
      /* 3xFLOAT: vertex */
      if(!fread( (void*)(&mmd->vertices[ i * 3 ]), SIZE_FLOAT, 3, f ))
         return( RETURN_FAIL );

      /* 3xFLOAT: normal */
      if(!fread( (void*)(&mmd->normals[ i * 3 ]), SIZE_FLOAT, 3, f ))
         return( RETURN_FAIL );

      /* 3xFLOAT: texture coordinate */
      if(!fread( (void*)(&mmd->coords[ i * 2 ]), SIZE_FLOAT, 2, f ))
         return( RETURN_FAIL );

      /* UNSIGNED INT: bone indices */
      if(!fread( (void*)(&mmd->bone_indices[i]), SIZE_INTEGER, 1, f ))
         return( RETURN_FAIL );

      /* BYTE: bone weights */
      if(!fread( (void*)(&mmd->bone_weight[i]), SIZE_BYTE, 1, f ))
         return( RETURN_FAIL );

      /* BYTE: edge flag */
      if(!fread( (void*)(&mmd->edge_flag[i]), SIZE_BYTE, 1, f ))
         return( RETURN_FAIL );
   }

   return( RETURN_OK );
}

/* Read index data */
int mmd_readIndexData( FILE *f, mmd_data *mmd )
{
   /* UNSIGNED INT: index count */
   if(!fread( (void*)(&mmd->num_indices), SIZE_INTEGER, 1, f ))
      return( RETURN_FAIL );

   /* indices */
   mmd->indices = malloc( mmd->num_indices * sizeof(unsigned int) );
   if(!mmd->indices)
      return( RETURN_FAIL );

   /* UNSIGNED SHORT ARRAY: indices */
   if(!fread( (void*)(mmd->indices), SIZE_SHORT, mmd->num_indices, f ))
      return( RETURN_FAIL );

   return( RETURN_OK );
}

/* Read material data */
int mmd_readMaterialData( FILE *f, mmd_data *mmd )
{
   unsigned int i;

   /* UNSIGNED INT: material count */
   if(!fread( (void*)(&mmd->num_materials), SIZE_INTEGER, 1, f ))
      return( RETURN_FAIL );

   /* diffuse */
   mmd->diffuse      = malloc( mmd->num_materials * 3 * sizeof(float) );
   if(!mmd->diffuse)
      return( RETURN_FAIL );

   /* alpha */
   mmd->alpha        = malloc( mmd->num_materials * sizeof(float) );
   if(!mmd->alpha)
      return( RETURN_FAIL );

   /* pwoer */
   mmd->power        = malloc( mmd->num_materials * sizeof(float) );
   if(!mmd->power)
      return( RETURN_FAIL );

   /* specular */
   mmd->specular     = malloc( mmd->num_materials * 3 * sizeof(float) );
   if(!mmd->specular)
      return( RETURN_FAIL );

   /* ambient */
   mmd->ambient      = malloc( mmd->num_materials * 3 * sizeof(float) );
   if(!mmd->ambient)
      return( RETURN_FAIL );

   /* toon flag? */
   mmd->toon         = malloc( mmd->num_materials * sizeof(uint8_t));
   if(!mmd->toon)
      return( RETURN_FAIL );

   /* edge flag? maybe for normal fix? */
   mmd->edge         = malloc( mmd->num_materials * sizeof(uint8_t) );
   if(!mmd->edge)
      return( RETURN_FAIL );

   /* face index */
   mmd->face         = malloc( mmd->num_materials * sizeof(unsigned int) );
   if(!mmd->face)
      return( RETURN_FAIL );

   /* texture */
   mmd->texture      = malloc( mmd->num_materials * sizeof(mmd_texture) );
   if(!mmd->texture)
      return( RETURN_FAIL );

   i = 0;
   for(; i != mmd->num_materials; ++i)
   {
      /* 3xFLOAT: diffuse */
      if(!fread( (void*)(&mmd->diffuse[ i * 3 ]), SIZE_FLOAT, 3, f ))
         return( RETURN_FAIL );

      /* FLOAT: alpha */
      if(!fread( (void*)(&mmd->alpha[i]), SIZE_FLOAT, 1, f ))
         return( RETURN_FAIL );

      /* FLOAT: power */
      if(!fread( (void*)(&mmd->power[i]), SIZE_FLOAT, 1, f ))
         return( RETURN_FAIL );

      /* 3xFLOAT: specular */
      if(!fread( (void*)(&mmd->specular[ i * 3 ]), SIZE_FLOAT, 3, f ))
         return( RETURN_FAIL );

      /* 3xFLOAT: ambient */
      if(!fread( (void*)(&mmd->ambient[ i * 3 ]), SIZE_FLOAT, 3, f ))
         return( RETURN_FAIL );

      /* BYTE: toon flag */
      if(!fread( (void*)(&mmd->toon[i]), SIZE_BYTE, 1, f ))
         return( RETURN_FAIL );

      /* BUTE: edge flag */
      if(!fread( (void*)(&mmd->edge[i]), SIZE_BYTE, 1, f ))
         return( RETURN_FAIL );

      /* UNSIGNED INT: face indices */
      if(!fread( (void*)(&mmd->face[i]), SIZE_INTEGER, 1, f ))
         return( RETURN_FAIL );

      /* STRING: texture */
      if(!fread( (void*)(&mmd->texture[i]), SIZE_BYTE, 20, f ))
         return( RETURN_FAIL );
   }

   return( RETURN_OK );
}

/* Read bone data */
int mmd_readBoneData( FILE *f, mmd_data *mmd )
{
   unsigned int i;
   char name[20];

   /* UNSIGNED SHORT: bone count */
   if(!fread( (void*)(&mmd->num_bones), SIZE_SHORT, 1, f ))
      return( RETURN_FAIL );

   /* allocate bones */
   mmd->bones = malloc( mmd->num_bones * sizeof(mmd_bone) );
   if(!mmd->bones)
      return( RETURN_FAIL );

   i = 0;
   for(; i != mmd->num_bones; ++i)
   {
      /* SJIS STRING: bone name */
      if(!fread( name, 1, 20, f ))
         return( RETURN_FAIL );

      /* UNSIGNED SHORT: parent bone index */
      if(!fread( (void*)(&mmd->bones[i].parent_bone_index), SIZE_SHORT, 1, f ))
         return( RETURN_FAIL );

      /* UNSIGNED SHORT: tail bone index */
      if(!fread( (void*)(&mmd->bones[i].tail_pos_bone_index), SIZE_SHORT, 1, f ))
         return( RETURN_FAIL );

      /* BYTE: bone type */
      if(!fread( (void*)(&mmd->bones[i].type), SIZE_BYTE, 1, f ))
         return( RETURN_FAIL );

      /* UNSIGNED SHORT: parent bone index */
      if(!fread( (void*)(&mmd->bones[i].parent_bone_index), SIZE_SHORT, 1, f ))
         return( RETURN_FAIL );

       /* 3xFLOAT: head bone position */
      if(!fread( (void*)(&mmd->bones[i].head_pos), SIZE_FLOAT, 3, f ))
         return( RETURN_FAIL );

#if ICONV_SJIS_PMD
      convertSjisToUtf8( name, mmd->bones[i].name, 20 );
      //glPuts( mmd->bones[i].name );
#else
      sprintf( mmd->bones[i].name, name );
#endif
   }

   return( RETURN_OK );
}

/* Read IK data */
int mmd_readIKData( FILE *f, mmd_data *mmd )
{
   unsigned int i, i2;

   /* UNSIGNED SHORT: IK count */
   if(!fread( (void*)(&mmd->num_ik), SIZE_SHORT, 1, f ))
      return( RETURN_FAIL );

   /* alloc IKs */
   mmd->ik = malloc( mmd->num_ik * sizeof(mmd_ik) );
   if(!mmd->ik)
      return( RETURN_FAIL );

   /* null stuff first to be safe */
   i = 0;
   for(; i != mmd->num_ik; ++i)
      mmd->ik[i].child_bone_index = NULL;

   i = 0;
   for(; i != mmd->num_ik; ++i)
   {
      /* UNSIGNED SHORT: ik bone index */
      if(!fread( (void*)(&mmd->ik[i].bone_index), SIZE_SHORT, 1, f ))
         return( RETURN_FAIL );

      /* UNSIGNED SHORT: bone index */
      if(!fread( (void*)(&mmd->ik[i].target_bone_index), SIZE_SHORT, 1, f ))
         return( RETURN_FAIL );

      /* BYTE: chain length */
      if(!fread( (void*)(&mmd->ik[i].chain_length), SIZE_BYTE, 1, f ))
         return( RETURN_FAIL );

      /* UNSIGNED SHORT: iterations */
      if(!fread( (void*)(&mmd->ik[i].iterations), SIZE_SHORT, 1, f ))
         return( RETURN_FAIL );

      /* FLOAT: cotrol weight */
      if(!fread( (void*)(&mmd->ik[i].cotrol_weight), SIZE_FLOAT, 1, f ))
         return( RETURN_FAIL );

      /* alloc child bone idices */
      mmd->ik[i].child_bone_index = malloc( mmd->ik[i].chain_length * sizeof(unsigned short) );
      if(!mmd->ik[i].child_bone_index)
         return( RETURN_FAIL );

      i2 = 0;
      for(; i2 != mmd->ik[i].chain_length; ++i2)
      {
         /* UNSIGNED SHORT: child bone index */
         if(!fread( (void*)(&mmd->ik[i].child_bone_index[i2]), SIZE_SHORT, 1, f ))
            return( RETURN_FAIL );
      }
   }

   return( RETURN_OK );
}

/* Read Skin data */
int mmd_readSkinData( FILE *f, mmd_data *mmd )
{
   unsigned int i, i2;
   char name[20];

   /* UNSIGNED SHORT: Skin count */
   if(!fread( (void*)(&mmd->num_skins), SIZE_SHORT, 1, f ))
      return( RETURN_FAIL );

   /* alloc Skins */
   mmd->skin = malloc( mmd->num_skins * sizeof(mmd_skin) );
   if(!mmd->skin)
      return( RETURN_FAIL );

   i = 0;
   for(; i != mmd->num_skins; ++i)
   {
      /* SJIS STRING: skin name */
      if(!fread( name, 1, 20, f ))
         return( RETURN_FAIL );

      /* UNSIGNED INT: vertex count */
      if(!fread( (void*)(&mmd->skin[i].num_vertices), SIZE_INTEGER, 1, f ))
         return( RETURN_FAIL );

      /* BYTE. skin type */
      if(!fread( (void*)(&mmd->skin[i].type), SIZE_BYTE, 1, f ))
         return( RETURN_FAIL );

      /* alloc Skin data */
      mmd->skin[i].vertices = malloc( mmd->skin[i].num_vertices * sizeof(mmd_skin_vertices) );
      if(!mmd->skin[i].vertices)
         return( RETURN_FAIL );

      i2 = 0;
      for(; i2 != mmd->skin[i].num_vertices; ++i2)
      {
         /* UNSIGNED INT: vertex index */
         if(!fread( (void*)(&mmd->skin[i].vertices[i2].index), SIZE_INTEGER, 1, f ))
            return( RETURN_FAIL );

         /* 3xFLOAT: translation */
         if(!fread( (void*)(&mmd->skin[i].vertices[i2].translation), SIZE_FLOAT, 3, f ))
            return( RETURN_FAIL );
      }

#if ICONV_SJIS_PMD
      convertSjisToUtf8( name, mmd->skin[i].name, 20 );
      //glPuts( mmd->bones[i].name );
#else
      sprintf( mmd->skin[i].name, name );
#endif
   }

   return( RETURN_OK );
}

/* Read Skin display data */
int mmd_readSkinDisplayData( FILE *f, mmd_data *mmd )
{
   /* BYTE: Skin display count */
   if(!fread( (void*)(&mmd->num_skin_displays), SIZE_BYTE, 1, f ))
      return( RETURN_FAIL );

   /* alloc skin displays */
   mmd->skin_display = malloc( mmd->num_skin_displays * sizeof( unsigned int ) );
   if(!mmd->skin_display)
      return( RETURN_FAIL );

   /* UNSIGNED INT ARRAY: indices */
   if(!fread( (void*)(mmd->skin_display), SIZE_INTEGER, mmd->num_skin_displays, f ))
      return( RETURN_FAIL );

   return( RETURN_OK );
}

/* Read bone name data */
int mmd_readBoneNameData( FILE *f, mmd_data *mmd )
{
   unsigned int i;
   char name[50];

   /* BYTE: Bone name count */
   if(!fread( (void*)(&mmd->num_bone_names), SIZE_BYTE, 1, f ))
      return( RETURN_FAIL );

   return( RETURN_OK );

   /* alloc bone names */
   mmd->bone_name = malloc( mmd->num_bone_names * sizeof( mmd_bone_name ));
   if(!mmd->bone_name)
      return( RETURN_FAIL );

   i = 0;
   for(; i != mmd->num_bone_names; ++i)
   {
      /* SJIS STRING: bone name */
      if(!fread( name, 1, 50, f ))
         return( RETURN_FAIL );

#if ICONV_SJIS_PMD
      convertSjisToUtf8( name, mmd->bone_name[i].name, 50 );
      //glPuts( mmd->bones[i].name );
#else
      sprintf( mmd->bone_name[i].name, name );
#endif
   }

   return( RETURN_OK );
}

/* Allocate new mmd_data structure */
mmd_data* newMMD(void)
{
   mmd_data *mmd;

   mmd = calloc( 1, sizeof(mmd_data) );
   if(!mmd)
      return( NULL );

   /* vertex */
   mmd->vertices  = NULL;
   mmd->normals   = NULL;
   mmd->coords    = NULL;

   /* index */
   mmd->indices   = NULL;

   /* bone index */
   mmd->bone_indices = NULL;
   mmd->bone_weight  = NULL;
   mmd->edge_flag    = NULL;

   /* bones array */
   mmd->bones        = NULL;
   mmd->ik           = NULL;
   mmd->bone_name    = NULL;

   /* skins */
   mmd->skin         = NULL;
   mmd->skin_display = NULL;

   /* material */
   mmd->diffuse      = NULL;
   mmd->alpha        = NULL;
   mmd->power        = NULL;
   mmd->specular     = NULL;
   mmd->ambient      = NULL;
   mmd->toon         = NULL;
   mmd->edge         = NULL;
   mmd->face         = NULL;
   mmd->texture      = NULL;

   return( mmd );
}

/* Free mmd_data structure */
void freeMMD( mmd_data *mmd )
{
   unsigned int i;

   /* vertices */
   if(mmd->vertices) free( mmd->vertices ); mmd->vertices = NULL;
   if(mmd->normals)  free( mmd->normals  ); mmd->normals  = NULL;
   if(mmd->coords)   free( mmd->coords   ); mmd->coords   = NULL;

   /* indices */
   if(mmd->indices)  free( mmd->indices  ); mmd->indices  = NULL;

   /* bone indices */
   if(mmd->bone_indices) free(mmd->bone_indices); mmd->bone_indices = NULL;
   if(mmd->bone_weight)  free(mmd->bone_weight);  mmd->bone_weight  = NULL;
   if(mmd->edge_flag)    free(mmd->edge_flag);    mmd->edge_flag    = NULL;

   /* bones array */
   if(mmd->bones)        free(mmd->bones);        mmd->bones        = NULL;
   if(mmd->bone_name)    free(mmd->bone_name);    mmd->bone_name    = NULL;

   if(mmd->ik)
   {
      i = 0;
      for(; i != mmd->num_ik; ++i)
      {
         if( mmd->ik[i].child_bone_index )
            free( mmd->ik[i].child_bone_index );
      }

      free(mmd->ik); mmd->ik = NULL;
   }

   /* skin */
   if(mmd->skin)
   {
      i = 0;
      for(; i != mmd->num_skins; ++i)
      {
         if( mmd->skin[i].vertices )
            free( mmd->skin[i].vertices );
      }

      free(mmd->skin); mmd->skin = NULL;
   }
   if(mmd->skin_display) free(mmd->skin_display); mmd->skin_display = NULL;

   /* materials */
   if(mmd->diffuse)      free(mmd->diffuse);      mmd->diffuse      = NULL;
   if(mmd->alpha)        free(mmd->alpha);        mmd->alpha        = NULL;
   if(mmd->power)        free(mmd->power);        mmd->power        = NULL;
   if(mmd->specular)     free(mmd->specular);     mmd->specular     = NULL;
   if(mmd->ambient)      free(mmd->ambient);      mmd->ambient      = NULL;
   if(mmd->toon)         free(mmd->toon);         mmd->toon         = NULL;
   if(mmd->edge)         free(mmd->edge);         mmd->edge         = NULL;
   if(mmd->face)         free(mmd->face);         mmd->face         = NULL;
   if(mmd->texture)      free(mmd->texture);      mmd->texture      = NULL;

   /* finally free the struct itself */
   free( mmd );
}

/* EoF */
