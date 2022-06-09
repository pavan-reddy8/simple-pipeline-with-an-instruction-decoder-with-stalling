#include <stdio.h>
#include <stdlib.h>
unsigned char *inputbuf;
unsigned char *memory;
unsigned int *instr;
long int size;
int instrptr;
int prgstack[256];
int top = -1 ;
int data_hazard_count = 0;
int control_hazard_count = 0;
int execution_cycles = -1;
int completed_instructions = 0;
int regs[16];
int status[16];
int nstatus[16];
int pc =0;
int pipe[9];
int b =0;
int hazard =0;
int exitv =0;

void read(char s[128]){
    printf("%s",s);
    FILE *in = fopen(s, "rb");
    if (!in)
	{
		fprintf(stderr, "Unable to open file");
		return;
	}
    fseek(in, 0, SEEK_END);
    size = ftell(in);
    fclose(in);
    printf("%d\n",size);
    in = fopen(s, "rb");
    if (!in)
	{
		fprintf(stderr, "Unable to open file");
		return;
	}
    inputbuf =(char*) malloc(size);
    instr =(int*) malloc(size);
    int bytes_read = fread(inputbuf, sizeof(int), size, in);
    for (int i=0; i<size; i+=4){
        instr[i+3] = inputbuf[i];
        instr[i+2] = inputbuf[i+1];
        instr[i+1] = inputbuf[i+2];
        instr[i] = inputbuf[i+3];
    }
    for (int i=0; i<size; i++){
        printf("%d: %d\n" ,i,instr[i]);
    }
    

}
void read1(){
    FILE *in = fopen("memory_map", "rb");
    if (!in)
	{
		fprintf(stderr, "Unable to open file");
		return;
	}
    fseek(in, 0, SEEK_END);
    size = ftell(in);
    fclose(in);
    printf("%d\n",size);
    in = fopen("memory_map", "rb");
    if (!in)
	{
		fprintf(stderr, "Unable to open file");
		return;
	}

    memory =(char*) malloc(size);
    if (memory == NULL) {
        fprintf(stderr, "Fatal: failed to allocate %zu bytes.\n", size);
        abort();
    }
    int bytes_read = fread(memory, sizeof(char), size, in);
    int a = memory[4000];
    printf("%d",a);
}



void writeback(){
    if(pipe[8]==-1)
        return;
    completed_instructions++;
    if(instr[pipe[8]] >= 0x00 && instr[pipe[8]] <= 0x51){
        status[instr[pipe[8]+1]] = 1;
    }
    int ptr =pipe[8];
    printf("writeback :%d %d %d %d %d\n",ptr,instr[ptr],instr[ptr+1],instr[ptr+2],instr[ptr+3]);

    switch(instr[ptr]){
        case 0x00: {
            int rx = instr[ptr+1];
            int imm = (instr[ptr+2] << 8) + instr[ptr+3];
            regs[rx] = imm;
            printf("%d",regs[rx]);
        }break;
        case 0x10: {
            int rx = instr[ptr+1];
            int ry = instr[ptr+2];
            int rz = instr[ptr+3];
            regs[rx] = regs[ry] + regs[rz];
        }break;
        case 0x11: {
            int rx = instr[ptr+1];
            int ry = instr[ptr+2];
            int imm = instr[ptr+3];
            regs[rx] = regs[ry] + imm;           
        }break;
        case 0x20: {
            int rx = instr[ptr+1];
            int ry = instr[ptr+2];
            int rz = instr[ptr+3];
            regs[rx] = regs[ry] - regs[rz];          
        }break;
        case 0x21: {
            int rx = instr[ptr+1];
            int ry = instr[ptr+2];
            int imm = instr[ptr+3];
            regs[rx] = regs[ry] - imm;           
        }break;
        case 0x30: {
            int rx = instr[ptr+1];
            int ry = instr[ptr+2];
            int rz = instr[ptr+3];
            regs[rx] = regs[ry] * regs[rz];          
        }break;
        case 0x31: {
            int rx = instr[ptr+1];
            int ry = instr[ptr+2];
            int imm = instr[ptr+3];
            regs[rx] = regs[ry] * imm;           
        }break;
        case 0x40: {
            int rx = instr[ptr+1];
            int ry = instr[ptr+2];
            int rz = instr[ptr+3];
            regs[rx] = regs[ry] / regs[rz];          
        }break;
        case 0x41: {
            int rx = instr[ptr+1];
            int ry = instr[ptr+2];
            int imm = instr[ptr+3];
            regs[rx] = regs[ry] / imm;           
        }break;
        case 0x50: {
            //load
            int rx = instr[ptr+1];
            int imm = (instr[ptr+2] << 8) + instr[ptr+3];
            regs[rx] = memory[imm];
            //exit(0);
        }break;
        case 0x51: {
            //load
            int rx = instr[ptr+1];
            int rz = instr[ptr+3];
            regs[rx] = memory[regs[rz]];

        }break;
        case 0xFF:  {
            exitv =1;
        }

    }
}
void memorywrite2(){
    if(pipe[7]==-1)
        return;
    int ptr =pipe[7];
    switch(instr[ptr]){
        case 0x60: {
            int rx = instr[ptr+1];
            int imm = (instr[ptr+2] << 8) + instr[ptr+3];
            memory[imm] = regs[rx];
        }break;
        case 0x61: {
            int rx = instr[ptr+1];
            int rz = instr[ptr+3];
            memory[regs[rz]] = regs[rx];
        }break;
    }
    printf("memorywrite 2 :%d %d %d %d %d\n",ptr,instr[ptr],instr[ptr+1],instr[ptr+2],instr[ptr+3]);
    
}
void memorywrite1(){
    if(pipe[6]==-1)
        return;
    int ptr =pipe[6];
    printf("memorywrite1 :%d %d %d %d %d\n",ptr,instr[ptr],instr[ptr+1],instr[ptr+2],instr[ptr+3]);
}
void execute2(){
    if(pipe[5]==-1)
        return;
    int ptr =pipe[5];
    printf("execute2 :%d %d %d %d %d\n",ptr,instr[ptr],instr[ptr+1],instr[ptr+2],instr[ptr+3]);
}
void branch(){
    if(pipe[4]==-1)
        return;
    int ptr =pipe[4];
    int rx = instr[ptr+1];
    int imm = (instr[ptr+2] << 8) + instr[ptr+3];
    switch(instr[ptr]){
            case 0x70: {
                b =0;
                if (regs[rx] == 0){
                    
                    instrptr = imm;
                }
            }break;
            case 0x71: {
                b=0;
                if (regs[rx] >= 0){
                    instrptr = imm;
                }
            }break;
            case 0x72: {
                b=0;
                if (regs[rx] <= 0){
                    instrptr = imm;
                }      
            }break;
            case 0x73: {
                b=0;
                if (regs[rx] > 0){
                    
                    instrptr = imm;
                }  
            }break;
            case 0x74: {
                b=0;
                if (regs[rx] < 0){
                    
                    instrptr = imm;
                }     
            }break;
            case 0xFF: { 
                b=0;   
                int a;
                //scanf("%d",&a);
            }
            break;
    }
        
    printf("branch :%d %d %d %d %d\n",ptr,instr[ptr],instr[ptr+1],instr[ptr+2],instr[ptr+3]);
}
void execute1(){
    if(pipe[3]==-1)
        return;
    int ptr =pipe[3];
    printf("execute1 :%d %d %d %d %d\n",ptr,instr[ptr],instr[ptr+1],instr[ptr+2],instr[ptr+3]);
}
void analyze(){
    if(pipe[2]==-1)
        return;
    int ptr = pipe[2];



    if(instr[ptr] == 0x10 || instr[ptr] == 0x20 || instr[ptr] == 0x30 || instr[ptr] == 0x40){
        int rx = instr[ptr+1];
        int ry = instr[ptr+2];
        int rz = instr[ptr+3];
        if(status[ry] == 0 || status[rz] == 0){
            hazard =1;
        }
        else
            hazard =0;
    }
    else if(instr[ptr] == 0x11 || instr[ptr] == 0x21 || instr[ptr] == 0x31 || instr[ptr] == 0x41){

        int ry = instr[ptr+2];
        if(status[ry] == 0){
            hazard =1;
        }
        else
            hazard =0;
    }

    else if(instr[ptr] == 0x51){
        int rz = instr[ptr+3];
        if(status[rz] == 0){
            hazard =1;
        }
        else
            hazard =0;
    }

    else if(instr[ptr] == 0x60){
        int rx = instr[ptr+1];
        if(status[rx] == 0){
            hazard =1;
        }
        else
            hazard =0;
    }

    else if(instr[ptr] == 0x61){
        int rx = instr[ptr+1];
        int rz = instr[ptr+3];
        if(status[rx] == 0 || status[rz] ==0){
            hazard =1;
        }
        else
            hazard =0;
    }
    else if(instr[ptr] >= 0x70 && instr[ptr] <= 0x74){
        int rx = instr[ptr+1];
        if(status[rx] == 0){
            hazard =1;
        }
        else
            hazard =0;
    }

    if(instr[ptr] >= 0x00 && instr[ptr] <= 0x51){
    int rx = instr[ptr+1];
    status[rx] = 0;
    }
    ptr =pipe[2];
    printf("analyse :%d %d %d %d %d\n",ptr,instr[ptr],instr[ptr+1],instr[ptr+2],instr[ptr+3]);
}
void decode(){
    if(pipe[1]==-1)
        return;
    int ptr =pipe[1];
    printf("decode :%d %d %d %d %d\n",ptr,instr[ptr],instr[ptr+1],instr[ptr+2],instr[ptr+3]);
}

void fetch(){
    if(pipe[0]==-1)
        return;
    int ptr =pipe[0];
    printf("fetch :%d %d %d %d %d\n",ptr,instr[ptr],instr[ptr+1],instr[ptr+2],instr[ptr+3]);
    if(instr[ptr] >= 0x70 && instr[ptr] <= 0x74){
        b = 1;
    }
}
int main(int argc, char const *argv[]){
    size = 0;
    for(int i = 0; i < 16; i++){
        regs[i] = 0;
        status[i] = 1;
        // pipe[i] = -1;
    }
    for(int i = 0; i < 9; i++){
        pipe[i] = -1;
    }
    read(argv[1]);
    read1();
    instrptr = 0; 
    int phazard = 0;
    while(instrptr < size && !exitv){

        fetch();
        decode();
        analyze();
        execute1();
        branch();
        execute2();
        memorywrite1();
        memorywrite2();
        writeback();

        pipe[8] = pipe[7];
        pipe[7] = pipe[6];
        pipe[6] = pipe[5];
        pipe[5] = pipe[4];
        pipe[4] = pipe[3];
        pipe[3] = pipe[2];
        if (hazard == 1){
            pipe[3] = -1;
            data_hazard_count++;
            printf("\ndata_hazard_count:%d\n",data_hazard_count);
        }
        
        else{
            pipe[2] = pipe[1];
            pipe[1]=pipe[0];
            if(b==0){
                pipe[0] = instrptr;
                
                instrptr+=4;
            }
            else{
                pipe[0] = -1;
                control_hazard_count++;
            }
        }

        printf("\nexecution_cycles:%d\n",execution_cycles++);

        for (int reg=0; reg<16; reg++) {
            printf("REG[%2d]   |   Value=%d | status=%d \n",reg, regs[reg],status[reg]);
            printf("--------------------------------\n");
        }
        printf("================================\n\n");

        // int a;
        // scanf("%d",&a);


    }
    // while(instrptr < size || top != -1){
        

    //     writeback();

    //     pipe[8] = pipe[7];

    //     memorywrite2();

    //     pipe[7] = pipe[6];

    //     memorywrite1();

    //     pipe[6] = pipe[5];

    //     execute2();

    //     pipe[5] = pipe[4];

    //     branch();

    //     pipe[4] = pipe[3];

    //     execute1();

        
    //     pipe[3] = pipe[2];

    //     analyze();

    //     if(hazard == 1){
    //         pipe[3] = -1;
    //     }

        

    //     if(hazard == 0){
    //         pipe[2] = pipe[1];
    //         decode();
    //         pipe[1] = pipe[0];

    //         fetch();
    //         if(b==0){
    //         pipe[0] = instrptr;

    //         // if(instr > size && top > -1){
    //         //     instr = prgstack[top];
    //         //     } 
            
    //         instrptr+=4;
    //         }
    //         else{
    //             pipe[0]=-1;
    //         }

    //     }
    //     phazard = hazard;
    //     for(int i=0; i < 16;i++)
    //     printf("================================\n");
    //     printf("--------------------------------\n");
    //     for (int reg=0; reg<16; reg++) {
    //         printf("REG[%2d]   |   Value=%d | status=%d \n",reg, regs[reg],status[reg]);
    //         printf("--------------------------------\n");
    //     }
    //     printf("================================\n\n");

    //     int a;
    //     scanf("%d",&a);


    // }
    printf("================================\n");
    printf("--------------------------------\n");
    for (int reg=0; reg<16; reg++) {
        printf("REG[%2d]   |   Value=%d  \n",reg, regs[reg]);
        printf("--------------------------------\n");
    }
    printf("================================\n\n");

    printf("Stalled cycles due to data hazard: %d \n", data_hazard_count);
    printf("Stalled cycles due to control hazard: %d \n", control_hazard_count);
    printf("\n");
    printf("Total stalls: %d \n", data_hazard_count+control_hazard_count);
    printf("Total execution cycles: %d\n", execution_cycles);
    printf("Total instruction simulated: %d\n", completed_instructions);
    printf("IPC: %f\n", ((double)completed_instructions/execution_cycles));




    FILE *fp;
    int myInt = 5;
    fp = fopen("Output.txt", "w");
    fprintf(fp,"================================\n");
    fprintf(fp,"--------------------------------\n");
    for (int reg=0; reg<16; reg++) {
        fprintf(fp,"REG[%2d]   |   Value=%d  \n",reg, regs[reg]);
        fprintf(fp,"--------------------------------\n");
    }
    fprintf(fp,"================================\n\n");


    fprintf(fp,"Stalled cycles due to data hazard: %d \n", data_hazard_count);
    fprintf(fp,"Stalled cycles due to control hazard: %d \n", control_hazard_count);
    fprintf(fp,"\n");
    fprintf(fp,"Total stalls: %d \n", data_hazard_count+control_hazard_count);
    fprintf(fp,"Total execution cycles: %d\n", execution_cycles);
    fprintf(fp,"Total instruction simulated: %d\n", completed_instructions);
    fprintf(fp,"IPC: %f\n", ((double)completed_instructions/execution_cycles));

    return 0;
}