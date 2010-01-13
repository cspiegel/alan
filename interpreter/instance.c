#include "instance.h"

/* IMPORTS */
#include "memory.h"
#include "syserr.h"
#include "attribute.h"
#include "current.h"
#include "Container.h"
#include "options.h"
#include "debug.h"
#include "checkentry.h"
#include "inter.h"
#include "output.h"
#include "class.h"
#include "msg.h"
#include "actor.h"
#include "literal.h"
#include "dictionary.h"
#include "Location.h"


/* PUBLIC DATA */

InstanceEntry *instances;   /* Instance table pointer */

AdminEntry *admin;      /* Administrative data about instances */
AttributeEntry *attributes; /* Dynamic attribute values */

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* Instance query methods */

/*======================================================================*/
Bool isA(int instance, int ancestor)
{
    int parent;

    if (isLiteral(instance))
        parent = literals[instance-header->instanceMax].class;
    else
        parent = instances[instance].parent;
    while (parent != 0 && parent != ancestor)
        parent = classes[parent].parent;

    return (parent != 0);
}


Bool isObject(int instance)
{
  return isA(instance, OBJECT);
}

Bool isContainer(int instance)
{
  return instance != 0 && instances[instance].container != 0;
}

Bool isActor(int instance)
{
  return isA(instance, ACTOR);
}

Bool isLocation(int instance)
{
  return isA(instance, LOCATION);
}


Bool isLiteral(int instance)
{
  return instance > header->instanceMax;
}

Bool isNumeric(int instance)
{
  return isLiteral(instance) && literals[literalFromInstance(instance)].type == NUMERIC_LITERAL;
}

Bool isString(int instance)
{
  return isLiteral(instance) && literals[literalFromInstance(instance)].type == STRING_LITERAL;
}


/*======================================================================*/
void setInstanceAttribute(int instance, int attribute, Aword value)
{
    char str[80];

    if (instance > 0 && instance <= header->instanceMax) {
        setAttribute(admin[instance].attributes, attribute, value);
        if (isLocation(instance))   /* May have changed so describe next time */
            admin[instance].visitsCount = 0;
    } else {
        sprintf(str, "Can't SET/MAKE instance (%d).", instance);
        syserr(str);
    }
}


/*======================================================================*/
void setInstanceStringAttribute(int instance, int attribute, char *string)
{
    free((char *)getInstanceAttribute(instance, attribute));
    setInstanceAttribute(instance, attribute, (Aword)string);
}


/*======================================================================*/
void setInstanceSetAttribute(int instance, int attribute, Aword set)
{
    freeSet((Set *)getInstanceAttribute(instance, attribute));
    setInstanceAttribute(instance, attribute, (Aword)set);
}


/*----------------------------------------------------------------------*/
static Aword literalAttribute(int literal, int attribute)
{
    char str[80];

    if (attribute == 1)
        return literals[literalFromInstance(literal)].value;
    else {
        sprintf(str, "Unknown attribute for literal (%d).", attribute);
        syserr(str);
    }
    return(EOF);
}


/*======================================================================*/
Aword getInstanceAttribute(int instance, int attribute)
{
    char str[80];

    if (isLiteral(instance))
        return literalAttribute(instance, attribute);
    else {
        if (instance > 0 && instance <= header->instanceMax)
            return getAttribute(admin[instance].attributes, attribute);
        else {
            sprintf(str, "Can't ATTRIBUTE item (%d).", instance);
            syserr(str);
        }
    }
    return(EOF);
}


/*======================================================================*/
char *getInstanceStringAttribute(int instance, int attribute)
{
    return strdup((char *)getInstanceAttribute(instance, attribute));
}


/*======================================================================*/
Set *getInstanceSetAttribute(int instance, int attribute)
{
    return copySet((Set *)getInstanceAttribute(instance, attribute));
}


/*----------------------------------------------------------------------*/
static void verifyInstance(int instance, char *action) {
    char message[200];

    if (instance == 0) {
        sprintf(message, "Can't %s instance (%d).", action, instance);
        syserr(message);
    } else if (instance > header->instanceMax) {
        sprintf(message, "Can't %s instance (%d > instanceMax).", action, instance);
        syserr(message);
    }
}


/*======================================================================*/
Bool isHere(int id, Bool directly)
{
    verifyInstance(id, "HERE");

    if (directly)
        return(admin[id].location == current.location);
    else
        return at(id, current.location, directly);
}


/*======================================================================*/
Bool isNearby(int instance, Bool directly)
{
    verifyInstance(instance, "NEARBY");

    if (isLocation(instance))
        return exitto(current.location, instance);
    else
        return exitto(current.location, where(instance, directly));
}


/*======================================================================*/
Bool isNear(int instance, int other, Bool directly)
{
    Aint l1, l2;

    verifyInstance(instance, "NEAR");

    if (isLocation(instance))
        l1 = instance;
    else
        l1 = where(instance, directly);
    if (isLocation(other))
        l2 = other;
    else
        l2 = where(other, directly);
    return exitto(l2, l1);
}


/*======================================================================*/
/* Look in a container to see if the instance is in it. */
Bool in(int instance, int container, Bool directly)
{
    int loc;

    if (!isContainer(container))
        syserr("IN in a non-container.");

    if (directly)
        return admin[instance].location == container;
    else {
        loc = admin[instance].location;
        while (loc != 0)
            if (loc == container)
                return TRUE;
            else
                loc = admin[loc].location;
        return FALSE;
    }
}



/*======================================================================*/
/* Look see if an instance is AT another. */
Bool at(int instance, int other, Bool directly)
{
    if (instance == 0 || other == 0) return FALSE;

    if (directly) {
        if (isLocation(other))
            return admin[instance].location == other;
        else
            return admin[instance].location == admin[other].location;
    } else {
        if (!isLocation(other))
            /* If the other is not a location, compare their locations */
            return locationOf(instance) == locationOf(other);
        else if (locationOf(instance) == other)
            return TRUE;
        else if (locationOf(other) != 0)
            return at(instance, locationOf(other), FALSE);
        else
            return FALSE;
    }
}


/*======================================================================*/
/* Return the *location* of an instance, transitively, i.e. not directly */
int locationOf(int instance)
{
    int loc;
    int container = 0;

    verifyInstance(instance, "LOCATION");

    loc = admin[instance].location;
    while (loc != 0 && !isLocation(loc)) {
        container = loc;
        loc = admin[loc].location;
    }
    if (loc != 0)
        return loc;
    else {
        if (container == 0)
            if (!isA(instance, THING) && !isLocation(instance))
                return locationOf(HERO);
            else
                return 0;       /* Nowhere */
        else
            if (!isA(container, THING) && !isLocation(container))
                return locationOf(HERO);
            else
                return 0;       /* Nowhere */
    }
}


/*======================================================================*/
/* Return the current position of an instance, directly or not */
int where(int instance, Bool directly)
{
    verifyInstance(instance, "WHERE");

    if (isLocation(instance))
        return 0;
    else if (directly)
        return admin[instance].location;
    else
        return locationOf(instance);
}


/*----------------------------------------------------------------------*/
static Bool executeInheritedMentioned(int class) {
    if (class == 0) return FALSE;

    if (classes[class].mentioned) {
        interpret(classes[class].mentioned);
        return TRUE;
    } else
        return executeInheritedMentioned(classes[class].parent);
}


/*----------------------------------------------------------------------*/
static Bool mention(int instance) {
    if (instances[instance].mentioned) {
        interpret(instances[instance].mentioned);
        return TRUE;
    } else
        return executeInheritedMentioned(instances[instance].parent);
}


/*======================================================================*/
void sayInstance(int instance)
{
#ifdef SAY_INSTANCE_WITH_PLAYER_WORDS_IF_PARAMETER
    int p, i;

    /* Find the id in the parameters... */
    if (params != NULL)
        for (p = 0; params[p].code != EOF; p++)
            if (params[p].code == instance) {
                /* Found it so.. */
                if (params[p].firstWord == EOF) /* Any words he used? */
                    break;      /* No... */
                else {              /* Yes, so use them... */
                    char *capitalized;
                    /* Assuming the noun is the last word we can simply output the adjectives... */
                    for (i = params[p].firstWord; i <= params[p].lastWord-1; i++)
                        output((char *)pointerTo(dict[wrds[i]].wrd));
                    /* ... and then the noun, capitalized if necessary */
                    if (header->capitalizeNouns) {
                        capitalized = strdup((char *)pointerTo(dict[wrds[params[p].lastWord]].wrd));
                        capitalized[0] = IsoToUpperCase(capitalized[0]);
                        output(capitalized);
                        free(capitalized);
                    } else
                        output((char *)pointerTo(dict[wrds[params[p].lastWord]].wrd));
                }
                return;
            }
#endif
    if (!mention(instance))
        interpret(instances[instance].name);
}


/*======================================================================*/
void sayInteger(int value)
{
    char buf[25];

    if (isHere(HERO, FALSE)) {
        sprintf(buf, "%d", value);
        output(buf);
    }
}


/*======================================================================*/
void sayString(char *string)
{
    if (isHere(HERO, FALSE))
        output(string);
    free(string);
}


/*----------------------------------------------------------------------*/
static void sayLiteral(int literal)
{
    char *str;

    if (isNumeric(literal))
        sayInteger(literals[literal-header->instanceMax].value);
    else {
        str = (char *)strdup((char *)literals[literal-header->instanceMax].value);
        sayString(str);
    }
}


/*----------------------------------------------------------------------*/
static char *wordWithCode(int classBit, int code) {
    int w;
    char str[50];

    for (w = 0; w < dictionarySize; w++)
        if (dictionary[w].code == code && ((classBit&dictionary[w].classBits) != 0))
            return pointerTo(dictionary[w].string);
    sprintf(str, "Could not find word of class %d with code %d.", classBit, code);
    syserr(str);
    return NULL;
}


/*----------------------------------------------------------------------*/
static Bool sayInheritedDefiniteForm(int class) {
    if (class == 0) {
        syserr("No default definite article");
        return FALSE;
    } else {
        if (classes[class].definite.address) {
            interpret(classes[class].definite.address);
            return classes[class].definite.isForm;
        } else
            return sayInheritedDefiniteForm(classes[class].parent);
    }
}


/*----------------------------------------------------------------------*/
static void sayDefinite(int instance) {
    if (instances[instance].definite.address) {
        interpret(instances[instance].definite.address);
        if (!instances[instance].definite.isForm)
            sayInstance(instance);
    } else
        if (!sayInheritedDefiniteForm(instances[instance].parent))
            sayInstance(instance);
}


/*----------------------------------------------------------------------*/
static Bool sayInheritedIndefiniteForm(int class) {
    if (class == 0) {
        syserr("No default indefinite article");
        return FALSE;
    } else {
        if (classes[class].indefinite.address) {
            interpret(classes[class].indefinite.address);
            return classes[class].indefinite.isForm;
        } else
            return sayInheritedIndefiniteForm(classes[class].parent);
    }
}


/*----------------------------------------------------------------------*/
static void sayIndefinite(int instance) {
    if (instances[instance].indefinite.address) {
        interpret(instances[instance].indefinite.address);
        if (!instances[instance].indefinite.isForm)
            sayInstance(instance);
    } else
        if (!sayInheritedIndefiniteForm(instances[instance].parent))
            sayInstance(instance);
}


/*----------------------------------------------------------------------*/
static Bool sayInheritedNegativeForm(int class) {
    if (class == 0) {
        syserr("No default negative form");
        return FALSE;
    } else {
        if (classes[class].negative.address) {
            interpret(classes[class].negative.address);
            return classes[class].negative.isForm;
        } else
            return sayInheritedNegativeForm(classes[class].parent);
    }
}


/*----------------------------------------------------------------------*/
static void sayNegative(int instance) {
    if (instances[instance].negative.address) {
        interpret(instances[instance].negative.address);
        if (!instances[instance].negative.isForm)
            sayInstance(instance);
    } else
        if (!sayInheritedNegativeForm(instances[instance].parent))
            sayInstance(instance);
}


/*----------------------------------------------------------------------*/
static void sayInheritedPronoun(int instance) {
    if (instance == 0)
        syserr("No default pronoun");
    else {
        if (classes[instance].pronoun != 0)
            output(wordWithCode(PRONOUN_BIT, classes[instance].pronoun));
        else
            sayInheritedPronoun(classes[instance].parent);
    }
}


/*----------------------------------------------------------------------*/
static void sayPronoun(int instance) {
    if (instances[instance].pronoun != 0)
        output(wordWithCode(PRONOUN_BIT, instances[instance].pronoun));
    else
        sayInheritedPronoun(instances[instance].parent);
}


/*----------------------------------------------------------------------*/
static void sayArticleOrForm(int id, SayForm form)
{
    if (!isLiteral(id))
        switch (form) {
        case SAY_DEFINITE:
            sayDefinite(id);
            break;
        case SAY_INDEFINITE:
            sayIndefinite(id);
            break;
        case SAY_NEGATIVE:
            sayNegative(id);
            break;
        case SAY_PRONOUN:
            sayPronoun(id);
            break;
        case SAY_SIMPLE:
            say(id);
            break;
        default:
            syserr("Unexpected form in 'sayArticleOrForm()'");
        }
    else
        say(id);
}


/*======================================================================*/
void say(int instance)
{
    Aword previousInstance = current.instance;
    current.instance = instance;

    if (isHere(HERO, FALSE)) {
        if (isLiteral(instance))
            sayLiteral(instance);
        else {
            verifyInstance(instance, "SAY");
            sayInstance(instance);
        }
    }
    current.instance = previousInstance;
}


/*======================================================================*/
void sayForm(int instance, SayForm form)
{
    Aword previousInstance = current.instance;
    current.instance = instance;

    sayArticleOrForm(instance, form);

    current.instance = previousInstance;
}


/*======================================================================*/
Bool isDescribable(int instance) {
    return isObject(instance) || isActor(instance);
}


/*----------------------------------------------------------------------*/
static Bool inheritsDescriptionFrom(int class)
{
    if (classes[class].description != 0)
        return TRUE;
    else if (classes[class].parent != 0)
        return inheritsDescriptionFrom(classes[class].parent);
    else
        return FALSE;
}


/*======================================================================*/
Bool hasDescription(int instance)
{
    if (instances[instance].description != 0)
        return TRUE;
    else if (instances[instance].parent != 0)
        return inheritsDescriptionFrom(instances[instance].parent);
    else
        return FALSE;
}


/*----------------------------------------------------------------------*/
static void describeClass(int instance)
{
    if (classes[instance].description != 0) {
        /* This class has a description, run it */
        interpret(classes[instance].description);
    } else {
        /* Search up the inheritance tree, if any, to find a description */
        if (classes[instance].parent != 0)
            describeClass(classes[instance].parent);
    }
}


/*======================================================================*/
void describeAnything(int instance)
{
    if (instances[instance].description != 0) {
        /* This instance has its own description, run it */
        interpret(instances[instance].description);
    } else {
        /* Search up the inheritance tree to find a description */
        if (instances[instance].parent != 0)
            describeClass(instances[instance].parent);
    }
    admin[instance].alreadyDescribed = TRUE;
}


/*----------------------------------------------------------------------*/
static void describeObject(int object)
{
    if (hasDescription(object))
        describeAnything(object);
    else {
        printMessageWithInstanceParameter(M_SEE_START, object);
        printMessage(M_SEE_END);
        if (instances[object].container != 0)
            describeContainer(object);
    }
    admin[object].alreadyDescribed = TRUE;
}


/*======================================================================*/
void describeInstances(void)
{
    int i;
    int lastInstanceFound = 0;
    int found = 0;

    /* First describe every object here with its own description */
    for (i = 1; i <= header->instanceMax; i++)
        if (admin[i].location == current.location && isObject(i) &&
                !admin[i].alreadyDescribed && hasDescription(i))
            describe(i);

    /* Then list all things without a description */
    for (i = 1; i <= header->instanceMax; i++)
        if (admin[i].location == current.location
                && !admin[i].alreadyDescribed
                && isObject(i)) {
            if (found == 0)
                printMessageWithInstanceParameter(M_SEE_START, i);
            else if (found > 1)
                printMessageWithInstanceParameter(M_SEE_COMMA, lastInstanceFound);
            admin[i].alreadyDescribed = TRUE;

            if (instances[i].container && containerSize(i, TRUE) > 0 && !getInstanceAttribute(i, OPAQUEATTRIBUTE)) {
                if (found > 0)
                    printMessageWithInstanceParameter(M_SEE_AND, i);
                printMessage(M_SEE_END);
                describeContainer(i);
                found = 0;
                continue;       /* Actually start another list. */
            }
            found++;
            lastInstanceFound = i;
        }

    if (found > 0) {
        if (found > 1) {
            printMessageWithInstanceParameter(M_SEE_AND, lastInstanceFound);
        }
        printMessage(M_SEE_END);
    }

    /* Finally all actors with a separate description */
    for (i = 1; i <= header->instanceMax; i++)
        if (admin[i].location == current.location && i != HERO && isActor(i)
        && !admin[i].alreadyDescribed)
            describe(i);

    /* Clear the describe flag for all instances */
    for (i = 1; i <= header->instanceMax; i++)
        admin[i].alreadyDescribed = FALSE;
}


/*----------------------------------------------------------------------*/
static Bool inheritedDescriptionCheck(int class)
{
    if (class == 0) return TRUE;
    if (!inheritedDescriptionCheck(classes[class].parent)) return FALSE;
    if (classes[class].descriptionChecks == 0) return TRUE;
    return !checksFailed(classes[class].descriptionChecks, TRUE);
}

/*----------------------------------------------------------------------*/
static Bool descriptionCheck(int instance)
{
    if (inheritedDescriptionCheck(instances[instance].parent)) {
        if (instances[instance].checks == 0) return TRUE;
        return !checksFailed(instances[instance].checks, TRUE);
    } else
        return FALSE;
}


/*======================================================================*/
Bool describe(int instance)
{
    Bool descriptionOk;
    int previousInstance = current.instance;

    current.instance = instance;
    verifyInstance(instance, "DESCRIBE");
    if (descriptionCheck(instance)) {
        descriptionOk = TRUE;
        if (isObject(instance)) {
            describeObject(instance);
        } else if (isActor(instance)) {
            describeActor(instance);
        } else
            describeAnything(instance);
    } else
        descriptionOk = FALSE;
    current.instance = previousInstance;
    return descriptionOk;
}


/*----------------------------------------------------------------------*/
static void locateIntoContainer(Aword theInstance, Aword theContainer) {
    if (!isA(theInstance, containers[instances[theContainer].container].class))
        printMessageUsing2InstanceParameters(M_CANNOTCONTAIN, theContainer, theInstance);
    else if (passesContainerLimits(theContainer, theInstance))
        admin[theInstance].location = theContainer;
    else
        abortPlayerCommand();
}


/*----------------------------------------------------------------------*/
static void locateLocation(Aword loc, Aword whr)
{
    Aint l = whr;

    /* Ensure this does not create a recursive location chain */
    while (l != 0) {
        if (admin[l].location == loc)
            apperr("Locating a location that would create a recursive loop of locations containing each other.");
        else
            l = admin[l].location;
    }
    admin[loc].location = whr;
}


/*----------------------------------------------------------------------*/
static void locateObject(Aword obj, Aword whr)
{
    if (isContainer(whr)) { /* Into a container */
        locateIntoContainer(obj, whr);
    } else {
        admin[obj].location = whr;
        /* Make sure the location is described since it's changed */
        admin[whr].visitsCount = 0;
    }
}


/*----------------------------------------------------------------------*/
static void executeInheritedEntered(Aint theClass) {
    if (theClass == 0) return;
    if (classes[theClass].entered)
        interpret(classes[theClass].entered);
    else
        executeInheritedEntered(classes[theClass].parent);
}


/*----------------------------------------------------------------------*/
static void locateActor(Aword movingActor, Aword whr)
{
    Aint previousCurrentLocation = current.location;
    Aint previousActorLocation = admin[movingActor].location;
    Aint previousActor = current.actor;
    Aint previousInstance = current.instance;

    /* TODO Actors locating into containers is dubious, anyway as it
   is now it allows the hero to be located into a container. And what
   happens with current location if so... */
    if (isContainer(whr))
        locateIntoContainer(movingActor, whr);
    else {
        current.location = whr;
        admin[movingActor].location = whr;
    }

    /* Now we have moved so show what is needed... */
    current.instance = current.location;

    if (movingActor == HERO) {
        if (admin[admin[movingActor].location].visitsCount % (current.visits+1) == 0)
            look();
        else {
            if (anyOutput)
                para();
            say(where(HERO, TRUE));
            printMessage(M_AGAIN);
            newline();
            describeInstances();
        }
        admin[where(HERO, TRUE)].visitsCount++;
        admin[where(HERO, TRUE)].visitsCount %= (current.visits+1);
    } else
        admin[whr].visitsCount = 0;

    /* Execute possible entered */
    current.actor = movingActor;
    if (instances[current.location].entered != 0) {
        if (previousActorLocation != current.location) {
            interpret(instances[current.location].entered);
            current.instance = previousInstance;
        }
    } else
        executeInheritedEntered(instances[current.location].parent);
    current.actor = previousActor;

    if (current.actor != movingActor)
        current.location = previousCurrentLocation;

    current.instance = previousInstance;
}


/*======================================================================*/
void locate(int instance, int whr)
{
    int containerId;
    ContainerEntry *theContainer;
    int previousInstance = current.instance;

    verifyInstance(instance, "LOCATE");
    verifyInstance(whr, "LOCATE AT");

    /* First check if the instance is in a container, if so run extract checks */
    if (isContainer(admin[instance].location)) {    /* In something? */
        current.instance = admin[instance].location;
        containerId = instances[admin[instance].location].container;
        theContainer = &containers[containerId];

        if (theContainer->extractChecks != 0) {
            if (sectionTraceOption) {
                printf("\n<EXTRACT from ");
                traceSay(instance);
                printf("(%d, container %d), Checking:>\n", instance, containerId);
            }
            if (checksFailed(theContainer->extractChecks, EXECUTE_CHECK_BODY_ON_FAIL)) {
                fail = TRUE;
		  // TODO: this should be done for the above return as well as before exiting the extract checks : current.instance = previousInstance;
                return;
            }
        }
        if (theContainer->extractStatements != 0) {
            if (sectionTraceOption) {
                printf("\n<EXTRACT from ");
                traceSay(instance);
                printf("(%d, container %d), Executing:>\n", instance, containerId);
            }
            interpret(theContainer->extractStatements);
        }
	current.instance = previousInstance;
    }

    if (isActor(instance))
        locateActor(instance, whr);
    else if (isLocation(instance))
        locateLocation(instance, whr);
    else
        locateObject(instance, whr);

    gameStateChanged = TRUE;
}

