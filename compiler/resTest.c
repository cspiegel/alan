/*======================================================================*\

  resTest.c

  Unit tests for Restriction Nodes in the Alan compiler

\*======================================================================*/

#include "res.c"

#include <cgreen/cgreen.h>

#include "stx_x.h"
#include "elm_x.h"


Describe(Restriction);
BeforeEach(Restriction) {}
AfterEach(Restriction) {}

Ensure(Restriction, can_say_has_restriction_for_symbol) {
  Symbol *verbSymbol = newVerbSymbol(newId(nulsrcp, "verb"));

  IdNode *parameterId1 = newId(nulsrcp, "p1");
  IdNode *parameterId2 = newId(nulsrcp, "p2");

  Restriction *res1 = newRestriction(nulsrcp, parameterId1, ID_RESTRICTION, newId(nulsrcp, "resclas"), NULL);
  List *restrictions = concat(NULL, res1, RESTRICTION_LIST);
  Syntax *syntax = newSyntaxWithEOS(nulsrcp, NULL, restrictions, nulsrcp);

  addElement(syntax, newParameterElement(nulsrcp, parameterId1, 0));
  addElement(syntax, newParameterElement(nulsrcp, parameterId2, 0));

  List *parameterList = concat(concat(NULL, getMember(syntax->elements, 1), ELEMENT_LIST), getMember(syntax->elements, 2), ELEMENT_LIST);

  setParameters(verbSymbol, parameterList);
  assert_true(hasRestriction(parameterId1->symbol, syntax));
  assert_true(!hasRestriction(parameterId2->symbol, syntax));
}


TestSuite *resTests()
{
    TestSuite *suite = create_test_suite(); 

    add_test_with_context(suite, Restriction, can_say_has_restriction_for_symbol);

    return suite;
}

