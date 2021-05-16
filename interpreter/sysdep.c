/*----------------------------------------------------------------------*\
  sysdep.c

  This file contains necessary system dependent routines.

  Routines possibly not implemented in the current environment:

  strdup()
  strlow()
  strupp()

  Notes on string handling:

  - Native - means the "natural" character set/encoding for the
    platform, nowadays often UTF-8, but that was not the case in the
    beginning.

  - Internal - is always ISO8859-1 in which encoding everything
    internal should use (even dictionary entries, which is not certain
    it does currently. TODO!

  - Current - the compiler (currently) has options for different
    charsets, if that is used all input files are considered to be in
    that encoding, which might be different from the native encoding.
    It (will) also auto-detect an UTF BOM and enforce UTF-8 for that
    single file, which again might be different from native or the one
    given using the -charset option.

\*----------------------------------------------------------------------*/

#include <time.h>
#include "sysdep.h"
#include "syserr.h"


#ifdef HAVE_GLK
#include "glk.h"
#endif


#ifdef HAVE_GLK

/* Note to Glk maintainers: 'native' characters are used for output, in this
   case, Glk's Latin-1.  ISO characters are Alan's internal representation,
   stored in the .A3C file, and must be converted to native before printing.
   Glk could just use the ISO routines directly, but its safer to maintain
   its own tables to guard against future changes in either Alan or Glk (ie. a
   move to Unicode).
 */

static char spaceCharacters[] =
{
  0x0A, /* linefeed */
  0x20, /* space */
  0xA0, /* non-breaking space */
  0x00
};

static char lowerCaseCharacters[] =
{
  0x61, /* a */  0x62, /* b */  0x63, /* c */  0x64, /* d */
  0x65, /* e */  0x66, /* f */  0x67, /* g */  0x68, /* h */
  0x69, /* i */  0x6A, /* j */  0x6B, /* k */  0x6C, /* l */
  0x6D, /* m */  0x6E, /* n */  0x6F, /* o */  0x70, /* p */
  0x71, /* q */  0x72, /* r */  0x73, /* s */  0x74, /* t */
  0x75, /* u */  0x76, /* v */  0x77, /* w */  0x78, /* x */
  0x79, /* y */  0x7A, /* z */  0xDF, /* ss <small sharp s> */
  0xE0, /* a grave */           0xE1, /* a acute */
  0xE2, /* a circumflex */      0xE3, /* a tilde */
  0xE4, /* a diaeresis */       0xE5, /* a ring */
  0xE6, /* ae */                0xE7, /* c cedilla */
  0xE8, /* e grave */           0xE9, /* e acute */
  0xEA, /* e circumflex */      0xEB, /* e diaeresis */
  0xEC, /* i grave */           0xED, /* i acute */
  0xEE, /* i circumflex */      0xEF, /* i diaeresis */
  0xF0, /* <small eth> */       0xF1, /* n tilde */
  0xF2, /* o grave */           0xF3, /* o acute */
  0xF4, /* o circumflex */      0xF5, /* o tilde */
  0xF6, /* o diaeresis */       0xF8, /* o slash */
  0xF9, /* u grave */           0xFA, /* u acute */
  0xFB, /* u circumflex */      0xFC, /* u diaeresis */
  0xFD, /* y acute */           0xFE, /* <small thorn> */
  0xFF, /* y diaeresis */       0x00
};

/* FIXME: ss <small sharp s> and y diaeresis have no UC analogues
   Are they really considered LC?
 */

static char upperCaseCharacters[] =
{
  0x41, /* A */  0x42, /* B */  0x43, /* C */  0x44, /* D */
  0x45, /* E */  0x46, /* F */  0x47, /* G */  0x48, /* H */
  0x49, /* I */  0x4A, /* J */  0x4B, /* K */  0x4C, /* L */
  0x4D, /* M */  0x4E, /* N */  0x4F, /* O */  0x50, /* P */
  0x51, /* Q */  0x52, /* R */  0x53, /* S */  0x54, /* T */
  0x55, /* U */  0x56, /* V */  0x57, /* W */  0x58, /* X */
  0x59, /* Y */  0x5A, /* Z */
  0xC0, /* A grave */           0xC1, /* A acute */
  0xC2, /* A circumflex */      0xC3, /* A tilde */
  0xC4, /* A diaeresis */       0xC5, /* A ring */
  0xC6, /* AE */                0xC7, /* C cedilla */
  0xC8, /* E grave */           0xC9, /* E acute */
  0xCA, /* E circumflex */      0xCB, /* E diaeresis */
  0xCC, /* I grave */           0xCD, /* I acute */
  0xCE, /* I circumflex */      0xCF, /* I diaeresis */
  0xD0, /* <capital eth> */     0xD1, /* N tilde */
  0xD2, /* O grave */           0xD3, /* O acute */
  0xD4, /* O circumflex */      0xD5, /* O tilde */
  0xD6, /* O diaeresis */       0xD8, /* O slash */
  0xD9, /* U grave */           0xDA, /* U acute */
  0xDB, /* U circumflex */      0xDC, /* U diaeresis */
  0xDD, /* Y acute */           0xDE, /* <capital thorn> */
  0x00
};

#else

/* These work on native (ISO8859-1) character sets */

static unsigned char spaceCharacters[] = " \t\n";

/* Use native characters */
/*                                        "abcdefghijklmnopqrstuvwxyz   à   á   â   ã   ä   å   æ   ç   è   é   ê   ë   ì   í   î   ï   ð   ñ   ò   ó   ô   õ   ö   ø   ù   ú   û   ü   ý   þ   ÿ"; */
static const char lowerCaseCharacters[] = "abcdefghijklmnopqrstuvwxyz\xE0\xE1\xE2\xE3\xE4\xE5\xE6\xE7\xE8\xE9\xEA\xEB\xEC\xED\xEE\xEF\xF0\xF1\xF2\xF3\xF4\xF5\xF6\xF8\xF9\xFA\xFB\xFC\xFF\xFE\xFF";

/*                                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ   À   Á   Â   Ã   Ä   Å   Æ   Ç   È   É   Ê   Ë   Ì   Í   Î   Ï   Ð   Ñ   Ò   Ó   Ô   Õ   Ö   Ø   Ù   Ú   Û   Û   Ý   Þ   ß"; */
static const char upperCaseCharacters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\xC0\xC1\xC2\xC3\xC4\xC5\xC6\xC7\xC8\xC9\xCA\xCB\xCC\xCD\xCE\xCF\xD0\xD1\xD2\xD3\xD4\xD5\xD6\xD8\xD9\xDA\xDB\xDC\xDD\xDE\xDF";

#endif

int isSpace(unsigned int c)              /* IN - Native character to test */
{
    return (c != '\0' && strchr((char *)spaceCharacters, c) != 0);
}


int isLower(unsigned int c)              /* IN - Native character to test */
{
    return (c != '\0' && strchr((char *)lowerCaseCharacters, c) != 0);
}


int isUpper(unsigned int c)              /* IN - Native character to test */
{
    return (c != '\0' && strchr((char *)upperCaseCharacters, c) != 0);
}

int isLetter(unsigned int c)             /* IN - Native character to test */
{
  return(c != '\0' && (isLower(c)? !0: isUpper(c)));
}


int toLower(unsigned int c)              /* IN - Native character to convert */
{
#ifdef HAVE_GLK
  return glk_char_to_lower(c);
#else
  return (isUpper(c)? c + ('a' - 'A'): c);
#endif
}

int toUpper(unsigned int c)              /* IN - Native character to convert */
{
#ifdef HAVE_GLK
  return glk_char_to_upper(c);
#else
  return (isLower(c)? c - ('a' - 'A'): c);
#endif
}

char *strlow(char str[])        /* INOUT - Native string to convert */
{
  char *s;

  for (s = str; *s; s++)
    *s = toLower(*s);
  return(str);
}


char *strupp(char str[])        /* INOUT - Native string to convert */
{
  char *s;

  for (s = str; *s; s++)
    *s = toUpper(*s);
  return(str);
}


/* The following work on ISO characters */

int isLowerCase(unsigned int c)          /* IN - ISO character to test */
{
  int i;
  for (i = 0; i < strlen(lowerCaseCharacters); i++)
    if (((unsigned int)lowerCaseCharacters[i]) == c) return 1;
  return 0;
}


int isUpperCase(unsigned int c)          /* IN - ISO character to test */
{
  int i;
  for (i = 0; i < strlen(upperCaseCharacters); i++)
    if (upperCaseCharacters[i] == c) return 1;
  return 0;
}


int isISOLetter(int c)          /* IN - ISO character to test */
{
  return (isLowerCase(c) || isUpperCase(c));
}


char IsoToLowerCase(int c)         /* IN - ISO character to convert */
{
  return (isUpperCase(c)? c + ('a' - 'A'): c);
}


char IsoToUpperCase(int c)         /* IN - ISO character to convert */
{
  return (isLowerCase(c)? c - ('a' - 'A'): c);
}


char *stringLower(char str[])   /* INOUT - ISO string to convert */
{
  char *s;

  for (s = str; *s; s++)
    *s = IsoToLowerCase(*s);
  return(str);
}


char *stringUpper(char str[])   /* INOUT - ISO string to convert */
{
  char *s;

  for (s = str; *s; s++)
    *s = IsoToUpperCase(*s);
  return(str);
}

/*----------------------------------------------------------------------*/
int compareStrings(char *str1, char *str2)
{
  char *s1 = str1, *s2 = str2;

  while (*s1 != '\0' && *s2 != '\0') {
    if (IsoToLowerCase(*s1) < IsoToLowerCase(*s2)) return -1;
    if (IsoToLowerCase(*s1) > IsoToLowerCase(*s2)) return 1;
    s1++;
    s2++;
  }
  return IsoToLowerCase(*s2) - IsoToLowerCase(*s1);
}



/*----------------------------------------------------------------------
  toIso

  Converts the incoming string to ISO character set. The original is
  in the current character set which in the case of the compiler might
  be other than the native.

  */
void toIso(char copy[], /* OUT - Mapped  string */
       char original[],	/* IN - string to convert */
       int charset)		/* IN - the current character set */
{
static unsigned char macMap[256]
= {
0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0A,0x0E,0x0F,
0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
0xC4,0xC5,0xC7,0xC9,0xD1,0xD6,0xDC,0xE1,0xE0,0xE2,0xE4,0xE3,0xE5,0xE7,0xE9,0xE8,
0xEA,0xEB,0xED,0xEC,0xEE,0xEF,0xF1,0xF3,0xF2,0xF4,0xF6,0xF5,0xFA,0xF9,0xFB,0xFC,
0xB9,0xB0,0xA2,0xA3,0xA7,0xB7,0xB6,0xDF,0xAE,0xA9,0xB2,0xB4,0xA8,0xD7,0xC6,0xD8,
0xA4,0xB1,0xCD,0xCC,0xA5,0xB5,0xF0,0xCA,0xDE,0xFE,0xA6,0xAA,0xBA,0xD4,0xE6,0xF8,
0xBF,0xA1,0xAC,0xCE,0xCF,0xC8,0xD0,0xAB,0xBB,0xCB,0xA0,0xC0,0xC3,0xD5,0xDD,0xFD,
0xAD,0xAF,0xDA,0xD9,0xB8,0xB3,0xF7,0xC2,0xFF,0xBC,0xBD,0xBE,0xC1,0xD2,0xD3,0xDB,
0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F};

static unsigned char dosMap[256]
= {
0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0A,0x0E,0x0F,
0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
0xC7,0xFC,0xE9,0xE2,0xE4,0xE0,0xE5,0xE7,0xEA,0xEB,0xE8,0xEF,0xEE,0xEC,0xC4,0xC5,
0xC9,0xE6,0xC6,0xF4,0xF6,0xF2,0xFB,0xF9,0xFF,0xD6,0xDC,0xA2,0xA3,0xA5,0xDE,0xA6,
0xE1,0xED,0xF3,0xFA,0xF1,0xD1,0xAA,0xBA,0xBF,0xC0,0xC1,0xBD,0xBC,0xCF,0xAB,0xBB,
0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
0xA1,0xA7,0xAD,0xB3,0xB8,0xB9,0xC3,0xCE,0xD2,0xD3,0xDB,0xDD,0xE3,0xF5,0xF8,0xFD,
0xA9,0xDF,0xC8,0xB6,0xCA,0xA4,0xB5,0xAE,0xD5,0xD0,0xD4,0xF0,0xD7,0xD8,0xCB,0xC2,
0xBE,0xB1,0xD9,0xDA,0xCD,0xCC,0xF7,0xA8,0xB0,0xB7,0xAF,0xAC,0xFE,0xB2,0xB4,0xA0};
  unsigned char *o, *c;

  switch (charset) {
  case 0: /* ISO */
    if (copy != original)
      (void)strcpy(copy, original);
    break;
  case 1:			/* Mac */
    for (o = (unsigned char *)original, c = (unsigned char *)copy; *o; o++, c++)
      *c = macMap[*o];
    *c = '\0';
    break;

  case 2:			/* Dos */
    for (o = (unsigned char *)original, c = (unsigned char *)copy; *o; o++, c++)
      *c = dosMap[*o];
    *c = '\0';
    break;
  }
}


/*======================================================================*/
int littleEndian() {
  int x = 1;
  return (*(char *)&x == 1);
}


/*======================================================================*/
char *baseNameStart(char *fullPathName) {
  static char *delimiters = "\\>]/:";
  int i;

  for (i = strlen(fullPathName)-1; i > 0; i--)
    if (strchr(delimiters, fullPathName[i]) != NULL)
      return &fullPathName[i+1];
  return(fullPathName);
}
