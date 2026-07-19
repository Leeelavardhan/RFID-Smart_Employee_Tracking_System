#include <stdio.h>
#include <stdio_ext.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <time.h>
#include <sys/select.h>
#include <stdlib.h>
#include <ctype.h>

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define BOLD    "\033[1m"

#define SESSION_TIME_SEC  360  // seconds
#define TIME_BTW_IN_OUT    10  // seconds

char ID[10];
char purpose[5];
char Time[10];
char DataFrame[50];
char UART_BUFFER[50];
char NAME[25];
char IN_FIELD[10];

int proceed = 0;
int frame_index = 0;
int file_descriptor;

int Open_Serial(const char *dev);
int Transmit_String_To_Port(const char *string);
int Receive_String_From_Port(char *received_string);
int Transmit_Char_To_Port(char);
int Receive_Char_From_Port(char *);
void Extract_Frame(char *frame);
char Check_ID(char *ID);
int fgets_with_timeout(char *buf, int size, int timeout_sec);
char Add_Employee(char *ID);
int Frame_Formation(int bytes_read);
void Operation_To_Be_Done();
char Edit_Employee(char *ID);
char Delete_Employee(char *ID);
char Log_Employee(char *ID);
void Write_Working_Hours(char *ptr);
int Write_In_Time(char *);
void Write_Out_Time(char *);
char Update_Working_Hours(char *working_hours, char *in_field);
int Valid_In_Time(char *);
int Check_All_In_Out_Fields(void);
void Roundoff_Out_Time(char *, char *);
void Extract_Time_Frame(int *hour, int *min, int *sec, char *ptr);
int valid_name(char *name);
char *itoa(int);
void Extract_Employee_Name(char *ptr);
void Extract_IN_Field(char *ptr);

int main() {

    	//const char *device = "/dev/ttyUSB0"; // USB to Serial Converter
    	const char *device = "/dev/ttyACM0"; // Arduino
	
    	file_descriptor = Open_Serial(device);
    	if (file_descriptor < 0) return 1;

	//File
	FILE *fp = fopen("employee_details.csv", "r");
	if(!fp){
		printf("error: source file not found\n");
		return 0;
	}
	fputs("S.NO,EMPLOYEE NAME,EMPLOYEE ID,WORKING HOURS,IN TIME,OUT TIME,IN TIME,OUT TIME,IN TIME,OUT TIME\n", fp);
	fclose(fp);
	
	time_t start_time = time(NULL);

    	printf("\nUART session started (valid for " BOLD CYAN "%d Seconds" RESET ")\n\n", SESSION_TIME_SEC);
		
	while (1) {

        	// ---- TIME BOUND CHECK ----
        	if (difftime(time(NULL), start_time) >= SESSION_TIME_SEC) {
			printf("\n"BOLD CYAN "%d Seconds" RESET " completed. Closing session.\n\n", SESSION_TIME_SEC);
            		break;
        	}

      		// ---- RECEIVE DATA ----
        	int bytes_read = Receive_String_From_Port(UART_BUFFER);
		
		if(Frame_Formation(bytes_read)){
		
        		printf(GREEN "Received " RESET ": '" YELLOW "%s" RESET "'\n", DataFrame);

			if(!strcmp(DataFrame, "[CHECK ALL IN_OUT FIELDS}")){
				if(Check_All_In_Out_Fields())
					Transmit_Char_To_Port('V');
			}
			else{
			
        			Extract_Frame(DataFrame);
			
				Operation_To_Be_Done();

			}

		}

        	usleep(10000);  // 10 ms 
	}

    	close(file_descriptor);
    	return 0;
}

int Open_Serial(const char *dev) {
	// better: blocking
    	int file_descriptor = open(dev, O_RDWR | O_NOCTTY);

    	//int fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
    	if (file_descriptor == -1) { perror("open"); return -1; }

    	struct termios options;
    	tcgetattr(file_descriptor, &options);

    	cfsetispeed(&options, B9600);
    	cfsetospeed(&options, B9600);

	options.c_cflag &= ~PARENB;     // no parity
    	options.c_cflag &= ~CSTOPB;     // 1 stop bit
    	options.c_cflag &= ~CSIZE;
    	options.c_cflag |= CS8;         // 8 data bits
    	options.c_cflag |= CREAD | CLOCAL;

    	options.c_lflag = 0;            // raw input
    	options.c_iflag = 0;
    	options.c_oflag = 0;

    	//This is the useful part for our requirement FOCUS ON THIS MORE
    	options.c_cc[VMIN]  = 25;        // no blocking 
    	options.c_cc[VTIME] = 0;        // no timeout

    	tcflush(file_descriptor, TCIFLUSH);
    	tcsetattr(file_descriptor, TCSANOW, &options);
    	return file_descriptor;
}

int Transmit_Char_To_Port(char ch){

        int bytes_written = write(file_descriptor, &ch, 1);
        if(bytes_written < 0){
                perror("write");
                return 1;
        }
        else
                printf(BLUE "Sent     " RESET ": '" YELLOW "%c" RESET "'\n", ch);
}

int Transmit_String_To_Port(const char *string){
    	
	int bytes_written = write(file_descriptor, string, strlen(string));
    	if(bytes_written < 0){
		perror("write"); 
		return 1; 
	}
	else{
		printf(BLUE "Sent     " RESET ": '");
		while(*string){
			if(*string == '?')
				break;
			else
				printf(YELLOW "%c" RESET, *string);
			string++;
		}
		printf("'\n");	
		//printf(BLUE "Sent     " RESET ": '" YELLOW "%s" RESET "'\n", string);
	}
}

int Receive_Char_From_Port(char *ch){

        int bytes_read = read(file_descriptor, ch, 1);
        if (bytes_read < 0) { perror("read"); return 1; }

        return bytes_read;
}

int Receive_String_From_Port(char *received_string){

    	int bytes_read = read(file_descriptor, received_string, 30);
    	if (bytes_read < 0) { perror("read"); return 1; }

	return bytes_read;
}

void Extract_Frame(char *frame){
	char *pID = ID;
	char *ppurpose = purpose;
	char *pTime = Time;
	while(*frame){
		if(*frame == '['){
			frame++;
			while(!(*frame == ']')){
				*pID++ = *frame++;
			}
			*pID = '\0';	
		}
		else if(*frame == '('){
			frame++;
			while(!(*frame == ')')){
				*ppurpose++ = *frame++;
			}
			*ppurpose = '\0';
		}
		else if(*frame == '{'){
			frame++;
			while(!(*frame == '}')){
				*pTime++ = *frame++;
			}
			*pTime = '\0';	
		}
		frame++;
	}	
}

char Check_ID(char *ID){
	FILE *fp = fopen("employee_details.csv", "r");
	if(!fp)
		return 'A';

	char buffer[4096] = {0};  // 4 KB buffer
	fread(buffer, 1, 4095, fp);
	
	fclose(fp);	

	if(strstr(buffer, ID))
		return 'N';         // ID Existed
	else
		return 'Y';         // ID Not Existed
}

int valid_name(char *name){
	while(*name){
		if(isalpha(*name) || *name == '\n')
			name++;
		else
			return 0;
	}
	return 1;
}

char Add_Employee(char *ID){

	//Checking the file if it has already this Employee ID or NOT
	switch(Check_ID(ID)){
		case 'N': return 'N';
			  break;
		case 'Y': //Do nothing
			  break;
		case 'A': return 'A';
			  break;
	}

	FILE *fp = fopen("employee_details.csv", "r");
        if(!fp){
                return 'A';
        }

        char *str[50] = {0};       // Array of 50 Pointers
        char buffer[1024] = {0};   // 1 KB Buffer

        int lines = 0;
        while(fgets(buffer, 1023, fp)){
                str[lines] = malloc(strlen(buffer));
                strcpy(str[lines++], buffer);
        }
	
	fclose(fp);

	fp = fopen("employee_details.csv", "a+");
        if(!fp)
                return 'A';

	fprintf(fp,"%d",lines);
	fputc(',', fp);
	
	printf("\n[Note: Max. 20 characters]");
	printf(GREEN "\nEnter Employee Name " RESET ": ");
	fflush(stdout);

	char name[21] = {0};
	__fpurge(stdin);
	fgets(name, 21, stdin);
	name[strcspn(name,"\n")]='\0'; 

	if(!valid_name(name))
		return 'B';

	fflush(stdout);
	
	char *ptr = name;
	while(*ptr){
		if(*ptr == '\n'){
			*ptr = '\0';
		}
		ptr++;
	}
	fputs(name, fp);
	fputc(',', fp);

	fputs(ID, fp);
	fputc(',', fp);

	// Working Hours field
	fputs("        ", fp);
	fputc(',', fp);

	// IN Field - 1
	fputs("        ", fp);
	fputc(',', fp);

	// OUT Field - 1
	fputs("        ", fp);
	fputc(',', fp);

	// IN Field - 2
	fputs("        ", fp);
	fputc(',', fp);
	
	// OUT Field - 2
	fputs("        ", fp);
	fputc(',', fp);
	
	// IN Field - 3
	fputs("        ", fp);
	fputc(',', fp);
	
	// OUT Field - 3
	fputs("        ", fp);
	fputc(',', fp);
	
	fputc('\n', fp);
	fclose(fp);
	return 'Y';
}

int Frame_Formation(int bytes_read){
	
	for(int i = 0; i < bytes_read; i++){

    		char ch = UART_BUFFER[i];

    		if(!proceed){
        		if(ch == '['){
           			proceed = 1;
          			frame_index = 0;
            			DataFrame[frame_index++] = ch;
        		}
       	 		continue;
    		}

    		DataFrame[frame_index++] = ch;

    		if(ch == '}'){
        		DataFrame[frame_index] = '\0';
        		proceed = 0;
        		frame_index = 0;
			return 1;
		}
	}
		return 0;
}

void Operation_To_Be_Done(){
        
	if(!strcmp(purpose, "ADD")){
        
		switch(Add_Employee(ID)){
			case 'A': printf(RED"Error: Source File Not Found\n"RESET);
				  Transmit_Char_To_Port('A');
				  break;
			case 'B': printf(RED"Error: Invalid Name\n"RESET);
			          Transmit_Char_To_Port('B');
				  break;
			case 'N': printf(RED"Error: User Already Existed\n"RESET);
			          Transmit_Char_To_Port('N');
				  break;
			case 'Y': printf("User Added into DataBase\n");
			          Transmit_Char_To_Port('Y');
				  break;
		}

        }
	else if(!strcmp(purpose, "EDT")){

		switch(Edit_Employee(ID)){
			case 'A': printf(RED"Error: Source File Not Found\n"RESET);
				  Transmit_Char_To_Port('A');
				  break; 
			case 'B': printf(RED"Error: Invalid Name\n"RESET);
			          Transmit_Char_To_Port('B');
				  break;
			case 'N': printf(RED"Error: User Not Found\n"RESET);
			          Transmit_Char_To_Port('N');
				  break;
			case 'Y': printf("User Name Edited Successfully\n");
			          Transmit_Char_To_Port('Y');
				  break;
		}

	}
	else if(!strcmp(purpose, "DEL")){

		switch(Delete_Employee(ID)){
			case 'A': printf(RED"Error: Source File Not Found\n"RESET);
				  Transmit_Char_To_Port('A');
				  break; 
			case 'N': printf(RED"Error: User Not Found\n"RESET);
			          Transmit_Char_To_Port('N');
				  break;
			case 'Y': printf("User Deleted Successfully\n");
			          Transmit_Char_To_Port('Y');
				  break;
                }

	}
	else if(!strcmp(purpose, "LOG")){
		
		switch(Log_Employee(ID)){
			case 'A': printf(RED"Error: Source File Not Found\n"RESET);
				  Transmit_Char_To_Port('A');
				  break;
			case 'B': printf(RED"Error: Time Violated\n"RESET);
				  Transmit_Char_To_Port('B');
				  Transmit_String_To_Port(IN_FIELD);
				  break;
			case 'C': printf(RED"Error: Try After 09:00:00\n"RESET);
				  Transmit_Char_To_Port('C');
                                  break;
			case 'D': printf(RED"Error: Too Fast OUT Time (%d Seconds)\n"RESET, TIME_BTW_IN_OUT);
				  Transmit_Char_To_Port('D');
                                  break;
			case 'Y': printf("User Logged\n");
				  Transmit_Char_To_Port('Y');
				  Transmit_String_To_Port(NAME);
				  break;
			case 'S': printf(RED"Error: Maximum IN & OUT Reached\n"RESET);
				  Transmit_Char_To_Port('S');
				  break;
			case 'N': printf(RED"Error: User Not Existed\n"RESET);
				  Transmit_Char_To_Port('N');
				  break;
		}
	
	}

}

char Log_Employee(char *ID){
	
	//Checking the file if it has already this Employee ID or NOT
	switch(Check_ID(ID)){
		case 'Y': return 'N';
			  break;
		case 'N': //Do nothing
			  break;
		case 'A': return 'A';
			  break;
	}

	FILE *fp = fopen("employee_details.csv", "r");
        if(!fp)
                return 'A';

        char *str[50] = {0};       // Array of 50 Pointers
        char buffer[1024] = {0};   // 1 KB Buffer

	int lines = 0;

	while(fgets(buffer, 1023, fp)){
                str[lines] = malloc(strlen(buffer));
                strcpy(str[lines++], buffer);
        }

	lines = 0;
	char* Employee_name;
        while(str[lines]){
                if(strstr(str[lines], ID)){
			Employee_name = str[lines];
                        break;    // To know in which line ID Found
                }
                lines++;
        }
	
	char *working_hours = NULL;
	char *in_field = NULL;
	char *out_field = NULL;
	
	char *temp = strchr(str[lines], ',');
	int comma_count = 0;
	while(temp){
		++comma_count;
		temp++;
		temp = strchr(temp, ',');
		if(comma_count == 2){
			temp++;
			working_hours = temp;    // Pointing to Working Hours
		}
		else if(comma_count == 3){
			temp++;
			in_field = temp;         // Pointing to IN Field - 1
		}
		else if(comma_count == 4){
			temp++;
			out_field = temp;        // Pointing to OUT Field - 1
		}
	}

	char ch;
	if(*working_hours == ' '){
		Write_Working_Hours(working_hours);
		if(Write_In_Time(in_field))
			return 'C';
	}
	else if(*out_field == ' '){
		Write_Out_Time(out_field);
		ch = Update_Working_Hours(working_hours, in_field);
		if(ch == 'D')
			return 'D';
		else if(ch == 'B')
			return 'B';
	}
	else if(*(in_field + 19) == ' '){
		if(Valid_In_Time(out_field))
			Write_In_Time((in_field + 18));
		else
			return 'B';
	}
	else if(*(out_field + 18) == ' '){
		Write_Out_Time(out_field + 18);
                ch = Update_Working_Hours(working_hours, in_field + 18);
		if(ch == 'D')
			return 'D';
		else if(ch == 'B')
			return 'B';
	}
	else if(*(in_field + 38) == ' '){
		if(Valid_In_Time(out_field + 18))
			Write_In_Time((in_field + 36));
		else
			return 'B';
	}
	else if(*(out_field + 36) == ' '){
		Write_Out_Time(out_field + 36);
                ch = Update_Working_Hours(working_hours, in_field + 36);
		if(ch == 'D')
			return 'D';
		else if(ch == 'B')
			return 'B';
	}
	else{
		return 'S';
	}

	fclose(fp);

	fp = fopen("employee_details.csv", "w");
	if(!fp)
		return 'A';

	lines = 0;
	while(str[lines]){
		fputs(str[lines], fp);
		lines++;
	}

	fclose(fp);
	Extract_Employee_Name(Employee_name);
	return 'Y';
}

void Extract_Employee_Name(char *ptr){
	while(*ptr){
		if(isalpha(*ptr)){
			int i;
			for(i = 0; *ptr != ','; i++){
				NAME[i] = *ptr;
				ptr++;
			}
			NAME[i] = '?';
		}
		else
			ptr++;
	}	
}

int Valid_In_Time(char *out_field){
	//Compare previous out field and then write in field.
	int out_hour = 0, out_min = 0, out_sec = 0;
        out_hour = *out_field - 48;
        out_hour = (out_hour * 10) + (*(out_field + 1) - 48);

        out_min = *(out_field + 3) - 48;
        out_min = (out_min * 10) + (*(out_field + 4) - 48);

        out_sec = *(out_field + 6) - 48;
        out_sec = (out_sec * 10) + (*(out_field + 7) - 48);

	int in_hour = 0, in_min = 0, in_sec = 0;
        in_hour = *Time - 48;
        in_hour = (in_hour * 10) + (*(Time + 1) - 48);

        in_min = *(Time + 3) - 48;
        in_min = (in_min * 10) + (*(Time + 4) - 48);

        in_sec = *(Time + 6) - 48;
        in_sec = (in_sec * 10) + (*(Time + 7) - 48);

	if(((in_hour * 60 * 60) + (in_min * 60) + in_sec) > ((out_hour * 60 * 60) + (out_min * 60) + out_sec))
		return 1;        // Valid In Time
	else{
		Extract_IN_Field(out_field);
		return 0;        // Invalid In Time
	}
}

void Extract_Time_Frame(int *hour, int *min, int *sec, char *ptr){
	*hour = *ptr - 48;
        *hour = (*hour * 10) + (*(ptr + 1) - 48);

        *min = *(ptr + 3) - 48;
        *min = (*min * 10) + (*(ptr + 4) - 48);

        *sec = *(ptr + 6) - 48;
        *sec = (*sec * 10) + (*(ptr + 7) - 48);
}

char Update_Working_Hours(char *working_hours, char *in_field){
	
	int in_hour = 0, in_min = 0, in_sec = 0;
	Extract_Time_Frame(&in_hour, &in_min, &in_sec, in_field);

	int out_hour = 0, out_min = 0, out_sec = 0;
	Extract_Time_Frame(&out_hour, &out_min, &out_sec, Time);

	int working_hour = 0, working_min = 0, working_sec = 0;
	Extract_Time_Frame(&working_hour, &working_min, &working_sec, working_hours);

	int In_Time = (in_hour * 60 * 60) + (in_min * 60) + in_sec;
	int Out_Time = (out_hour * 60 * 60) + (out_min * 60) + out_sec;
	int Working_Time = (working_hour * 60 * 60) + (working_min * 60) + working_sec;
	
	if((unsigned int)(Out_Time - In_Time) <= TIME_BTW_IN_OUT){
		return 'D';                 // Fast OUT Less than given Seconds
	}
	else if(Out_Time > In_Time){
		Working_Time += Out_Time - In_Time;
	}
	else{
		Working_Time += 60;
		Extract_IN_Field(in_field);
		return 'B';                 // Time Travel to Back OUT < IN
	}

	int updated_hour = 0, updated_min = 0, updated_sec = 0;
	updated_hour = Working_Time / 3600;
	Working_Time -= ((Working_Time / 3600) * 3600);

	updated_min = Working_Time / 60;
	updated_sec = Working_Time % 60;

	char updated_working_hours[10] = {0};
	int i = 0;

	updated_working_hours[i++] = (updated_hour / 10) + 48;
	updated_working_hours[i++] = (updated_hour % 10) + 48;
	updated_working_hours[i++] = ':';
	updated_working_hours[i++] = (updated_min / 10) + 48;
	updated_working_hours[i++] = (updated_min % 10) + 48;
	updated_working_hours[i++] = ':';
	updated_working_hours[i++] = (updated_sec / 10) + 48;
	updated_working_hours[i++] = (updated_sec % 10) + 48;

	i = 0;
	while(updated_working_hours[i]){
		*(working_hours + i) = updated_working_hours[i];
		i++;
	}
	return 'Y';                        // Successfull Transmission
}	

void Extract_IN_Field(char *ptr){
	int i;
	for(i = 0; *ptr != ','; i++){
		IN_FIELD[i] = *ptr;
		ptr++;
	}
	IN_FIELD[i] = '?';
}

void Write_Working_Hours(char *ptr){
	char str[] = "00:00:00";
	for(int i = 0; str[i]; i++){
		*ptr = str[i];
		ptr++;
	}
}

void Write_Out_Time(char *ptr){
        for(int i = 0; Time[i]; i++){
                *ptr = Time[i];
                ptr++;
        }
}

int Write_In_Time(char *ptr){
	
	int in_hour = 0, in_min = 0, in_sec = 0;
        in_hour = *Time - 48;
        in_hour = (in_hour * 10) + (*(Time + 1) - 48);

        in_min = *(Time + 3) - 48;
        in_min = (in_min * 10) + (*(Time + 4) - 48);

        in_sec = *(Time + 6) - 48;
        in_sec = (in_sec * 10) + (*(Time + 7) - 48);

	if(((in_hour * 60 * 60) + (in_min * 60) + in_sec) >= 32400){
		for(int i = 0; Time[i]; i++){
			*ptr = Time[i];
			ptr++;
		}
		return 0;
	}
	else
		return 1;
}

char Edit_Employee(char *ID){
	
	//Checking the file if it has already this Employee ID or NOT
	switch(Check_ID(ID)){
		case 'Y': return 'N';
			  break;
		case 'N': //Do nothing
			  break;
		case 'A': return 'A';
			  break;
	}

	FILE *fp = fopen("employee_details.csv", "r");
	if(!fp)
		return 'A';

	char buffer[1024] = {0};       // 1 KB Buffer
	fread(buffer, 1, 1024, fp);

	char *foundat = strstr(buffer, ID);
	int name_length = 0;
	char tail_part[1024];
	foundat--;
	strcpy(tail_part, foundat);     // Backup of Tail part means after the name
        foundat--;
        while(1){
		if(*foundat == ','){
			foundat++;
                	break;
		}
     		foundat--;
		name_length++;
        }
	char *existing_name = foundat;    // Backup of starting address of name

	printf("\nExisting Name : ");
	for(int i = 0; i < name_length; i++){
		printf("%c", *(existing_name + i));
	}
	printf(GREEN"\nEdit Employee Name : "RESET);
	
	char name[21] = {0};
	__fpurge(stdin);
	fgets(name, 21, stdin);
	name[strcspn(name,"\n")]='\0'; 
		
	if(!valid_name(name))
		return 'B'; 
	
	for(int i = 0; ((name[i] != '\n') && (name[i] != '\0')); i++){
		*existing_name++ = name[i];
	}

	strcpy(existing_name, tail_part);
		
	fclose(fp);
	FILE *fp_overwrite = fopen("employee_details.csv", "w");
		
	fprintf(fp_overwrite, "%s", buffer);

	fclose(fp_overwrite);
	return 'Y';
	
}

char Delete_Employee(char *ID){
	
	//Checking the file if it has already this Employee ID or NOT
	switch(Check_ID(ID)){
		case 'Y': return 'N';
			  break;
		case 'N': //Do nothing
			  break;
		case 'A': return 'A';
			  break;
	}

	FILE *fp = fopen("employee_details.csv", "r");
	if(!fp){
		printf("error: source file not found\n");
		return 'A';
	}

	char *str[50] = {0};       // Array of 50 Pointers
	char buffer[1024] = {0};   // 1 KB Buffer

	//Collecting file content as a strings into *str[]
	int lines = 0;
	while(fgets(buffer, 1023, fp)){
		str[lines] = malloc(strlen(buffer));
		strcpy(str[lines++], buffer);
	}
	
	//Knowing which line has the ID to delete
	lines = 0;
	while(str[lines]){
		if(strstr(str[lines], ID)){
			break;    // To know in which line ID Found
		}
		lines++;
	}

	//Skipping that line means removing that line in *str[]
	while(str[lines + 1]){
		strcpy(str[lines], str[lines + 1]);
		char *temp = itoa(lines);
		char *temp2 = str[lines];
		while(*temp)
			*temp2++ = *temp++;		
		lines++;
	}
	str[lines] = '\0';

	fclose(fp);
	fp = fopen("employee_details.csv", "w");

	lines = 0;
	while(str[lines]){
		fputs(str[lines], fp);
		lines++;
	}

	fclose(fp);
	return 'Y';
}

int Check_All_In_Out_Fields(void){
	
	FILE *fp = fopen("employee_details.csv", "r");
        if(!fp){
                printf("error: source file not found\n");
                return 0;
        }

        char *str[50] = {0};       // Array of 50 Pointers
        char buffer[1024] = {0};   // 1 KB Buffer

        int lines = 0;
        while(fgets(buffer, 1023, fp)){
                str[lines] = malloc(strlen(buffer));
                strcpy(str[lines++], buffer);
        }

	lines = 1;

	while(str[lines]){

		char *ptofield = str[lines];

		ptofield = strchr(str[lines], ','); 
		ptofield++;                         // Name field
		ptofield = strchr(ptofield, ',');   
		ptofield++;                         // ID field
	
		if((*(ptofield + 18) != ' ') && (*(ptofield + 27) == ' ')){
			Roundoff_Out_Time((ptofield + 18), (ptofield + 27));
			
		}
		else if((*(ptofield + 36) != ' ') && (*(ptofield + 45) == ' ')){
			Roundoff_Out_Time((ptofield + 36), (ptofield + 45));

		}
		else if((*(ptofield + 54) != ' ') && (*(ptofield + 63) == ' ')){
			Roundoff_Out_Time((ptofield + 54), (ptofield + 63));
		}
		lines++;
	}
	
	fclose(fp);
        
	fp = fopen("employee_details.csv", "w");

        lines = 0;
        while(str[lines]){
                fputs(str[lines], fp);
                lines++;
        }
	
        fclose(fp);
	return 1;
}

void Roundoff_Out_Time(char *in_field, char *out_field){

	int in_hour = 0, in_min = 0, in_sec = 0;
        in_hour = *(in_field) - 48;
       	in_hour = (in_hour * 10) + (*(in_field + 1) - 48);

        in_min = *(in_field + 3) - 48;
	in_min = (in_min * 10) + (*(in_field + 4) - 48);
	in_sec = *(in_field + 6) - 48;
	in_sec = (in_sec * 10) + (*(in_field + 7) - 48);

	int Working_Time = (in_hour * 60 * 60) + (in_min * 60) + in_sec;
	Working_Time += 60;

	int updated_hour = 0, updated_min = 0, updated_sec = 0;
	updated_hour = Working_Time / 3600;
	Working_Time -= ((Working_Time / 3600) * 3600);
	updated_min = Working_Time / 60;
	updated_sec = Working_Time % 60;

	*out_field++ = (updated_hour / 10) + 48;
	*out_field++ = (updated_hour % 10) + 48;
	*out_field++ = ':';
	*out_field++ = (updated_min / 10) + 48;
	*out_field++ = (updated_min % 10) + 48;
	*out_field++ = ':';
	*out_field++ = (updated_sec / 10) + 48;
	*out_field++ = (updated_sec % 10) + 48;

}

char *itoa(int data){
	char* str = malloc(12);   // enough for int + sign + '\0'
    	if(str == NULL)
        	return NULL;

    	int i = 0;

    	if(data == 0)
        	str[i++] = '0';
    	else{
    	    	while(data > 0){
            		str[i++] = (data % 10) + '0';
            		data /= 10;
        	}
    	}

    	str[i] = '\0';

    	// Reverse string
    	for(int j = 0, k = i - 1; j < k; j++, k--){
        	char temp = str[j];
        	str[j] = str[k];
        	str[k] = temp;
    	}

    	return str;
}



