/*----------------------------------------------------------------------*\

				SLT.C
			      Slot Nodes

\*----------------------------------------------------------------------*/

#include "slt_x.h"

/* IMPORT */
#include <stdio.h>
#include "alan.h"
#include "util.h"
#include "dump.h"
#include "emit.h"
#include "lmList.h"


#include "atr_x.h"
#include "cla_x.h"
#include "cnt_x.h"
#include "ext_x.h"
#include "id_x.h"
#include "lst_x.h"
#include "nam_x.h"
#include "scr_x.h"
#include "stm_x.h"
#include "sym_x.h"
#include "vrb_x.h"
#include "whr_x.h"


/*======================================================================

  newEmptySlots()

  Allocates and initialises a empty slots node.

  */
Slots *newEmptySlots(void)
{
  return NEW(Slots);
}



/*======================================================================

  newSlots()

  Allocates and initialises a slots node.

  */
Slots *newSlots(List *names,
		    WhrNod *whr,
		    List *attributes,
		    CntNod *container,
		    List *description,
		    Srcp *descriptionSrcp,
		    List *mentioned,
		    Srcp *mentionedSrcp,
		    List *article,
		    Srcp *articleSrcp,
		    List *does,
		    List *exits,
		    List *verbs,
		    List *scripts)
{
  Slots *new;                  /* The newly allocated area */

  if (verbose) { printf("%8ld\b\b\b\b\b\b\b\b", counter++); fflush(stdout); }

  new = NEW(Slots);

  new->names = names;
  new->whr = whr;
  new->attributes = attributes;

  new->container = container;
  if (new->container != NULL)
    new->container->ownerSlots = new;

  new->description = description;
  new->descriptionSrcp = *descriptionSrcp;
  new->mentioned = mentioned;
  new->mentionedSrcp = *mentionedSrcp;
  new->article = article;
  new->articleSrcp = *articleSrcp;
  new->verbs = verbs;
  new->exits = exits;
  new->scripts = scripts;

  return(new);
}



/*----------------------------------------------------------------------

  symbolizeParent()

  Symbolize parent of a Slots node.

 */
static void symbolizeParent(Slots *slots)
{
  Symbol *parent;

  if (slots->parentId != NULL) {
    parent = lookup(slots->parentId->string);
    if (parent == NULL)
      lmLog(&slots->parentId->srcp, 310, sevERR, slots->parentId->string);
    else if (parent->kind != CLASS_SYMBOL)
      lmLog(&slots->parentId->srcp, 350, sevERR, "");
    else {
      slots->parentId->symbol = parent;
      setParent(slots->id->symbol, slots->parentId->symbol);
    }
  }
}


/*======================================================================

  symbolizeSlots()

  Symbolize data for one Slots node.

 */
void symbolizeSlots(Slots *slots)
{
  symbolizeParent(slots);
  checkMultipleAttributes(slots->attributes);
  symbolizeWhere(slots->whr);
  symbolizeExits(slots->exits);
}


/*----------------------------------------------------------------------

  analyzeName()

*/

static void analyzeName(Slots *slots)
{
  long fpos;
  int len = 0;
  StmNod *stm;

  if (slots->mentioned == NULL) {
    /* Generate a mentioned from the first of the names */
    /* First output the formated name to the text file */
    fpos = ftell(txtfil);
    len = annams(slots->names, slots->id,
		 inheritsFrom(slots->id->symbol, locationSymbol) ||
		 inheritsFrom(slots->id->symbol, actorSymbol));

    /* Then create a PRINT statement */
    stm = newstm(&nulsrcp, STM_PRINT);
    stm->fields.print.fpos = fpos;
    stm->fields.print.len = len;
    slots->mentioned = concat(NULL, stm, STM_LIST);
  } else
    analyzeStatements(slots->mentioned, NULL);
}


/*======================================================================

  analyzeSlots()

  Analyze one Slots node.

 */
void analyzeSlots(Slots *slots, Context *context)
{
  if (slots->whr != NULL) verifyInitialLocation(slots->whr);
  if (inheritsFrom(slots->id->symbol, locationSymbol) && slots->whr != NULL)
    lmLog(&slots->whr->srcp, 405, sevERR, "");
  if (inheritsFrom(slots->id->symbol, actorSymbol)
      && slots->whr != NULL && slots->whr->kind == WHR_IN)
    lmLog(&slots->whr->srcp, 402, sevERR, "An Actor");

  analyzeName(slots);
  analyzeStatements(slots->description, context);
  analyzeVerbs(slots->verbs, context);
  analyzeContainer(slots->container, context);

  if (slots->exits && !inheritsFrom(slots->id->symbol, locationSymbol))
    lmLog(&slots->id->srcp, 352, sevERR, slots->id->string);
  analyzeExits(slots->exits, context);

  if (slots->scripts && !inheritsFrom(slots->id->symbol, actorSymbol))
    lmLog(&slots->id->srcp, 353, sevERR, slots->id->string);
  analyzeScripts(slots->scripts, context);
}


/*======================================================================

  generateSlotsData()

  Generate data for one Slots node.

 */
void generateSlotsData(Slots *slots)
{
  slots->idAddress = emadr();
  emitString(slots->id->string);

  slots->attributeAddress = generateAttributes(slots->attributes);

  if (slots->description != NULL) {
    slots->descriptionAddress = emadr();
    gestms(slots->description, slots->id->symbol->code);
    emit0(C_STMOP, I_RETURN);
  }

  if (slots->mentioned != NULL) {
    slots->mentionedAddress = emadr();
    gestms(slots->mentioned, slots->id->symbol->code);
    emit0(C_STMOP, I_RETURN);
  } else
    emit(0);

  slots->scriptsAddress = generateScripts(slots->scripts, slots->id->symbol->code);
  slots->verbsAddress = generateVerbs(slots->verbs, slots->id->symbol->code);
  slots->exitsAddress = generateExits(slots->exits, slots->id->symbol->code);
}


/*======================================================================

  generateSlotsEntry()

  Generate entries for one Slots node.

 */
void generateSlotsEntry(InstanceEntry *entry, Slots *slots)
{
  entry->code = slots->id->symbol->code; /* First own code */
  entry->idAddress = slots->idAddress; /* Address to the id string */

  if (slots->parentId == NULL)	/* Then parents */
    entry->parentClass = 0;
  else
    entry->parentClass = slots->parentId->symbol->code;

  entry->location = generateInitialLocation(slots->whr);
  entry->attributes = slots->attributeAddress;
  entry->description = slots->descriptionAddress;
  if (slots->container != NULL)
    entry->container = slots->container->code;
  else
    entry->container = 0;
  entry->mentioned = slots->mentionedAddress;
  entry->article = slots->articleAddress;
  entry->exits = slots->exitsAddress;
  entry->verbs = slots->verbsAddress;
  entry->scripts = slots->scriptsAddress;
}


/*======================================================================

  dumpSlots()

  Dump a Slots node.

 */
void dumpSlots(Slots *slots)
{
  put("SLOTS: "); dumpPointer(slots); in();
  put("id: "); dumpId(slots->id); nl();
  put("names: "); dumpList(slots->names, LIST_NAM); nl();
  put("whr: "); duwhr(slots->whr); nl();
  put("container: "); dumpContainer(slots->container); nl();
  put("attributes: "); dumpList(slots->attributes, LIST_ATR); nl();
  put("attributeAddress: "); dumpAddress(slots->attributeAddress); nl();
  put("description: "); dumpList(slots->description, LIST_STM); nl();
  put("descriptionAddress: "); dumpAddress(slots->descriptionAddress); nl();
  put("article: "); dumpList(slots->article, LIST_STM); nl();
  put("articleAddres: "); dumpAddress(slots->articleAddress); nl();
  put("mentioned: "); dumpList(slots->mentioned, LIST_STM); nl();
  put("mentionedAddress: "); dumpAddress(slots->mentionedAddress); nl();
  put("scripts: "); dumpList(slots->scripts, LIST_SCR); nl();
  put("scriptsAddress: "); dumpAddress(slots->scriptsAddress); nl();
  put("verbs: "); dumpList(slots->verbs, LIST_VRB); nl();
  put("verbsAddress: "); dumpAddress(slots->verbsAddress); nl();
  put("exits: "); dumpList(slots->exits, LIST_EXT); nl();
  put("exitsAddress: "); dumpAddress(slots->exitsAddress); out();
}
