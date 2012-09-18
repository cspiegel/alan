/*======================================================================*\

  addTest.c

  Unit tests for ADD node in the Alan compiler

\*======================================================================*/

#include "add.c"

#include <cgreen/cgreen.h>

#include "cla_x.h"
#include "sym.h"


Ensure(testMultipleAddAttribute) {
  IdNode *theId = newId(nulsrcp, "aClassId");
  Class *theClass;
  Attribute *theFirstAttribute = newBooleanAttribute(nulsrcp, newId(nulsrcp, "firstAttribute"), FALSE);
  Attribute *theSecondAttribute = newBooleanAttribute(nulsrcp, newId(nulsrcp, "secondAttribute"), FALSE);
  Properties *theFirstAttributeProps = newProps(NULL, NULL,
						nulsrcp, NULL,
						newList(theFirstAttribute, ATTRIBUTE_LIST),
						NULL, NULL,
						nulsrcp, NULL, NULL, NULL, NULL,
						NULL, NULL,
						nulsrcp, NULL,
						NULL, NULL);
  Properties *theSecondAttributeProps = newProps(NULL, NULL,
						 nulsrcp, NULL,
						 newList(theSecondAttribute, ATTRIBUTE_LIST),
						 NULL, NULL,
						 nulsrcp, NULL, NULL, NULL, NULL,
						 NULL, NULL,
						 nulsrcp, NULL,
						 NULL, NULL);
  AddNode *add1 = newAdd(nulsrcp, theId, NULL, theFirstAttributeProps);
  AddNode *add2 = newAdd(nulsrcp, theId, NULL, theSecondAttributeProps);
  Symbol *aSymbol;

  theClass = newClass(&nulsrcp, theId, NULL, NULL);
  aSymbol = lookup("aClassId");
  assert_true(aSymbol != NULL);
  assert_true(length(aSymbol->fields.entity.props->attributes) == 0);

  addAttributes(add1, aSymbol);
  assert_true(length(aSymbol->fields.entity.props->attributes) == 1);
  
  addAttributes(add2, aSymbol);
  assert_true(length(aSymbol->fields.entity.props->attributes) == 2);
}


Ensure(testAddDescription) {
  Description *addedDescription = newDescription(nulsrcp, NULL, nulsrcp, NULL);
  Properties *addProps = newProps(NULL, NULL,
				  nulsrcp, NULL,
				  NULL, NULL, /*description*/ NULL,
				  nulsrcp, NULL, NULL, NULL, NULL,
				  NULL, NULL,
				  nulsrcp, NULL,
				  NULL, NULL);
  AddNode *add = newAdd(nulsrcp, NULL, NULL, addProps);

  Properties *originalProps = newProps(NULL, NULL,
				       nulsrcp, NULL,
				       NULL, NULL, /*description*/ NULL,
				       nulsrcp, NULL, NULL, NULL, NULL,
				       NULL, NULL,
				       nulsrcp, NULL,
				       NULL, NULL);
  Symbol *symbol = newClassSymbol(newId(nulsrcp, "testAddId"), originalProps, NULL);
  List list;			/* Dummy list */

  /* Start by testing when the target symbols props are NULL */
  /* Added description is NULL */
  addDescriptionCheck(add, symbol);
  assert_true(symbol->fields.entity.props->description == NULL);

  /* Added description exists but empty */
  addProps->description = addedDescription;
  addDescriptionCheck(add, symbol);
  assert_true(symbol->fields.entity.props->description == NULL);

  addedDescription->checks = &list;
  addDescriptionCheck(add, symbol);
  assert_true(symbol->fields.entity.props->description->checks == &list);


  /* Now test descriptionDoes */
  /* No description */
  originalProps->description = NULL; /* Reset to empty */
  addProps->description = NULL; /* Reset to empty */
  addDescription(add, symbol);
  assert_true(symbol->fields.entity.props->description == NULL);

  /* Empty description */
  addProps->description = addedDescription;
  addDescription(add, symbol);
  assert_true(symbol->fields.entity.props->description == NULL);

  /* Description with does */
  originalProps->description = NULL; /* Reset to empty */
  addedDescription->does = &list;
  addDescription(add, symbol);
  assert_true(symbol->fields.entity.props->description->does == &list);
}

TestSuite *addTests() {
    TestSuite *suite = create_test_suite();
    add_test(suite, testMultipleAddAttribute);
    add_test(suite, testAddDescription);
    return suite;
}
