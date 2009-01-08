#ifndef _RULES_H_
#define _RULES_H_
/*----------------------------------------------------------------------*\

  rules.h

  Header file for rules handler in Alan interpreter

\*----------------------------------------------------------------------*/

/* TYPES */
#include "types.h"


/* DATA */
extern RulEntry *ruls;      /* Rule table pointer */


/* FUNCTIONS */
extern void rules(void);

#endif
