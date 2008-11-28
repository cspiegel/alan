#include "AltInfo.h"

/*----------------------------------------------------------------------*\

  An info node about the Verb Alternatives found and possibly executed

\*----------------------------------------------------------------------*/

#include "types.h"
#include "options.h"
#include "main.h"
#include "inter.h"
#include "parse.h"
#include "debug.h"
#include "CheckEntryArray.h"


/*======================================================================*/
// TODO This is actually an AltInfoArray function since primeAltInfo modifies the
// next element in the array too!
void primeAltInfo(AltInfo *altInfo, int level, int parameter, int instance, int class)
{
  altInfo->level = level;
  altInfo->parameter = parameter;
  altInfo->instance = instance;
  altInfo->class = class;
  altInfo->done = FALSE;
  altInfo->end = FALSE;

  altInfo++;
  altInfo->end = TRUE;

}

/*----------------------------------------------------------------------*/
static void traceAltInfo(AltInfo *alt) {
  switch (alt->level) {
  case GLOBAL_LEVEL:
    printf("GLOBAL");
    break;
  case LOCATION_LEVEL:
    printf("in LOCATION");
    // TODO: need to handle nested locations and inheritance too
    break;
  case PARAMETER_LEVEL:
    printf("in parameter #%d (", alt->parameter);
    traceSay(parameters[alt->parameter-1].instance);
    printf(")");
    if (alt->class != NO_CLASS)
      printf(", inherited from class #%d (%s)", alt->class, (char *)pointerTo(class[alt->class].id));
    break;
  }
}


/*----------------------------------------------------------------------*/
static void traceVerbCheck(AltInfo *alt)
{
  printf("\n<VERB %d, ", current.verb);
  traceAltInfo(alt);
  printf(", CHECK:>\n");
}


/*======================================================================*/
Bool checkPerformedOk(AltInfo *alt, Bool execute)
{
  if (alt->alt != NULL && alt->alt->checks != 0) {
    if (sectionTraceOption && execute)
      traceVerbCheck(alt);
    if (!tryChecks(alt->alt->checks, execute)) return FALSE;
    if (fail) return FALSE;
  }
  return TRUE;
}


/*======================================================================*/
static void traceVerbExecution(AltInfo *alt)
{
  if (sectionTraceOption) {
    printf("\n<VERB %d, ", current.verb);
    traceAltInfo(alt);
    printf(", DOES");
    switch (alt->alt->qual) {
    case Q_BEFORE: printf(" (BEFORE)"); break;
    case Q_ONLY: printf(" (ONLY)"); break;
    case Q_AFTER: printf(" (AFTER)"); break;
    case Q_DEFAULT: break;
    }
    printf(":>\n");  }
}


/*======================================================================*/
Bool executedOk(AltInfo *altInfo)
{
  fail = FALSE;
  if (!altInfo->done && altInfo->alt->action != 0) {
    traceVerbExecution(altInfo);
    current.instance = altInfo->instance;
    interpret(altInfo->alt->action);
  }
  altInfo->done = TRUE;
  return !fail;
}


/*======================================================================*/
Bool executable(AltInfo *altInfo) {
  return altInfo->alt != NULL && altInfo->alt->action != 0;
}
