/*----------------------------------------------------------------------*\

			       Slot.c

			     Slot Nodes

\*----------------------------------------------------------------------*/

#include "alan.h"

#include "Slot.h"
#include "Class.h"
#include "Attribute.h"
#include "Exit.h"
#include "Script.h"
#include "Statement.h"
#include "Symbol.h"
#include "Verb.h"

#include "dump.h"
#include "lmList.h"


/* PUBLIC */


/* PRIVATE */


/*======================================================================

  newSlot()

  Creates a new Slot and insert it in the symbol table.

  */
#ifdef _PROTOTYPES_
Slot *newSlot(List *heritage,
	      List *name,
	      Where *where,
	      List *attributes,
	      Container *container,
	      List *surroundings,
	      List *description,
	      List *mentioned,
	      Does *does,
	      List *exits,
	      List *verbs,
	      List *scripts)
#else
Slot *newSlot(heritage, name, where, attributes, container, surroundings,
		description, mentioned, does, exits, verbs, scripts)
     List *heritage;
     List *name;
     Where *where;
     List *attributes;
     Container *container;
     List *surroundings;
     List *description;
     List *mentioned;
     Does *does;
     List *exits;
     List *verbs;
     List *scripts;
#endif
{
  Slot *new;			/* The newly created Slot */

  new = NEW(Slot);

  new->state = IDLE;
  new->heritage = heritage;
  new->name = name;
  new->where = where;
  new->attributes = attributes;
  new->inheritedAttributeLists = NULL;
  new->container = container;
  new->surroundings = surroundings;
  new->description = description;
  new->mentioned = mentioned;
  new->does = does;
  new->exits = exits;
  new->verbs = verbs;
  new->scripts = scripts;
  
  return(new);
}


/*======================================================================

  analyseSlot()

  Do the analysis for a Slot.

  */
#ifdef _PROTOTYPES_
void analyseSlot(Id *id,	/* IN - Identifier with source position */
		 Slot *slot)	/* IN - The slot to analyse */
#else
void analyseSlot(id, slot)
     Id *id;
     Slot *slot;
#endif
{
  List *list, *sentinel = NULL;
  Symbol *symbol;
  Bool remove;			/* Forced to remove the item? */
  List *localAttributes;
  Attribute *attribute;
  List *localScripts;
  Script *script;
  List *localExits;
  Exit *exit;
  List *localVerbs;
  Verb *verb;

  slot->state = LOOKING_FOR_CIRCLES;
  /* Verify that the inherited slots exist and are classes */
  /* Also collect inherited attributes, scripts, etc. */
  for (list = slot->heritage; list; list = list->next) {
    remove = FALSE;
    if ((symbol = symbolCheck(list->element.id, CLASS_SYMBOL)) != NULL) {
      /* Ok, it was a class, now check for circular inheritance */
      if (symbol->info.class->slot->state == LOOKING_FOR_CIRCLES) {
	lmLog(&list->element.id->srcp, 226, sevERR, list->element.id->string);
	remove = TRUE;
      } else {
	if (symbol->info.class->slot->state != FINISHED)
	  analyseSlot(symbol->info.class->id, symbol->info.class->slot);
	/* Now collect the inhertied attributes, scripts etc. into the list */
	inheritAttributes(symbol->info.class->slot, &slot->inheritedAttributeLists);
	inheritExits(symbol->info.class->slot, &slot->inheritedExitLists);
	inheritVerbs(symbol->info.class->slot, &slot->inheritedVerbLists);
	inheritScripts(symbol->info.class->slot, &slot->inheritedScriptLists);
      }
    } else			/* Not a class, remove it! */
      remove = TRUE;
    if (remove) {
      if (sentinel == NULL)	/* At first item */
	slot->heritage = list->next;
      else
	sentinel->next = list->next; /* Unlink the item */
    } else
      sentinel = list;
  }
  slot->state = NUMBERING_ATTRIBUTES;
  /* Analyse and number the attributes */
  analyseAttributes(slot->attributes);
  /* 4f - check that the attributes that are inherited but have no local definition don't clash */
  for (localAttributes = slot->attributes; localAttributes; localAttributes = localAttributes->next) {
    attribute = findAttributeInLists(&id->srcp, localAttributes->element.attribute->id,
				     slot->inheritedAttributeLists);
    if (attribute == NULL)
      localAttributes->element.attribute->code = ++attributeCount;
    else if (!equalTypes(localAttributes->element.attribute->type, attribute->type))
      lmLog(&localAttributes->element.attribute->srcp, 228, sevERR, NULL);
    else
      localAttributes->element.attribute->code = attribute->code;
  }

  /* 4f - Analyse the name */

  /* Analyse the where */
  if (slot->where)
    analyseWhere(slot->where, NULL);

  /* Analyse the container */
  if (slot->container) {
    if (!anyIsA(slot->heritage, "container"))
      lmLogv(&slot->container->srcp, 223, sevERR, id->string, "container", NULL);
    analyseContainer(slot->container);
  }

  /* Analyse the surroundings */
  if (slot->surroundings) {
    if (!anyIsA(slot->heritage, "location"))
      lmLogv(&slot->surroundings->element.statement->srcp, 223, sevERR,
	     id->string, "location", NULL);
    analyseStatements(slot->surroundings, NULL, NULL);
  }

  /* Analyse the description */
  if (slot->description)
    analyseStatements(slot->description, NULL, NULL);

  /* Analyse the mentioned */
  if (slot->mentioned)
    analyseStatements(slot->mentioned, NULL, NULL);

  /* Analyse the does */
  if (slot->does) {
    if (!anyIsA(slot->heritage, "location"))
      lmLogv(&slot->does->srcp, 223, sevERR, id->string, "location", NULL);
    analyseDoes(slot->does, NULL);
  }

  /* Analyse the exits */
  if (slot->exits) {
    if (!anyIsA(slot->heritage, "location"))
      lmLogv(&slot->exits->element.exit->srcp, 223, sevERR, id->string, "location",
	     NULL);
    analyseExits(slot->exits);
    /* Find and resolve inherited exits */
    for (localExits = slot->exits; localExits; localExits = localExits->next) {
      List *direction;

      for (direction = localExits->element.exit->directions; direction;
	   direction = direction->next)
	exit = findExitInLists(&id->srcp, direction->element.id,
			       slot->inheritedExitLists);
      /* The directions are numbered through the symbol table ! */
    }
    /* 4f - check that exits that are inherited but have no local definition
       don't clash */
  }

  /* Analyse the verbs */
  if (slot->verbs) {
    analyseVerbs(slot->verbs);
    /* Find and resolve inherited verbs */
    for (localVerbs = slot->verbs; localVerbs; localVerbs = localVerbs->next) {
      List *ids;

      for (ids = localVerbs->element.verb->ids; ids; ids = ids->next)
	verb = findVerbInLists(&id->srcp, ids->element.id,
			       slot->inheritedVerbLists);
      /* The verbs are numbered through the symbol table ! */
    }
    /* 4f - check that verbs that are inherited but have no local definition
       don't clash */
  }
  
  /* Analyse the scripts */
  slot->state = NUMBERING_SCRIPTS;
  if (slot->scripts) {
    if (!anyIsA(slot->heritage, "actor"))
      lmLogv(&slot->scripts->element.script->srcp, 223, sevERR, id->string, "actor", NULL);
    analyseScripts(slot->scripts);
    /* Find and resolve inherited scripts */
    for (localScripts = slot->scripts; localScripts; localScripts = localScripts->next) {
      script = findScriptInLists(&id->srcp, localScripts->element.script->id,
				 slot->inheritedScriptLists);
      if (script == NULL)
	localScripts->element.script->code = ++scriptCount;
      else
	localScripts->element.script->code = script->code;
    }
    /* 4f - check that scripts that are inherited but have no local definition
       don't clash */
  }

  slot->state = FINISHED;
}



/*======================================================================

  dumpSlot()

  Dump a Slot.

  */
#ifdef _PROTOTYPES_
void dumpSlot(Slot *slot)
#else
void dumpSlot(slot)
     Slot *slot;
#endif
{
  if (slot == NULL) {
    put("NULL");
    return;
  }

  put("SLOT: "); in();
  put("heritage: "); dumpList(slot->heritage, ID_NODE); nl();
  put("name: "); dumpList(slot->name, ID_NODE); nl();
  put("where: "); dumpWhere(slot->where); nl();
  put("attributes: "); dumpList(slot->attributes, ATTRIBUTE_NODE); nl();
  put("inheritedAttributeLists: "); dumpList(slot->inheritedAttributeLists, ADDRESS_NODE); nl();
  put("container: "); dumpContainer(slot->container); nl();
  put("surroundings: "); dumpList(slot->surroundings, STATEMENT_NODE); nl();
  put("description: "); dumpList(slot->description, STATEMENT_NODE); nl();
  put("mentioned: "); dumpList(slot->mentioned, STATEMENT_NODE); nl();
  put("does: "); dumpDoes(slot->does); nl();
  put("exits: "); dumpList(slot->exits, EXIT_NODE); nl();
  put("verbs: "); dumpList(slot->verbs, VERB_NODE); nl();
  put("scripts: "); dumpList(slot->scripts, SCRIPT_NODE); nl();
  put("inheritedScriptLists: "); dumpList(slot->inheritedScriptLists, ADDRESS_NODE); out();
}
