/* 

tree counter program by Alexandros Stamatakis released under GNU GPL 

Alexandros.Stamatakis@gmail.com

*/

#include <assert.h>
#include <math.h>
#include <time.h> 
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <gmp.h>

#define TRUE             1
#define FALSE            0


typedef unsigned int hashNumberType;

typedef  int boolean;
typedef  struct noderec
{  
  struct noderec  *next;
  struct noderec  *back; 
  int              number;
  char             x;
}
  node, *nodeptr;



struct stringEnt
{
  int nodeNumber;
  char *word;
  struct stringEnt *next;
};


typedef struct stringEnt stringEntry;

typedef struct
{
  hashNumberType tableSize;
  stringEntry **table;
}
  stringHashtable;

typedef  struct  {
 
  stringHashtable  *nameHash; 
  node           **nodep;
  node            *start;
  int              mxtips;  
  int              ntips;
  int              detectedTips;
  int              nextnode;
 
  boolean          rooted;
  boolean          parseTree;
 
  char **nameList;   
} tree;


#define nmlngth        256


static boolean whitechar (int ch);
static int treeFinishCom (FILE *fp, char **strp);
static void  treeEchoContext (FILE *fp1, FILE *fp2, int n);

static stringHashtable *initStringHashTable(hashNumberType n)
{
  /* 
     init with primes 
  */
    
  static const hashNumberType initTable[] = {53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317,
					     196613, 393241, 786433, 1572869, 3145739, 6291469, 12582917, 25165843,
					     50331653, 100663319, 201326611, 402653189, 805306457, 1610612741};
 

  /* init with powers of two

  static const  hashNumberType initTable[] = {64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384,
					      32768, 65536, 131072, 262144, 524288, 1048576, 2097152,
					      4194304, 8388608, 16777216, 33554432, 67108864, 134217728,
					      268435456, 536870912, 1073741824, 2147483648U};
  */
  
  stringHashtable *h = (stringHashtable*)malloc(sizeof(stringHashtable));
  
  hashNumberType
    tableSize,
    i,
    primeTableLength = sizeof(initTable)/sizeof(initTable[0]),
    maxSize = (hashNumberType)-1;    

  assert(n <= maxSize);

  i = 0;

  while(initTable[i] < n && i < primeTableLength)
    i++;

  assert(i < primeTableLength);

  tableSize = initTable[i];  

  h->table = (stringEntry**)calloc(tableSize, sizeof(stringEntry*));
  h->tableSize = tableSize;    

  return h;
}


static hashNumberType  hashString(char *p, hashNumberType tableSize)
{
  hashNumberType h = 0;
  
  for(; *p; p++)
    h = 31 * h + *p;
  
  return (h % tableSize);
}

 

static void addword(char *s, stringHashtable *h, int nodeNumber)
{
  hashNumberType position = hashString(s, h->tableSize);
  stringEntry *p = h->table[position];
  
  for(; p!= NULL; p = p->next)
    {
      if(strcmp(s, p->word) == 0)		 
	return;	  	
    }

  p = (stringEntry *)malloc(sizeof(stringEntry));

  assert(p);
  
  p->nodeNumber = nodeNumber;
  p->word = (char *)malloc((strlen(s) + 1) * sizeof(char));

  strcpy(p->word, s);
  
  p->next =  h->table[position];
  
  h->table[position] = p;
}

static int lookupWord(char *s, stringHashtable *h)
{
  hashNumberType position = hashString(s, h->tableSize);
  stringEntry *p = h->table[position];
  
  for(; p!= NULL; p = p->next)
    {
      if(strcmp(s, p->word) == 0)		 
	return p->nodeNumber;	  	
    }

  return -1;
}



static int treeGetCh (FILE *fp)         /* get next nonblank, noncomment character */
{ /* treeGetCh */
  int  ch;

  while ((ch = getc(fp)) != EOF) {
    if (whitechar(ch)) ;
    else if (ch == '[') {                   /* comment; find its end */
      if ((ch = treeFinishCom(fp, (char **) NULL)) == EOF)  break;
    }
    else  break;
  }
 return  ch;
} /* treeGetCh */

static boolean treeNeedCh (FILE *fp, int c1, char *where)
{
  int  c2;
  
  if ((c2 = treeGetCh(fp)) == c1)  return TRUE;
  
  printf("ERROR: Expecting '%c' %s tree; found:", c1, where);
  if (c2 == EOF) 
    {
      printf("End-of-File");
    }
  else 
    {      	
      ungetc(c2, fp);
      treeEchoContext(fp, stdout, 40);
    }
  putchar('\n');

  if(c1 == ':')    
    printf("RAxML may be expecting to read a tree that contains branch lengths\n");

  return FALSE;
} 


static boolean whitechar (int ch)
{
  return (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r');
}

static int treeFinishCom (FILE *fp, char **strp)
{
  int  ch;
  
  while ((ch = getc(fp)) != EOF && ch != ']') {
    if (strp != NULL) *(*strp)++ = ch;    /* save character  */
    if (ch == '[') {                      /* nested comment; find its end */
      if ((ch = treeFinishCom(fp, strp)) == EOF)  break;
      if (strp != NULL) *(*strp)++ = ch;  /* save closing ]  */
    }
  }
  
  if (strp != NULL) **strp = '\0';        /* terminate string  */
  return  ch;
} /* treeFinishCom */

static void  treeEchoContext (FILE *fp1, FILE *fp2, int n)
{ /* treeEchoContext */
  int      ch;
  boolean  waswhite;
  
  waswhite = TRUE;
  
  while (n > 0 && ((ch = getc(fp1)) != EOF)) {
    if (whitechar(ch)) {
      ch = waswhite ? '\0' : ' ';
      waswhite = TRUE;
    }
    else {
      waswhite = FALSE;
    }
    
    if (ch > '\0') {putc(ch, fp2); n--;}
  }
} /* treeEchoContext */


static boolean treeLabelEnd (int ch)
{
  switch (ch) 
    {
    case EOF:  
    case '\0':  
    case '\t':  
    case '\n':  
    case '\r': 
    case ' ':
    case ':':  
    case ',':   
    case '(':   
    case ')':  
    case ';':
      return TRUE;
    default:
      break;
    }
  return FALSE;
} 


static boolean  treeGetLabel (FILE *fp, char *lblPtr, int maxlen)
{
  int      ch;
  boolean  done, quoted, lblfound;

  if (--maxlen < 0) 
    lblPtr = (char *) NULL; 
  else 
    if (lblPtr == NULL) 
      maxlen = 0;

  ch = getc(fp);
  done = treeLabelEnd(ch);

  lblfound = ! done;
  quoted = (ch == '\'');
  if (quoted && ! done) 
    {
      ch = getc(fp); 
      done = (ch == EOF);
    }

  while (! done) 
    {
      if (quoted) 
	{
	  if (ch == '\'') 
	    {
	      ch = getc(fp); 
	      if (ch != '\'') 
		break;
	    }
        }
      else 
	if (treeLabelEnd(ch)) break;     

      if (--maxlen >= 0) *lblPtr++ = ch;
      ch = getc(fp);
      if (ch == EOF) break;
    }

  if (ch != EOF)  (void) ungetc(ch, fp);

  if (lblPtr != NULL) *lblPtr = '\0';

  return lblfound;
}


static boolean  treeFlushLabel (FILE *fp)
{ 
  return  treeGetLabel(fp, (char *) NULL, (int) 0);
} 

static int treeFindTipByLabelString(char  *str, tree *tr)                    
{
  int lookup = lookupWord(str, tr->nameHash);

  if(lookup > 0)
    {
      assert(! tr->nodep[lookup]->back);
      return lookup;
    }
  else
    { 
      printf("ERROR: Cannot find tree species: %s\n", str);
      return  0;
    }
}

static int randomInt(int n)
{
  return rand() %n;
}


static int treeFindTipName(FILE *fp, tree *tr)
{
  char    str[nmlngth+2];
  int      n;

  if(treeGetLabel(fp, str, nmlngth+2))
    n = treeFindTipByLabelString(str, tr);
  else
    n = 0;
   

  return  n;
} 

static boolean treeProcessLength (FILE *fp, double *dptr)
{
  int  ch;
  
  if ((ch = treeGetCh(fp)) == EOF)  return FALSE;    /*  Skip comments */
  (void) ungetc(ch, fp);
  
  if (fscanf(fp, "%lf", dptr) != 1) {
    printf("ERROR: treeProcessLength: Problem reading branch length\n");
    treeEchoContext(fp, stdout, 40);
    printf("\n");
    return  FALSE;
  }
  
  return  TRUE;
}


static int treeFlushLen (FILE  *fp)
{
  double  dummy;  
  int     ch;
  
  ch = treeGetCh(fp);
  
  if (ch == ':') 
    {
      ch = treeGetCh(fp);
      
      ungetc(ch, fp);
      if(!treeProcessLength(fp, & dummy)) return 0;
      return 1;	  
    }
  
  
  
  if (ch != EOF) (void) ungetc(ch, fp);
  return 1;
} 


static void extractTaxaFromTopology(tree *tr, char *fileName)
{
  FILE *f = fopen(fileName, "rb");

  char 
    **nameList,
    buffer[nmlngth + 2]; 

  int
    i = 0,
    c,
    taxaSize = 1024,
    taxaCount = 0;
   
  nameList = (char**)malloc(sizeof(char*) * taxaSize);  

  while((c = fgetc(f)) != ';')
    {
      if(c == '(' || c == ',')
	{
	  c = fgetc(f);
	  if(c ==  '(' || c == ',')
	    ungetc(c, f);
	  else
	    {	      
	      i = 0;	      	     
	      
	      do
		{
		  buffer[i++] = c;
		  c = fgetc(f);
		}
	      while(c != ':' && c != ')' && c != ',');
	      buffer[i] = '\0';	    

	      for(i = 0; i < taxaCount; i++)
		{
		  if(strcmp(buffer, nameList[i]) == 0)
		    {
		      printf("A taxon labelled by %s appears twice in the first tree of tree collection %s, exiting ...\n", buffer, fileName);
		      exit(-1);
		    }
		}	     
	     
	      if(taxaCount == taxaSize)
		{		  
		  taxaSize *= 2;
		  nameList = (char **)realloc(nameList, sizeof(char*) * taxaSize);		 
		}
	      
	      nameList[taxaCount] = (char*)malloc(sizeof(char) * (strlen(buffer) + 1));
	      strcpy(nameList[taxaCount], buffer);
	     
	      taxaCount++;
			    
	      ungetc(c, f);
	    }
	}   
    }
  
  printf("\nFound a total of %d taxa in constraint tree %s\n", taxaCount, fileName);
 

  tr->detectedTips = taxaCount;

  tr->nameList = (char **)malloc(sizeof(char *) * (taxaCount + 1));  
  for(i = 1; i <= taxaCount; i++)
    tr->nameList[i] = nameList[i - 1];
  
  free(nameList);



  if(taxaCount < 4)
    {    
      printf("TOO FEW SPECIES, tree contains only %d species\n", taxaCount);
      assert(0);
    }

  tr->nameHash = initStringHashTable(10 * taxaCount);
  for(i = 1; i <= taxaCount; i++)
    addword(tr->nameList[i], tr->nameHash, i);

  fclose(f);
}


int *partA;
int partCount = 0;
char treeFileName[2048] = "";

static void hookupDefault (nodeptr p, nodeptr q)
{
  p->back = q;
  q->back = p;
}


static boolean  addElementLenMULT (FILE *fp, tree *tr, nodeptr p)
{ 
  nodeptr  q, r, s;
  int      n, ch, fres, rn;
  double randomResolution;
  int old;   

  if ((ch = treeGetCh(fp)) == '(') 
    {     
      partCount++;
      old = partCount;      

      partA[partCount] = partA[partCount] + 2;
      
      n = (tr->nextnode)++;
      if (n > 2*(tr->mxtips) - 2) 
	{
	  if (tr->rooted || n > 2*(tr->mxtips) - 1) 
	    {
	      printf("ERROR: Too many internal nodes.  Is tree rooted?\n");
	      printf("       Deepest splitting should be a trifurcation.\n");
	      return FALSE;
	    }
	  else 
	    {
	      tr->rooted = TRUE;	    
	    }
	}
      q = tr->nodep[n];
     
      if (! addElementLenMULT(fp, tr, q->next))        return FALSE;
      if (! treeNeedCh(fp, ',', "in"))             return FALSE;
      if (! addElementLenMULT(fp, tr, q->next->next))  return FALSE;
                 
      hookupDefault(p, q);

      while((ch = treeGetCh(fp)) == ',')
	{ 	 
	  partA[old] = partA[old] + 1; 

	  n = (tr->nextnode)++;
	  if (n > 2*(tr->mxtips) - 2) 
	    {
	      if (tr->rooted || n > 2*(tr->mxtips) - 1) 
		{
		  printf("ERROR: Too many internal nodes.  Is tree rooted?\n");
		  printf("       Deepest splitting should be a trifurcation.\n");
		  return FALSE;
		}
	      else 
		{
		  tr->rooted = TRUE;
		}
	    }
	  r = tr->nodep[n];
	 

	  rn = randomInt(10000);
	  if(rn == 0) 
	    randomResolution = 0;
	  else 
	    randomResolution = ((double)rn)/10000.0;
	   	  
	   if(randomResolution < 0.5)
	    {	    
	      s = q->next->back;	      
	      r->back = q->next;
	      q->next->back = r;	      
	      r->next->back = s;
	      s->back = r->next;	      
	      addElementLenMULT(fp, tr, r->next->next);	     
	    }
	  else
	    {	  
	      s = q->next->next->back;	      
	      r->back = q->next->next;
	      q->next->next->back = r;	      
	      r->next->back = s;
	      s->back = r->next;	      
	      addElementLenMULT(fp, tr, r->next->next);	     
	    }	    	  	  
	}            

      if(ch != ')')
	{
	  printf("Missing /) in treeReadLenMULT\n");
	  exit(-1);	        
	}
	


      (void) treeFlushLabel(fp);
    }
  else 
    {                             
      ungetc(ch, fp);
      if ((n = treeFindTipName(fp, tr)) <= 0)          return FALSE;
      q = tr->nodep[n];         

      if (tr->start->number > n)  tr->start = q;
      (tr->ntips)++;
      hookupDefault(p, q);
    }
  
  fres = treeFlushLen(fp);
  if(!fres) return FALSE;
    
  return TRUE;          
} 


static size_t numTrees(int taxa)
{
  size_t 
    i,
    t = (size_t)taxa,
    p = 1;

  for(i = 3; i <= t; i++)
    p = p * (2 * i - 3);

  return p;

}


static boolean treeReadLenMULT (FILE *fp, tree *tr)
{
  nodeptr  p, r, s;
  int      i, ch, n, rn;
  double randomResolution;

  srand((unsigned int) time(NULL));
  
  partA = (int*)calloc(tr->mxtips, sizeof(int));

  for (i = 1; i <= tr->mxtips; i++) 
    tr->nodep[i]->back = (node *) NULL;

  for(i = tr->mxtips + 1; i < 2 * tr->mxtips; i++)
    {
      tr->nodep[i]->back = (nodeptr)NULL;
      tr->nodep[i]->next->back = (nodeptr)NULL;
      tr->nodep[i]->next->next->back = (nodeptr)NULL;
      tr->nodep[i]->number = i;
      tr->nodep[i]->next->number = i;
      tr->nodep[i]->next->next->number = i;
    }


  tr->start       = tr->nodep[tr->mxtips];
  tr->ntips       = 0;
  tr->nextnode    = tr->mxtips + 1;  

  tr->rooted      = FALSE;
 
  p = tr->nodep[(tr->nextnode)++]; 
  while((ch = treeGetCh(fp)) != '(');
      
  if (! addElementLenMULT(fp, tr, p))                 return FALSE;
  if (! treeNeedCh(fp, ',', "in"))                return FALSE;
  if (! addElementLenMULT(fp, tr, p->next))           return FALSE;
  if (! tr->rooted) 
    {
      if ((ch = treeGetCh(fp)) == ',') 
	{       
	  if (! addElementLenMULT(fp, tr, p->next->next)) return FALSE;

	  while((ch = treeGetCh(fp)) == ',')
	    { 
	      n = (tr->nextnode)++;
	      assert(n <= 2*(tr->mxtips) - 2);
	
	      r = tr->nodep[n];	
	     	   
	      
	      rn = randomInt(10000);
	      if(rn == 0) 
		randomResolution = 0;
	      else 
		randomResolution = ((double)rn)/10000.0;

	      printf("TOP-LEVEL MULTI\n");
	      assert(0);


	      if(randomResolution < 0.5)
		{	
		  s = p->next->next->back;		  
		  r->back = p->next->next;
		  p->next->next->back = r;		  
		  r->next->back = s;
		  s->back = r->next;		  
		  addElementLenMULT(fp, tr, r->next->next);	
		}
	      else
		{
		  s = p->next->back;		  
		  r->back = p->next;
		  p->next->back = r;		  
		  r->next->back = s;
		  s->back = r->next;		  
		  addElementLenMULT(fp, tr, r->next->next);
		}
	    }	  	  	      	  

	  if(ch != ')')
	    {
	      printf("Missing /) in treeReadLenMULT\n");
	      exit(-1);	        	      	      
	    }
	  else
	    ungetc(ch, fp);
	}
      else 
	{ 
	  tr->rooted = TRUE;
	  if (ch != EOF)  (void) ungetc(ch, fp);
	}       
    }
  else 
    {
      p->next->next->back = (nodeptr) NULL;
    }
    
  if (! treeNeedCh(fp, ')', "in"))                return FALSE;
  (void) treeFlushLabel(fp);
  if (! treeFlushLen(fp))                         return FALSE;
   
  if (! treeNeedCh(fp, ';', "at end of"))       return FALSE;
  

  assert(tr->ntips == tr->mxtips);
     
  {
    mpz_t 
      integ,
      treeNum;

    int 
      n,
      max = 0;
    
    char 
      *b = (char*)NULL;
    
    mpz_init(integ);
    mpz_init(treeNum);

    mpz_set_ui(integ, 1);

    for(i = 0; i < tr->mxtips; i++)
      {
	if(partA[i] > 2)
	  {
	    /*printf("K %d %Zu\n", partA[i], numTrees(partA[i]));*/

	    if(partA[i] > max)	      
	      max = partA[i];	      	     
	
	    mpz_set_ui(treeNum, (unsigned long int)(numTrees(partA[i])));

	    mpz_mul(integ, integ, treeNum);	     	   
	  }
		       

	
      }
   
      
    printf("\n\nMaximum size unresolved multifurcation has %d taxa\n\n", max);

    b = mpz_get_str (b, 10, integ);
      
    printf("Number of unrooted binary trees under this constraint: %s\n\n", b);
    
    n = strlen(b);

    if(n > 3)            
      printf("Approximately %c.%c%c times 10^%d\n\n\n", b[0], b[1], b[2], n - 1);   

    free(b);       
  }
 
  return TRUE; 
}


static int mygetopt(int argc, char **argv, char *opts, int *optind, char **optarg)
{
  static int sp = 1;
  register int c;
  register char *cp;

  if(sp == 1)
    {
      if(*optind >= argc || argv[*optind][0] != '-' || argv[*optind][1] == '\0')
	return -1;
    }
  else
    {
      if(strcmp(argv[*optind], "--") == 0)
	{
	  *optind =  *optind + 1;
	  return -1;
	}
    }

  c = argv[*optind][sp];
  if(c == ':' || (cp=strchr(opts, c)) == 0)
    {
      printf(": illegal option -- %c \n", c);
      if(argv[*optind][++sp] == '\0')
	{
	  *optind =  *optind + 1;
	  sp = 1;
	}
      return('?');
    }
  if(*++cp == ':')
    {
      if(argv[*optind][sp+1] != '\0')
	{
	  *optarg = &argv[*optind][sp+1];
	  *optind =  *optind + 1;
	}
      else
	{
	  *optind =  *optind + 1;
	  if(*optind >= argc)
	    {
	      printf(": option requires an argument -- %c\n", c);
	      sp = 1;
	      return('?');
	    }
	  else
	    {
	      *optarg = argv[*optind];
	      *optind =  *optind + 1;
	    }
	}
      sp = 1;
    }
  else
    {
      if(argv[*optind][++sp] == '\0')
	{
	  sp = 1;
	  *optind =  *optind + 1;
	}
      *optarg = 0;
    }
  return(c);
  }


static void printHelp(void)
{
  printf("\n\nThis is the tree counter program released in June 2011 by Alexandros Stamatakis\n");
  printf("it can either compute the number of possible bifurcating trees for n taxa via the\n\n");
  printf(" -n numberOfTaxa\n\n");
  printf("option or .... \n\n");
  printf("Compute the number of possible bifurcating trees for a multi-furcating \n");
  printf("Newick constraint tree passed via the \n\n");
  printf(" -t constraintTreeFileName\n\n");
  printf("option. The constraint tree format must be RAxML readable\n");
  printf("\n\n");
}

static void get_args(int argc, char *argv[], tree *tr)
{
  boolean
    bad_opt    =FALSE;

  char          
    *optarg;
    
  int  
    optind = 1,        
    c;

  boolean 
    numSet = FALSE,
    constraintSet = FALSE;

  tr->mxtips = 0;
  tr->ntips = 0;
  tr->detectedTips = 0;
  tr->rooted = FALSE;
  tr->parseTree = FALSE;
  tr->nextnode = 0;
  
  /*treeFileName = "";*/
 
  
  /********* tr inits end*************/


  while(!bad_opt &&
	((c = mygetopt(argc,argv,"n:t:h", &optind, &optarg))!=-1))
    {
    switch(c)
      {
      case 'n':
	sscanf(optarg,"%d", &(tr->mxtips));
	if(tr->mxtips <= 2)
	  {
	    printf("A tree with less than 3 taxa?\n");	   
	    exit(-1);
	  }
	numSet = TRUE;
	break;      
      case 't':
	tr->parseTree = TRUE;
	strcpy(treeFileName, optarg);
	constraintSet = TRUE;
	break;           
      case 'h':
	printHelp();
	exit(0);
	break;
      default:
	printf("Option %c not supported\n", c);
    }
  }

 
  if((numSet && constraintSet) || (!(numSet || constraintSet)))
    {
      printf("Usage error you need to either specify a constraint via -t\n");
      printf("or the number of taxa via -n \n");
      exit(-1);
    }

  return;
}


static void computeNumberOfTrees(tree *tr)
{
   mpz_t 
      integ,
      treeNum;  
   
   int 
     n,
     i;
   
   char 
     *b = (char*)NULL,
     *c = (char*)NULL;
    
   mpz_init(integ);
   mpz_init(treeNum);

   mpz_set_ui(integ, 1);

   for(i = 3; i <= tr->mxtips; i++)
     {
       mpz_set_ui(treeNum, (unsigned long int)(2 * i - 5));
       mpz_mul(integ, integ, treeNum);
     }
   
   b = mpz_get_str (b, 10, integ);
	       
   printf("Number of unrooted binary trees for %d taxa: %s\n\n", tr->mxtips, b);  

   n = strlen(b);

   if(n > 3)            
     printf("Approximately %c.%c%c times 10^%d\n\n\n", b[0], b[1], b[2], n - 1);   

   free(b);           

   
   mpz_set_ui(treeNum, (unsigned long int)(2 * (tr->mxtips + 1) - 5));
   mpz_mul(integ, integ, treeNum);

   c = mpz_get_str (c, 10, integ);
	       
   printf("Number of rooted binary trees for %d taxa: %s\n\n", tr->mxtips, c); 

   n = strlen(c);

   if(n > 3)            
     printf("Approximately %c.%c%c times 10^%d\n\n\n", c[0], c[1], c[2], n - 1);  

   free(c);   

}


int main (int argc, char *argv[])
{
  tree         
    *tr = (tree *)malloc(sizeof(tree));

  get_args(argc,argv, tr); 

  printf("\n\nGNU GPL tree number calculator released June 2011 by Alexandros Stamatakis\n\n");

 
  

  if(tr->parseTree)
    {
      nodeptr 
	p0,
	p,
	q;
      
      int
	j,
	i,
	tips,
	inter;

      FILE *f = fopen(treeFileName, "rb");

      extractTaxaFromTopology(tr, treeFileName);
      
      tr->mxtips = tr->detectedTips;
      
      tips  = tr->mxtips;
      inter = tr->mxtips - 1;
 
      if (!(p0 = (nodeptr) malloc((tips + 3 * inter) * sizeof(node))))
	{
	  printf("ERROR: Unable to obtain sufficient tree memory\n");
	  return 0;
	}

      if (!(tr->nodep = (nodeptr *) malloc((2 * tr->mxtips) * sizeof(nodeptr))))
	{
	  printf("ERROR: Unable to obtain sufficient tree memory, too\n");
	  return  0;
	}

      tr->nodep[0] = (node *) NULL;

      for (i = 1; i <= tips; i++)
	{
	  p = p0++;     
	  p->x      =  0;
	  p->number =  i;
	  p->next   =  p;
	  p->back   = (node *)NULL;	  
	  tr->nodep[i] = p;
	}

      for (i = tips + 1; i <= tips + inter; i++)
	{
	  q = (node *) NULL;
	  for (j = 1; j <= 3; j++)
	    {	 
	      p = p0++;
	      if(j == 1)
		p->x = 1;
	      else
		p->x =  0;
	      p->number = i;
	      p->next   = q;	  
	      p->back   = (node *) NULL;	  
	      q = p;
	    }
	  p->next->next->next = p;
	  tr->nodep[i] = p;
	}

      tr->start       = (node *) NULL;

      treeReadLenMULT(f, tr);

      fclose(f);
	
    }
  else
    computeNumberOfTrees(tr);

  return 0;
}
