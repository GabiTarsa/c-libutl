/* 
**  (C) by Remo Dentato (rdentato@gmail.com)
** 
** This software is distributed under the terms of the BSD license:
**   http://creativecommons.org/licenses/BSD/
**   http://opensource.org/licenses/bsd-license.php 
*/


#define UTL_UNITTEST
#include "libutl.h"
#include <time.h>

int main(void)
{
  vec_t vt = NULL;
  char *ss;
  long   ii, kk, jj;
  long  mm, ll;
  char fstr[] = {"abcdefghijklm"};
  chs_t str;
 
  srand(time(0));

  TSTSECTION("vec Basics") {
 
    TSTGROUP("Create") {
      vt = vecNew();
      TST("Created: count == 0",vt != NULL && vecCount(vt) == 0);
      vt = vecFree(vt);
      TST("VEC destroyed: count = 0", vt == NULL && vecCount(vt) == 0);
    }
    
    TSTGROUP("NULL vector") {
      jj = vecGetN(vt,323,-1);
      TST("Default value", jj == -1);
      
      TST("Count == 0", vt == NULL && vecCount(vt) == 0);
      
      vt = vecSetN(vt,4,1);
      TST("Count == 1", vt != NULL && vecCount(vt) == 5);
      TSTIF_NOTOK {
        TSTNOTE("vt=%p count:%d",vt,vecCount(vt));
      }
    }
    
    TSTGROUP("Integer Values") {
      vt = vecFree(vt);
      TST("VEC destroyed: count = 0", vt == NULL && vecCount(vt) == 0);
    
      jj = 0;                                
      for (kk=0; kk < 100; kk++) {
        ii = rand() & 0x3FF;
       _dbgmsg("index: [%ld] = %ld\n",ii,kk);
        if (jj < ii) jj = ii;
        vt = vecSetN(vt,ii,vecGetN(vt,ii,0)+1);
      }
       
      TSTNOTE("max ndx: [%ld] count: %ld", jj, vecCount(vt));
      TST("Integer values inserted", vecCount(vt) - 1 == jj);

      jj = vecGetN(vt,2230,-342);
      TST("Non existant value",vt != NULL && jj == -342);
      
      ii = 0;
      for (kk = 0; kk < vecCount(vt); kk++) {
        ii += vecGetN(vt,kk,0); 
      }
      TST("sum matched", ii == 100);
      
    }      
    
    TSTGROUP("String Values") {
      vt = vecFree(vt);
      TST("VEC destroyed: count = 0", vt == NULL && vecCount(vt) == 0);
    
      jj = 0;                                
      for (kk=0; kk < 100; kk++) {
        ii = (rand() & 0x3F) | 0x040;
        fstr[0]=ii;
        vt = vecSetS(vt,kk,fstr);
      }
       
      TST("String values inserted", vecCount(vt) == kk);
      TSTIF_NOTOK {
        TSTNOTE("vt=%p count:%d", vt,vecCount(vt));
      }

      ss = vecGetS(vt,2230,"?");
      TST("Non existant value", vt != NULL && ss[0] == '?');
            
    }      

    _TSTGROUP("print strings") {
       for (kk=0; kk<vecCount(vt); kk++) {
         ss = vecGetS(vt,kk,NULL);
         if (ss) TSTNOTE("%4d %s",kk,ss);
       }
    }

    vt = vecFree(vt);
    TST("VEC destroyed: count = 0", vt == NULL && vecCount(vt) == 0);
  }
  
  TSTDONE();

  exit(0);
}