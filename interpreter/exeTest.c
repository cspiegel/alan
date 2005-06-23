/*======================================================================*\

  exeTest.c

  Unit tests for exe module in the Alan interpreter

\*======================================================================*/

#include "set.h"
#include "exe.c"


static void testCountTrailingBlanks() {
  char *threeBlanks = "h   ";
  char *fiveBlanks = "     ";
  char *empty = "";
  ASSERT(countTrailingBlanks(threeBlanks, strlen(threeBlanks)-1) == 3);
  ASSERT(countTrailingBlanks(threeBlanks, 1) == 1);
  ASSERT(countTrailingBlanks(threeBlanks, 0) == 0);
  ASSERT(countTrailingBlanks(fiveBlanks, strlen(fiveBlanks)-1) == 5);
  ASSERT(countTrailingBlanks(empty, strlen(empty)-1) == 0);
}

static void testSkipWordForwards() {
  char *string = "a string of words";

  ASSERT(skipWordForwards(string, 0) == 1);
  ASSERT(skipWordForwards(string, 2) == 8);
  ASSERT(skipWordForwards(string, 9) == 11);
  ASSERT(skipWordForwards(string, 12) == 17);
  ASSERT(skipWordForwards(string, strlen(string)-1) == strlen(string));
}

static void testSkipWordBackwards() {
  char *string = "a string of words";
  char *emptyString = "";

  ASSERT(skipWordBackwards(string, 0) == 0);
  ASSERT(skipWordBackwards(string, 4) == 2);
  ASSERT(skipWordBackwards(string, 10) == 9);
  ASSERT(skipWordBackwards(string, strlen(string)) == 12);

  ASSERT(skipWordBackwards(emptyString, strlen(emptyString)) == 0);
}

static void testStripCharsFromString() {
  char *characters;
  char *rest;
  char *result;

  characters = "abcdef";
  result = stripCharsFromStringForwards(3, characters, &rest);
  ASSERT(strcmp(result, "abc")==0);
  ASSERT(strcmp(rest, "def")==0);

  characters = "ab";
  result = stripCharsFromStringForwards(3, characters, &rest);
  ASSERT(strcmp(result, "ab")==0);
  ASSERT(strcmp(rest, "")==0);

  characters = "";
  result = stripCharsFromStringForwards(3, characters, &rest);
  ASSERT(strcmp(result, "")==0);
  ASSERT(strcmp(rest, "")==0);

  characters = "abcdef";
  result = stripCharsFromStringBackwards(3, characters, &rest);
  ASSERT(strcmp(result, "def")==0);
  ASSERT(strcmp(rest, "abc")==0);

  characters = "ab";
  result = stripCharsFromStringBackwards(3, characters, &rest);
  ASSERT(strcmp(result, "ab")==0);
  ASSERT(strcmp(rest, "")==0);

  characters = "";
  result = stripCharsFromStringBackwards(3, characters, &rest);
  ASSERT(strcmp(result, "")==0);
  ASSERT(strcmp(rest, "")==0);
}

static void testStripWordsFromString() {
  char *testString = "this is four  words";
  char *empty = "";
  char *result;
  char *rest;

  result = stripWordsFromStringForwards(3, empty, &rest);
  ASSERT(strcmp(result, "") == 0);
  ASSERT(strcmp(rest, "") == 0);

  result = stripWordsFromStringForwards(3, testString, &rest);
  ASSERT(strcmp(result, "this is four") == 0);
  ASSERT(strcmp(rest, "words") == 0);

  result = stripWordsFromStringForwards(7, testString, &rest);
  ASSERT(strcmp(result, "this is four  words") == 0);
  ASSERT(strcmp(rest, "") == 0);

  result = stripWordsFromStringBackwards(3, empty, &rest);
  ASSERT(strcmp(result, "") == 0);
  ASSERT(strcmp(rest, "") == 0);

  result = stripWordsFromStringBackwards(3, testString, &rest);
  ASSERT(strcmp(result, "is four  words") == 0);
  ASSERT(strcmp(rest, "this") == 0);

  result = stripWordsFromStringBackwards(7, testString, &rest);
  ASSERT(strcmp(result, "this is four  words") == 0);
  ASSERT(strcmp(rest, "") == 0);

  testString = " an initial space";
  result = stripWordsFromStringBackwards(7, testString, &rest);
  ASSERT(strcmp(result, "an initial space") == 0);
  ASSERT(strcmp(rest, "") == 0);
}


static char testFileName[] = "getStringTestFile";
static FILE *testFile;
static void writeAndOpenGetStringTestFile(int fpos, char *testString)
{
  int i;

  testFile = fopen(testFileName, "wb");
  for (i = 0; i < fpos; i++) fputc(' ', testFile);
  fprintf(testFile, testString);
  fclose(testFile);
  textFile = fopen(testFileName, "rb");
}


void testGetString()
{
  int fpos = 55;
  char testString[] = "hejhopp";

  writeAndOpenGetStringTestFile(fpos, testString);
  header->pack = FALSE;
  header->stringOffset = 0;
  ASSERT(strcmp(getStringFromFile(fpos, strlen(testString)), testString)==0);
  header->stringOffset = 1;
  ASSERT(strcmp(getStringFromFile(fpos, strlen(testString)-1), &testString[1])==0);
  fclose(textFile);
  unlink(testFileName);
}


static void testIncreaseEventQueue()
{
  eventQueueSize = 0;
  eventQueue = NULL;
  eventQueueTop = 0;

  increaseEventQueue();

  ASSERT(eventQueueSize != 0);
  ASSERT(eventQueue != NULL);
}

static void testPushGameState() {
  admin = allocate(sizeof(AdminEntry));
  attributes = allocate(3);
  gameState = NULL;
  gameStateTop = 0;
  pushGameState();
  ASSERT(gameState != NULL);
  ASSERT(gameStateTop == 1);
}


static void testPopGameState() {
  attributes = allocate(3*sizeof(AttributeEntry));
  admin = allocate(3*sizeof(AdminEntry));

  header->attributesAreaSize = 3*sizeof(AttributeEntry)/sizeof(Aword);
  header->instanceMax = 2;

  gameState = NULL;
  gameStateTop = 0;
  gameStateSize = 0;
  attributes[0].value = 12;
  attributes[2].value = 3;
  pushGameState();
  ASSERT(gameState != NULL);
  ASSERT(gameStateTop == 1);

  eventQueue = allocate(5*sizeof(EventQueueEntry));
  eventQueueTop = 2;
  eventQueue[1].time = 47;
  attributes[0].value = 11;
  attributes[2].value = 4;
  admin[2].location = 12;
  admin[2].alreadyDescribed = 2;
  admin[2].visitsCount = 13;
  admin[2].script = 33;
  admin[2].step = 3886;
  admin[2].waitCount = 38869878;
  pushGameState();

  admin[2].location = 55;
  admin[2].alreadyDescribed = 55;
  admin[2].visitsCount = 55;
  admin[2].script = 55;
  admin[2].step = 55;
  admin[2].waitCount = 55;

  eventQueueTop = 0;
  eventQueue[1].time = 1;
  popGameState();
  ASSERT(admin[2].location == 12);
  ASSERT(admin[2].alreadyDescribed == 2);
  ASSERT(admin[2].visitsCount == 13);
  ASSERT(admin[2].script == 33);
  ASSERT(admin[2].step == 3886);
  ASSERT(admin[2].waitCount == 38869878);

  ASSERT(eventQueueTop == 2);
  ASSERT(eventQueue[1].time == 47);

  popGameState();

  ASSERT(attributes[0].value == 12);
  ASSERT(attributes[2].value == 3);
}

static void testWhereIllegalId() {
  header->instanceMax = 1;
  expectSyserr = TRUE;
  hadSyserr = FALSE;
  if (setjmp(syserr_label) == 0)
    where(0, TRUE);
  ASSERT(hadSyserr);
  hadSyserr = FALSE;
  if (setjmp(syserr_label) == 0)
    where(2, TRUE);
  ASSERT(hadSyserr);
  expectSyserr = FALSE;
}

static void testHereIllegalId() {
  header->instanceMax = 1;
  expectSyserr = TRUE;
  hadSyserr = FALSE;
  if (setjmp(syserr_label) == 0)
    isHere(0, FALSE);
  ASSERT(hadSyserr);
  hadSyserr = FALSE;
  if (setjmp(syserr_label) == 0)
    isHere(2, FALSE);
  ASSERT(hadSyserr);
  expectSyserr = FALSE;
}

static void testLocateIllegalId() {
  header->instanceMax = 1;
  expectSyserr = TRUE;
  hadSyserr = FALSE;
  if (setjmp(syserr_label) == 0)
    locate(0, 1);
  ASSERT(hadSyserr);
  hadSyserr = FALSE;
  if (setjmp(syserr_label) == 0)
    locate(2, 1);
  ASSERT(hadSyserr);
  hadSyserr = FALSE;
  if (setjmp(syserr_label) == 0)
    locate(1, 0);
  ASSERT(hadSyserr);
  hadSyserr = FALSE;
  if (setjmp(syserr_label) == 0)
    locate(1, 2);
  ASSERT(hadSyserr);
  expectSyserr = FALSE;
}


static void testSaveRestore() {
  FILE *saveFile = fopen("testSaveFile", "w");
  Aword scoreTable = EOF;
  int i;

  /* Set up empty eventQ and scores and other irrelevant data */
  eventQueueTop = 0;
  scores = &scoreTable;
  adventureName = "adventure";
  adventureFileName = "adventure.a3c";

  /* Init header */
  header->instanceMax = 3;
  header->attributesAreaSize = 21*sizeof(AttributeEntry)/sizeof(Aword);

  /* Initialize a fake instance table */
  instance = malloc(4*sizeof(InstanceEntry));
  instance[0].initialAttributes = 12; /* Shouldn't matter where.. */
  instance[1].initialAttributes = 12; /* Shouldn't matter where.. */
  instance[2].initialAttributes = 12; /* Shouldn't matter where.. */
  instance[3].initialAttributes = 12; /* Shouldn't matter where.. */

  /* Allocate an attribute area and initialize it */
  attributes = malloc(21*sizeof(AttributeEntry));
  for (i = 0; i<20; i++) {
    attributes[i].code = i;
    attributes[i].value = i;
    attributes[i].stringAddress = 0;
  }
  attributes[20].code = EOF;

  /* Fake admin areas for 3 instances */
  admin = allocate(5*sizeof(AdminEntry));
  admin[1].attributes = &attributes[0];
  admin[1].attributes[0].code = 11;
  admin[1].attributes[0].value = 11;
  admin[2].attributes = &attributes[5];
  admin[2].attributes[0].code = 22;
  admin[2].attributes[0].value = 22;
  admin[3].attributes = &attributes[7];
  admin[3].attributes[0].code = 33;
  admin[3].attributes[0].value = 33;

  /* Save the game data */
  saveGame(saveFile);
  fclose(saveFile);
  free(attributes);

  /* Get another attribute area and initialize admin areas */
  attributes = malloc(20*sizeof(AttributeEntry));
  for (i = 0; i<20; i++) {
    attributes[19-i].code = i;
    attributes[19-i].value = i;
  }
  admin[1].attributes = &attributes[0];
  admin[2].attributes = &attributes[5];
  admin[3].attributes = &attributes[7];

  saveFile = fopen("testSaveFile", "r");
  restoreGame(saveFile);
  fclose(saveFile);
  unlink("testSaveFile");

  ASSERT(admin[1].attributes[0].code == 11);
  ASSERT(admin[1].attributes[0].value == 11);
  ASSERT(admin[2].attributes[0].code == 22);
  ASSERT(admin[2].attributes[0].value == 22);
  ASSERT(admin[3].attributes[0].code == 33);
  ASSERT(admin[3].attributes[0].value == 33);
}

static void testSaveStrings() {
  char *testFileName = "testSaveStringFile";
  char *testString = "hejhopp";
  FILE *saveFile = fopen(testFileName, "w");
  Aword scoreTable = EOF;
  StringInitEntry *initEntry;

  /* Set up empty eventQ and scores and other irrelevant data */
  eventQueueTop = 0;
  header->scoreCount = 0;
  scores = &scoreTable;
  adventureName = "adventure";
  adventureFileName = "adventure.a3c";

  /* Init header for one instance with one attribute */
  header->instanceMax = 1;
  header->attributesAreaSize = sizeof(AttributeEntry)/sizeof(Aword);

  /* Initialize a fake instance table */
  instance = malloc(2*sizeof(InstanceEntry));
  instance[1].parent = 0;

  /* Allocate an attribute area and initialize it */
  attributes = malloc(2*sizeof(AttributeEntry));
  attributes[0].code = 1;
  attributes[0].value = (Aword)strdup(testString);
  attributes[0].stringAddress = 0;
  attributes[1].code = EOF;

  /* Fake admin areas for one instances */
  admin = allocate(2*sizeof(AdminEntry));
  admin[1].attributes = &attributes[0];

  /* A String Init Table is required */
  memory = allocate(3*sizeof(StringInitEntry));
  header->stringInitTable = 1;
  initEntry = (StringInitEntry*)pointerTo(1);
  initEntry->instanceCode = 1;
  initEntry->attributeCode = 1;
  *((Aword *)&initEntry[1]) = EOF;

  /* Save the game data */
  saveGame(saveFile);
  fclose(saveFile);
  strcpy((char *)admin[1].attributes[0].value, "i lingonskogen");

  admin[1].attributes[0].value = (Aword)strdup("i lingonskogen");

  saveFile = fopen(testFileName, "r");
  restoreGame(saveFile);
  fclose(saveFile);
  unlink(testFileName);

  ASSERT(strcmp((char *)admin[1].attributes[0].value, testString) == 0);
}

static void testSaveSets() {
  char *testFileName = "testSaveSetFile";
  Set *testSet[4];
  FILE *saveFile = fopen(testFileName, "w");
  Aword scoreTable = EOF;
  SetInitEntry *initEntry;
  int i,j;

  /* Set up empty eventQ and scores and other irrelevant data */
  eventQueueTop = 0;
  header->scoreCount = 0;
  scores = &scoreTable;
  adventureName = "adventure";
  adventureFileName = "adventure.a3c";

  /* Init header for one instance with four attributes */
  header->instanceMax = 1;
  header->attributesAreaSize = 4*sizeof(AttributeEntry)/sizeof(Aword);
  header->scoreCount = 0;
  header->stringInitTable = 0;

  /* Initialize a fake instance table */
  instance = malloc(2*sizeof(InstanceEntry));
  instance[1].parent = 0;

  /* Set up the test sets */
  for (i = 0; i < 4; i++) {
    testSet[i] = newSet(i);
    for (j = 0; j < i; j++)
      addToSet(testSet[i], j);
  }

  /* Allocate an attribute area and initialize it */
  attributes = malloc(5*sizeof(AttributeEntry));
  for (i = 0; i < 4; i++) {
    attributes[i].code = i+1;
    attributes[i].value = (Aword)copySet(testSet[i]);
    attributes[i].stringAddress = 0;
  }
  attributes[4].code = EOF;

  /* Fake admin areas for one instances */
  admin = allocate(2*sizeof(AdminEntry));
  admin[1].attributes = &attributes[0];

  /* A Set Init Table is required */
  memory = allocate(5*sizeof(SetInitEntry));
  header->setInitTable = 1;
  initEntry = (SetInitEntry*)pointerTo(1);
  initEntry[0].instanceCode = 1;
  initEntry[0].attributeCode = 1;
  initEntry[1].instanceCode = 1;
  initEntry[1].attributeCode = 2;
  initEntry[2].instanceCode = 1;
  initEntry[2].attributeCode = 3;
  initEntry[3].instanceCode = 1;
  initEntry[3].attributeCode = 4;
  *((Aword *)&initEntry[4]) = EOF;

  /* Save the game data */
  saveGame(saveFile);
  fclose(saveFile);

  /* Set new values */
  for (i = 0; i < 4; i++)
    admin[1].attributes[i].value = (Aword)newSet(0);

  saveFile = fopen(testFileName, "r");
  restoreGame(saveFile);
  fclose(saveFile);
  unlink(testFileName);

  for (i = 0; i < 4; i++)
    ASSERT(equalSets((Set *)admin[1].attributes[i].value, testSet[i]));
}

static void testWhere() {
  admin = malloc(4*sizeof(AdminEntry));
  instance = malloc(4*sizeof(InstanceEntry));
  class = malloc(4*sizeof(ClassEntry));
  header = malloc(sizeof(ACodeHeader));

  header->locationClassId = 1;
  header->instanceMax = 4;

  instance[1].parent = 1;	/* A location */
  admin[1].location = 3;
  ASSERT(where(1, TRUE) == 3);
  ASSERT(where(1, FALSE) == 1);

  instance[2].parent = 0;	/* Not a location */
  admin[2].location = 1;	/* At 1 */
  ASSERT(where(2, TRUE) == 1);
  ASSERT(where(2, FALSE) == 1);

  instance[3].parent = 0;	/* Not a location */
  admin[3].location = 2;	/* In 2 which is at 1*/
  ASSERT(where(3, TRUE) == 2);
  ASSERT(where(3, FALSE) == 1);

  instance[4].parent = 0;	/* Not a location */
  admin[4].location = 3;	/* In 3 which is in 2 which is at 1*/
  ASSERT(where(4, TRUE) == 3);
  ASSERT(where(4, FALSE) == 1);

  free(admin);
  free(instance);
  free(class);
  free(header);
}

static void testAddSet() {
  Set *set1 = NEW(Set);
  Set *set2 = NEW(Set);

  /* Test adding an empty set */
  addSet(set1, set2);
  ASSERT(sizeOfSet(set2)==0);

  addToSet(set2, 6);
  addToSet(set2, 7);
  addToSet(set2, 8);
  addSet(set1, set2);
  ASSERT(sizeOfSet(set2)==3);

  addToSet(set1, 4);
  addToSet(set1, 6);
  addSet(set1, set2);
  ASSERT(sizeOfSet(set2)==4);
}

void testSaveRestoreScore() {
  char *fileName = "testSaveRestoreScore";
  FILE *saveFile;
  int i;
  Aword *oldScores;
  int scoreCount = 48;

  oldScores = scores = allocate(scoreCount*sizeof(Aword));
  header->scoreCount = scoreCount;

  for (i = 0; i < scoreCount; i++)
    scores[i] = i;

  saveFile = fopen(fileName, "wb");
  saveScores(saveFile);
  fclose(saveFile);

  scores = allocate(scoreCount*sizeof(Aword));

  for (i = 0; i < scoreCount; i++)
    scores[i] = 50-i;

  saveFile = fopen(fileName, "rb");
  restoreScores(saveFile);

  ASSERT(memcmp(scores, oldScores, scoreCount*sizeof(Aword)) == 0);
  ASSERT(fgetc(saveFile) == EOF);

  fclose(saveFile);
  unlink(fileName);
  free(scores);
  free(oldScores);
}

void registerExeUnitTests()
{
  registerUnitTest(testCountTrailingBlanks);
  registerUnitTest(testSkipWordForwards);
  registerUnitTest(testSkipWordBackwards);
  registerUnitTest(testStripCharsFromString);
  registerUnitTest(testStripWordsFromString);
  registerUnitTest(testGetString);
  registerUnitTest(testIncreaseEventQueue);
  registerUnitTest(testPushGameState);
  registerUnitTest(testPopGameState);
  registerUnitTest(testWhereIllegalId);
  registerUnitTest(testHereIllegalId);
  registerUnitTest(testLocateIllegalId);
  registerUnitTest(testSaveRestore);
  registerUnitTest(testSaveStrings);
  registerUnitTest(testSaveSets);
  registerUnitTest(testWhere);
  registerUnitTest(testAddSet);
  registerUnitTest(testSaveRestoreScore);
}
