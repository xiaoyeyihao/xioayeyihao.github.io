#ifndef __DATABASE_H__
#define __DATABASE_H__
#include "DB_4thSpan.h"

/*Database_t*********************************************/
typedef enum
{
    Database_PR_NOTHING,	/* No components present */
    Database_PR_4thSpan,

}Database_PR;

typedef struct Database
{
    Database_PR present;
    union Database_u 
    {
        DB_4thSpan_t span4th;
	} choice;

}Database_t;
/*********************************************************/

#endif