#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

typedef struct header {
    uint16_t id;
    int QR;
    char opcode[5];
    int TC;
    int RD;
    char Rcode[5];
    uint16_t QD;
    uint16_t AN;
    uint16_t NS;
    uint16_t AR;


} Header;

typedef struct question{
    uint8_t* name;
    uint16_t type;
    uint16_t class;
} Question;

typedef struct answer{
    uint16_t name;
    uint16_t type;
    uint16_t class;
    uint32_t TTL;
    uint16_t RDlength;
    uint8_t* IP;
} Answer;

uint16_t combineUint8(uint8_t first, uint8_t second){
    return ((uint16_t)first << 8) | second;
}
uint32_t combineUint16(uint16_t first, uint16_t second){
    return ((uint32_t)first << 16) | second;
}

void decodeMsg(uint8_t* msg,FILE* f){
    int count =0;
    while (msg[count]){
        int temp_byte_label = msg[count];      
        for (int i = 0; i < temp_byte_label; i++) {
            int temp = msg[count+i+1];
            fprintf(f,"%c",temp);
        }
        count = count + temp_byte_label+1;
        if (msg[count])   
            fprintf(f,".");   
        
    }
}

void toBinary(uint16_t a,char* out)
{
    int j = 0;
    for(uint16_t i=0x8000;i!=0;i>>=1){
        /*printf("%c",(a&i)?'1':'0');*/
        char c;
        if (a&i){
            c = '1';
        }else{
            c = '0';
        }
        out[j] = c;
        j++;
    }
    out[j]='\0';
    printf("heheheeh %s",out);
}

int hex_to_int(char c){
        int first = c / 16 - 3;
        int second = c % 16;
        int result = first*10 + second;
        if(result > 9) result--;
        return result;
}

int hex_to_ascii(char c, char d){
        int high = hex_to_int(c) * 16;
        int low = hex_to_int(d);
        return high+low;
}
int main(int argc, char* argv[]) {
    uint8_t buffer;
    /*read the first 2 bytes to determine the length of the message*/
    uint8_t first2_bytes[2];

    /*open the file and read the first 2 bytes*/
    /*FILE *fp;
    fp = fopen((argv[1]),"r");*/

    for (int i =0; i <2 ; i++){
        /*read(fileno(fp), &first2_bytes[i], 1);*/
        read(STDIN_FILENO, &first2_bytes[i], 1);

    }
    /*cast 2 bytes to decimal to determine the length of the message*/
    int len_message = ((uint16_t)first2_bytes[0] << 8) | first2_bytes[1];
    printf("The length of the message is %d \n",len_message);

    /*assign the value for the message*/
    uint8_t *messages = (uint8_t*)malloc(len_message*sizeof(uint8_t));

    for(int i=0; i < len_message;i++) {
        read(STDIN_FILENO, &buffer, 1);
        messages[i] = buffer;
    }


    Header* header = (Header*)malloc(sizeof(Header));
    header->id = combineUint8(messages[0],messages[1]);
    printf("The id of the header is %02x \n",header->id);

    uint16_t properties = combineUint8(messages[2],messages[3]);
    char temp[17];
    toBinary(properties,temp);
    printf("The properties of the header is %s \n",temp);
    header->QR = (int)temp[0] - '0';
    printf("The QR of the header is %d \n",header->QR);

    char temp_opcode[5];
    temp_opcode[0] = temp[1];
    temp_opcode[1] = temp[2];
    temp_opcode[2] = temp[3];
    temp_opcode[3] = temp[4];
    temp_opcode[4] = '\0';
    strcpy (header->opcode, temp_opcode);
    printf("The OP code of the header is %s \n",header->opcode);

    header->TC = (int)temp[6]-'0';
    printf("The TC of the header is %d \n",header->TC);
    header->RD = (int)temp[7] - '0';
    printf("The RD of the header is %d \n",header->RD);

    char temp_rcode[5];
    temp_rcode[0] = temp[12];
    temp_rcode[1] = temp[13];
    temp_rcode[2] = temp[14];
    temp_rcode[3] = temp[15];
    temp_rcode[4] = '\0';
    strcpy (header->Rcode, temp_rcode);
    printf("The  R code of the header is %s \n",header->Rcode);

    header->QD = combineUint8(messages[4],messages[5]);
    printf("The QD of the header is %04x \n",header->QD);
    header->AN = combineUint8(messages[6],messages[7]);
    printf("The AN of the header is %04x \n",header->AN);
    header->NS = combineUint8(messages[8],messages[9]);
    printf("The NS of the header is %04x \n",header->NS);
    header->AR = combineUint8(messages[10],messages[11]);
    printf("The AR of the header is %04x \n",header->AR);


    /*read the question section*/
    printf("Number of question: %d\n",(int)header->QD);
    Question** questions =  (Question**)malloc(sizeof(Question*)*(int)header->QD);
    int question_count = 12;
    for (int i =0; i< (int)header->QD;i++){
        questions[i] = (Question*)malloc(sizeof(Question));
        int temp_count = question_count;
        while (messages[question_count]){
            int temp_byte_label = messages[question_count];   
            question_count = question_count + temp_byte_label+1;
        }
        question_count = question_count+1;
        int len_q_name = question_count-temp_count;

        questions[i]->name = (uint8_t*)malloc(sizeof(uint8_t)*len_q_name);
        for (int j =0; j< len_q_name;j++){
            questions[i]->name[j] = messages[temp_count+j];
        }
        printf("The message question name: ");
        /*
        decodeMsg(questions[i]->name);*/
        printf("\n");
        questions[i]->type = combineUint8(messages[question_count],messages[question_count+1]);
        printf("The type of the question is %d \n",questions[i]->type);
        questions[i]->class = combineUint8(messages[question_count+2],messages[question_count+3]);
        printf("The class of the question is %04x \n",questions[i]->class);
        question_count = question_count+4;
    }
    

    /*read the answer part*/
    printf("Number of answer: %d\n",(int)header->AN);
    Answer** answers =  (Answer**)malloc(sizeof(Answer*)*(int)header->AN);
    int answer_count = question_count;
    for (int i =0; i< (int)header->AN;i++){
        answers[i] = (Answer*)malloc(sizeof(Answer));  
        answers[i]->name = combineUint8(messages[answer_count],messages[answer_count+1]);
        printf("The name of the answer is %04x \n",answers[i]->name);
        answers[i]->type = combineUint8(messages[answer_count+2],messages[answer_count+3]);
        printf("The type of the answer is %04x \n",answers[i]->type);
        answers[i]->class = combineUint8(messages[answer_count+4],messages[answer_count+5]);
        printf("The class of the answer is %04x \n",answers[i]->class);
        answers[i]->TTL = combineUint16(combineUint8(messages[answer_count+6],messages[answer_count+7]),combineUint8(messages[answer_count+8],messages[answer_count+9]));
        printf("The TTL of the answer is %08x \n",answers[i]->TTL);
        answers[i]->RDlength = combineUint8(messages[answer_count+10],messages[answer_count+11]);
        printf("The RD length of the answer is %d \n",answers[i]->RDlength);
        int len_IP = answers[i]->RDlength;
        answers[i]->IP = (uint8_t*)malloc(len_IP*sizeof(uint8_t));
        printf("The RD of the answer:");
        for (int j=0; j< len_IP;j++){
            answers[i]->IP[j] = messages[answer_count+12+j];
            printf("%02x",answers[i]->IP[j]);
        }
        printf("\n");
        answer_count = answer_count +12+len_IP;
    }

    /*read the Authority part*/
    printf("Number of authority: %d\n",(int)header->NS);
    int authority_count = answer_count;

    /*read the additional part*/
    printf("Number of additional: %d\n",(int)header->AR);
    int add_count = authority_count;
    uint8_t* additional = (uint8_t*)malloc((len_message-add_count)*sizeof(uint8_t));
    printf("The additional part of the message: ");
    for (int i=0; i< (len_message-add_count);i++){
        additional[i] = messages[add_count+i];
        printf("%02x",additional[i]);
    }
    printf("\n");


    FILE *fptr;

    // use appropriate location if you are using MacOS or Linux
    fptr = fopen("dns_svr.log","w");

    if(fptr == NULL){
        printf("Error!");   
      exit(1);    

    }
    /*fprintf(fptr,"requested ");*/
    decodeMsg(questions[0]->name,fptr);
    if ( (int)header->AN  == 1){  
        char ans[200]; 
        fprintf(fptr," is at ");
        int j =0;
        int k =0;
        while (j<answers[0]->RDlength){
            char str[20];
            uint16_t temp = combineUint8(answers[0]->IP[j],answers[0]->IP[j+1]);
            sprintf(str, "%04x", temp);
            ans[0+k*5] = str[0];
            ans[1+k*5] = str[1];
            ans[2+k*5] = str[2];
            ans[3+k*5] = str[3];
            ans[4+k*5] = ':';
            j+=2;
            k++;
        }
        ans[k*5-1] = '\0';
        printf("Value of %s \n",ans);
    unsigned char buf[sizeof(struct in6_addr)];
    int domain, s;
    char str[INET6_ADDRSTRLEN];

    domain = AF_INET6;

    s = inet_pton(domain, ans, buf);
    if (s <= 0) {
       if (s == 0)
        printf("Not in presentation format");
        else{
            perror("inet_pton");
        }
    }

    if (inet_ntop(domain, buf, str, INET6_ADDRSTRLEN) == NULL) {
        perror("inet_ntop");
    }
    fprintf(fptr,"%s", str);
    }
    fprintf(fptr,"\n");
    

    

    if ((int)questions[0]->type != 28){
        fprintf(fptr,"unimplemented request\n");
    }
    fclose(fptr);
    
    
        
    return 0;
}
