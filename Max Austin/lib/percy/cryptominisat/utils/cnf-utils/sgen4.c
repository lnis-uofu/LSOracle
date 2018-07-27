/*
sgen4 - a generator for satisfiability instances.

Copyright 2010, 2012 Ivor Spence, Allen van Gelder

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// Define command-line options

#define SAT 0
#define UNSAT 1
#define REORDER 2
#define HELP 3
#define MIN_VARIABLES 4
#define HELP_SHORT 5
#define MAX_BOOLEAN_OPTIONS 6
int *booleanOptions;


#define MODEL 0
#define MAX_STRING_OPTIONS 1
char **stringOptions;

#define NUM_VARIABLES 0
#define RANDOM_SEED 1
#define MAX_INT_OPTIONS 2

#define MAX_CHUMS 200

typedef struct ClauseStruct
{
    int numOfVariables;
    int variables[5];
} Clause;

Clause *clauseArray;
int clauseCount;
int clauses;
int globall=0;

int *intOptions;

int *model;
int numOfVariables, isSat, groups, reorder;
int groupSize;
int *perm1,*uperm1,*perm2,*uperm2;


typedef struct ChumsStruct
{
	int n;
	int v[MAX_CHUMS];
} Chums, *ChumsPtr;

int inChums (ChumsPtr c, int v)
{
	int result=0;
	int i;

	for (i=0;!result && i<c->n; i++)
		result = c->v[i] == v;

	return result;
}

void addChum (ChumsPtr c, int v)
{
	if (!inChums (c,v))
	{
		if (c->n == MAX_CHUMS)
		{
			fprintf (stderr, "Chums limit exceeded\n");
			exit(0);
		}
		else
			c->v[c->n++] = v;
	}
}

int *permute, *unpermute, *globalPermute, *permute1, *unpermute1, *idpermute;


Chums *chums1, *chums2, *chums3;

void printChums (ChumsPtr c)
{
	int i;

	printf ("{");
	for (i=0;i<c->n;i++)
	{
		printf ("%d", c->v[i]);
		if (i != c->n-1)
			printf (", ");
	}
	printf ("}");
}
void initChums()
{
	int i, v,g,v1,v2,x,y;
	int uv, uv1;

	chums1 = (Chums*) malloc ((1+numOfVariables)*sizeof(Chums));
	chums2 = (Chums*) malloc ((1+numOfVariables)*sizeof(Chums));
	chums3 = (Chums*) malloc ((1+numOfVariables)*sizeof(Chums));

	for (v=1;v<=numOfVariables;v++)
	{
		chums1[v].n = 0;
		chums2[v].n = 0;
		chums3[v].n = 0;
	}

	for (v=1;v<=numOfVariables;v++)
	{
		g = (v-1) / groupSize;
		for (v1=1; v1<=groupSize; v1++)
		{
			v2 = g*groupSize+v1;
			if (v != v2)
				addChum (&chums1[v], v2);
		}

	}

	for (v=1;v<=numOfVariables;v++)
	{
		uv = permute1[v];
		g = (v-1) / groupSize;
		for (uv1=1; uv1<=groupSize; uv1++)
		{
			v2 = permute1[g*groupSize+uv1];
			if (uv != v2)
				addChum (&chums1[uv], v2);
		}
	}

	for (x=1;x<=numOfVariables;x++)
	{
		for (i=0;i<chums1[x].n;i++)
		{
			v = chums1[x].v[i];
			for (y=0;y<chums1[v].n;y++)
				if (x != chums1[v].v[y])
					addChum(&chums2[x], chums1[v].v[y]);
		}
	}

	for (x=1;x<=numOfVariables;x++)
	{
		for (i=0;i<chums2[x].n;i++)
		{
			v = chums2[x].v[i];
			for (y=0;y<chums1[v].n;y++)
				if (x != chums1[v].v[y])
					addChum(&chums3[x], chums1[v].v[y]);
		}
	}


}

#define MYRAND_MAX 32767

char  **booleanOptionTags, **stringOptionTags, **intOptionTags;
char commandLine[1000];

void initOptions()
{
    booleanOptionTags = (char **) malloc (MAX_BOOLEAN_OPTIONS*sizeof (char *));
    booleanOptions = (int *) malloc (MAX_BOOLEAN_OPTIONS*sizeof(int));

    booleanOptionTags[SAT] = "sat";
    booleanOptions[SAT] = 0;
    booleanOptionTags[UNSAT] = "unsat";
    booleanOptions[UNSAT] = 0;
    booleanOptionTags[REORDER] = "reorder";
    booleanOptions[REORDER] = 1;
    booleanOptionTags[HELP] = "help";
    booleanOptions[HELP] = 0;
    booleanOptionTags[HELP_SHORT] = "h";
    booleanOptions[HELP_SHORT] = 0;
    booleanOptionTags[MIN_VARIABLES] = "min-variables";
    booleanOptions[MIN_VARIABLES] = 0;

    intOptionTags = (char **) malloc (MAX_INT_OPTIONS*sizeof (char *));
    intOptions = (int *) malloc (MAX_INT_OPTIONS*sizeof(int));

    intOptionTags[NUM_VARIABLES] = "n";
    intOptions[NUM_VARIABLES] = 0;
    intOptionTags[RANDOM_SEED] = "s";
    intOptions[RANDOM_SEED] = 1;

    stringOptionTags = (char **) malloc (MAX_STRING_OPTIONS*sizeof (char *));
    stringOptions = (char **) malloc (MAX_STRING_OPTIONS*sizeof(char *));

    stringOptionTags[MODEL] = "m";
    stringOptions[MODEL] = NULL;

}

int checkBooleanOption (char *option)
{
    int o;

    for (o=0;o<MAX_BOOLEAN_OPTIONS;o++)
    {
        if (strcmp (booleanOptionTags[o], &option[1]) == 0)
        {
            booleanOptions[o] = 1;
            return 1;
        }
    }

    if (strlen (option) > 3 && option[1] == 'n' && option[2] == 'o')
        for (o=0;o<MAX_BOOLEAN_OPTIONS;o++)
        {
            if (strcmp (booleanOptionTags[o], &option[3]) == 0)
            {
                booleanOptions[o] = 0;
                return 1;
            }
        }
    return 0;
}

int checkStringOption (char *option, char *val)
{
    int o;

    for (o=0;o<MAX_STRING_OPTIONS;o++)
    {
        if (strcmp (stringOptionTags[o], &option[1]) == 0)
        {
            stringOptions[o] = val;
            return 1;
        }
    }

    return 0;
}

int checkIntOption (char *option, char *val)
{
    int o;

    for (o=0;o<MAX_INT_OPTIONS;o++)
    {
        if (strcmp (intOptionTags[o], &option[1]) == 0)
        {
            intOptions[o] = atoi (val);
            return 1;
        }
    }

    return 0;
}

void printUsage()
{
    fprintf (stderr, "Usage: sgen4 -n num-of-variables [-sat | -unsat] [-s random-seed] [-m satisfying-model-file] [-min-variables] [-[no]reorder]\n");
    exit(0);
}

void printHelp()
{
    int i;
    printf("Usage:");

    for (i=0;i<MAX_BOOLEAN_OPTIONS;i++)
        fprintf (stdout, "  -[no]%s\n", booleanOptionTags[i]);
    fprintf (stdout, "\n");

    for (i=0;i<MAX_INT_OPTIONS;i++)
        fprintf (stdout, "  -%s int\n", intOptionTags[i]);
    fprintf (stdout, "\n");

    for (i=0;i<MAX_STRING_OPTIONS;i++)
        fprintf (stdout, "  -%s string\n", stringOptionTags[i]);
    fprintf (stdout, "\n");

}

void getOptions (int *argc, char * argv[])
{
    char **newArgv;
    int oldp, newp, p, pos;

    strcpy (commandLine, "Command: ");
    pos=strlen(commandLine);
    for (p=0; p<*argc && pos+1+strlen(argv[p])<1000;p++)
    {
        strcat (commandLine, argv[p]);
        strcat (commandLine, " ");
        pos += 1 + strlen (argv[p]);
    }


    initOptions();

    newArgv = (char **) malloc ( (*argc)*sizeof(char *));

    oldp = 0;
    newp = 0;

    newArgv[newp++] = argv[oldp++];

    while (oldp < *argc)
    {
        if (strlen (argv[oldp]) == 0 || argv[oldp][0] != '-')
            newArgv[newp++] = argv[oldp++];
        else
        {
            if (checkBooleanOption (argv[oldp]))
                oldp++;
            else
                if (oldp+1 < *argc && checkStringOption (argv[oldp],argv[oldp+1]))
                    oldp += 2;
            else
                if (oldp+1 < *argc && checkIntOption (argv[oldp],argv[oldp+1]))
                    oldp += 2;
            else
            {
                fprintf (stderr, "Unrecognised option %s\n", argv[oldp]);
                oldp++;
            }
        }

    }

    for (p=0;p<newp;p++)
        argv[p] = newArgv[p];
    *argc = newp;

    free (newArgv);

    if (booleanOptions[HELP] || booleanOptions[HELP_SHORT])
    {
        printHelp();
        exit(1);
    }


}

void checkOptions()
{
    if ( (booleanOptions[SAT] && booleanOptions[UNSAT])
        || (!booleanOptions[SAT] && !booleanOptions[UNSAT])
    )
    {
        fprintf (stderr, "You must specify either -sat or -unsat\n");
        printUsage();
    }

    if (intOptions[NUM_VARIABLES] <= 0)
    {
        fprintf (stderr, "You must specify num-of-variables to be at least 1\n");
        printUsage();
    }

    if (booleanOptions[UNSAT] && stringOptions[MODEL] != NULL)
    {
        fprintf (stderr, "You have specified -unsat and -m, but no model can be created for an unsatisfiable instance\n");
        printUsage();
    }
}

static unsigned long next = 1;

int myrand(void)
{
    next = next * 1103515245 + 12345;
    return((unsigned)(next/65536) % 32768);
}

void mysrand(unsigned seed)
{
    next = seed;
}






void heading(int n)
{

    if (isSat)
                clauses = groups*12;
    else
                clauses = 2*(4*groups+6);


    printf ("c %s\n", commandLine);
    printf ("p cnf %d %d\n", n, clauses);

    clauseArray = (Clause *) malloc (clauses*sizeof(Clause));

}


void setPermute(int sign)
{
    int i,j,k,l;

    for (i=1;i<=numOfVariables;i++)
        permute[i] = 0;

    for (i=1;i<=numOfVariables;i++)
    {
        j = myrand() % (numOfVariables+1-i) + 1;
        k = 1;
        for (l=0;l<j;l++)
        {
            if (k == numOfVariables)
                k = 1;
            else
                k++;
            while (permute[k] != 0)
            {
                if (k == numOfVariables)
                    k = 1;
                else
                    k++;
            }
        }
        permute[k] = sign*i;
    }


}


void setPlainPermute ()
{
    int i;
    for (i=1;i<=numOfVariables;i++)
        permute[i] = i;
}

int beatProbability (double power)
{
    int result;
    double p;

    if (power > 0.0)
        result = 1;
    else
    {
        p = exp (power);
        if (p >= 1)
            result = 1;
        else
        {
            result = myrand() < p*MYRAND_MAX;
        }
    }
    return result;
}

int groupFor (int p1)
{
    if (!isSat && p1 == numOfVariables)
        return (p1-2) / groupSize;
    else
        return (p1-1)/groupSize;
}



int absolute (int i)
{
    return i>0?i:-i;
}

int neighbours (int p1, int pp1)
{
    int result = 0;
    int i,j, k, g1, gp1, p2, g2, p3, pp3,g3;



    if (globall == 0)
/*
	{
		g1 = groupFor (perm1[p1]);

		gp1 = groupFor (pp1);
		for (i=1;i<=groupSize;i++)
		{
			if (pp1 != groupSize*gp1+i)
			{
				p2 = perm2[groupSize*gp1+i];
				g2 = groupFor (uperm1[p2]);
				if (g2 == g1)
				{
					 result+=100;
				}

				for (j=1; j<=groupSize; j++)
				{
					p3 = perm1[groupSize*g2+j];
					if (p2 != p3)
					{
						pp3 = uperm2[p3];
						g3 = groupFor(pp3);
						for (k=1;k<=groupSize;k++)
							if (groupSize*g3+k != pp3 && groupFor(uperm1[perm2[groupSize*g3+k]]) == g1)
								result += 1;
					}
				}
			}
		}
	}
*/
	{
		g1 = groupFor (perm1[p1]);

		gp1 = groupFor (pp1);
		for (i=1;i<=groupSize;i++)
		{
			if (p1 != groupSize*g1+i)
			{
				p2 = perm2[groupSize*g1+i];
				g2 = groupFor (uperm1[p2]);
				if (g2 == gp1)
				{
					 result+=100;
				}

				for (j=1; j<=groupSize; j++)
				{
					p3 = perm1[groupSize*g2+j];
					if (p2 != p3)
					{
						pp3 = uperm2[p3];
						g3 = groupFor(pp3);
						for (k=1;k<=groupSize;k++)
							if (groupSize*g3+k != pp3 && groupFor(uperm1[perm2[groupSize*g3+k]]) == gp1)
								result += 1;
					}
				}
			}
		}
	}
	else
	{
		g1 = groupFor (p1);

		gp1 = groupFor (pp1);
 		for (i=1;i<=groupSize;i++)
		{
			if (p1 != groupSize*g1+i)
			{
				p2 = perm2[groupSize*g1+i];
				if (inChums (&chums1[pp1], p2)) result += 50;
				if (inChums (&chums2[pp1], p2)) result += 10;
				if (inChums (&chums3[pp1], p2)) result += 1;

			}
		}
	}




    if (!isSat && g1 == (numOfVariables-1)/groupSize && p1 != numOfVariables)
        if (groupFor (uperm2[numOfVariables]) == g1)
            result+=16;


    return result;
}

double swapGain (int p1, int p2)
{

    double result;

    uperm1 = idpermute;
    perm1 = idpermute;
    perm2 = permute;
    uperm2 = unpermute;

     result = (double) (neighbours (p1, permute[p1]) + neighbours (p2, permute[p2])) -
        (neighbours (p1, permute[p2]) + neighbours (p2, permute[p1]));


    return result;
}

void setShiftPermute (int s)
{
	int v;
	int ss[6];
	ss[0] = 0;
	for (v=1;v<=5;v++)
		ss[v] = myrand()%200;



	for (v=1;v<=numOfVariables;v++)
		permute[v] = 1 + (v-1+groupSize*ss[(v-1)%groupSize] )%numOfVariables;

	for (v=1;v<=numOfVariables;v++)
		unpermute[permute[v]] = v;

}

void setAnnealingPermute ()
{
    int i, p1, p2, t;
    double temperature;
    double gain;

    for (i=1;i<=numOfVariables;i++)
    {
        permute[i] = i;
        unpermute[i] = i;
    }
    uperm1 = idpermute;
    perm1 = idpermute;
    perm2 = permute;
    uperm2 = unpermute;

    temperature = 20;

    while (temperature > 0.2)
    {
        for (i=1;i<=4*numOfVariables;i++)
        {
            p1 = 1 + myrand() % numOfVariables;
            p2 = 1 + myrand() % numOfVariables;
            gain = swapGain (p1, p2);
            if (beatProbability (gain/temperature))
            {
                t = permute[p1];
                permute[p1] = permute[p2];
                permute[p2] = t;
                unpermute[permute[p1]] = p1;
                unpermute[permute[p2]] = p2;

            }
       }
        temperature *= 0.997;
     }

}


void setAnnealingPermute1 (int l)
{
    int i, p1, p2, t;
    double temperature;
    double gain;

    globall = l;

    for (i=1;i<=numOfVariables;i++)
    {
        permute[i] = i;
        unpermute[i] = i;
    }


    temperature = 50;

    while (temperature > 0.2)
    {
        for (i=1;i<=2*numOfVariables;i++)
        {
            do
            {
                p1 = 1 + myrand() % numOfVariables;
            } while (model[p1]== 1);

            do
            {
                p2 = 1 + myrand() % numOfVariables;
            } while (model[p2] == 1);
            gain = swapGain (p1, p2);
            if (beatProbability (gain/temperature))
            {
               t = permute[p1];
                permute[p1] = permute[p2];
                permute[p2] = t;
                unpermute[permute[p1]] = p1;
                unpermute[permute[p2]] = p2;

            }
        }
        fflush (stdout);
        temperature *= 0.997;
     }



}

void set2Clause (Clause *c, int v1, int v2)
{
    c->numOfVariables = 2;
    c->variables[0] = v1;
    c->variables[1] = v2;
}

void set3Clause (Clause *c, int v1, int v2, int v3)
{
    c->numOfVariables = 3;
    c->variables[0] = v1;
    c->variables[1] = v2;
    c->variables[2] = v3;
}

void set4Clause (Clause *c, int v1, int v2, int v3, int v4)
{
    c->numOfVariables = 3;
    c->variables[0] = v1;
    c->variables[1] = v2;
    c->variables[2] = v3;
    c->variables[3] = v4;
}

void set5Clause (Clause *c, int v1, int v2, int v3, int v4, int v5)
{
    c->numOfVariables = 3;
    c->variables[0] = v1;
    c->variables[1] = v2;
    c->variables[2] = v3;
    c->variables[3] = v4;
    c->variables[4] = v5;
}

void lastUnsatGroup(int sign)
{
    int i,j,k;
    int start = numOfVariables-5;

    for (i=1;i<=3;i++)
        for (j=i+1;j<=4;j++)
            for (k=j+1;k<=5;k++)
                set3Clause (&(clauseArray[clauseCount++]), sign*permute[i+start], sign*permute[j+start], sign*permute[k+start]);
}



void mainUnsatGroup (int g, int sign)
{
    int i,j, k;

        for (i=1;i<=groupSize;i++)
        {
            clauseArray[clauseCount].numOfVariables = groupSize-1;
            k = 0;
            for (j=1;j<=groupSize;j++)
                if (i != j) clauseArray[clauseCount].variables[k++] = sign*permute[groupSize*g+j];
            clauseCount++;
        }


}



void firstSatGroup (int g, int sign)
{
    int i,j;

    for (i=1;i<groupSize;i++)
        for (j=i+1;j<=groupSize;j++)
            set2Clause (&(clauseArray[clauseCount++]), sign*permute[g*groupSize+i], sign*permute[g*groupSize+j]);

}



void swap (int i, int j)
{
    int temp;

    temp = permute[i];
    permute[i] = permute[j];
    permute[j] = temp;
}


void secondSatGroup(int l, int sign)
{
    int g,h;

    for (g=0;g<groups;g++)
    {

        clauseArray[clauseCount].numOfVariables = groupSize;
        for (h=1;h<=groupSize;h++)
            clauseArray[clauseCount].variables[h-1] = sign*permute[g*groupSize+h];
        clauseCount++;
    }

}

int unPermuteVar (int v)
{
    if (v > 0)
        return globalPermute[v];
    else
        return -globalPermute[-v];
}

void initModel(int n)
{
    int v,g;

    model = (int *) malloc ((n+1)*sizeof(int));
    for (v=1; v<=n; v++)
        model[v] = -1;

    for (g=0;g<groups;g++)
    {
        v = g*groupSize + 1 + (myrand()%groupSize);
        model[v] = 1;
    }


}

void randomizeArray (int numOfElements, int *array)
{
    int i, p1,p2,temp;

    for (i=0;i<numOfElements*numOfElements;i++)
    {
        p1 = myrand() % numOfElements;
        p2 = myrand() % numOfElements;
        temp = array[p1];
        array[p1] = array[p2];
        array[p2] = temp;
    }
}

int *getIdentityFromZero (int numOfElements)
{
    int *result,i;

    result = (int *) malloc (numOfElements*sizeof(int));

    for (i=0; i<numOfElements; i++)
        result[i] = i;

    return result;
}

int *getRandomFromZero (int numOfElements)
{
    int *result;

    result = getIdentityFromZero (numOfElements);
    randomizeArray (numOfElements, result);

    return result;
}

int *getRandomFromOne (int numOfElements)
{
    int *result;

    result = getIdentityFromZero (numOfElements+1);
    randomizeArray (numOfElements, result+1);

    return result;
}

void initGlobalPermute()
{
    int v;

    if (booleanOptions[REORDER])
    {
        globalPermute = getRandomFromOne (numOfVariables);
        for (v=1;v<=numOfVariables;v++)
            if (myrand() % 2 ==1 )
                globalPermute[v] = -globalPermute[v];
    }
    else
        globalPermute = getIdentityFromZero (numOfVariables+1);

}

void printModel (char *filename)
{
    FILE *f;
    int v, varsPerLine;

    f = fopen (filename, "w");
    if (f != NULL)
    {
        fprintf (f, "c Satisfiability model produced by sgen4\n");
        fprintf (f, "c %s\n", commandLine);
        fprintf (f, "s SATISFIABLE\n");

        varsPerLine = 20;

        for (v=1;v<=numOfVariables;v++)
        {
            if ( v % varsPerLine == 1)
                fprintf (f, "v ");
            fprintf (f, "%d ", unPermuteVar(v)*model[v]);
            if (v % varsPerLine == 0)
                fprintf (f, "\n");
        }
        if (numOfVariables % varsPerLine == 0)
            fprintf (f, "v ");
        fprintf (f, "0 \n");
        fclose (f);
    }
    else
    {
        fprintf (stderr, "Unable to write model to file <%s>\n", filename);
        exit(0);
    }

}

void minVariables ()
{
    int allClauses,numOfClauses, v,c, sign, deletedClause;

     initModel(numOfVariables);
     permute = (int *) malloc ( (numOfVariables+1) * sizeof(int) );
     unpermute = (int *) malloc ( (numOfVariables+1) * sizeof(int) );
     groupSize = 5;
     initGlobalPermute();
     setPlainPermute();
    allClauses =1;
    for (v=1;v<=numOfVariables;v++)
        allClauses *=2;
    if (isSat)
    {
        numOfClauses = allClauses - 1;
        deletedClause = (myrand() % numOfClauses);
        sign = 1;
        for (v=1;v<=numOfVariables;v++)
        {
            if (sign & deletedClause)
                model[v] = -1;
            else
                model[v] = 1;

            sign *= 2;
        }

    }
    else
    {
        numOfClauses = allClauses;
        deletedClause = -1;
    }
    printf ("p cnf %d %d\n", numOfVariables, numOfClauses);
    for (c=0;c<allClauses;c++)
        if (c != deletedClause)
        {
            sign = 1;
            for (v=1;v<=numOfVariables;v++)
            {
                if (sign & c)
                    printf ("%d ",globalPermute[v]);
                else
                   printf ("%d ",-globalPermute[v]);

                sign *= 2;
            }
            printf ("0\n");
        }

    if (stringOptions[MODEL] != NULL)
        printModel (stringOptions[MODEL]);



}

void printClause (Clause *c)
{
    int i;
    int *varOrder;

    if (booleanOptions[REORDER])
        varOrder = getRandomFromZero (c->numOfVariables);
    else
        varOrder = getIdentityFromZero (c->numOfVariables);

    for (i=0; i<c->numOfVariables; i++)
        printf ("%d ", unPermuteVar(c->variables[varOrder[i]]));
    printf ("0\n");

    free (varOrder);
}

void printClauseArray()
{
    int i;

    int *clauseOrder;

    if (booleanOptions[REORDER])
        clauseOrder = getRandomFromZero (clauses);
    else
        clauseOrder = getIdentityFromZero (clauses);

    for (i=0;i<clauses;i++)
        printClause (&clauseArray[clauseOrder[i]]);

    free (clauseOrder);
}
void minLiterals ()
{
    int g,l,v;

    if (isSat)
    {
        groupSize = 5;
        numOfVariables = ((numOfVariables-1)/groupSize + 1) * groupSize;
        groups = numOfVariables/groupSize;
        initModel(numOfVariables);
        idpermute = (int *) malloc ( (numOfVariables+1) * sizeof(int) );
        permute = (int *) malloc ( (numOfVariables+1) * sizeof(int) );
        unpermute = (int *) malloc ( (numOfVariables+1) * sizeof(int) );
         permute1 = (int *) malloc ( (numOfVariables+1) * sizeof(int) );
         unpermute1 = (int *) malloc ( (numOfVariables+1) * sizeof(int) );
        heading(numOfVariables);
       for (v=1;v<=numOfVariables;v++)
           idpermute[v] = v;

        initGlobalPermute();
        setPlainPermute();
        for (g=0;g<groups;g++)
            firstSatGroup (g, -1);


        for (l=0;l<2;l++)
        {
            setAnnealingPermute1(l);
            secondSatGroup(l, 1);
            if (l==0)
            {
                for (v=1;v<=numOfVariables;v++)
                {
                    permute1[v] = permute[v];
                    unpermute1[v] = unpermute[v];
                }
				initChums();
            }
        }
        if (stringOptions[MODEL] != NULL)
            printModel (stringOptions[MODEL]);
    }
    else
    {
        groupSize = 4;
        if (numOfVariables == 1)
            numOfVariables = 2;
        groups = (numOfVariables+2)/groupSize;
        numOfVariables = groupSize*groups + 1;
        permute = (int *) malloc ( (numOfVariables+1) * sizeof(int) );
        unpermute = (int *) malloc ( (numOfVariables+1) * sizeof(int) );
        idpermute = (int *) malloc ( (numOfVariables+1) * sizeof(int) );
        for (v=1;v<=numOfVariables;v++)
        {
            idpermute[v] = v;
        }
        heading(numOfVariables);


        initGlobalPermute();
        setPlainPermute();
        for (g=0;g<groups-1;g++)
            mainUnsatGroup (g,-1);
        lastUnsatGroup (-1);
        setAnnealingPermute();
        for (g=0;g<groups-1;g++)
            mainUnsatGroup (g,1);
        lastUnsatGroup (1);
    }
}

int main(int argc, char **argv)
{

    getOptions (&argc, argv);

    checkOptions();

    numOfVariables = intOptions[NUM_VARIABLES];
    isSat = booleanOptions[SAT];
    reorder = booleanOptions[REORDER];

    mysrand (intOptions[RANDOM_SEED]);

    if (booleanOptions[MIN_VARIABLES])
    {
        if (numOfVariables > 30)
        {
            fprintf (stderr, "For min-variables, the greatest number of variables permitted is 30\n");
            exit(0);
        }
        minVariables ();
    }
    else
    {
        minLiterals ();
    }

    printClauseArray();

    return 0;
}
