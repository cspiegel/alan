#ifndef _SLT_H_
#define _SLT_H_
/*----------------------------------------------------------------------*\

				SLT.H
			     Slots Nodes
                                                                     
\*----------------------------------------------------------------------*/


/* USE: */
#include "types.h"
#include "lst.h"
#include "whr.h"
#include "cnt.h"

#include "acode.h"


/* Types */

typedef struct Slots {		/* SLOTS  */
  IdNode *id;			/* Identifier */
  Aaddr idAddress;
  IdNode *parentId;		/* The instances parent */
  List *names;			/* List of lists of adjectives and noun (ids)*/
  WhrNod *whr;			/* Where is it initially? */
  List *attributes;
  Aword attributeAddress;	/* ACODE address to attribute list */
  List *description;		/* Long description statements */
  Srcp descriptionSrcp;
  Aword descriptionAddress;	/* ACODE address to long description code */
  List *article;		/* Article printing statements */
  Srcp articleSrcp;
  Aword articleAddress;		/* ACODE address to article statements */
  List *mentioned;		/* Short (inventory like) statements */
  Srcp mentionedSrcp;
  Aword mentionedAddress;	/* ACODE address to short description (mentioned or name) code */
  CntNod *container;		/* Pointer to container property node */
  List *verbs;			/* List of verbs */
  Aword verbsAddress;		/* ACODE address to local verb table */
  List *scripts;		/* List of scripts */
  Aaddr scriptsAddress;		/* ACODE address to scripts */
  List *exits;			/* List of exits */
  Aaddr exitsAddress;		/* ACODE address to exits table */
} Slots;


#endif
