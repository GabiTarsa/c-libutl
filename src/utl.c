/* 
**  (C) by Remo Dentato (rdentato@gmail.com)
** 
** This sofwtare is distributed under the terms of the BSD license:
**   http://creativecommons.org/licenses/BSD/
**   http://opensource.org/licenses/bsd-license.php 
*/


#ifdef UTL_MEMCHECK
#undef UTL_MEMCHECK
#endif

#include "libutl.h"

#ifndef __GNUC__
const int utlZero = 0 ;
#endif

void utlEmptyFun(void) {}
char *utlEmptyString = "";

char *utlErrInternal = "Internal error" ;

FILE *utl_stderr = NULL;
/*************************************/

int utl_fsmcur = 0;
int utl_fsmcnt = 0;
int utl_fsmret = 0;
int utl_fsmrets[utl_fsmmax] = {0} ;

/*************************************/

FILE *log_file = NULL;
int logLevel = logOFF ;
/*
** These variables are used internally to compose and print log messages:
**
**  .['{=log_abbrev}] The string abbreviations for the levels. 
**  ..
*/

char *log_abbrev[] = {"ALL","DBG","INF","WRN","ERR","FTL","OFF"};

/* 
**  .['{=log_timestr}]  A buffer to compose current time.
**   ['{=log_time}]     A '|time_t| structure to compose current time. 
**  .. 
*/

char log_timestr[32];
time_t log_time = 0 ;

/*************************************/

static char *BEG_CHK = "\xBE\xEF\xF0\x0D";
static char *END_CHK = "\xDE\xAD\xC0\xDA";
static char *CLR_CHK = "\xDE\xFA\xCE\xD0";

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
    logError("Invalid or double freed %p [%s - %d]",p->data,file,line);     
    return utlMemInvalid; 
  }
  if (memcmp(p->data+p->size,END_CHK,4)) {
    logError("Boundary overflow detected %p [%d] [%s - %d]",p->data, p->size, file,line); 
    return utlMemOverflow;
  }
  return utlMemValid; 
}

void *utl_malloc(size_t size, char *file, int line )
{
  utl_mem_t *p;
  
  if (size == 0) logWarn("Shouldn't allocate 0 bytes [%s - %d]",file,line);
  p = malloc(sizeof(utl_mem_t) +size);
  if (p == NULL) {
    logError("Out of Memory [%s - %d]",file,line);
    return NULL;
  }
  p->size = size;
  memcpy(p->chk,BEG_CHK,4);
  memcpy(p->data+p->size,END_CHK,4);
  logInfo("alloc %p [%d] [%s - %d]",p->data,size,file,line);
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
    case utlMemNull  :    logInfo("free NULL [%s - %d]", file,line);
                          break;
                          
    case utlMemValid :    p = utl_mem(ptr); 
                          memcpy(p->chk,CLR_CHK,4);
                          logInfo("free %p [%d] [%s - %d]",ptr,p?p->size:0,file,line);
    case utlMemOverflow : free(p);
                          break;
  }
}

void *utl_realloc(void *ptr, size_t size, char *file, int line)
{
  utl_mem_t *p;
  
  if (size == 0) {
    logInfo("realloc %p -> [0] [%s - %d]\n",ptr,file,line);
    utl_free(ptr,file,line); 
  } 
  else {
    switch (utl_check(ptr,file,line)) {
      case utlMemNull     : logInfo("realloc NULL [%s - %d]\n",file,line);
                            return utl_malloc(size,file,line);
                          
      case utlMemValid    : p = utl_mem(ptr); 
                            p = realloc(p,sizeof(utl_mem_t) + size); 
                            if (p == NULL) {
                              logError("Out of Memory [%s - %d]\n",file,line);
                              return NULL;
                            }
                            logInfo("realloc %p [%d] -> %p [%d] [%s - %d]",ptr, p->size, p->data, size, file,line);
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
    logWarn("strdup NULL [%s - %d]", file,line);
    return NULL;
  }
  size = strlen(ptr);

  dest = utl_malloc(size+1,file,line);
  if (dest) memcpy(dest,ptr,size+1);
  logInfo("strdup %p [%d] -> %p [%s - %d]",ptr, size, dest, file,line);
  return dest;
}
