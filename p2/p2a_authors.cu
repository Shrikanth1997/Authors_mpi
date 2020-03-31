#include <stdio.h>
#include <stdlib.h>

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


__global__ void add( int *a, int *b, int *c, int vector_size ) {
    
    // Calculate the index in the vector for the thread using the internal variables
    int tid = blockIdx.x * blockDim.x + threadIdx.x; // HERE
    
    // This if statement is added in case we have more threads executing
    // Than number of elements in the vectors. How can this help?
    if (tid < vector_size){
        
        // Compute the addition
        c[tid] = a[tid] + b[tid];
        
    }
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


int main( int argc, char* argv[] ) { 

    // Parse Input arguments

    // Check the number of arguments
    if (argc != 3) {
        // Tell the user how to run the program
        printf ("Usage: %s vector_size block_size\n", argv[0]);
        // "Usage messages" are a conventional way of telling the user
        // how to run a program if they enter the command incorrectly.
        return 1;
    }
    
    // Set GPU Variables based on input arguments
    int vector_size = atoi(argv[1]);
    int block_size  = atoi(argv[2]);
    int grid_size   = ((vector_size-1)/block_size) + 1;

    // Set device that we will use for our cuda code
    cudaSetDevice(0);
        
    // Time Variables
    cudaEvent_t start, stop;
    float time;
    cudaEventCreate (&start);
    cudaEventCreate (&stop);

    // Input Arrays and variables
    int *a        = new int [vector_size]; 
    int *b        = new int [vector_size]; 
    int *c_cpu    = new int [vector_size]; 
    int *c_gpu    = new int [vector_size];

    // Pointers in GPU memory
    int *dev_a;
    int *dev_b;
    int *dev_c;

    // fill the arrays 'a' and 'b' on the CPU
    printf("Initializing input arrays.\n");
    for (int i = 0; i < vector_size; i++) {
        a[i] = rand()%10;
        b[i] = rand()%10;
    }

    //
    // CPU Calculation
    //////////////////

    printf("Running sequential job.\n");
    cudaEventRecord(start,0);

    // Calculate C in the CPU
    for (int i = 0; i < vector_size; i++) {
            c_cpu[i] = a[i] + b[i];
    }

    cudaEventRecord(stop,0);
    cudaEventSynchronize(stop);
    cudaEventElapsedTime(&time, start, stop);
    printf("\tSequential Job Time: %.2f ms\n", time);

    int actual_size = vector_size * sizeof(int);

    // allocate the memory on the GPU
    cudaMalloc(&dev_a,actual_size);
    cudaMalloc(&dev_b,actual_size);
    cudaMalloc(&dev_c,actual_size);

    // copy the arrays 'a' and 'b' to the GPU
    cudaMemcpy(dev_a,a,actual_size,cudaMemcpyHostToDevice);
    cudaMemcpy(dev_b,b,actual_size,cudaMemcpyHostToDevice);

    //
    // GPU Calculation
    ////////////////////////

    printf("Running parallel job.\n");

    cudaEventRecord(start,0);

    // call the kernel
    //add<<<grid_size,block_size>>>(dev_a,dev_b,dev_c,actual_size);
    add<<<vector_size,1>>>(dev_a,dev_b,dev_c,actual_size);
    
    cudaEventRecord(stop,0);
    cudaEventSynchronize(stop);

    cudaEventElapsedTime(&time, start, stop);
    printf("\tParallel Job Time: %.2f ms\n", time);

    // copy the array 'c' back from the GPU to the CPU
    // HERE (there's one more at the end, don't miss it!)
    cudaMemcpy(c_gpu,dev_c,actual_size,cudaMemcpyDeviceToHost);
    
    // compare the results
    int error = 0;
    for (int i = 0; i < vector_size; i++) {
        if (c_cpu[i] != c_gpu[i]){
            error = 1;
            printf( "Error starting element %d, %d != %d\n", i, c_gpu[i], c_cpu[i] );    
        }
        if (error) break; 
    }

    if (error == 0){
        printf ("Correct result. No errors were found.\n");
    }

    // free CPU data
    free (a);
    free (b);
    free (c_cpu);
    free (c_gpu);

    // free the memory allocated on the GPU
    // HERE
    cudaFree(dev_a);
    cudaFree(dev_b);
    cudaFree(dev_c);

    return 0;
}

