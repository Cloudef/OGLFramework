#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <limits.h>
#include <unistd.h>
#include <libgen.h>

#include "dlMaterial.h"
#include "dlSceneobject.h"
#include "dlImport.h"
#include "dlTypes.h"
#include "dlConfig.h"
#include "dlCore.h"
#include "dlLog.h"

#define DL_DEBUG_CHANNEL "IMPORT"

/* I used to have own image importers,
 * but then I stumbled against SOIL which seems to do a lots
 * of stuff with it's tiny size, so why reinvent the wheel?
 *
 * Din't want to write a wrapper for it either like assimp,
 * since it would just bloat my texture structure. It's much better like this :) */
#include "SOIL.h"


#define HEADER_MAX 20 /* Should be good enough,
                       * dont think anyone would use this long
                       * file header anyways.
                       */

#define HEADER_MIN 3  /* Minimun header length.
                       * I seriously hope there are no headers with only
                       * 1 or 2 characters <_<
                       */

/* TO-DO :
 * If any plans to add ASCII formats,
 * then they should be handled too..
 *
 * Maybe check against common stuff on that format?
 * And increase possibility by each match, then pick the one with highest possibility.
 * */

/* Model format defines here */
#define MODEL_FORMAT_OCTM     "OCTM"
#define MODEL_FORMAT_B3D      "BB3Dd"
#define MODEL_FORMAT_PMD      "Pmd"    /* really? */

/* Model format enumeration here */
typedef enum
{
   M_NOT_FOUND = 0,
#if WITH_OPENCTM
   M_OCTM,
#endif
#if WITH_PMD
   M_PMD,
#endif
#if WITH_ASSIMP
   M_ASSIMP,
#endif
} model_format_t;

/* Read first HEADER_MAX bytes from
 * the file, and try using that information
 * to figure out which format it is.
 *
 * returned char array must be freed */
static char* parseHeader( const char *file )
{
   char *MAGIC_HEADER;
   char bit = '0';
   FILE *f;
   unsigned int bytesRead = 0;
   size_t bytesTotal;
   CALL("%s", file);

   /* open file */
   f = fopen( file, "rb" );
   if(!f)
   {
      LOGERRP("File: %s, could not open", file);

      RET("%p", NULL);
      return( NULL );
   }

   /* allocate our header */
   MAGIC_HEADER = malloc( HEADER_MAX + 1 );

   /* read bytes */
   while( fread( &bit, 1, 1, f ) )
   {
      /* check ascii range */
      if( (bit >= 45 && bit <= 90) ||
          (bit >= 97 && bit <= 122) )
      {
         MAGIC_HEADER[bytesRead++] = bit;
      }
      else if( bytesRead )
         break;

      /* don't exceed the maximum */
      if( bytesRead == HEADER_MAX )
         break;
   }

   /* check that our header has minimum length */
   if( bytesRead + 1 <= HEADER_MIN )
   {
      bytesRead  = 0;

      /* Some formats tend to be hipster and store the header/description
       * at end of the file.. I'm looking at you TGA!! */
      fseek( f, 0L, SEEK_END );
      bytesTotal = (size_t)(ftell(f)) - HEADER_MAX;
      fseek( f, bytesTotal ,SEEK_SET );

      while( fread( &bit, 1, 1, f ) )
      {
         /* check ascii range */
         if( (bit >= 45 && bit <= 90) ||
             (bit >= 97 && bit <= 122) )
         {
            MAGIC_HEADER[bytesRead++] = bit;
         }
         else if( bytesRead )
            break;

         /* don't exceed the maximum */
         if( bytesRead == HEADER_MAX )
            break;
      }
   }

   /* close the file */
   fclose( f );

   /* if nothing */
   if(!bytesRead)
   {
      LOGERRP( "File: %s, failed to parse header", file );
      free( MAGIC_HEADER );

      RET("%p", NULL);
      return( NULL );
   }
   MAGIC_HEADER[ bytesRead ] = '\0';

   RET("%s", MAGIC_HEADER);
   return( MAGIC_HEADER );
}

/* check against known model format headers */
static model_format_t modelFormat( const char *MAGIC_HEADER )
{
   CALL("%s", MAGIC_HEADER);

   /* --------- FORMAT HEADER CHECKING ------------ */

#if WITH_OPENCTM
   /* OpenCTM check */
   if( strcmp( MODEL_FORMAT_OCTM, MAGIC_HEADER ) == 0 )
   { RET("%s", "M_OCTM"); return( M_OCTM ); }
#endif /* WITH_OPENCTM */

#if WITH_PMD
   /* PMD check */
   if( strcmp( MODEL_FORMAT_PMD, MAGIC_HEADER ) == 0 )
   { RET("%s", "M_PMD"); return( M_PMD ); }
#endif /* WITH_PMD */

#if WITH_ASSIMP
   /* Our importers cant handle this, let's try ASSIMP */
   RET("%s", "M_ASSIMP"); return( M_ASSIMP );
#endif /* WITH_ASSIMP */

   /* ------- ^^ FORMAT HEADER CHECKING ^^ -------- */

   LOGERR("No suitable importers found");
   LOGERR("If the model format is supported, make sure you have compiled library with it.");
   LOGERRP("Magic header: %s", MAGIC_HEADER);

   RET("%s", "M_NOT_FOUND");
   return( M_NOT_FOUND );
}

/* Figure out the file type
 * Call the right importer
 * And let it fill the object structure
 * Return the object
 */
int dlImportModel( dlObject *object,
                   const char *file,
                   int bAnimated     )
{
   model_format_t fileFormat;
   char *header;

   /* default for fail, as in no importer found */
   int import_return = RETURN_FAIL;

   CALL("%p, %s, %d", object, file, bAnimated);
   LOGINFOP("Model: %s", file);

   /* read file header */
   header = parseHeader( file );
   if(!header)
   { RET("%d", RETURN_FAIL); return( RETURN_FAIL ); }

   /* figure out the model format */
   fileFormat = modelFormat( header ); free(header); /* free header after use */
   if( fileFormat == M_NOT_FOUND )
   { RET("%d", RETURN_FAIL); return( RETURN_FAIL ); }

   /* --------- FORMAT IMPORT CALL ----------- */

   switch( fileFormat )
   {
      /* bail out */
      case M_NOT_FOUND:
         break;

#if WITH_OPENCTM
      /* OpenCTM */
      case M_OCTM:
         import_return = dlImportOCTM( object, file, bAnimated );
         break;
#endif /* WITH_OPENCTM */

#if WITH_PMD
      /* PMD */
      case M_PMD:
         import_return = dlImportPMD( object, file, bAnimated );
         break;
#endif /* WITH_PMD */

#if WITH_ASSIMP
      /* Use asssimp */
      case M_ASSIMP:
         import_return = dlImportASSIMP( object, file, bAnimated );
         break;
#endif /* WITH_ASSIMP */
   }

   /* ---------- ^^ FORMAT IMPORT ^^ ---------- */

   RET("%d", import_return);
   return( import_return );
}

/* Import using SOIL */
int dlImportImage( dlTexture *texture,
                   const char *file, unsigned int flags )
{
   int channels = 0;
   CALL("%p, %s, %u", texture, file, flags);
   LOGINFOP("Image: %s", file);

   /* load using SOIL */
   texture->object = SOIL_load_OGL_texture_EX
      (
            file,
            &texture->data,
            &texture->width,
            &texture->height,
            &channels,
            SOIL_LOAD_AUTO,
            SOIL_CREATE_NEW_ID,
            flags
      );
   texture->channels = channels;

   /* check succes */
   if( !texture->object )
   {
      LOGERRP("Failed to load %s", file);

      RET("%d", RETURN_FAIL);
      return( RETURN_FAIL );
   }

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}


/* ------------------ PORTABILTY ------------------ */

char *gnu_basename(char *path)
{
    char *base;
    CALL("%s", path);

    base = strrchr(path, '/');

    RET("%s", base ? base+1 : path);
    return base ? base+1 : path;
}

/* ------------ SHARED FUNCTIONS ---------------- */

/* maybe we could have a _default_ texture for missing files? */
char* dlImportTexturePath( const char* oddTexturePath,
                           const char* modelPath )
{
   char *textureFile, *modelFolder;
   char textureInModelFolder[PATH_MAX];
   CALL("%s, %s", oddTexturePath, modelPath);

   /* these are must to check */
   if(!oddTexturePath)
   { RET("%p", NULL); return( NULL ); }

   if(strcmp( oddTexturePath, "" ) == 0)
   { RET("%p", NULL); return( NULL ); }

   /* lets try first if it contains real path to the texture */
   textureFile = (char*)oddTexturePath;

   /* guess not, lets try basename it */
   if(access( textureFile, F_OK ) != 0)
      textureFile = gnu_basename( (char*)oddTexturePath );
   else
   { RET("%s", textureFile); return( strdup(textureFile) ); }

   /* hrmm, we could not even basename it?
    * I think we have a invalid path here Watson! */
   if(!textureFile)
   { RET("%p", NULL); return( NULL ); } /* Sherlock, you are a genius */

   /* these are must to check */
   if(!modelPath)
   { RET("%p", NULL); return( NULL ); }

   if(strcmp( modelPath, "" ) == 0)
   { RET("%p", NULL); return( NULL ); }

   /* grab the folder where model resides */
   modelFolder = dirname( strdup( modelPath ) );

   /* ok, maybe the texture is in same folder as the model? */
   snprintf( textureInModelFolder, PATH_MAX, "%s/%s",
             modelFolder, textureFile );

   /* free this */
   free( modelFolder ); modelFolder = NULL;

   /* gah, don't give me missing textures damnit!! */
   if(access( textureInModelFolder, F_OK ) != 0)
   { RET("%p", NULL); return( NULL ); }

   /* return, remember to free */
   RET("%s", textureInModelFolder);
   return( strdup( textureInModelFolder ) );
}
