/* 
**  (C) by Remo Dentato (rdentato@gmail.com)
** 
** This software is distributed under the terms of the BSD license:
**   http://creativecommons.org/licenses/BSD/
**   http://opensource.org/licenses/bsd-license.php 
*/

#ifndef TBL_H
#define TBL_H

typedef union {
  void           *p;
  char           *s;
  long            n;
  unsigned long   u;
  float           f;
} val_u;


#define valGetM(v)   ((v).p)
#define valGetT(v)   ((tbl_t)((v).p))
#define valGetP(v)   ((v).p)
#define valGetS(v)   ((v).s)
#define valGetN(v)   ((v).n)
#define valGetU(v)   ((v).u)
#define valGetF(v)   ((v).f)

#define valM(x) valP(x)
#define valT(x) valP(x)
val_u   valP(void *val);        
val_u   valS(char *val);         
val_u   valN(long val);
val_u   valU(unsigned long val);
val_u   valF(float val);


char *val_Sdup(char *s);
char *val_Sfree(char *s);


/* A table slot.
*/ 
typedef struct {
  val_u  key;
  val_u  val;
  char key_type;
  char val_type;
  unsigned char dist;
  unsigned char flags;
} tbl_slot_t;    

typedef struct {
  long count;
  long size;
  unsigned char max_dist;
  unsigned char flags;
  unsigned char pad1;
  unsigned char pad2;
  tbl_slot_t slot[1];  /* start of the slots array */
} tbl_table_t;

typedef tbl_table_t *tbl_t;
 
tbl_t tbl_new(long nslots);
#define tblNew(tb) (tb = tbl_new(2))

tbl_t tbl_free(tbl_t tb);
#define tblFree(tb) (tb = tbl_free(tb)) 
 
#define tblCount(tb) (tb?(tb)->count:0)

tbl_t tbl_set(tbl_t tb, char k_type, val_u key, char v_type, val_u val);
 
#define tbl_setXX(tb,tk,k,fk,tv,v,fv) \
   do {val_u ak,av; ak.fk = k; av.fv = v; tb = tbl_set(tb,tk,ak,tv,av);} while (0)
 
#define tblSetPM(tb,k,v) tbl_setXX(tb,'P',k,p,'M',v,p)
#define tblSetPT(tb,k,v) tbl_setXX(tb,'P',k,p,'T',v,p)
#define tblSetPP(tb,k,v) tbl_setXX(tb,'P',k,p,'P',v,p)
#define tblSetPS(tb,k,v) tbl_setXX(tb,'P',k,p,'S',val_Sdup(v),s)
#define tblSetPN(tb,k,v) tbl_setXX(tb,'P',k,p,'N',v,n)
#define tblSetPU(tb,k,v) tbl_setXX(tb,'P',k,p,'U',v,u)
#define tblSetPF(tb,k,v) tbl_setXX(tb,'P',k,p,'F',v,f)

#define tblSetNM(tb,k,v) tbl_setXX(tb,'N',k,n,'M',v,p)
#define tblSetNT(tb,k,v) tbl_setXX(tb,'N',k,n,'T',v,p)
#define tblSetNP(tb,k,v) tbl_setXX(tb,'N',k,n,'P',v,p)
#define tblSetNS(tb,k,v) tbl_setXX(tb,'N',k,n,'S',val_Sdup(v),s)
#define tblSetNN(tb,k,v) tbl_setXX(tb,'N',k,n,'N',v,n)
#define tblSetNU(tb,k,v) tbl_setXX(tb,'N',k,n,'U',v,u)
#define tblSetNF(tb,k,v) tbl_setXX(tb,'N',k,n,'F',v,f)

#define tblSetSM(tb,k,v) tbl_setXX(tb,'S',val_Sdup(k),n,'M',v,p)
#define tblSetST(tb,k,v) tbl_setXX(tb,'S',val_Sdup(k),n,'T',v,p)
#define tblSetSP(tb,k,v) tbl_setXX(tb,'S',val_Sdup(k),n,'P',v,p)
#define tblSetSS(tb,k,v) tbl_setXX(tb,'S',val_Sdup(k),n,'S',val_Sdup(v),s)
#define tblSetSN(tb,k,v) tbl_setXX(tb,'S',val_Sdup(k),n,'N',v,n)
#define tblSetSU(tb,k,v) tbl_setXX(tb,'S',val_Sdup(k),n,'U',v,u)
#define tblSetSF(tb,k,v) tbl_setXX(tb,'S',val_Sdup(k),n,'F',v,f)

val_u tbl_get(tbl_t tb, char k_type, val_u key, char v_type, val_u def);

#define tblGetNM(tb,k,d) valGetM(tbl_get(tb,'N',valN(k),'M',valM(d)))
#define tblGetNT(tb,k,d) valGetT(tbl_get(tb,'N',valN(k),'T',valT(d)))
#define tblGetNP(tb,k,d) valGetP(tbl_get(tb,'N',valN(k),'P',valP(d)))
#define tblGetNS(tb,k,d) valGetS(tbl_get(tb,'N',valN(k),'S',valS(d)))
#define tblGetNN(tb,k,d) valGetN(tbl_get(tb,'N',valN(k),'N',valN(d)))
#define tblGetNU(tb,k,d) valGetU(tbl_get(tb,'N',valN(k),'U',valU(d)))
#define tblGetNF(tb,k,d) valGetF(tbl_get(tb,'N',valN(k),'F',valF(d)))

#define tbl_delX(tb,tk,k,fk) \
       do {val_u key; key.fk = k; tb = tbl_del(tb,tk,key);} while (0)

#define tblDelM(tb,k) tbl_delX(tb,'M',k,p)
#define tblDelT(tb,k) tbl_delX(tb,'T',k,p)
#define tblDelP(tb,k) tbl_delX(tb,'P',k,p)
#define tblDelS(tb,k) tbl_delX(tb,'S',k,s)
#define tblDelN(tb,k) tbl_delX(tb,'N',k,n)
#define tblDelU(tb,k) tbl_delX(tb,'U',k,u)
#define tblDelF(tb,k) tbl_delX(tb,'F',k,f)

typedef long tblptr_t;

tblptr_t tblNext(tbl_t tb, tblptr_t ndx);
#define tblFirst(tb) tblNext(tb,0)

tblptr_t tbl_find(tbl_t tb, char k_type, val_u key);

#define tblFindM(tb,k)  tbl_find(tb,'M',valM(k))
#define tblFindT(tb,k)  tbl_find(tb,'T',valT(k))
#define tblFindP(tb,k)  tbl_find(tb,'P',valP(k))
#define tblFindS(tb,k)  tbl_find(tb,'S',valS(k))
#define tblFindN(tb,k)  tbl_find(tb,'N',valN(k))
#define tblFindU(tb,k)  tbl_find(tb,'U',valU(k))
#define tblFindF(tb,k)  tbl_find(tb,'F',valF(k))

char tblKeyType(tbl_t tb, tblptr_t ndx);
char tblValType(tbl_t tb, tblptr_t ndx);

val_u tbl_key(tbl_t tb, tblptr_t ndx);
val_u tbl_val(tbl_t tb, tblptr_t ndx);

#define tblKeyM(tb,n)  valGetM(tbl_key(tb,n))
#define tblKeyT(tb,n)  valGetT(tbl_key(tb,n))
#define tblKeyP(tb,n)  valGetP(tbl_key(tb,n))
#define tblKeyS(tb,n)  valGetS(tbl_key(tb,n))
#define tblKeyN(tb,n)  valGetN(tbl_key(tb,n))
#define tblKeyU(tb,n)  valGetU(tbl_key(tb,n))
#define tblKeyF(tb,n)  valGetF(tbl_key(tb,n))

#define tblValM(tb,n)  valGetM(tbl_val(tb,n))
#define tblValT(tb,n)  valGetT(tbl_val(tb,n))
#define tblValP(tb,n)  valGetP(tbl_val(tb,n))
#define tblValS(tb,n)  valGetS(tbl_val(tb,n))
#define tblValN(tb,n)  valGetN(tbl_val(tb,n))
#define tblValU(tb,n)  valGetU(tbl_val(tb,n))
#define tblValF(tb,n)  valGetF(tbl_val(tb,n))

#endif