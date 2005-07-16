/*----------------------------------------------------------------------*\

  save.c

  Amachine save and restore module of Alan interpreter

\*----------------------------------------------------------------------*/

#include "sysdep.h"
#include "types.h"
#include "main.h"
#include "exe.h"
#include "set.h"
#include "readline.h"
#include "syserr.h"


#ifndef HAVE_GLK
static char saveFileName[256];
typedef FILE *AFILE;
#else
#include "glk.h"
#define MAP_STDIO_TO_GLK
#include "glkio.h"
typedef strid_t AFILE;
#endif


/*----------------------------------------------------------------------*/
static void saveStrings(AFILE saveFile) {

  StringInitEntry *initEntry;

  if (header->stringInitTable != 0)
    for (initEntry = (StringInitEntry *)pointerTo(header->stringInitTable);
	 !endOfTable(initEntry); initEntry++) {
      char *attr = (char *)getStringAttribute(initEntry->instanceCode, initEntry->attributeCode);
      Aint length = strlen(attr) + 1;
      fwrite((void *)&length, sizeof(length), 1, saveFile);
      fwrite((void *)attr, 1, length, saveFile);
    }
}


/*----------------------------------------------------------------------*/
static void saveSets(AFILE saveFile) {
  SetInitEntry *initEntry;

  if (header->setInitTable != 0)
    for (initEntry = (SetInitEntry *)pointerTo(header->setInitTable);
	 !endOfTable(initEntry); initEntry++) {
      Set *attr = (Set *)getSetAttribute(initEntry->instanceCode, initEntry->attributeCode);
      fwrite((void *)&attr->size, sizeof(attr->size), 1, saveFile);
      fwrite((void *)attr->members, sizeof(attr->members[0]), attr->size, saveFile);
    }
}


/*----------------------------------------------------------------------*/
static void saveGameInfo(AFILE saveFile) {
  fwrite((void *)"ASAV", 1, 4, saveFile);
  fwrite((void *)&header->vers, 1, sizeof(Aword), saveFile);
  fwrite((void *)adventureName, 1, strlen(adventureName)+1, saveFile);
  fwrite((void *)&header->uid, 1, sizeof(Aword), saveFile);
}


/*----------------------------------------------------------------------*/
static void saveAdmin(AFILE saveFile) {
  fwrite((void *)&admin[1], sizeof(AdminEntry), header->instanceMax, saveFile);
}


/*----------------------------------------------------------------------*/
static void saveAttributeArea(AFILE saveFile) {
  fwrite((void*)attributes, header->attributesAreaSize, sizeof(Aword), saveFile);
}


/*----------------------------------------------------------------------*/
static void saveEventQueue(AFILE saveFile) {
  fwrite((void *)&eventQueueTop, sizeof(eventQueueTop), 1, saveFile);
  fwrite((void *)&eventQueue[0], sizeof(eventQueue[0]), eventQueueTop, saveFile);
}


/*----------------------------------------------------------------------*/
static void saveCurrentValues(AFILE saveFile) {
  fwrite((void *)&current, sizeof(current), 1, saveFile);
}


/*----------------------------------------------------------------------*/
static void saveScores(AFILE saveFile) {
  fwrite((void *)scores, sizeof(Aword), header->scoreCount, saveFile);
}


/*----------------------------------------------------------------------*/
static void saveGame(AFILE saveFile) {
  /* Save tag, version of interpreter, name and uid of game */
  saveGameInfo(saveFile);

  /* Save current values */
  saveCurrentValues(saveFile);

  saveAttributeArea(saveFile);
  saveAdmin(saveFile);

  saveEventQueue(saveFile);

  saveScores(saveFile);

  saveStrings(saveFile);
  saveSets(saveFile);
}


/*----------------------------------------------------------------------*/
void save(void)
{
#ifdef HAVE_GLK
  frefid_t saveFileRef;
  strid_t saveFile;
  saveFileRef = glk_fileref_create_by_prompt(fileusage_SavedGame, filemode_Write, 0);
  if (saveFileRef == NULL)
    error(M_SAVEFAILED);
  saveFile = glk_stream_open_file(saveFileRef, filemode_Write, 0);

#else

  FILE *saveFile;
  char str[256];

  /* First save ? */
  if (saveFileName[0] == '\0') {
    strcpy(saveFileName, adventureName);
    strcat(saveFileName, ".sav");
  }
  printMessage(M_SAVEWHERE);
  sprintf(str, "(%s) : ", saveFileName);
  output(str);
#ifdef USE_READLINE
  readline(str);
#else
  gets(str);
#endif
  if (str[0] == '\0')
    strcpy(str, saveFileName);
  col = 1;
  if ((saveFile = fopen(str, READ_MODE)) != NULL)
    /* It already existed */
    if (!confirm(M_SAVEOVERWRITE))
      error(MSGMAX);            /* Return to player without saying anything */
  if ((saveFile = fopen(str, WRITE_MODE)) == NULL)
    error(M_SAVEFAILED);
  strcpy(saveFileName, str);
#endif

  saveGame(saveFile);

  fclose(saveFile);
}


/*----------------------------------------------------------------------*/
static void restoreStrings(AFILE saveFile) {
  StringInitEntry *initEntry;

  if (header->stringInitTable != 0)
    for (initEntry = (StringInitEntry *)pointerTo(header->stringInitTable);
	 !endOfTable(initEntry); initEntry++) {
      Aint length;
      char *string;
      fread((void *)&length, sizeof(Aint), 1, saveFile);
      string = allocate(length+1);
      fread((void *)string, 1, length, saveFile);
      setValue(initEntry->instanceCode, initEntry->attributeCode, (Aword)string);
    }
}


/*----------------------------------------------------------------------*/
static void restoreSets(AFILE saveFile) {
  SetInitEntry *initEntry;

  if (header->setInitTable != 0)
    for (initEntry = (SetInitEntry *)pointerTo(header->setInitTable);
	 !endOfTable(initEntry); initEntry++) {
      Aint setSize;
      Set *set;
      int i;

      fread((void *)&setSize, sizeof(setSize), 1, saveFile);
      set = newSet(setSize);
      for (i = 0; i < setSize; i++) {
	Aword member;
	fread((void *)&member, sizeof(member), 1, saveFile);
	addToSet(set, member);
      }
      setValue(initEntry->instanceCode, initEntry->attributeCode, (Aword)set);
    }
}


/*----------------------------------------------------------------------*/
static void restoreScores(AFILE saveFile) {
  fread((void *)scores, sizeof(Aword), header->scoreCount, saveFile);
}


/*----------------------------------------------------------------------*/
static void restoreEventQueue(AFILE saveFile) {
  fread((void *)&eventQueueTop, sizeof(eventQueueTop), 1, saveFile);
  if (eventQueueTop > eventQueueSize) {
    free(eventQueue);
    eventQueue = allocate(eventQueueTop*sizeof(eventQueue[0]));
  }
  fread((void *)&eventQueue[eventQueueTop], sizeof(eventQueue[0]), eventQueueTop, saveFile);
}


/*----------------------------------------------------------------------*/
static void restoreAdmin(AFILE saveFile) {
  /* Restore admin for instances, remember to reset attribute area pointer */
  int i;
  for (i = 1; i <= header->instanceMax; i++) {
    AttributeEntry *currentAttributesArea = admin[i].attributes;
    fread((void *)&admin[i], sizeof(AdminEntry), 1, saveFile);
    admin[i].attributes = currentAttributesArea;
  }
}


/*----------------------------------------------------------------------*/
static void restoreAttributeArea(AFILE saveFile) {
  fread((void *)attributes, header->attributesAreaSize, sizeof(Aword), saveFile);
}


/*----------------------------------------------------------------------*/
static void restoreCurrentValues(AFILE saveFile) {
  fread((void *)&current, sizeof(current), 1, saveFile);
}


/*----------------------------------------------------------------------*/
static void verifyGameId(AFILE saveFile) {
  Aword savedUid;

  fread((void *)&savedUid, sizeof(Aword), 1, saveFile);
  if (!ignoreErrorOption && savedUid != header->uid)
    error(M_SAVEVERS);
}


/*----------------------------------------------------------------------*/
static void verifyGameName(AFILE saveFile) {
  char savedName[256];
  int i = 0;

  while ((savedName[i++] = fgetc(saveFile)) != '\0');
  if (strcmp(savedName, adventureName) != 0)
    error(M_SAVENAME);
}


/*----------------------------------------------------------------------*/
static void verifyCompilerVersion(AFILE saveFile) {
  char savedVersion[4];

  fread((void *)&savedVersion, sizeof(Aword), 1, saveFile);
  if (!ignoreErrorOption && strncmp(savedVersion, header->vers, 4))
    error(M_SAVEVERS);
}


/*----------------------------------------------------------------------*/
static void verifySaveFile(AFILE saveFile) {
  char string[256];

  fread((void *)&string, 1, 4, saveFile);
  string[4] = '\0';
  if (strcmp(string, "ASAV") != 0)
    error(M_NOTASAVEFILE);
}


/*----------------------------------------------------------------------*/
static void restoreGame(AFILE saveFile)
{
  if (saveFile == NULL) syserr("'restoreGame()' from a null fileref");

  verifySaveFile(saveFile);

  /* Verify version of compiler/interpreter of saved game with us */
  verifyCompilerVersion(saveFile);

  /* Verify name of game */
  verifyGameName(saveFile);

  /* Verify unique id of game */
  verifyGameId(saveFile);

  restoreCurrentValues(saveFile);
  restoreAttributeArea(saveFile);
  restoreAdmin(saveFile);
  restoreEventQueue(saveFile);
  restoreScores(saveFile);
  restoreStrings(saveFile);
  restoreSets(saveFile);
}


/*----------------------------------------------------------------------*/
void restore(void)
{
#ifdef HAVE_GLK
  frefid_t saveFileRef;
  strid_t saveFile;
  saveFileRef = glk_fileref_create_by_prompt(fileusage_SavedGame, filemode_Read, 0);
  if (saveFileRef == NULL) return;
  saveFile = glk_stream_open_file(saveFileRef, filemode_Read, 0);

#else
  char str[1000];
  FILE *saveFile;

  /* First save ? */
  if (saveFileName[0] == '\0') {
    strcpy(saveFileName, adventureName);
    strcat(saveFileName, ".sav");
  }
  printMessage(M_RESTOREFROM);
  sprintf(str, "(%s) : ", saveFileName);
  output(str);
#ifdef USE_READLINE
  readline(str);
#else
  gets(str);
#endif

  if (str[0] == '\0')
    strcpy(str, saveFileName);
  col = 1;
  if (str[0] == '\0')
    strcpy(str, saveFileName);        /* Use the name temporarily */
  if ((saveFile = fopen(str, READ_MODE)) == NULL)
    error(M_SAVEMISSING);
  strcpy(saveFileName, str);          /* Save it for future use */

#endif

  restoreGame(saveFile);

  fclose(saveFile);
}