/*----------------------------------------------------------------------*\

				Syntax.c

			      Syntax Nodes

\*----------------------------------------------------------------------*/


#include "Syntax.h"
#include "Statement.h"

#include "lmList.h"
#include "dump.h"




/*======================================================================

  newSyntax()

  Allocates and initialises a Syntax.

 */
#ifdef _PROTOTYPES_
Syntax *newSyntax(Srcp *srcp,	/* IN - Source position */
		  Id *id,	/* IN - Verb name */
		  List *elements, /* IN - Syntax elements */
		  List *restrictions) /* IN - Class restrictions */
#else
Syntax *newSyntax(srcp, id, elements, restrictions)
     Srcp *srcp;
     Id *id;
     List *elements;
     List *restrictions;
#endif
{
  Syntax *new;			/* The newly allocated area */

  new = NEW(Syntax);

  new->srcp = *srcp;
  new->id = id;
  new->elements = elements;
  new->restrictions = restrictions;

  return(new);
}


/*======================================================================

  analyseSyntax()

  Analyze a SYNTAX.

 */
#ifdef _PROTOTYPES_
void analyseSyntax(Syntax *syntax)
#else
void analyseSyntax(syntax)
     Syntax *syntax;
#endif
{
  /* 4f - Analyse syntax */
}



/*======================================================================

  analyseSyntaxes()

  Analyze a list of SYNTAX nodes.

 */
#ifdef _PROTOTYPES_
void analyseSyntaxes(List *syntaxes)
#else
void analyseSyntaxes(syntaxes)
     List *syntaxes;
#endif
{
  List *list, *other;

  for (list = syntaxes; list != NULL; list = list->next) {
    if (verbose) printf(".");
    analyseSyntax(list->element.syntax);
    
    /* Check for multiple definitions of the syntax for a verb */
    for (other = list->next; other != NULL; other = other->next)
      if (equalIds(other->element.syntax->id, list->element.syntax->id)) {
	lmLog(&other->element.syntax->id->srcp, 206, sevWAR,
	      other->element.syntax->id->string);
	break;
      }
  }
}


/*======================================================================

  dumpSyntax()

  Dump a Syntax node.

 */
#ifdef _PROTOTYPES_
void dumpSyntax(Syntax *syntax)
#else
void dumpSyntax(syntax)
     Syntax *syntax;
#endif
{
  if (syntax == NULL) {
    put("NULL");
    return;
  }

  put("SYNTAX: "); dumpSrcp(&syntax->srcp); in();
  put("id: "); dumpId(syntax->id); nl();
  put("elements: "); dumpList(syntax->elements, ELEMENT_NODE); nl();
  put("restrictions: "); dumpList(syntax->restrictions, RESTRICTION_NODE); out();
}
