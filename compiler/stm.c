/*----------------------------------------------------------------------*\

                                STM.C
			   Statement Nodes

\*----------------------------------------------------------------------*/

#include "stm_x.h"


#include "alan.h"
#include "util.h"

#include "atr_x.h"
#include "cnt_x.h"
#include "exp_x.h"
#include "id_x.h"
#include "lst_x.h"
#include "srcp_x.h"
#include "sym_x.h"
#include "whr_x.h"
#include "wht_x.h"

#include "lmList.h"

#include "adv.h"                /* ADV-node */
#include "scr.h"                /* SCR-nodes */
#include "sco.h"                /* SCORES */
#include "opt.h"                /* OPTIONS */

#include "emit.h"
#include "encode.h"

#include "dump.h"




/*======================================================================

  newstm()

  Allocates and initialises a stmnod.

  */
StmNod *newstm(Srcp *srcp,	/* IN - Source Position */
	       StmKind class)	/* IN - The statement class */
{
  StmNod *new;                  /* The newly allocated area */

  showProgress();

  new = NEW(StmNod);

  new->srcp = *srcp;
  new->class = class;

  return(new);
}



/*----------------------------------------------------------------------

  andescribe()

  Analyze a DESCRIBE statement.

  */
static void andescribe(StmNod *stm,
		       Context *context)
{
  Symbol *sym;

  switch (stm->fields.describe.wht->kind) {
  case WHAT_LOCATION:
    break;
  case WHAT_ACTOR:
    if (context->kind == EVENT_CONTEXT)
      lmLog(&stm->fields.describe.wht->srcp, 412, sevERR, "");
    break;
  case WHAT_ID:
    sym = symcheck(stm->fields.describe.wht->id, INSTANCE_SYMBOL, context);
    break;
  default:
    unimpl(&stm->srcp, "Analyzer");
    break;
  }
}



/*----------------------------------------------------------------------*/
static void analyzeSay(StmNod *stm, Context *context)
{
  analyzeExpression(stm->fields.say.exp, context);
  if (stm->fields.say.exp->type == BOOLEAN_TYPE)
    lmLog(&stm->srcp, 337, sevERR, "");
}


/*----------------------------------------------------------------------

  analyzeList()

  Analyze a LIST statement.

  */
static void analyzeList(StmNod *stm,
			Context *context)	
{
  verifyContainer(stm->fields.list.wht, context);
}


/*----------------------------------------------------------------------

  anempty()

  Analyze an EMPTY statement.

  */
static void anempty(StmNod *stm,
		    Context *context)
{
  verifyContainer(stm->fields.list.wht, context);
  analyzeWhere(stm->fields.empty.where, context);
}


/*----------------------------------------------------------------------

  analyzeLocate()

  */
static void analyzeLocate(StmNod *stm,
			  Context *context)
{
  Symbol *whtSymbol = NULL;

  switch (stm->fields.locate.wht->kind) {
  case WHAT_ACTOR:
    if (context->kind == EVENT_CONTEXT)
      lmLog(&stm->fields.locate.wht->srcp, 412, sevERR, "");
    break;
  case WHAT_LOCATION:
    lmLog(&stm->srcp, 311, sevERR, "an Object or an Actor");
    break;
  case WHAT_ID:
    whtSymbol = symcheck(stm->fields.locate.wht->id, INSTANCE_SYMBOL, context);
    break;
  default:
    unimpl(&stm->srcp, "Analyzer");
    break;
  }

  analyzeWhere(stm->fields.locate.whr, context);
  switch (stm->fields.locate.whr->kind) {
  case WHR_HERE:
  case WHERE_AT:
    break;
  case WHR_IN:
    switch (stm->fields.locate.wht->kind) {
    case WHAT_LOCATION:
      lmLog(&stm->srcp, 402, sevERR, "A Location");
    case WHAT_ACTOR:
      lmLog(&stm->srcp, 402, sevERR, "An  Actor");
      break;
    case WHAT_ID:
      if (inheritsFrom(whtSymbol, actorSymbol))
	lmLog(&stm->srcp, 402, sevERR, "An Actor");
      if (inheritsFrom(whtSymbol, locationSymbol))
	lmLog(&stm->srcp, 402, sevERR, "A Location");
      break;
    default:
      syserr("Unexpected WhtKind in analyzeLocate()");
    }
    break;
  case WHR_NEAR:
    lmLog(&stm->srcp, 415, sevERR, "LOCATE");
    break;
  default:
    syserr("Unrecognized switch in analyzeLocate()");
    break;
  }
}



/*----------------------------------------------------------------------

  verifyMakeAttribute()

  Verify that a found attribute can be used in a MAKE statement.

*/
static void verifyMakeAttribute(IdNode *attributeId, Attribute *foundAttribute)
{
  if (foundAttribute != NULL) {
    if (foundAttribute->type != BOOLEAN_TYPE)
      lmLog(&attributeId->srcp, 408, sevERR, "MAKE statement");
    else
      attributeId->code = foundAttribute->id->code;
  }
}
      


/*----------------------------------------------------------------------*/
static void analyzeMake(StmNod *stm,
			Context *context)
{
  Attribute *atr = NULL;

  switch (stm->fields.make.wht->kind) {

  case WHAT_ACTOR:
    if (context->kind == EVENT_CONTEXT)
      lmLog(&stm->fields.make.wht->srcp, 412, sevERR, "");
    break;

  case WHAT_LOCATION:
  case WHAT_ID:
    break;

  default:
    unimpl(&stm->srcp, "Analyzer");
    break;
  }

  atr = resolveAttributeReference(stm->fields.make.wht, stm->fields.make.atr, context);
  verifyMakeAttribute(stm->fields.make.atr, atr);
}



/*----------------------------------------------------------------------

  verifySetTarget()

*/
static void verifySetTarget(IdNode *attributeId, Attribute  *foundAttribute)
{
  if (foundAttribute) {
    if (foundAttribute->type != INTEGER_TYPE && foundAttribute->type != STRING_TYPE)
      lmLog(&attributeId->srcp, 419, sevERR, "Target for");
    else
      attributeId->code = foundAttribute->id->code;
  }
}


/*----------------------------------------------------------------------*/
static void analyzeSet(StmNod *stm,
		       Context *context)
{
  Attribute *atr;

  switch (stm->fields.set.wht->kind) {

  case WHAT_ACTOR:
    if (context->kind == EVENT_CONTEXT)
      lmLog(&stm->fields.set.wht->srcp, 412, sevERR, "");
    break;

  case WHAT_LOCATION:
  case WHAT_ID:
    break;

  default:
    unimpl(&stm->srcp, "Analyzer");
    break;
  }
  atr = resolveAttributeReference(stm->fields.set.wht, stm->fields.set.atr, context);
  verifySetTarget(stm->fields.set.atr, atr);

  if (stm->fields.set.exp != NULL) {
    analyzeExpression(stm->fields.set.exp, context);
    if (stm->fields.set.exp->type != INTEGER_TYPE &&
        stm->fields.set.exp->type != STRING_TYPE)
      lmLog(&stm->fields.set.exp->srcp, 419, sevERR, "Expression in");
    if (atr && !equalTypes(stm->fields.set.exp->type, atr->type))
      lmLog(&stm->srcp, 331, sevERR, "SET statement");
  }
}


/*----------------------------------------------------------------------*/
static void analyzeIncrease(StmNod *stm,
			    Context *context)
{
  Attribute *atr;

  switch (stm->fields.incr.wht->kind) {

  case WHAT_ACTOR:
    if (context->kind == EVENT_CONTEXT)
      lmLog(&stm->fields.incr.wht->srcp, 412, sevERR, "");
    atr = findAttribute(NULL, stm->fields.incr.atr);
    if (atr == NULL)          /* attribute not found globally */
      lmLog(&stm->fields.incr.atr->srcp, 404, sevERR, "ACTOR");
    else
      stm->fields.incr.atr->code = atr->id->code;
    break;

  case WHAT_LOCATION:
    atr = findAttribute(NULL, stm->fields.incr.atr);
    if (atr == NULL)            /* attribute not found globally */
      lmLog(&stm->fields.incr.atr->srcp, 404, sevERR, "LOCATION");
    else
      stm->fields.incr.atr->code = atr->id->code;
    break;

  case WHAT_ID:
    atr = resolveAttributeReference(stm->fields.incr.wht, stm->fields.incr.atr, context);
    if (atr) {
      if (atr->type != INTEGER_TYPE)
        lmLog(&stm->fields.incr.atr->srcp, 413, sevERR, "INCREASE/DECREASE");
      else
        stm->fields.incr.atr->code = atr->id->code;
    }
    break;
  default:
    unimpl(&stm->srcp, "Analyzer");
    break;
  }

  if (stm->fields.incr.step != NULL) {
    analyzeExpression(stm->fields.incr.step, context);
    if (stm->fields.incr.step->type != INTEGER_TYPE)
      lmLog(&stm->fields.incr.step->srcp, 413, sevERR, "INCREASE/DECREASE");
  }
}


/*----------------------------------------------------------------------

  anschedule()

  Analyze a SCHEDULE statement.

  */
static void anschedule(StmNod *stm,
		       Context *context)
{
  Symbol *sym;

  sym = symcheck(stm->fields.schedule.id, EVENT_SYMBOL, NULL);

  /* Now lookup where */
  analyzeWhere(stm->fields.schedule.whr, context);
  switch (stm->fields.schedule.whr->kind) {
  case WHR_DEFAULT:
    stm->fields.schedule.whr->kind = WHR_HERE;
    break;
  case WHR_HERE:
  case WHERE_AT:
    break;
  case WHR_IN:
  case WHR_NEAR:
    lmLog(&stm->fields.schedule.whr->srcp, 415, sevERR, "SCHEDULE");
    break;
  default:
    syserr("Unrecognized switch in anschedule()");
    break;
  }

  /* Analyze the when expression */
  analyzeExpression(stm->fields.schedule.when, context);
  if (stm->fields.schedule.when->type != INTEGER_TYPE)
    lmLog(&stm->fields.schedule.when->srcp, 413, sevERR, "when-clause of SCHEDULE statement");

}


/*----------------------------------------------------------------------*/
static void analyzeCancel(StmNod *stm) /* IN - The statement to analyze */
{
  Symbol *sym;

  sym = symcheck(stm->fields.cancel.id, EVENT_SYMBOL, NULL);
}


/*----------------------------------------------------------------------*/
static void analyzeIf(StmNod *stm,
		      Context *context)
{
  analyzeExpression(stm->fields.iff.exp, context);
  if (!equalTypes(stm->fields.iff.exp->type, BOOLEAN_TYPE))
    lmLogv(&stm->fields.iff.exp->srcp, 330, sevERR, "boolean", "'IF'", NULL);
  analyzeStatements(stm->fields.iff.thn, context);
  if (stm->fields.iff.els != NULL)
    analyzeStatements(stm->fields.iff.els, context);
}


/*----------------------------------------------------------------------

  analyzeUse()

  Analyze a USE statement. It must refer a script that is defined
  within the mentioned actor. If the actor is not specified the
  actor is assumed to be the one we are in (it is an error if we are
  not).

  */
static void analyzeUse(StmNod *stm,
		       Context *context)
{
  Symbol *sym;
  Script *script;
  IdNode *actorId = NULL;

  if (stm->fields.use.actor == NULL && context->instance != NULL)
    lmLog(&stm->srcp, 401, sevERR, "");
  else {
    if (stm->fields.use.actor != NULL) {
      /* Lookup specified actors symbol */
      sym = symcheck(stm->fields.use.actor, INSTANCE_SYMBOL, context);
      if (sym) {
	if (sym->kind == PARAMETER_SYMBOL)
	  lmLog(&stm->fields.use.actor->srcp, 410, sevERR, "USE statement");
	actorId = sym->fields.entity.props->id;
      }
    } else {
      if (context->instance == NULL && context->instance->props == NULL)
	syserr("Unexpected context in analyzeUse()");
      actorId = context->instance->props->id;
    }

    /* Find the script */
    script = lookupScript(actorId->symbol, stm->fields.use.script);
    if (script != NULL)
      stm->fields.use.scriptno = script->id->code;
    else
      lmLog(&stm->fields.use.script->srcp, 400, sevERR, actorId->string);
  }  
}  


/*----------------------------------------------------------------------

  Analyze a DEPENDING statement. It has partial expressions in the
  cases which must be connected to the depend expression.

  */
static void analyzeDepend(StmNod *stm, Context *context)
{
  List *cases;

 /* The expression will be analysed once for each case so no need to
    do this separately, is there?

    FIXME - performance may be somewhat improved by not re-analyze the
    expression for every case => some indication of an anlyzed
    expression must be available (the type?) in the expressions nodes.
 */

  for (cases = stm->fields.depend.cases; cases != NULL; cases =
	 cases->next) {

    if (cases->element.stm->fields.depcase.exp != NULL) {
      /* Unless it is an ELSE clause set left hand of case expression
         to be the depend expression */
      switch (cases->element.stm->fields.depcase.exp->kind) {
      case BINARY_EXPRESSION:
	cases->element.stm->fields.depcase.exp->fields.bin.left =
	  stm->fields.depend.exp;
	break;
      case WHERE_EXPRESSION:
	cases->element.stm->fields.depcase.exp->fields.whr.wht =
	  stm->fields.depend.exp;
	break;
      case ATTRIBUTE_EXPRESSION:
	cases->element.stm->fields.depcase.exp->fields.atr.wht =
	  stm->fields.depend.exp;
	break;
      case BETWEEN_EXPRESSION:
	cases->element.stm->fields.depcase.exp->fields.btw.val =
	  stm->fields.depend.exp;
	break;
      default:
	syserr("andep(): Unrecognized switch case on expkd.");
      }
    } else
      /* If this is an ELSE-case there can not be any other afterwards */
      if (cases->next != NULL)
	lmLog(&cases->element.stm->srcp, 335, sevERR, "");	

    /* Analyze the expression and the statements */
    analyzeExpression(cases->element.stm->fields.depcase.exp, context);
    analyzeStatements(cases->element.stm->fields.depcase.stms, context);

  }
}



/*----------------------------------------------------------------------*/
static void analyzeEach(StmNod *stm,
			Context *context)
{
  Symbol *classSymbol = NULL;
  Symbol *loopSymbol;

  /* Analyze loop class and identifier */
  if (stm->fields.each.classId != NULL)
    classSymbol = symcheck(stm->fields.each.classId, CLASS_SYMBOL, context);

  /* Create a new frame and register the loop variable */
  newFrame();
  loopSymbol = newSymbol(stm->fields.each.loopId, LOCAL_SYMBOL);
  if (classSymbol != NULL)
    loopSymbol->fields.local.class = classSymbol;
  else
    loopSymbol->fields.local.class = entitySymbol;
  /* Can only loop over instances */
  loopSymbol->fields.local.type = INSTANCE_TYPE;

  /* Analyze the statements in the loop body */
  analyzeStatements(stm->fields.each.stms, context);

  deleteFrame();
}


/*----------------------------------------------------------------------*/
static void analyzeStatement(StmNod *stm,
			     Context *context)
{
  switch (stm->class) {
  case NOP_STATEMENT:
  case PRINT_STATEMENT:
  case QUIT_STATEMENT:
  case LOOK_STATEMENT:
  case SAVE_STATEMENT:
  case RESTORE_STATEMENT:
  case RESTART_STATEMENT:
  case VISITS_STATEMENT:
  case SYSTEM_STATEMENT:
    /* Nothing to analyse */
    break;
  case SCORE_STATEMENT:
    if (stm->fields.score.count != 0) {
      adv.scores[stm->fields.score.count] = stm->fields.score.score;
      scotot += stm->fields.score.score;
    }
    break;
  case DESCRIBE_STATEMENT:
    andescribe(stm, context);
    break;
  case SAY_STATEMENT:
    analyzeSay(stm, context);
    break;
  case LIST_STATEMENT:
    analyzeList(stm, context);
    break;
  case EMPTY_STATEMENT:
    anempty(stm, context);
    break;
  case LOCATE_STATEMENT:
    analyzeLocate(stm, context);
    break;
  case MAKE_STATEMENT:
    analyzeMake(stm, context);
    break;
  case SET_STATEMENT:
    analyzeSet(stm, context);
    break;
  case INCREASE_STATEMENT:
  case DECREASE_STATEMENT:
    analyzeIncrease(stm, context);
    break;
  case SCHEDULE_STATEMENT:
    anschedule(stm, context);
    break;
  case CANCEL_STATEMENT:
    analyzeCancel(stm);
    break;
  case IF_STATEMENT:
    analyzeIf(stm, context);
    break;
  case USE_STATEMENT:
    analyzeUse(stm, context);
    break;
  case DEPEND_STATEMENT:
    analyzeDepend(stm, context);
    break;
  case EACH_STATEMENT:
    analyzeEach(stm, context);
    break;
  default:
    unimpl(&stm->srcp, "Analyzer");
    break;
  }
}



/*======================================================================*/
void analyzeStatements(List *stms,
		       Context *context)
{
  while (stms != NULL) {
    analyzeStatement(stms->element.stm, context);
    stms = stms->next;
  }
}


/*----------------------------------------------------------------------

  Generate the code for a PRINT-stm. The text is found and copied to the
  data file (and encoded if requested!).

  */
static void generatePrint(StmNod *stm)
{
  if (!stm->fields.print.encoded)
    encode(&stm->fields.print.fpos, &stm->fields.print.len);
  stm->fields.print.encoded = TRUE;
  emit2(I_PRINT, stm->fields.print.fpos, stm->fields.print.len);
}



/*----------------------------------------------------------------------*/
static void generateScore(StmNod *stm)
{
  emitConstant(stm->fields.score.count);
  emit0(I_SCORE);
}



/*----------------------------------------------------------------------*/
static void generateDescribe(StmNod *stm)
{
  switch (stm->fields.describe.wht->kind) {

  case WHAT_LOCATION:
    emitVariable(V_CURLOC);
    break;

  case WHAT_ACTOR:
    emitVariable(V_CURACT);
    break;

  case WHAT_ID:
    generateId(stm->fields.describe.wht->id);
    break;

  default:
    unimpl(&stm->srcp, "Code Generator");
    return;
  }
  emit0(I_DESCRIBE);
}


/*----------------------------------------------------------------------*/
static void generateSay(StmNod *stm)
{
  generateExpression(stm->fields.say.exp);
  switch (stm->fields.say.exp->type) {
  case INTEGER_TYPE:
    emit0(I_SAYINT);
    break;
  case STRING_TYPE:
    emit0(I_SAYSTR);
    break;
  case INSTANCE_TYPE:
    emit0(I_SAY);
    break;
  default:
    unimpl(&stm->srcp, "Code Generator");
    break;
  }
}


/*----------------------------------------------------------------------*/
static void generateList(StmNod *stm)
{
  if (stm->fields.list.wht->kind == WHAT_ID) {
    generateId(stm->fields.list.wht->id);
    emit0(I_LIST);
  } else
    unimpl(&stm->srcp, "Code Generator");
}



/*----------------------------------------------------------------------*/
static void generateEmpty(StmNod *stm)
{
  if (stm->fields.empty.wht->kind == WHAT_ID) {
    generateWhere(stm->fields.empty.where);
    generateId(stm->fields.empty.wht->id);
    emit0(I_EMPTY);
  } else
    unimpl(&stm->srcp, "Code Generator");
}



/*----------------------------------------------------------------------*/
static void generateLocate(StmNod *stm)
{
  generateWhere(stm->fields.locate.whr);
  generateWhat(stm->fields.locate.wht);
  emit0(I_LOCATE);
}



/*----------------------------------------------------------------------*/
static void generateMake(StmNod *stm)
{
  emitConstant(!stm->fields.make.not);
  emitConstant(stm->fields.make.atr->code);
  generateWhat(stm->fields.make.wht);
  emit0(I_MAKE);
}




/*----------------------------------------------------------------------*/
static void generateSet(StmNod *stm)
{
  generateExpression(stm->fields.set.exp);

  emitConstant(stm->fields.set.atr->code);
  generateWhat(stm->fields.set.wht);
  if (stm->fields.set.exp->type == STRING_TYPE)
    emit0(I_STRSET);
  else
    emit0(I_SET);
}



/*----------------------------------------------------------------------*/
static void generateIncrease(StmNod *stm)
{
  if (stm->fields.incr.step != NULL)
    generateExpression(stm->fields.incr.step);
  else
    emitConstant(1);

  emitConstant(stm->fields.incr.atr->code);
  generateWhat(stm->fields.incr.wht);
  if (stm->class == INCREASE_STATEMENT)
    emit0(I_INCR);
  else
    emit0(I_DECR);
}



/*----------------------------------------------------------------------*/
static void generateSchedule(StmNod *stm)
{
  generateExpression(stm->fields.schedule.when);

  /* NOTE: we can't use gewhr() because the semantics of the schedule */
  /* statement is such that at scheduling AT something does not mean */
  /* where that something is now but where it is when the event is run! */
  switch (stm->fields.schedule.whr->kind) {
  case WHR_DEFAULT:
  case WHR_HERE:
    emitVariable(V_CURLOC);
    break;
    
  case WHERE_AT:
    generateWhat(stm->fields.schedule.whr->what);
    break;

  default:
    unimpl(&stm->srcp, "Code Generator");
    return;
  }
  generateId(stm->fields.schedule.id);
  emit0(I_SCHEDULE);
}


/*----------------------------------------------------------------------*/
static void generateCancel(StmNod *stm) /* IN - Statement to generate */
{
  generateId(stm->fields.schedule.id);
  emit0(I_CANCEL);
}

/*----------------------------------------------------------------------*/
static void generateIf(StmNod *stm)
{
  generateExpression(stm->fields.iff.exp);
  emit0(I_IF);
  generateStatements(stm->fields.iff.thn);
  if (stm->fields.iff.els != NULL) {
    emit0(I_ELSE);
    generateStatements(stm->fields.iff.els);
  }
  emit0(I_ENDIF);
}



/*----------------------------------------------------------------------*/
static void generateUse(StmNod *stm)
{
  if (stm->fields.use.actor == NULL) { /* No actor specified, use current */
    emitConstant(stm->fields.use.scriptno);
    emitVariable(V_CURRENT_INSTANCE);
    emit0(I_USE);
  } else {
    emitConstant(stm->fields.use.scriptno);
    generateId(stm->fields.use.actor);
    emit0(I_USE);
  }
}



/*----------------------------------------------------------------------

  Generate just the right hand part of the expression and the
  operator of a DEPEND case.

*/
static void generateDependCase(Expression *exp)
{
  switch (exp->kind) {
  case BINARY_EXPRESSION:
    generateExpression(exp->fields.bin.right);
    generateBinaryOperator(exp);
    break;
  case ATTRIBUTE_EXPRESSION:
    generateId(exp->fields.atr.atr);
    generateAttributeAccess(exp);
    break;
  case BETWEEN_EXPRESSION:
    generateBetweenCheck(exp);
    break;
  default:
    syserr("generateDependingCase(): Unrecognized switch case on expression kind.");
  }
}


/*----------------------------------------------------------------------

  Generate DEPENDING statement.

  Code generation principle:				Stack:

      DEPEND

      depend expression					d-exp

      DEPCASE ----------+ (not present for first case)	d-exp
      DUP               |				d-exp	d-exp  
      case1 expression  |				c-exp	d-exp	d-exp
      case1 operator    |				case?	d-exp
      DEPEXEC           > repeat for each case		d-exp
      stms1 ------------+

      DEPELSE--+ optional
      stmsn----+

      ENDDEP

  DEPSTART does nothing but must be there to indicate start of a new
  level for skipping over statements.

  Executing a DEPCASE or DEPELSE indicates the end of executing a
  matching case so skip to the ENDDEP (on this level).

  After the DEPCASE is a DUP to duplicate the depend expression, then
  comes the case expression and then the operator which does the
  compare.

  DEPEXEC inspects the results on the stack top and if true continues
  else skips to the instruction after next DEPCASE, DEPELSE or to the ENDDEP.

  ENDDEP just pops off the initially pushed depend expression.

  */
static void generateDepend(StmNod *stm)
{
  List *cases;

  emit0(I_DEPEND);
  generateExpression(stm->fields.depend.exp);
  /* For each case: */
  for (cases = stm->fields.depend.cases; cases != NULL; cases = cases->next) {
    /* If it is not the ELSE clause ... */
    if (cases->element.stm->fields.depcase.exp != NULL) {
      /* Generate a DEPCASE (if not first case) and a DUP */
      if (cases != stm->fields.depend.cases)
	emit0(I_DEPCASE);
      emit0(I_DUP);
      /* ...and the case expression (right hand + operator) */
      generateDependCase(cases->element.stm->fields.depcase.exp);
      emit0(I_DEPEXEC);
    } else
      emit0(I_DEPELSE);
    /* ...and then the statements */
    generateStatements(cases->element.stm->fields.depcase.stms);
  }
  emit0(I_ENDDEP);
}

/*----------------------------------------------------------------------*/
static void generateEach(StmNod *statement)
{
  /* Generate a new FRAME */
  emit1(I_FRAME, 1);		/* One local variable in this block */
  frameLevel++;

  /* Loop variable is initialised to 0
     which works since the EACH statement will
     increment it to 1 (first instance number) */

  /* Start of loop */
  emit1(I_EACH, 1);

  /* Generate check for class membership */
  if (statement->fields.each.classId) {
    emit2(I_GETLOCAL, 0, 1);
    generateId(statement->fields.each.classId);
    emit0(I_ISA);
    emit0(I_NOT);
    emit0(I_IF);
    emit0(I_NEXTEACH);
    emit0(I_ENDIF);
  }

  generateStatements(statement->fields.each.stms);

  /* End of loop */
  emit0(I_ENDEACH);

  /* End of block */
  emit0(I_ENDFRAME);
  frameLevel--;
}



/*----------------------------------------------------------------------*/
static void generateSystem(StmNod *stm)
{
  encode(&stm->fields.system.fpos, &stm->fields.system.len);
  emitConstant(stm->fields.system.len);
  emitConstant(stm->fields.system.fpos);
  emit0(I_SYSTEM);
}



/*----------------------------------------------------------------------*/
static void generateStatement(StmNod *stm)
{
  switch (stm->class) {

  case NOP_STATEMENT:
    break;

  case PRINT_STATEMENT:
    generatePrint(stm);
    break;

  case QUIT_STATEMENT:
    emit0(I_QUIT);
    break;

  case LOOK_STATEMENT:
    emit0(I_LOOK);
    break;

  case SAVE_STATEMENT:
    emit0(I_SAVE);
    break;

  case RESTORE_STATEMENT:
    emit0(I_RESTORE);
    break;

  case RESTART_STATEMENT:
    emit0(I_RESTART);
    break;

  case VISITS_STATEMENT:
    emitConstant(stm->fields.visits.count);
    emit0(I_VISITS);
    break;

  case SCORE_STATEMENT:
    generateScore(stm);
    break;

  case DESCRIBE_STATEMENT:
    generateDescribe(stm);
    break;

  case SAY_STATEMENT:
    generateSay(stm);
    break;

  case LIST_STATEMENT:
    generateList(stm);
    break;

  case EMPTY_STATEMENT:
    generateEmpty(stm);
    break;

  case LOCATE_STATEMENT:
    generateLocate(stm);
    break;

  case MAKE_STATEMENT:
    generateMake(stm);
    break;

  case SET_STATEMENT:
    generateSet(stm);
    break;

  case INCREASE_STATEMENT:
  case DECREASE_STATEMENT:
    generateIncrease(stm);
    break;

  case SCHEDULE_STATEMENT:
    generateSchedule(stm);
    break;

  case CANCEL_STATEMENT:
    generateCancel(stm);
    break;

  case IF_STATEMENT:
    generateIf(stm);
    break;

  case USE_STATEMENT:
    generateUse(stm);
    break;

  case DEPEND_STATEMENT:
    generateDepend(stm);
    break;

  case SYSTEM_STATEMENT:
    generateSystem(stm);
    break;

  case EACH_STATEMENT:
    generateEach(stm);
    break;

  default:
    unimpl(&stm->srcp, "Code Generator");
    break;
  }
}


/*======================================================================*/
void generateStatements(List *stms)
{
  List *current = stms;

  for (current = stms; current != NULL; current = current->next) {
    generateStatement(current->element.stm);
  }
}



/*======================================================================

  dustm()

  Dump a statement node.

  */
void dustm(StmNod *stm)
{
  if (stm == NULL) {
    put("NULL");
    return;
  }

  put("STM: ");
  switch(stm->class) {
  case PRINT_STATEMENT:
    put("PRINT ");
    break;
  case DESCRIBE_STATEMENT:
    put("DESCRIBE ");
    break;
  case SAY_STATEMENT:
    put("SAY ");
    break;
  case LIST_STATEMENT:
    put("LIST ");
    break;
  case IF_STATEMENT:
    put("IF ");
    break;
  case MAKE_STATEMENT:
    put("MAKE ");
    break;
  case SET_STATEMENT:
    put("SET ");
    break;
  case INCREASE_STATEMENT:
    put("INCR ");
    break;
  case DECREASE_STATEMENT:
    put("DECR ");
    break;
  case LOCATE_STATEMENT:
    put("LOCATE ");
    break;
  case EMPTY_STATEMENT:
    put("EMPTY ");
    break;
  case SCHEDULE_STATEMENT:
    put("SCHEDULE ");
    break;
  case CANCEL_STATEMENT:
    put("CANCEL ");
    break;
  case LOOK_STATEMENT:
    put("LOOK ");
    break;
  case QUIT_STATEMENT:
    put("QUIT ");
    break;
  case SCORE_STATEMENT:
    put("SCORE ");
    break;
  case USE_STATEMENT:
    put("USE ");
    break;
  case SAVE_STATEMENT:
    put("SAVE ");
    break;
  case RESTORE_STATEMENT:
    put("RESTORE ");
    break;
  case RESTART_STATEMENT:
    put("RESTART ");
    break;
  case VISITS_STATEMENT:
    put("VISITS ");
    break;
  default:
    put("*** UNKNOWN ***");
    break;
  }
  dumpSrcp(&stm->srcp);

  switch(stm->class) {
  case LOOK_STATEMENT:
  case QUIT_STATEMENT:
  case SAVE_STATEMENT:
  case RESTORE_STATEMENT:
  case RESTART_STATEMENT:
    break;
  default:
    in();
    switch(stm->class) {
    case PRINT_STATEMENT:
      put("fpos: "); dumpInt(stm->fields.print.fpos); nl();
      put("len: "); dumpInt(stm->fields.print.len);
      break;
    case SCORE_STATEMENT:
      put("count: "); dumpInt(stm->fields.score.count); nl();
      put("score: "); dumpInt(stm->fields.score.score);
      break;
    case DESCRIBE_STATEMENT:
      put("wht: "); dumpWhat(stm->fields.describe.wht);
      break;
    case SAY_STATEMENT:
      put("exp: "); dumpExpression(stm->fields.say.exp);
      break;
    case LIST_STATEMENT:
      put("wht: "); dumpWhat(stm->fields.list.wht);
      break;
    case EMPTY_STATEMENT:
      put("wht: "); dumpWhat(stm->fields.empty.wht); nl();
      put("whr: "); duwhr(stm->fields.empty.where);
      break;
    case LOCATE_STATEMENT:
      put("wht: "); dumpWhat(stm->fields.locate.wht); nl();
      put("whr: "); duwhr(stm->fields.locate.whr);
      break;
    case MAKE_STATEMENT:
      put("wht: "); dumpWhat(stm->fields.list.wht); nl();
      put("not: "); dumpBool(stm->fields.make.not); nl();
      put("atr: "); dumpId(stm->fields.make.atr);
      break;
    case SET_STATEMENT:
      put("wht: "); dumpWhat(stm->fields.set.wht); nl();
      put("atr: "); dumpId(stm->fields.set.atr); nl();
      put("exp: "); dumpExpression(stm->fields.set.exp);
      break;
    case INCREASE_STATEMENT:
    case DECREASE_STATEMENT:
      put("wht: "); dumpWhat(stm->fields.incr.wht); nl();
      put("atr: "); dumpId(stm->fields.incr.atr); nl();
      put("step: "); dumpExpression(stm->fields.incr.step);
      break;
    case SCHEDULE_STATEMENT:
      put("id: "); dumpId(stm->fields.schedule.id); nl();
      put("whr: "); duwhr(stm->fields.locate.whr); nl();
      put("when: "); dumpExpression(stm->fields.schedule.when);
      break;
    case CANCEL_STATEMENT:
      put("id: "); dumpId(stm->fields.cancel.id);
      break;
    case IF_STATEMENT:
      put("exp: "); dumpExpression(stm->fields.iff.exp); nl();
      put("thn: "); dumpList(stm->fields.iff.thn, STATEMENT_LIST); nl();
      put("els: "); dumpList(stm->fields.iff.els, STATEMENT_LIST);
      break;
    case USE_STATEMENT:
      put("script: "); dumpId(stm->fields.use.script); nl();
      put("scriptno: "); dumpInt(stm->fields.use.scriptno); nl();
      put("actor: "); dumpId(stm->fields.use.actor);
      break;
    case VISITS_STATEMENT:
      put("count: "); dumpInt(stm->fields.visits.count);
      break;
    default:
      break;
    }
    out();
  }
}
