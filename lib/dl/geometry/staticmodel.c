#include <stdio.h>

#include "dlAlloc.h"
#include "dlSceneobject.h"
#include "dlVbo.h"
#include "import/dlImport.h"
#include "dlLog.h"
#include "dlTypes.h"

/* create static object from model file */
dlObject* dlNewStaticModel( const char *file )
{
#if 0
   unsigned int i = 0;
#endif

   /* new sceneobject */
   dlObject *object = dlNewObject();
   if(!object)
      return(NULL);

   /* check VBO */
   if(!object->vbo)
      object->vbo = dlNewVBO();

   /* check IBO */
   if(!object->ibo)
      object->ibo = dlNewIBO();

   /* import model */
   if(dlImportModel( object, file, 0 ) != RETURN_OK)
   {
      dlFreeObject( object );
      return( NULL );
   }

#if 0
   /* info */
   printf("Vertices: %d\n", object->vbo->v_use);
   for(; i != object->vbo->v_use; i++)
      printf(" v: %f, %f, %f\n",
            object->vbo->vertices[i].x,
            object->vbo->vertices[i].y,
            object->vbo->vertices[i].z);

   printf("Coords: %d\n", object->vbo->t_use[0]);
   i = 0;
   for(; i != object->vbo->t_use[0]; i++)
      printf(" t: %f, %f\n",
            object->vbo->coords[0][i].x,
            object->vbo->coords[0][i].y );

   printf("Indices: %d\n", object->vbo->i_use);
   i = 0;
   for(; i != object->vbo->i_use; i++)
      printf("%u ",
            object->vbo->indices[i]);
   puts("");
#endif

#if 0
#if VERTEX_COLOR
   /* test, make it colorful */
   if(!object->vbo->colors)
   {
      dlResetColorBuffer( object->vbo, object->vbo->v_use );
      for(; i != object->vbo->v_use; i++)
         dlInsertColor( object->vbo, i , i , i , 255 );
   }
#endif
#endif

   /* assign aabb */
   if(dlObjectCalculateAABB(object) != RETURN_OK)
   {
      dlPuts("[STATIC MODEL] failed to calculate AABB\n");
      dlFreeObject( object );
      return( NULL );
   }

   return(object);
}
