#include<stdio.h>
#include<mpi.h>
#include<limits.h>
#include<stdlib.h>

struct node{
	int dst;
	struct node* next;
};

struct list{
	struct node *head;
};

struct graph{
	int n;
	struct list* set;
};

struct node* new_node(int dst){
	struct node* newnode = (struct node*)malloc(sizeof(struct node));
	newnode -> dst = dst;
	newnode -> next = NULL;

	return newnode;
}

struct graph* new_graph(int n){
	struct graph* newgraph = (struct graph*)malloc(sizeof(struct node));
	newgraph -> n = n;
	
	newgraph -> set = (struct list*)malloc(n * sizeof(struct list)) ;

	int i;
	for(i=0;i<n;i++)
		newgraph->set[i].head = NULL;

	return newgraph;

}

void addEdge(struct graph* gph, int src, int dst){
	struct node* newnode = new_node(dst);
	newnode->next = gph->set[src].head;
	gph->set[src].head = newnode;

	newnode = new_node(src);
        newnode->next = gph->set[dst].head;
        gph->set[dst].head = newnode;
}

int* get_counts(struct graph* gph, int nid, int tasks) 
{ 
    int v;
    static int auth_num[INT_MAX];
    int start = nid * (gph->n/tasks);
    int end = (nid+1) * (gph->n/tasks); 
    printf("Start: %d End: %d\n", start, end); 
    for (v = start; v < end; ++v) 
    { 
        int co_auth = 0; 
        struct node* pCrawl = gph->set[v].head; 
        //printf("\n Adjacency list of vertex %d\n head ", v); 
        while (pCrawl) 
        { 
            //printf("-> %d", pCrawl->dst); 
            pCrawl = pCrawl->next;
	    co_auth++; 
        }
        auth_num[v] = co_auth;
    }
   return auth_num; 
} 

long get_vert(char *str){
	char vert[20];
	int space_count = 0;
	int num_vert=0;	
	
	int i=0, j=0;
	while(str[i] != '\n'){
	
		if(str[i] == ' ')
			space_count++;
		if(space_count == 2){
			vert[j] = str[i];
			j++;
		}
		else if(space_count>2)	
			break;
		i++;
	}
	vert[j] = '\0';
    	//printf("%s\n", vert);
	num_vert = atoi(vert);
    	//printf("%d\n", num_vert);
	return num_vert;
	
}

int get_src(char *str){
	char s[20];
        int space_count = 0;
        int src=0;

        int i=0, j=0;
        while(str[i] != '\n'){

                if(str[i] == ' ')
                        space_count++;
                if(space_count == 0){
                        s[j] = str[i];
                        j++;
                }
		else
			break;
                i++;
        }
        s[j] = '\0';
        //printf("%s\n", s);
        src = atoi(s);
        //printf("%d\n", src);
        return src;
}

int get_dst(char *str){
	char d[20];
        int space_count = 0;
        int dst=0;

        int i=0, j=0;
        while(str[i] != '\n'){

                if(str[i] == ' ')
                        space_count++;
                if(space_count == 1){
                        d[j] = str[i];
                        j++;
                }
		else if(space_count>1)
			break;
                i++;
        }
        d[j] = '\0';
        //printf("%s\n", d);
        dst = atoi(d);
        //printf("%d\n", dst);
        return dst;
}

int compare (const void * a, const void * b)
{
  return ( *(int*)b - *(int*)a );
}

int main(int argc, char* argv[])
{
     
     //MPI Initialize
     int nodes, id;
     MPI_Status status;
     double mytime = MPI_Wtime();
     MPI_Init(&argc,&argv);
     MPI_Comm_size(MPI_COMM_WORLD,&nodes);
     MPI_Comm_rank(MPI_COMM_WORLD,&id); 


     
    int vert = 5; 

    FILE *fp;
    char str[200];
    char* file = "dblp-co-authors.txt";
 
    fp = fopen(file, "r");
    if (fp == NULL){
        printf("Could not open file %s",file);
        return 1;
    }
    

	    fgets(str, 200, fp);
	    fgets(str, 200, fp);
	    fgets(str, 200, fp);
	    fgets(str, 200, fp);
	    fgets(str, 200, fp);
	    //printf("%s", str);
	    vert = get_vert(str);
	    long src, dst;
	struct graph* gph = new_graph(vert); 
	    while (fgets(str, 200, fp) != NULL){
		//printf("%s", str);
		src = get_src(str);
		dst = get_dst(str);
		addEdge(gph,src,dst);
	    }
	
     

     int len = gph->n;
     int node_len = len / nodes;
     int count_local[len], count_global[len];
     int auth_num_global[len], auth_num_local[len];

     MPI_Scatter(count_global, node_len, MPI_INT, count_local, node_len, MPI_INT, 0, MPI_COMM_WORLD);
     //MPI_Scatter(auth_num_global, node_len, MPI_INT, auth_num_local, node_len, MPI_INT, 0, MPI_COMM_WORLD);
    
   
    int v, max = -1, i = 0;
    int start = id * (gph->n/nodes);
    int end = (id+1) * (gph->n/nodes); 
    printf("Start: %d End: %d\n", start, end); 
    for (v = start; v < end; ++v) 
    { 
        int co_auth = 0; 
        struct node* pCrawl = gph->set[v].head; 
        //printf("\n Adjacency list of vertex %d\n head ", v); 
        while (pCrawl) 
        { 
            //printf("-> %d", pCrawl->dst); 
            pCrawl = pCrawl->next;
	    co_auth++; 
        }
	
	if(max<co_auth){
		max = co_auth;
		auth_num_local[i] = v;
	}
	else if(max == co_auth){
		i++;
		auth_num_local[i] = v;
	}
     		//printf("Author: %d Count: %d\n", auth_num_global[0],max);
	        count_local[v] = co_auth;
    }

     MPI_Gather(count_local, node_len, MPI_INT, count_global, node_len, MPI_INT, 0, MPI_COMM_WORLD);
     //MPI_Gather(auth_num_local, node_len, MPI_INT, auth_num_global, node_len, MPI_INT, 0, MPI_COMM_WORLD);

     qsort(count_global, len, sizeof(int), compare);
     int j=0;
     for(j=0;j<=i;j++)
	printf("Author: %d Count: %d\n", auth_num_local[j], max);

     int max_local = count_global[0];
     if(id==0){
	max = count_global[0];
        MPI_Send(&max, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("Max: %d\n", max);
     }
     if(id != 0)
       	MPI_Recv(&max_local, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
     
     printf("Max Local: %d\n", max_local);

     int dist_local[max_local], dist_global[max_local], val;
     int node_max = (max_local+1)/nodes;

     MPI_Scatter(dist_global, node_max, MPI_INT, dist_local, node_max, MPI_INT, 0, MPI_COMM_WORLD);
     MPI_Scatter(count_global, len, MPI_INT, count_local, len, MPI_INT, 0, MPI_COMM_WORLD);

     int d;

     if(id == 0){
	d = id * (max_local/nodes);
      }
     else{
	d = (id) * (max_local/nodes) +1;
     }
     int dist_start = id * (max_local/nodes);
     int dist_end = (id+1) * (max_local/nodes) + 1;
     printf("dist_start = %d dist_end = %d\n",dist_start, dist_end);
     for(i=0;i<(max_local/nodes)+1;d++,i++){
	val = 0;
	for(j=0;j<len;j++){
		if(d==count_local[j]){
			val++;
		}
	}
	dist_local[i] = val;
	//printf("Distribution %d: %d\n",d, dist_local[d]);
     }

     MPI_Gather(dist_local, node_max, MPI_INT, dist_global, node_max, MPI_INT, 0, MPI_COMM_WORLD);
   
   if(id==0){ 
	printf("Max: %d\n",max_local);
     for(d=0;d<=max_local;d++){ 
	printf("Distribution %d: %d\n",d, dist_global[d]);
     }
   }
 
     fclose(fp); 
     
     MPI_Finalize();    
  
    return 0; 
}
