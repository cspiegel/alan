/*======================================================================*\

  whrTest.c

  Unit tests for WHR node in the Alan compiler

\*======================================================================*/

#include "whr.c"

void testInitialLocation()
{
  IdNode *id = newId(&nulsrcp, "atLoc");
  Where *whr = newwhr(&nulsrcp, WHERE_AT, newWhat(&nulsrcp, WHAT_ID, id));
  Instance *atLoc = newInstance(&nulsrcp, id, NULL, NULL);

  symbolizeInstance(atLoc);
  symbolizeWhere(whr);
  ASSERT(generateInitialLocation(NULL) == 0);

  ASSERT(generateInitialLocation(whr) == atLoc->props->id->symbol->code);
}

void registerWhrUnitTests()
{
  registerUnitTest(testInitialLocation);
}

