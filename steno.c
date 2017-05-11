#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_image_offset(FILE* bmp_offset) {
	fseek(bmp_offset,10,0);
	int offset=(int)fgetc(bmp_offset);
	return offset;
}

int get_bit(char byte,int bit) {
	return((byte>>8-bit)&0x01);
}

int get_length(FILE *fp) {
	fseek(fp, 0L, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	return(size);
}

void print_usage(){
	printf("LSB Stegonagraphy functions\nUsage: ./stego [-e][-d] <source file> <text file> [<message length>]\n-e : Encode text in image, requires destination image file and text file\n-d : Decode text from image, specify encoded image and length of message contained\n");
}

int main(int argc,char** argv) {

	FILE * file_handle;
	FILE * message_handle;
	FILE * hidden_handle;
	
	int mode;
	if(!strcmp(argv[1],"-e"))
		mode=1;
	else if(!strcmp(argv[1],"-d"))
		mode=0;
	else {
		print_usage();
		exit(1);
	}
	
	//if(mode && argc!=5){
		//print_usage();
		//exit(1);
	//}
	//if(!mode && argc!=4){
		//print_usage();
		//exit(1);
	//}
	if(argc!=4) {
		print_usage();
		exit(1);
	}

	
	file_handle=fopen(argv[2],"r");
	hidden_handle = NULL;
	if (file_handle == NULL) {
		fprintf(stderr, "Can't open file for reading %s\n",argv[2]);
		exit(1);
	}

	if(mode){
		hidden_handle=fopen("out.bmp","w");
		if (hidden_handle == NULL) {
			fprintf(stderr, "Can't open file for writing %s\n",argv[3]);
			exit(1);
		}
	}
	
	char c;
	int offset=get_image_offset(file_handle);
	rewind(file_handle);
	
	if(mode){
		//copy BMP header to output file	
		for(int i=0;i<offset;i++) {
			c=fgetc(file_handle);
			fputc(c,hidden_handle);
		}
	}
	else{fseek(file_handle, offset, SEEK_SET);}

	char image_buffer; 			// buffer for one byte from image file
	char message_buffer;		// buffer for one byte from message

	if(mode) {
		message_handle=fopen(argv[3],"r");
		if (message_handle == NULL) {
			fprintf(stderr, "Can't open text input file %s\n",argv[4]);
			exit(1);
		}

		fseek(message_handle, 0L, SEEK_END);
		
		int message_length = ftell(message_handle);		//store length of message
		
		fseek(message_handle, 0L, SEEK_SET);

		
		//store message length for decoding
		fputc(message_length, hidden_handle);
		do {
			int bit_of_message;

			message_buffer=fgetc(message_handle);
			//traverse each bit of message
			for(int i=1;i<=8;i++) { 

				image_buffer=fgetc(file_handle);
				int byte_lsb = image_buffer & 1; 

				bit_of_message=get_bit(message_buffer,i);
					
				//replace LSB of file byte with bit of message
				image_buffer = (image_buffer & ~1) | bit_of_message;
				fputc(image_buffer, hidden_handle);
			}	
		} while(!feof(file_handle));	
		
		c=fgetc(file_handle);
		fputc(c,hidden_handle);
		
		fclose(message_handle);	
		system("sha256sum out.bmp");
	}
	else {
		message_handle=fopen("out.txt","w");
			
		//use length of message as a "password"
		int message_length=fgetc(file_handle);
		if(message_length-1 != atoi(argv[3])){
			fprintf(stderr, "Incorrect message length!\n");
			exit(1);
		}
		for(int i=0;i<message_length;i++) {
			char c ='\0';
			for( int j=0;j<8;j++) {
				//extract each character of the message
				c = c<<1;
				image_buffer = fgetc(file_handle);
				int byte_lsb = image_buffer & 1; 
				c |= byte_lsb;
			}
			fputc(c,message_handle);
		}
		fclose(message_handle);	
		system("sha256sum out.txt");

	}

	fclose(file_handle);
	if(mode){fclose(hidden_handle);}

}
