#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <sys/time.h>   
#include <sys/resource.h>

#define NUM_THREADS 8

int **adjMatrix; // global declaration of adjacency matrix, all functions have access to it
int V; // global variable for #node

struct {
	time_t tv_sec;   //seconds option
	long tv_nsec;	  //nanoseconds option
}timespec;

typedef struct node
{
    int name;
    struct node * next;
} list;

//Add edge in adjacency matrix
void addEdge(int src, int dest)
{
    adjMatrix[src][dest] = 1;
    adjMatrix[V][dest]++;  //sum inDegree of col/node -- last row in each col
}


void printAdjMatrix()
{
    int i, j;
    
    for(i = 0; i <= V; i++)
    {
        for(j = 0; j <V; j++)
        {	
        	
            printf("%d ", adjMatrix[i][j]);
        }
        printf("\n");
    }
}

void init()
{
    int i,j;
    #pragma omp parallel for private(j,V,adjMatrix) schedule(dynamic) 
    for(i = 0; i <= V; i++)
        for(j = 0; j <V; j++)
            adjMatrix[i][j] = 0;
}


// ektipvsh listas

void printList(list * head)
{
	#pragma omp parallel 
	{
	#pragma omp single
	{
    list * current = head->next;

    while (current != NULL) {
   #pragma omp task
    printf("%d\t", current->name);
        current = current->next;}
    }}
  
}

//prosthiki sto telos tis listas.

void push(list * head, int name)
{
	#pragma omp parallel shared(head,name)
	{
	#pragma omp single nowait
	{
	list * current = head;
    while (current->next != NULL) //diatrexv thn lista xekinontas apo to head mexri na vrw to teleytaio stoixio tis.
    {	
    	
        current = current->next;
       
    }
	

    /* Prosthetoyme nea metavlith  */
    current->next = (list *) malloc(sizeof(list));
    //#pragma omp task
    current->next->name = name;
    //#pragma omp task
    current->next->next = NULL;
    }}
}


int remove_last(list * head) {
    int retval = 0;
    /*An yparxei mono ena stoixeio sthn lista afairese to */
    if (head->next == NULL) {
        retval = head->name;
        //free(head);
        return -1;
    }

    /* Phgaine ston proteleytaio komvo  */
    #pragma omp parallel shared(head)
    {
    #pragma omp single nowait
    {
    list * current = head;
    while (current->next->next != NULL) {
      // #pragma omp taskwait
        current = current->next;
        
    }   

	#pragma omp taskwait
    /* now current points to the second to last item of the list, so let's remove current->next */
    retval = current->next->name;
    free(current->next);
	#pragma omp task  
    current->next = NULL;}}
    
    
    return retval;

}

void inDegree(int **arr)		//upologismos tou inDegree tou ka8e komvou
{								//ston pinaka gitniasis
    int i,j,tempD;
#pragma omp parallel shared(j,arr,V)
{

#pragma omp for private(i) reduction(+:tempD) schedule(dynamic) 
    for (j=0;j<V;j++)			//afou ka8e grammi tou pinaka anaparista
    {						//olous tous proorismous enos komvou X
         tempD=0;

        for(i=0;i<V;i++)	//tote kai ka8e stili 8a anaparista
        {					//oles tis piges(sources X) me proorismo enan komvou Y
            if(arr[i][j]==1)
                tempD++;
        }

        arr[V][j]=tempD;
    }
    #pragma omp flush
}
}

void printID()
{
    int i;
 	//#pragma omp parallel shared(V,adjMatrix) 
	{
		//#pragma omp single
		{
    		printf("\nKomvoi:\t\t");
			//#pragma omp parallel for shared(V,adjMatrix) ordered
   				for(i=0;i<V;i++)
				printf("%d\t",i);
    			printf("\nInDegree:\t");
    			
			
    			
    			for(i=0;i<V;i++)
        		printf("%d\t",adjMatrix[V][i]);
    			printf("\n\n\n");
    			
		}
		//#pragma omp flush
	}
}



int existInList(list *L,int x)			//uparxei o komvos x sthn L?
{

  list * current = L;

    while (current != NULL) {
        if(current->name==x)
        {
            return 1;

        }
        current = current->next;
    }
    return 0;
}


void updateS2(int **arr,list * head,list * L)   //enimerwsi tis listas S h lista S periexei oloys tous komvous
{									            //toy grafimatos gia toys opoioys inDegree=0
    int j,bool1,bool2;                          //!! simantiko!! kai den periexontai stin L oute ston S!!!!!.
    
    
    #pragma omp parallel for private(bool1,bool2,j)  shared(arr,head,L,adjMatrix,V)  
    for(j=0;j<V;j++)				            //bool1,bool2=> metavlites elegxou gia lista L,S antistoixa
    {                                           //enimerwsi tis lista S 8a ginei mono an to stixio me in degree=0
	
        bool1=existInList(L,j);                 //den periexetai oute sthn lista S oute stin lista L.
	bool2=existInList(head,j);

        if(bool1==0&&bool2==0)
        {
            if(arr[V][j]==0 )
            {
		#pragma omp task 
                printf("---Eisagw komvo %d stin S---\n\n",j);
		#pragma omp task 
                push(head,j);
               #pragma omp taskwait
            }
        }
    }
}

void updateMatrix(int ** arr,int x)	//enimerwsi tou pinaka gitniasis arr[][]
{									//afairontas ton komvo X
    int j;
    #pragma omp parallel for shared(arr,x,V) schedule(dynamic)
    for(j=0;j<V;j++)
        arr[x][j]=0;
       #pragma omp flush
}

int untitled()
{
	int i,check=0;
	#pragma omp parallel for shared(check,adjMatrix,V) schedule(dynamic)
	for(i=0;i<V;i++)
    {
        if(adjMatrix[V][i]!=0)
        {
            check=1;
        }
    }
    return check;
}


int main()
{
    int delNode;                            //o komvos poy diagrafoyme apo to pinaka geitniasis
    int i,j,z;
   
    
    
   
    FILE * fp;


    ////////////////////////////////////////////////////////////////////////////////////////
    ///////////// STEP 1 => Diavazw to arxeio kai to topothetw ston pinaka geitniasis///////
    ////////////////////////////////////////////////////////////////////////////////////////

    fp = fopen("dag323.txt", "r");
    if(fp == NULL)
    {
        printf("Error opening file\n");
        exit(1);
    }

    fscanf(fp, "%d", &V); // these thn V me to plithos komvwn 

    adjMatrix = (int **)malloc((V+1) * sizeof(int *));
    for ( i=0; i<=(V+1); i++)
        adjMatrix[i] = (int *)malloc(V * sizeof(int)); // Dynamikh desmeysh toy pinaka 
    init();
    fscanf(fp, "%d %d", &j, &z); // prospernaei tiw epomenw dyo metavlhtew gia na ftasei stiw akmes

    
    while(fscanf(fp, "%d %d", &i, &j) != EOF) // Diabazei tiw grammes toy txt gia na parei tis akmes
    {

        
        addEdge(i,j); // prosthetei thn akmh poy diavase ston geitniashs 

    }
    fclose(fp);

    /////////////////////////////////////////////////////////////////////////////////////
    ///////////// STEP 2 => Ektiponw ton pinaka gitniasis pou dimioyrgisa////////////////
    /////////////////////////////////////////////////////////////////////////////////////


    printf("---------------------------------------------------------------------\n\n");
    printf("\nO dothen pinakas geitniasis einai:\n\n");
    printAdjMatrix();
    printID();
    printf("---------------------------------------------------------------------\n\n");

    /////////////////////////////////////////////////////////////////////////////////////
    ////////////// STEP 3 => Dimiourgw tis listes L , S kai enimerwnw S ////////////////
    ///////////////////////////////////////////////////////////////////////////////////



	struct timespec start, finish;
    clock_gettime(CLOCK_REALTIME, &start);
	
	

    //lista L==> lista topologikis diataxis
    list * headL = NULL;
    headL = (list *) malloc(sizeof(list));
    if (headL == NULL) {
        return 1;
    }
    //lista S==> lista me indegree=0
    list * headS = NULL;
    headS = (list *) malloc(sizeof(list));
    if (headS == NULL) {
        return 1;
    }



    headS->next = NULL;     //Arxikopoihsh tvn head
    headL->next = NULL;


    updateS2(adjMatrix,headS,headL);


    //////////////////////////////////////////////////////////////////////////////////
    //////////////// STEP 4 => Ektelw ton Kahn's algorithm ///////////////////////////
    //////////////////////////////////////////////////////////////////////////////////
    ////// STEP 4.1 => Loop gia enimerwsi S , L ////////
    ///////////////////////////////////////////////////
    while(headS->next!=NULL)
    {
        //printList(headS);
        //printf("=S\n\n");
        push(headL,delNode=remove_last(headS));
        
        //printList(headL);
        //printf("=L\n\n");
        printf("o komvos poy diagrafw einai %d\n",delNode);
        updateMatrix(adjMatrix,delNode);
        
        inDegree(adjMatrix);
       
        //printID();
        updateS2(adjMatrix,headS,headL);

        
        printf("---------------------------------------------------------------------\n\n");
    }

    //////////////////////////////////////////////////////////////////////
    ////// STEP 4.2  => Elegxw gia akmes sto grafima kai emfanizw porisma /////////
    ///////////////////////////////////////////////////////////////////////////////
   	
        if(untitled()!=0)
        {
            printf("To grafima periexei akomi akmes ara kai kiklo\nDen uparxei topologiki diataxi gia to sugkekrimeno grafima.\n");
            return(1);
        }
        
    printf("---------------------------------------------------------------------\n\n");   
    printf("H topologiki diataxi einai : ");
    printf("\t");
    printList(headL);
    printf("\n\n\n\n");
    printf("Euxaristw :)\n\n\n");
    
    clock_gettime(CLOCK_REALTIME, &finish);
    long seconds = finish.tv_sec - start.tv_sec;
    long ns = finish.tv_nsec - start.tv_nsec;
    printf("Total seconds: %lf\n", (double)seconds + (double)ns/(double)1000000000);
    
    if (start.tv_nsec > finish.tv_nsec) { // clock underflow 
	--seconds; 
	ns += 1000000000; 
    } 
    
    char sentence[1000];
	// ftiaxneis pointer gia na diatrexeiw to arxeio 
	FILE *fptr;
    
	// anoikse to arxeio se morfh write
	if((fptr=fopen("time323.txt","a")) == NULL) {
        printf("Error!");
        exit(1);
   					 }
		else
		 {
			fprintf(fptr, "Me %d threads o xronos einai %lf seconds\n",NUM_THREADS,(double)seconds + (double)ns/(double)1000000000);
		    	fclose(fptr);
    
		}
    
    for(i=0; i<(V+1);i++)
    {
        free(adjMatrix[i]);
    }

    free(adjMatrix);
 
    
    return(0);
}
