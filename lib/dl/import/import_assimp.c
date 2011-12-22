#include "dlConfig.h"
#if WITH_ASSIMP

#include <stdio.h>
#include <malloc.h>

#include <assimp/assimp.h>
#include <assimp/aiScene.h>
#include <assimp/aiPostProcess.h>

#include "dlSceneobject.h"
#include "dlVbo.h"
#include "dlTypes.h"
#include "dlImport.h"
#include "dlTexture.h"
#include "dlLog.h"
#include "dlCore.h"
#include "dlAlloc.h"

/* maybe map the basic GL enums
 * to own structure, so these become useless */
#ifdef GLES2
#  include <GLES2/gl2.h>
#elif  GLES1
#  include <GLES/gl.h>
#  include <GLES/glext.h>
#else
#  include <GL/glew.h>
#  include <GL/gl.h>
#endif

#define DL_DEBUG_CHANNEL "IMPORT_ASSIMP"

/* Set material */
static int setMaterial(const char *file, dlObject *object, struct aiMaterial *mtl)
{
   unsigned int i, numDiffuse;

   struct aiString         textureName;
   enum aiTextureMapping   textureMapping;
   enum aiTextureOp        op;
   enum aiTextureMapMode   textureMapMode[3];
   unsigned int            uvwIndex, flags;
   float                   blend;

   char *texturePath;

   CALL("%s, %p, %p", file, object, mtl);

   numDiffuse = aiGetMaterialTextureCount( mtl, aiTextureType_DIFFUSE );
   if(!numDiffuse)
   { RET("%d", RETURN_NOTHING); return( RETURN_NOTHING ); }

   if(numDiffuse > _dlCore.info.maxTextureUnits)
   {
      LOGINFOP("This model has more than %d Textures, some textures won't be shown", _dlCore.info.maxTextureUnits );
      numDiffuse = _dlCore.info.maxTextureUnits;
   }

   i = 0;
   for(; i != numDiffuse; ++i)
   {
      if( aiGetMaterialTexture( mtl, aiTextureType_DIFFUSE, i,
                                &textureName, &textureMapping,
                                &uvwIndex, &blend, &op,
                                textureMapMode, &flags ) != AI_SUCCESS )
      {
         LOGERR("Failed to fetch texture info from material");

         RET("%d", RETURN_FAIL);
         return( RETURN_FAIL );
      }

      texturePath = dlImportTexturePath( textureName.data,
                                         file );
      if( texturePath )
      {
         if(object->material) dlFreeMaterial(object->material);
         object->material = dlNewMaterialFromImage( texturePath, SOIL_FLAG_DEFAULTS );
         free( texturePath );

         RET("%d", RETURN_OK);
         return( RETURN_OK ); /* add multiple material support */
      }
   }

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}

const struct aiNode* findNode( const struct aiNode *nd, const char *name )
{
   unsigned int f;
   CALL("%p, %s", nd, name);

   f = 0;
   for(; f != nd->mNumChildren; ++f)
   {
      if(strcmp( nd->mChildren[f]->mName.data, name ) == 0)
      { RET("%p", nd->mChildren[f]); return( nd->mChildren[f] ); }
      else
      {
         const struct aiNode *node = findNode( nd->mChildren[f], name );
         if(node) { RET("%p", node); return( node ); }
      }
   }

   RET("%p", NULL);
   return( NULL );
}

const struct aiBone* findBone( const struct aiMesh *mesh, const char *name )
{
   unsigned int f;
   CALL("%p, %s", mesh, name);

   f = 0;
   for(; f != mesh->mNumBones; ++f)
   {
      if(strcmp( mesh->mBones[f]->mName.data, name ) == 0)
      { RET("%p", mesh->mBones[f]); return( mesh->mBones[f] ); }
   }

   RET("%p", NULL);
   return( NULL );
}

static dlBone*
add_bone(dlAnimator *object, const struct aiMesh *mesh, const struct aiNode *bND, const struct aiNode *nd);
static dlBone*
add_bone(dlAnimator *object, const struct aiMesh *mesh, const struct aiNode *bND, const struct aiNode *nd)
{
   dlBone *bone, *child;
   const struct aiBone* aBone;
   unsigned int f = 0;

   CALL("%p, %p, %p", mesh, bND, nd);

   /* invalid bone node */
   if(!bND)
   { RET("%p", NULL); return( NULL ); }

   assert( bND->mName.data );
   if(!strlen(bND->mName.data))
   { RET("%p", NULL); return( NULL ); }

   /* could not find bone */
   aBone = findBone( mesh, bND->mName.data );

   /* add bone */
   bone = dlAnimatorAddBone( object );
   if(!bone)
   { RET("%p", NULL); return( NULL ); }

   /* assign name */
   bone->name = strdup(bND->mName.data);
   if(!bone->name)
   {
      dlFreeBone( bone );

      RET("%p", NULL);
      return( NULL );
   }
   LOGINFOP("%s",  bone->name);

   /* only if found, otherwise create dummy bone with only relative translation info */
   if(aBone)
   {
      /* offset matrix */
      bone->offsetMatrix.mat[0]  = aBone->mOffsetMatrix.a1;
      bone->offsetMatrix.mat[1]  = aBone->mOffsetMatrix.a2;
      bone->offsetMatrix.mat[2]  = aBone->mOffsetMatrix.a3;
      bone->offsetMatrix.mat[3]  = aBone->mOffsetMatrix.a4;

      bone->offsetMatrix.mat[4]  = aBone->mOffsetMatrix.b1;
      bone->offsetMatrix.mat[5]  = aBone->mOffsetMatrix.b2;
      bone->offsetMatrix.mat[6]  = aBone->mOffsetMatrix.b3;
      bone->offsetMatrix.mat[7]  = aBone->mOffsetMatrix.b4;

      bone->offsetMatrix.mat[8]  = aBone->mOffsetMatrix.c1;
      bone->offsetMatrix.mat[9]  = aBone->mOffsetMatrix.c2;
      bone->offsetMatrix.mat[10] = aBone->mOffsetMatrix.c3;
      bone->offsetMatrix.mat[11] = aBone->mOffsetMatrix.c4;

      bone->offsetMatrix.mat[12] = aBone->mOffsetMatrix.d1;
      bone->offsetMatrix.mat[13] = aBone->mOffsetMatrix.d2;
      bone->offsetMatrix.mat[14] = aBone->mOffsetMatrix.d3;
      bone->offsetMatrix.mat[15] = aBone->mOffsetMatrix.d4;

      /* default global */
      bone->globalMatrix = bone->offsetMatrix;

      /* add weights */
      f = 0;
      for(; f != aBone->mNumWeights; ++f)
      {
         if(!dlBoneAddWeight( bone, aBone->mWeights[f].mVertexId,
                              aBone->mWeights[f].mWeight ))
         {
            dlFreeBone( bone );

            RET("%p", NULL);
            return( NULL );
         }
      }
   }

   /* relative matrix */
   bone->relativeMatrix.mat[0]  = bND->mTransformation.a1;
   bone->relativeMatrix.mat[1]  = bND->mTransformation.a2;
   bone->relativeMatrix.mat[2]  = bND->mTransformation.a3;
   bone->relativeMatrix.mat[3]  = bND->mTransformation.a4;

   bone->relativeMatrix.mat[4]  = bND->mTransformation.b1;
   bone->relativeMatrix.mat[5]  = bND->mTransformation.b2;
   bone->relativeMatrix.mat[6]  = bND->mTransformation.b3;
   bone->relativeMatrix.mat[7]  = bND->mTransformation.b4;

   bone->relativeMatrix.mat[8]  = bND->mTransformation.c1;
   bone->relativeMatrix.mat[9]  = bND->mTransformation.c2;
   bone->relativeMatrix.mat[10] = bND->mTransformation.c3;
   bone->relativeMatrix.mat[11] = bND->mTransformation.c4;

   bone->relativeMatrix.mat[12] = bND->mTransformation.d1;
   bone->relativeMatrix.mat[13] = bND->mTransformation.d2;
   bone->relativeMatrix.mat[14] = bND->mTransformation.d3;
   bone->relativeMatrix.mat[15] = bND->mTransformation.d4;

   /* assing childs */
   f = 0;
   for(; f != bND->mNumChildren; ++f)
   {
      child = add_bone( object, mesh, findNode( nd, bND->mChildren[f]->mName.data ), nd );
      dlBoneAddChild( bone, child );
   }

   RET("%p", bone);
   return(bone);
}

/* Construct bones */
static int construct_bones(dlAnimator *object, const struct aiNode *nd, const struct aiMesh *mesh)
{
   CALL("%p, %p, %p", object, nd, mesh);

   /* no bones, no milk */
   if(!mesh->mNumBones)
   { RET("%d", RETURN_NOTHING); return( RETURN_NOTHING ); }

   /* assign bones */
   LOGINFOP("Found %d bones", mesh->mNumBones);
   if(!add_bone( object, mesh, nd, nd ))
   {
      LOGERR("Bones failed");

      RET("%d", RETURN_FAIL);
      return( RETURN_FAIL );
   }

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}

/* construct animation */
static int construct_animation(dlAnimator *object, const struct aiScene *sc, const struct aiMesh *mesh)
{
   unsigned int i, f, t;
   dlBone         *bone;
   dlAnim         *anim;
   dlNodeAnim     *node;
   kmVec3         vec3value;
   kmQuaternion   quatvalue;

   CALL("%p, %p, %p", object, sc, mesh);

#if USE_KEY_ANIMATION
   /* mesh->mAnimMeshes
    * mesh->mNumAnimMeshes */
#endif

   /* no need to do this */
   if(!sc->mNumAnimations)
   { RET("%d", RETURN_NOTHING); return( RETURN_NOTHING ); }

   LOGINFOP("Found %d animations", sc->mNumAnimations);

   /* add animations */
   i = 0;
   for(; i != sc->mNumAnimations; ++i)
   {
      anim = dlAnimatorAddAnim( object );
      if(!anim)
      { RET("%d", RETURN_FAIL); return( RETURN_FAIL ); }

      anim->ticksPerSecond = sc->mAnimations[i]->mTicksPerSecond;
      anim->duration       = sc->mAnimations[i]->mDuration;

      /* check name */
      assert( sc->mAnimations[i]->mName.data );
      anim->name = strdup(sc->mAnimations[i]->mName.data);
      if(!anim->name)
      { RET("%d", RETURN_FAIL); return(RETURN_FAIL); }

      LOGINFOP("%s", anim->name);
      LOGINFOP("%d channels", sc->mAnimations[i]->mNumChannels);

      /* add nodes */
      f = 0;
      for(; f != sc->mAnimations[i]->mNumChannels; ++f)
      {
         bone = dlAnimatorGetBone( object, sc->mAnimations[i]->mChannels[f]->mNodeName.data );
         if(!bone || !bone->name) continue;

         node = dlAnimAddNode( anim );
         if(!node)
         { RET("%d", RETURN_FAIL); return( RETURN_FAIL ); }

         /* pointer to affected bone */
         node->bone = bone;

         /* add translation keys */
         t = 0;
         for(; t != sc->mAnimations[i]->mChannels[f]->mNumPositionKeys; ++t)
         {
            vec3value.x = sc->mAnimations[i]->mChannels[f]->mPositionKeys[t].mValue.x;
            vec3value.y = sc->mAnimations[i]->mChannels[f]->mPositionKeys[t].mValue.y;
            vec3value.z = sc->mAnimations[i]->mChannels[f]->mPositionKeys[t].mValue.z;

            if(!dlNodeAddTranslationKey( node, &vec3value,
                sc->mAnimations[i]->mChannels[f]->mPositionKeys[t].mTime ))
            { RET("%d", RETURN_FAIL); return( RETURN_FAIL ); }
         }

         /* add rotation keys */
         t = 0;
         for(; t != sc->mAnimations[i]->mChannels[f]->mNumRotationKeys; ++t)
         {
            quatvalue.x = sc->mAnimations[i]->mChannels[f]->mRotationKeys[t].mValue.x;
            quatvalue.y = sc->mAnimations[i]->mChannels[f]->mRotationKeys[t].mValue.y;
            quatvalue.z = sc->mAnimations[i]->mChannels[f]->mRotationKeys[t].mValue.z;
            quatvalue.w = sc->mAnimations[i]->mChannels[f]->mRotationKeys[t].mValue.w;
            if(!dlNodeAddRotationKey( node, &quatvalue,
                sc->mAnimations[i]->mChannels[f]->mRotationKeys[t].mTime ))
            { RET("%d", RETURN_FAIL); return( RETURN_FAIL ); }
         }

         /* add scaling keys */
         t = 0;
         for(; t != sc->mAnimations[i]->mChannels[f]->mNumScalingKeys; ++t)
         {
            vec3value.x = sc->mAnimations[i]->mChannels[f]->mScalingKeys[t].mValue.x;
            vec3value.y = sc->mAnimations[i]->mChannels[f]->mScalingKeys[t].mValue.y;
            vec3value.z = sc->mAnimations[i]->mChannels[f]->mScalingKeys[t].mValue.z;
            if(!dlNodeAddScalingKey( node, &vec3value,
                sc->mAnimations[i]->mChannels[f]->mPositionKeys[t].mTime ))
            { RET("%d", RETURN_FAIL); return( RETURN_FAIL ); }
         }
      }
   }

   /* return */
   RET("%d", RETURN_OK);
   return( RETURN_OK );
}

/* Construct our model */
static int construct(const char *file, dlObject *object, const struct aiScene *sc, const struct aiNode *nd, const struct aiMesh *mesh, int bAnimated)
{
   unsigned int i = 0;
   unsigned int index = 0;
   unsigned int f = 0, t = 0;
   CALL("%s, %p, %p, %p, %p, %d", file, object, sc, nd, mesh, bAnimated);

   /* Check out what our mesh has */
   if(mesh->mVertices)
      dlResetVertexBuffer( object->vbo, mesh->mNumVertices );
   if(mesh->mNormals)
      dlResetNormalBuffer( object->vbo, mesh->mNumVertices );

   object->vbo->v_use = object->vbo->v_num;
   object->vbo->n_use = object->vbo->n_num;

   /* Texture coords */
   i = 0;
   while( i != _dlCore.info.maxTextureUnits )
   {
      if(mesh->mTextureCoords[i])
         dlResetCoordBuffer( object->vbo, i, mesh->mNumVertices );

      object->vbo->uvw[i].c_use = object->vbo->uvw[i].c_num;
      i++;
   }

   /* We don't know how many indices there is total
    * so let the glInsertIndex command to allocate dynamically */
   dlResetIndexBuffer( object->ibo, 1 );

   /* Material check */
   if(mesh->mMaterialIndex)
      setMaterial( file, object,  sc->mMaterials[mesh->mMaterialIndex] );

   /* Yush! Then assing the data to our structure */
   for(; f != mesh->mNumFaces; ++f)
   {
      /* That's some beautiful face */
      const struct aiFace *face = &mesh->mFaces[f];
      if(!face)
      { RET("%d", RETURN_FAIL); return( RETURN_FAIL ); }

      i = 0;
      for(; i != face->mNumIndices; ++i)
      {
         index = face->mIndices[i];

         if(mesh->mVertices)
         {
            object->vbo->vertices[ index ].x =
               mesh->mVertices[index].x;
            object->vbo->vertices[ index ].y =
               mesh->mVertices[index].y;
            object->vbo->vertices[ index ].z =
               mesh->mVertices[index].z;
         }

         if(mesh->mNormals)
         {
            object->vbo->normals[ index ].x =
               mesh->mNormals[index].x;
            object->vbo->normals[ index ].y =
               mesh->mNormals[index].y;
            object->vbo->normals[ index ].z =
               mesh->mNormals[index].z;
         }

         t = 0;
         while( t != _dlCore.info.maxTextureUnits )
         {
            if(mesh->mTextureCoords[t])
            {
               object->vbo->uvw[t].coords[ index ].x =
                  mesh->mTextureCoords[t][ index ].x;
               object->vbo->uvw[t].coords[ index ].y =
                  mesh->mTextureCoords[t][ index ].y;
            }

            t++;
         }

         /* finally index */
         dlInsertIndex( object->ibo, index );
      }
   }

   /* be done, if we only want static stuff */
   if(!bAnimated || object->animator)
   { RET("%d", RETURN_OK); return( RETURN_OK ); }

   /* create new animator */
   object->animator = dlNewAnimator();

   /* assign bones and animations */
   if(construct_bones( object->animator, sc->mRootNode, mesh )     == RETURN_FAIL)
   {
      dlFreeAnimator( object->animator );
      object->animator = NULL;

      RET("%d", RETURN_OK);
      return( RETURN_OK );
   }
   if(construct_animation( object->animator, sc, mesh ) == RETURN_FAIL)
   {
      dlFreeAnimator( object->animator );
      object->animator = NULL;

      RET("%d", RETURN_OK);
      return( RETURN_OK );
   }

   /* calculate global transforms */
   dlAnimatorCalculateGlobalTransformations( object->animator );

   /* set animation to 0 */
   dlObjectSetAnimation( object, 0 );

   /* return our beautiful object */
   RET("%d", RETURN_OK);
   return( RETURN_OK );
}


/* Avoid overriding main object */
static int bConstructed = 0;

/* Process our model */
static int process(const char *file, dlObject *object, const struct aiScene *sc, const struct aiNode *nd, int bAnimated)
{
   dlObject *mObject;
   unsigned int m;
   const struct aiMesh *mesh;

   CALL("%s, %p, %p, %p, %d", file, object, sc, nd, bAnimated);

   mObject = object;
   m = 0;
   for(; m != nd->mNumMeshes; ++m)
   {
      mesh = sc->mMeshes[nd->mMeshes[m]];

      if(!bConstructed)
      {
         /* Construct */
         if(construct( file, mObject, sc, nd, mesh, bAnimated ) != RETURN_OK)
         { RET("%d", RETURN_FAIL); return( RETURN_FAIL ); }

         /* We modified main object */
         bConstructed = 1;
      }
      else
      {
#if 1
         mObject = dlNewObject();
         if(!mObject)
         { RET("%d", RETURN_FAIL); return( RETURN_FAIL ); }

         mObject->vbo = dlNewVBO();
         if(!mObject->vbo)
         {
            dlFreeObject( mObject );

            RET("%d", RETURN_FAIL);
            return( RETURN_FAIL );
         }
         mObject->ibo = dlNewIBO();
         if(!mObject->ibo)
         {
            dlFreeObject( mObject );

            RET("%d", RETURN_FAIL);
            return( RETURN_FAIL );
         }

         /* refence animation */
         mObject->animator = dlRefAnimator( object->animator );

         if(construct( file, mObject, sc, nd, mesh, bAnimated ) != RETURN_OK)
         {
            dlFreeObject( mObject );

            RET("%d", RETURN_FAIL);
            return( RETURN_FAIL );
         }

         dlObjectAddChild(object, mObject);
         dlObjectCalculateAABB(mObject);
         mObject->primitive_type = GL_TRIANGLES;
#endif
      }
   }

   /* Actual child code is above ^^ */
   m = 0;
   for(; m != nd->mNumChildren; ++m)
   {
      if(process( file, mObject, sc, nd->mChildren[m], bAnimated ) != RETURN_OK)
      { RET("%d", RETURN_FAIL); return( RETURN_FAIL ); }
   }

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}

/* Import using Assimp */
int dlImportASSIMP( dlObject *object, const char *file, int bAnimated )
{
   const struct aiScene *scene;
   LOGINFOP("Attempt to load: %s", file );

   /* Assimp magic */
   scene = aiImportFile(file,
                        aiProcess_Triangulate               |
                        aiProcess_JoinIdenticalVertices     |
                        aiProcess_SortByPType               |
                        aiProcess_CalcTangentSpace          |
                        aiProcess_JoinIdenticalVertices     |
                        aiProcess_GenSmoothNormals          |
                        aiProcess_LimitBoneWeights          |
                        aiProcess_RemoveRedundantMaterials  |
                        aiProcess_OptimizeMeshes            |
                        aiProcess_GenUVCoords               |
                        aiProcess_TransformUVCoords         );

   /* Import failed */
   if(!scene)
   {
      LOGERRP("%s", aiGetErrorString());

      RET("%d", RETURN_FAIL);
      return( RETURN_FAIL );
   }

   /* We haven't done nothing yet */
   bConstructed = 0;

   /* Process it */
   if(process( file, object, scene, scene->mRootNode, bAnimated ) != RETURN_OK)
   {
      LOGERR("Processing of mesh failed");

      RET("%d", RETURN_FAIL);
      return( RETURN_FAIL );
   }

   /* Release */
   aiReleaseImport( scene );

   /* Set to triangles
    * Add tristripper code? */
   object->primitive_type = GL_TRIANGLES;

   /* if was animated */
   if(object->animator)
      dlVBOPrepareTstance( object->vbo );

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}

#endif /* WITH_ASSIMP */
