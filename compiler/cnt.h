#ifndef _CNT_H_
#define _CNT_H_
/*----------------------------------------------------------------------*\

				CNT.H
			   Container Nodes

\*----------------------------------------------------------------------*/

/* USE: */
#include "../interpreter/acode.h"
#include "srcp.h"
#include "id.h"
#include "lst.h"


/* Types: */
typedef struct ContainerInfo {
  Srcp srcp;			/* Source position */
  Bool analyzed;		/* Is this container definition analyzed? */
  Bool generated;		/* And generated? */
  List *limits;			/* Limits */
  Aaddr limadr;			/* ACODE address to limit table */
  List *hstms;			/* Header statements */
  Aaddr hadr;			/* ACODE address to header statements */
  List *estms;			/* 'Empty' statements */
  Aaddr eadr;			/* ACODE address to 'empty' statements */  
} ContainerBody;

typedef struct Container {	/* To be used in instances */
  int code;			/* Code for this container */
  struct Properties *ownerProperties;	/* Pointer to parents properties */
  ContainerBody *body;		/* Common info */
} Container;



#endif
