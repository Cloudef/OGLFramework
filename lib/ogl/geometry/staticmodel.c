#include <stdio.h>

#include "alloc.h"
#include "sceneobject.h"
#include "vbo.h"
#include "import/import.h"
#include "types.h"

/* create static object from model file */
glObject* glNewStaticModel( const char *file )
{
#if 0
   unsigned int i = 0;
#endif

   /* new sceneobject */
   glObject *object = glNewObject();
   if(!object)
      return(NULL);

   /* check VBO */
   if(!object->vbo)
      object->vbo = glNewVBO();

   /* check IBO */
   if(!object->ibo)
      object->ibo = glNewIBO();

   /* import model */
   if(glImportModel( object, file, 0 ) != RETURN_OK)
   {
      glFreeObject( object );
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
      glResetColorBuffer( object->vbo, object->vbo->v_use );
      for(; i != object->vbo->v_use; i++)
         glInsertColor( object->vbo, i , i , i , 255 );
   }
#endif
#endif

   /* assign aabb */
   if(glCalculateAABB(object) != RETURN_OK)
   {
      puts("[STATIC MODEL] failed to calculate AABB\n");
      glFreeObject( object );
      return( NULL );
   }

   return(object);
}
