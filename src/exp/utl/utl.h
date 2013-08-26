/* 
**  (C) by Remo Dentato (rdentato@gmail.com)
** 
** This software is distributed under the terms of the BSD license:
**   http://creativecommons.org/licenses/BSD/
**
*/

#ifndef UTL_H
#define UTL_H

/* .% Overview
** ===========
** .v
**                           ___   __
**                       ___/  (_ /  )
**                ___  _(__   ___)  /
**               /  / /  )/  /  /  /
**              /  /_/  //  (__/  /
**             (____,__/(_____(__/
** ..
**
**   This file ('|utl.h|) provide the following basic elements:
**
**  .[Unit Testing]   A simple framework to create unit tests. Tests output
**                    is compliant with the TAP '(Test Anything Protocol)
**                    standard.
**
**   [Logging]        To print logging traces during program execution.
**                    It offers multilevel logging similar to '|log4j|
**                    but limited to files.
**
**   [Finite State Machine]
**                    Macros to use FSM as if they were a native C control
**                    structure (similar to switch).
**
**   [Exceptions]     A simple implementation of try/catch. Similar to C++.
**
**   [Guarded memory allocation]
**                    Replacement for malloc(), calloc(), realloc() and free()
**                    that account and report about misuse of memory.
**  ..
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <string.h>
#include <stdarg.h>
#include <stddef.h>

#include <setjmp.h>

/*
** .% How to use '|utl|
** ====================
**
**  To access '|utl| functions Just follow these easy steps:
**
**  .# include '|utl.h| in each source file
**   # in one of the source files (usually the one that has your '|main()|
**     function) #define the symbol '|ULT_C| before including '|utl.h|
**  ..
**
**    As an alternative to the second step above, you can create a source
**  file (say '|utl.c|) with only the following lines:
**  .{{ C
**       #define  UTL_C
**       #include "utl.h"
**  .}}
**  and link it to your project.
**
**  The '{utl_extern} macro will take care of actually initializing the 
**  variables needed by '|utl.c| instead of simply declaring them as '|extern|
*/

#ifdef UTL_C
#define utl_extern(n,v) n v
#else
#define utl_extern(n,v) extern n
#endif

#define utl_initvoid ;


/* .%% Enable/disable utl features
** -------------------------------
**
*/

#ifdef NDEBUG
#ifdef DEBUG
#undef DEBUG
#endif
#endif

#ifdef UTL_NOMEMCHECK
#ifdef UTL_MEMCHECK
#undef UTL_MEMCHECK
#endif
#endif

#ifdef UTL_MEMCHECK
#ifdef UTL_NOLOGGING
#undef UTL_NOLOGGING
#endif
#endif

#ifdef UTL_UNITTEST
#ifndef DEBUG
#define DEBUG
#endif
#endif


/* .% Constants
** ============
**
**  A set of constants for generic use. Provided by for convenience.
**
**  .[utlEmptyFun]  A pointer to a do-nothing function that can be used
**                  as a generic placeholder (or NULL indicator) for
**                  function pointers. This can be useful as the C standard
**                  doesn't guarantee that one could mix pointers to objects
**                  with pointers to function (even if in reality this is
**                  practically always the case).  
**  ..
*/

int utlEmptyFun(void); 
#ifdef UTL_C
int   utlEmptyFun(void) {return 0;}
#endif

/*  .[utlEmptyString]  A pointer to the empty string "" that provides unique
**                     representation for the empty string.  
**  ..
*/

utl_extern(char *utlEmptyString, = "");

/*   .[utlZero]  Is a constant whose value is 0 and that is to be used in
**               idioms like '|do { ... } while (utlZero)|.
**               This is useful to avoid compilers warning about testing
**               constant values.
**   ..
*/

  
#ifdef __GNUC__
#define utlZero 0
#else
utl_extern(const int utlZero, = 0);
#endif

/*  MS Visual C doesnt have '|snprintf()| ! How could it be?
*/
#ifdef _MSC_VER
#define snprintf _snprintf
#endif


/* .% Try/Catch
** ~~~~~~~~~~~~~~~~~~~~~~~
**   Exceptions can be very useful when dealing with error conditions is so
** complicate that the logic of errors handling obscures the logic of the 
** program itself.
**   If an error happens in the '|try| section, you can '|throw()| an exception
** (an error condition) and the control goes back to the '|catch| section
** where the proper actions can be taken.
**
**   Simple implementation of try/catch.
** .{{ c
**   tryenv env = NULL;
**   try(env) {
**      ... code ...
**      if (something_failed) throw(env, ERR_OUTOFMEM)  // must be > 0 
**      some_other_func(env); // you can trhow exceptions from other functions 
**      ... code ...          // as long as you pass it the try environment.
**   }  
**   catch({                  // NOTE the catch part is enclosed in special 
*       case ERR_OUTOFMEM :   // braces: '|({ ... })| and is the body of a
**      ... code ...          // '|switch()| statement (including fallthrough!)
**   });
** .}}
**
**  This comes useful when you throw an exception form a called function.
**  The example below, handles the "out of mem" condition in the same place
**  regardless of where the exception was raised.
**
** .{{ C
**
**   #define ERR_OUTOFMEM 0xF0CA
**   tryenv env = NULL; // Remember to set to NULL initally!
**   char *f1(tryenv ee, int x)   { ... throw(ee, ERR_OUTOFMEM} ... }
**   void *f2(tryenv ee, char *x) { ... throw(ee, ERR_OUTOFMEM} ... }
**   try(env) {
**      ... code ...
**      f1(env,3); 
**      ... code ...
**      if ... throw(env,ERR_OUTFOMEM) 
**      ... code ...
**      f2(env,"Hello"); 
**      ... code ...
**   }  
**   catch({                    // Note the use of '|({| and '|})| !!
**      case  ERR_OUTOFMEM : 
**                 ... code ... // Handle all your cleanup here!
**                 break;
**   });
**
** .}}
*/ 

#ifndef UTL_NOTRYCATCH

typedef struct utl_env_s { 
  jmp_buf jb;
  struct utl_env_s volatile *prev;
  struct utl_env_s volatile **orig;
} *tryenv; 

#define try(utl_env) \
            do { struct utl_env_s utl_cur_env; volatile int utl_err; \
                 utl_cur_env.prev = (void *)utl_env; \
                 utl_cur_env.orig = (void *)(&utl_env); \
                 utl_env = &utl_cur_env; \
                 if ((utl_err = setjmp(utl_cur_env.jb))==0)
                 
#define catch(y) if (utl_err) switch (utl_err) { \
                      y \
                   } \
                   utl_err = 0;\
                  *utl_cur_env.orig = utl_cur_env.prev; \
                } while(0)

#define throw(env,err) (env? longjmp(((tryenv)env)->jb, err): exit(err))
#define rethrow        (*utl_cur_env.orig = utl_cur_env.prev,throw(*utl_cur_env.orig,utl_err))

#endif


/*  .% Finite state machine
**  =======================
**
**    A Finite State Machine (FSM) is very useful when 
**
** .v
**      fsm ({            // Note the use of '|({| and '|})| !!
**
**        case : { ...
**                   if (c == 0) fsmGoto(z);
**                   fsmGoto(y);
**        }
**
**        case z : { ...
**                   break;  // exit from the FSM
**        }
**
**        case y : { ...
**                   if (c == 1) fsmGoto(x);
**                   fsmGoto(z);
**        }
**      });
** ..
**
**   It's a good practice to include a drawing of the FSM in the technical
** documentation (e.g including the GraphViz description in comments).
*/

#define fsmSTART  0
#define fsmEND   -1

#define fsm(x)  do { int fsm_next , fsm_state; \
                      for (fsm_next=fsmSTART; fsm_next>=0;) \
                        switch((fsm_state=fsm_next, fsm_next=-1, fsm_state)) { \
                        x \
                }} while (utlZero)
                         
#define fsmGoto(x)  fsm_next = x; break
#define fsmRestart fsm_next = fsmSTART; break
#define fsmExit    fsm_next = fsmEND; break


/* .% UnitTest
** ===========
**
**   These macros will help you writing unit tests.  The log produced
** is '<TAP 1.3=http://testanything.org> compatible and also contains
** all the information about passing/failing.
**
**   They are available only if the symbol '|UTL_UNITTEST| is defined before
** including the '|utl.h| header.
** 
**   '{UTL_UNITTEST} implies '{DEBUG}
*/

#ifdef UTL_UNITTEST

utl_extern(FILE *TST_FILE, = NULL);
#define TSTFILE (TST_FILE?TST_FILE:stderr)

/* Output is flushed every time to avoid we lose a message in case of
** abnormal exit. 
*/
#define TSTWRITE(...) (fprintf(TSTFILE,__VA_ARGS__),fflush(TSTFILE))

#define TSTTITLE(s) TSTWRITE("TAP version 13\n#\n# ** %s - (%s)\n",s,__FILE__)

#define TST_INIT0 (TSTRES=TSTNUM=TSTGRP=TSTSEC=TSTTOT= \
                               TSTGTT=TSTGPAS=TSTPASS=TSTNSK=TSTNTD=0)
                                 
#define TSTPLAN(s) for (TSTPASSED = TST_INIT0 + 1, TSTTITLE(s); \
                                             TSTPASSED; TSTDONE(),TSTPASSED=0)

/* Tests are divided in sections introduced by '{=TSTSECTION(title)} macro.
** The macro reset the appropriate counters and prints the section header 
*/
#define TSTSECTION(s)  if ((TSTSTAT(), TSTGRP = 0, TSTSEC++, \
                             TSTWRITE("#\n# * %d. %s (%s:%d)\n", \
                             TSTSEC, s, __FILE__, __LINE__), TSTPASS=0)) 0; \
                       else                                               

/* to disable an entire test section, just prepend '|_| or '|X|*/
 
#define XTSTSECTION(s) if (!utlZero) 0; else 
#define _TSTSECTION(s) XTSTSECTION(s)

/* In each section, tests can be logically grouped so that different aspects
** of related functions can be tested.
*/
#define TSTGROUP(s) \
    if ((TSTWRITE("#\n# *   %d.%d %s\n",TSTSEC,++TSTGRP,s),TSTNUM=0)) 0; \
    else
                     
/* to disable a n entire test group , just prepend '|_| or '|X| */
#define XTSTGROUP(s) if (!utlZero) 0; else  
#define _TSTGROUP(s) XTSTGROUP(s)

/* Test code will be skipped if needed */
#define TSTCODE   if (TSTSKP)   0; else  
#define XTSTCODE  if (!utlZero) 0; else
#define _TSTCODE  XTSTCODE
                     
/* The single test is defined  with the '|TST(s,x)| macro.
**   .['|s|] is a short string that identifies the test
**    ['|x|] an assertion that has to be true for the test to succeed.
**   ..
*/
#define XTST(s,x)

#define TST(s,x) (TST_DO(s,(TSTSKP?1:(x))),TST_WRDIR, TSTWRITE("\n"), TSTRES)

#define TST_DO(s,x) (TSTRES = (x), TSTGTT++, TSTTOT++, TSTNUM++, \
                     TSTPASSED = (TSTPASSED && (TSTRES || TSTTD)), \
                     TSTWRITE("%s %4d - %s (:%d)", \
                              (TSTRES? (TSTGPAS++,TSTPASS++,TSTOK) : TSTKO), \
                               TSTGTT, s, __LINE__))

#define TST_WRDIR \
           (TSTSKP ? (TSTNSK++, TSTWRITE(" # SKIP %s",TSTSKP)) \
                   : (TSTTD ? (TSTNTD++, (TSTWRITE(" # TODO %s%s",TSTTD, \
                                        (TSTRES?TSTWRN:utlEmptyString)))) : 0))

#define TSTFAILED  (!TSTRES)

/* You can skip a set of tests giving a reason.
** Nested skips are not supported!
*/
#define TSTSKIP(x,r) if (!(x)) 0; else for (TSTSKP=r; TSTSKP; TSTSKP=NULL)

#define TSTTODO(r)   for (TSTTD=r; TSTTD; TSTTD=NULL)

#define TSTNOTE(...) \
                 (TSTWRITE("#      "),TSTWRITE(__VA_ARGS__), TSTWRITE("\n"))
                                            
#define TSTFAILNOTE(...) (TSTRES? 0 : (TSTNOTE(__VA_ARGS__)))

#define TSTEXPECTED(f1,v1,f2,v2) \
                             (TSTRES? 0 : (TSTNOTE("Expected "f1" got "f2,v1,v2)))

#define TSTEQINT(s,e,r) do { int __exp = (e); int __ret = (r);\
                             TST(s,__exp==__ret);\
                             TSTEXPECTED("(int) %d",__exp,"%d",__ret); \
                           } while (utlZero)

#define TSTNEQINT(s,e,r) do { int __exp = (e); int __ret = (r);\
                             TST(s,__exp!=__ret);\
                             TSTEXPECTED("(int) other than %d",__exp,"%d",__ret); \
                           } while (utlZero)
                           
#define TSTEQPTR(s,e,r)  do { void *__exp = (e); void *__ret = (r); \
                              TST(s,__exp == __ret) ; \
                              TSTEXPECTED("(ptr) 0x%p",__exp,"0x%p",__ret); \
                            }  while (utlZero)

#define TSTNEQPTR(s,e,r) do { void *__exp = (e); void *__ret = (r); \
                              TST(s,__exp != __ret) ; \
                              TSTEXPECTED("(ptr) other than 0x%p",__exp,"0x%p",__ret); \
                            }  while (utlZero)

#define TSTNULL(s,r)     TSTEQPTR(s,NULL,r)

#define TSTNNULL(s,r)    TSTNEQPTR(s,NULL,r)

							
#define TSTBAILOUT(r) \
          if (!(r)) 0; else {TSTWRITE("Bail out! %s\n",r); TSTDONE(); exit(1);}

/* At the end of a section, the accumulated stats can be printed out */
#define TSTSTAT() \
          (TSTTOT == 0 ? 0 : ( \
           TSTWRITE("#\n# SECTION %d OK: %d/%d\n",TSTSEC,TSTPASS,TSTTOT), \
           TSTTOT = 0))

/* At the end of all the tests, the accumulated stats can be printed out */
#define TSTDONE() \
  (TSTGTT <= 0 ? 0 : ( TSTSTAT(),  \
  TSTWRITE("#\n# TOTAL OK: %d/%d SKIP: %d TODO: %d\n",TSTGPAS,TSTGTT, \
                                                              TSTNSK,TSTNTD), \
  TSTWRITE("#\n# TEST PLAN: %s \n",TSTPASSED ? "PASSED" : "FAILED"), \
  TSTWRITE("#\n1..%d\n",TSTGTT),fflush(TSTFILE)) )

/* Execute a statement if a test succeeded */
#define TSTIF_OK  if (TSTRES)

/* Execute a statement if a test failed */
#define TSTIF_NOTOK if (!TSTRES)

static int    TSTRES    = 0;  /* Result of the last performed '|TST()| */
static int    TSTNUM    = 0;  /* Last test number */
static int    TSTGRP    = 0;  /* Current test group */
static int    TSTSEC    = 0;  /* Current test SECTION*/
static int    TSTTOT    = 0;  /* Number of tests executed */
static int    TSTGTT    = 0;  /* Number of tests executed (Grand Total) */
static int    TSTGPAS   = 0;  /* Number of tests passed (Grand Total) */
static int    TSTPASS   = 0;  
static int    TSTPASSED = 1;  
static int    TSTNSK    = 0;  
static int    TSTNTD    = 0;  
static char  *TSTSKP    = NULL;
static char  *TSTTD     = NULL;

static const char *TSTOK  = "ok    ";
static const char *TSTKO  = "not ok";
static const char *TSTWRN = " (passed unexpectedly!)";


#endif /* UTL_UNITTEST */

/* .% Logging
** ==========
**
*/

#define log_D 7
#define log_I 6
#define log_M 5
#define log_W 4
#define log_E 3
#define log_C 2
#define log_A 1
#define log_F 0


/* Logging functions are available unless the symbol '{=UTL_NOLOGGING}
** has been defined before including '|utl.h|.
*/

#ifndef UTL_NOLOGGING

#define UTL_LOG_NEW 0x00
#define UTL_LOG_ADD 0x01
#define UTL_LOG_BIN 0x02
#define UTL_LOG_ROT 0x04
#define UTL_LOG_ERR 0x10

typedef struct {
  FILE          *file;
  unsigned char  level;
  unsigned char  flags;
  unsigned short count;
} utl_log_s, *logger;

#include <time.h>
#include <ctype.h>

/* .%% Logging levels
** ~~~~~~~~~~~~~~~~~~
**
**   Logging levels are hierarchical and structured as in log4j. The variable 
** '{=log_level} contains the current logging level.  Logging is off by default.
**
**   Use '{=logLevel()}    to set the desired level of logging.
**   Use '{=logLevelEnv()} to set the desired level of logging based on an
**                         enviroment variable.
*/

                                   /* 0   1   2   3   4   5   6   7   8   9    */
                                   /* 0   4   8   12  16  20  24  28  32  36   */
utl_extern(char const *log_abbrev, = "FTL ALT CRT ERR WRN MSG INF DBG OFF LOG   ");

int   log_level(logger lg);
int   log_chrlevel(char *l);
int   logLevel(logger lg, char *lv); 
int   logLevelEnv(logger lg, char *var, char *level);

/*
** The table below shows whether a message of a certain level will be
** printed (Y) or not (N) given the current level of logging.
** .v
**                          message level 
**                    DBG INF MSG WRN ERR CRT ALT FTL
**               DBG   Y   Y   Y   Y   Y   Y   Y   Y
**               INF   N   Y   Y   Y   Y   Y   Y   Y
**               MSG   N   N   Y   Y   Y   Y   Y   Y
**      current  WRN   N   N   N   Y   Y   Y   Y   Y
**      logging  ERR   N   N   N   N   Y   Y   Y   Y
**       level   CRT   N   N   N   N   N   Y   Y   Y
**               ALT   N   N   N   N   N   N   Y   Y
**               FTL   N   N   N   N   N   N   N   Y
**               OFF   N   N   N   N   N   N   N   N
** ..
*/

/* .%% Logging file rotate
** ~~~~~~~~~~~~~~~~~~~~~~~
**
** For long running programs (servers, daemons, ...) it is important to rotate 
** the log files from time to time so that they won't become too big.
** utl offers a very simple model, if rotation is enabled, every 65536 messages
** the file will be closed and a new file will be opened. Assuming
** Then new file will be renamed _1, _2, etc.
**
**   logRotateOn(lg)
**   logRotateOff(lg)
**

**
/* .%% Logging format
** ~~~~~~~~~~~~~~~~~~
** 
** Log files have the following format:
** .v
**     <date> <time> <level> <message>
** ..
**
**  For example:
** .v
**     2009-01-29 13:46:02 ERR An error!
**     2009-01-29 13:46:02 FTL An unrecoverable error
** ..
**
*/

/*    Log files can be opened in "write" or "append" mode as any normal file 
** using the '{=logOpen()} function.
** For example:
** .v  
**   logger lgr = NULL;
**   logOpen(lgr,"file1.log",UTL_LOG_NEW) // Delete old log file and create a new one
**   ...
**   logOpen(lgr,"file1.log",UTL_LOG_ADD) // Append to previous log file
** .. 
*/

#define logOpen(l,f,m)   (l=log_open(f,m))
#define logClose(l)      (log_close(l),l=NULL)

logger log_open(char *fname, unsigned char mode);
logger log_close(logger lg);
void log_write(logger lg,int lv, char *format, ...);
FILE *logFile(logger lg);

#define logIf(lg,lc) log_if(lg,log_chrlevel(lc))

#define log_if(lg,lv) if ((lv) >= log_level(lg)) utlZero ; else
          
#define logDebug(lg, ...)    log_write(lg,log_D, __VA_ARGS__)
#define logInfo(lg, ...)     log_write(lg,log_I, __VA_ARGS__)
#define logMessage(lg, ...)  log_write(lg,log_M, __VA_ARGS__)
#define logWarn(lg, ...)     log_write(lg,log_W, __VA_ARGS__)
#define logError(lg, ...)    log_write(lg,log_E, __VA_ARGS__)
#define logCritical(lg, ...) log_write(lg,log_C, __VA_ARGS__)
#define logAlarm(lg, ...)    log_write(lg,log_A, __VA_ARGS__)
#define logFatal(lg, ...)    log_write(lg,log_F, __VA_ARGS__)

#define logContinue(lg,...)  (fputs("                        ",logFile(lg),\
                             (fprintf(logFile,__VA_ARGS__), fputc('\n',logFile(lg)),fflush(logFile(lg))))

/*
** .v
**   logError("Too many items at counter %d (%d)",numcounter,numitems);
**   logContinue("Occured %d times",times++);
** ..
** will produce:
** .v
**     2009-01-29 13:46:02 ERR Too many items at counter 9 (5)
**                             Occured 3 times
** ..
*/

#ifdef UTL_C
int   log_level(logger lg) { return (int)(lg ? lg->level : log_W) ; }
FILE *logFile(logger lg) {FILE *f=NULL; if (lg) f = lg->file; return f?f:stderr; }

int   log_chrlevel(char *l) {
  int i=0;
  char c = l ? toupper(l[0]) : 'W';
  
  while (log_abbrev[i] != ' ' && log_abbrev[i] != c) i+=4;
  i = (i <= 4*7) ? (i>> 2) : log_W;
  return i;
}

int logLevel(logger lg, char *lv) 
{
  if (lg) {
    if (lv && lv[0])
      lg->level = log_chrlevel(lv);
  }
  return log_level(lg);  
}

int logLevelEnv(logger lg, char *var, char *level)
{
  char *lvl_str;
  
  lvl_str=getenv(var);
  if (!lvl_str) lvl_str = level;
  return logLevel(lg,lvl_str);
}

logger log_open(char *fname, unsigned char mode)
{
  char md[4];
  logger lg;
  
  lg = malloc(sizeof(utl_log_s));
  if (lg) {
	lg->file = NULL;
    if (fname) {
      md[0] = 'w'; md[1] = '\0';
      if (mode & UTL_LOG_ADD) md[0] = 'a'; 
      lg->file = fopen(fname,md);
    }
	if (!lg->file)
      lg->file = (mode & UTL_LOG_ERR)? stderr : stdout;

	if (lg->file != stderr && lg->file != stdout) {
      lg->level = 9;
      log_write(lg,9, "%s \"%s\"", (mode & UTL_LOG_ADD) ? "ADDEDTO" : "CREATED",fname); 
	}

    lg->level = log_W;
  }
  return lg;
}

logger log_close(logger lg)
{
  if (lg) {
    if (lg->file && lg->file != stderr && lg->file != stdout)
	  fclose(lg->file);
	lg->file = NULL;
	free(lg);
  }
  return NULL;
}

void log_write(logger lg, int lv, char *format, ...)
{
  va_list args;
  char tstr[32];
  time_t t;
  FILE *f = stderr;
  int lg_lv = log_W;
  if (lg) {
    f = lg->file;
    lg_lv = lg->level;
  }
  lv = lv & 0x0F;
  if(lg_lv <= lv) {
    time(&t);
    strftime(tstr,64,"%Y-%m-%d %X",localtime(&t));
    fprintf(f,"%s %.4s",tstr, log_abbrev+(lv<<2));
    va_start(args, format);
    vfprintf(f,format, args);
    va_end(args);
	fputc('\n',f);
    fflush(f);
  }    
}


#endif  /*- UTL_C */

#else

#define logLevel(lg,lv)       log_W
#define logLevelEnv(lg,v,l)   log_W     
#define logDebug(lg, ...)    
#define logInfo(lg, ...)     
#define logMessage(lg, ...)  
#define logWarn(lg, ...)     
#define logError(lg, ...)    
#define logCritical(lg, ...) 
#define logAlarm(lg, ...)    
#define logFatal(lg, ...)    

#define logIf(lg,lv) if (!utlZero) utlZero ; else

#define logContinue(lg,...)  

#define logOpen(lg,f,m) (lg=NULL)
#define logClose(lg)    (lg=NULL)

typedef void *logger;

#endif /*- UTL_NOLOGGING */

#ifdef NDEBUG
#undef logDebug
#define logDebug(lg,...) 
#endif

#define logNDebug(lg,...)

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*  .% Traced memory check
**  ======================
*/
#define utlMemInvalid    -2
#define utlMemOverflow   -1
#define utlMemValid       0
#define utlMemNull        1

#ifdef UTL_MEMCHECK
void *utl_malloc  (size_t size, char *file, int line );
void *utl_calloc  (size_t num, size_t size, char *file, int line);
void *utl_realloc (void *ptr, size_t size, char *file, int line);
void  utl_free    (void *ptr, char *file, int line );
void *utl_strdup  (void *ptr, char *file, int line);

int utl_check(void *ptr,char *file, int line);

#ifndef UTL_NOMEMLOGGER
logger memLogger();
#else
#define memlogger() NULL
#endif 

#ifdef UTL_C
/*************************************/

static char *BEG_CHK = "\xBE\xEF\xF0\x0D";
static char *END_CHK = "\xDE\xAD\xC0\xDA";
static char *CLR_CHK = "\xDE\xFA\xCE\xD0";

static size_t utl_mem_allocated = 0;

typedef struct {
   size_t size;
   char   chk[4];
   char   data[4];
} utl_mem_t;

#define utl_mem(x) ((utl_mem_t *)((char *)(x) -  offsetof(utl_mem_t, data)))

int utl_check(void *ptr,char *file, int line)
{
  utl_mem_t *p;
  
  if (ptr == NULL) return utlMemNull;
  p = utl_mem(ptr);
  if (memcmp(p->chk,BEG_CHK,4)) { 
    logError(memLogger(),"Invalid or double freed %p (%u %s %d)",p->data, \
                                               utl_mem_allocated, file, line);     
    return utlMemInvalid; 
  }
  if (memcmp(p->data+p->size,END_CHK,4)) {
    logError(memLogger(),"Boundary overflow detected %p [%d] (%u %s %d)", \
                              p->data, p->size, utl_mem_allocated, file, line); 
    return utlMemOverflow;
  }
  logInfo(memLogger(),"Valid pointer %p (%u %s %d)",ptr, utl_mem_allocated, file, line); 
  return utlMemValid; 
}

void *utl_malloc(size_t size, char *file, int line )
{
  utl_mem_t *p;
  
  if (size == 0) logWarn(memLogger(),"Shouldn't allocate 0 bytes (%u %s %d)", \
                                                utl_mem_allocated, file, line);
  p = malloc(sizeof(utl_mem_t) +size);
  if (p == NULL) {
    logCritical(memLogger(),"Out of Memory (%u %s %d)",utl_mem_allocated, file, line);
    return NULL;
  }
  p->size = size;
  memcpy(p->chk,BEG_CHK,4);
  memcpy(p->data+p->size,END_CHK,4);
  utl_mem_allocated += size;
  logInfo(memLogger(),"alloc %p [%d] (%u %s %d)",p->data,size,utl_mem_allocated,file,line);
  return p->data;
};

void *utl_calloc(size_t num, size_t size, char *file, int line)
{
  void *ptr;
  
  size = num * size;
  ptr = utl_malloc(size,file,line);
  if (ptr)  memset(ptr,0x00,size);
  return ptr;
};

void utl_free(void *ptr, char *file, int line)
{
  utl_mem_t *p=NULL;
  
  switch (utl_check(ptr,file,line)) {
    case utlMemNull  :    logWarn(memLogger(),"free NULL (%u %s %d)", 
                                                utl_mem_allocated, file, line);
                          break;
                          
    case utlMemOverflow : logWarn(memLogger(), "Freeing an overflown block  (%u %s %d)", 
                                                           utl_mem_allocated, file, line);
    case utlMemValid :    p = utl_mem(ptr); 
                          memcpy(p->chk,CLR_CHK,4);
                          utl_mem_allocated -= p->size;
                          if (p->size == 0)
                            logWarn(memLogger(),"Freeing a block of 0 bytes (%u %s %d)", 
                                                utl_mem_allocated, file, line);

                          logInfo(memLogger(),"free %p [%d] (%u %s %d)", ptr, 
                                    p?p->size:0,utl_mem_allocated, file, line);
                          free(p);
                          break;
                          
    case utlMemInvalid :  logError(memLogger(),"free an invalid pointer! (%u %s %d)", \
                                                utl_mem_allocated, file, line);
                          break;
  }
}

void *utl_realloc(void *ptr, size_t size, char *file, int line)
{
  utl_mem_t *p;
  
  if (size == 0) {
    logWarn(memLogger(),"realloc() used as free() %p -> [0] (%u %s %d)",ptr,utl_mem_allocated, file, line);
    utl_free(ptr,file,line); 
  } 
  else {
    switch (utl_check(ptr,file,line)) {
      case utlMemNull   : logWarn(memLogger(),"realloc() used as malloc() (%u %s %d)", \
                                             utl_mem_allocated, file, line);
                          return utl_malloc(size,file,line);
                        
      case utlMemValid  : p = utl_mem(ptr); 
                          p = realloc(p,sizeof(utl_mem_t) + size); 
                          if (p == NULL) {
                            logCritical(memLogger(),"Out of Memory (%u %s %d)", \
                                             utl_mem_allocated, file, line);
                            return NULL;
                          }
                          utl_mem_allocated -= p->size;
                          utl_mem_allocated += size; 
                          logInfo(memLogger(),"realloc %p [%d] -> %p [%d] (%u %s %d)", \
                                          ptr, p->size, p->data, size, \
                                          utl_mem_allocated, file, line);
                          p->size = size;
                          memcpy(p->chk,BEG_CHK,4);
                          memcpy(p->data+p->size,END_CHK,4);
                          ptr = p->data;
                          break;
    }
  }
  return ptr;
}

void *utl_strdup(void *ptr, char *file, int line)
{
  char *dest;
  size_t size;
	
  if (ptr == NULL) {
    logWarn(memLogger(),"strdup NULL (%u %s %d)", utl_mem_allocated, file, line);
    return NULL;
  }
  size = strlen(ptr)+1;

  dest = utl_malloc(size,file,line);
  if (dest) memcpy(dest,ptr,size);
  logInfo(memLogger(),"strdup %p [%d] -> %p (%u %s %d)", ptr, size, dest, \
                                                utl_mem_allocated, file, line);
  return dest;
}
#undef utl_mem

/*************************************/
#endif

#define malloc(n)     utl_malloc(n,__FILE__,__LINE__)
#define calloc(n,s)   utl_calloc(n,s,__FILE__,__LINE__)
#define realloc(p,n)  utl_realloc(p,n,__FILE__,__LINE__)
#define free(p)       utl_free(p,__FILE__,__LINE__)
#define strdup(p)     utl_strdup(p,__FILE__,__LINE__)

#define utlMemCheck(p) utl_check(p,__FILE__, __LINE__)
#define utlMemAllocated utl_mem_allocated
#define utlMemValidate(p) utl_mem_validate(p)

#define utlMalloc(n)     utl_malloc(n,__FILE__,__LINE__)
#define utlCalloc(n,s)   utl_calloc(n,s,__FILE__,__LINE__)
#define utlRealloc(p,n)  utl_realloc(p,n,__FILE__,__LINE__)
#define utlFree(p)       utl_free(p,__FILE__,__LINE__)
#define utlStrdup(p)     utl_strdup(p,__FILE__,__LINE__)


#else /* UTL_MEMCHECK */

#define utlMemCheck(p) utlMemValid
#define utlMemAllocated 0
#define utlMemValidate(p) (p)

#endif /* UTL_MEMCHECK */


#endif /* UTL_H */

/**************************************************************************/

