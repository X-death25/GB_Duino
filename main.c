#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_VERSION 	1
#define MIN_VERSION 	0

#include <libserialport.h>

//Helper function for error handling. */
/*int check(enum sp_return result)
{
		// For this example we'll just exit on any error by calling abort(). 
		char *error_message;
		switch (result) {
		case SP_ERR_ARG:
				printf("Error: Invalid argument.\n");
				abort();
		case SP_ERR_FAIL:
				error_message = sp_last_error_message();
				printf("Error: Failed: %s\n", error_message);
				sp_free_error_message(error_message);
				abort();
		case SP_ERR_SUPP:
				printf("Error: Not supported.\n");
				abort();
		case SP_ERR_MEM:
				printf("Error: Couldn't allocate memory.\n");
				abort();
		case SP_OK:
		default:
				return result;
		}
}*/

#if defined(_WIN32)
clock_t microsec_start;
clock_t microsec_end;
#else
struct timeval ostime;
long microsec_start = 0;
long microsec_end = 0;
#endif

void timer_start()
{
#if defined(_WIN32)
    microsec_start = clock();
#else
    gettimeofday(&ostime, NULL);
    microsec_start = ((unsigned long long)ostime.tv_sec * 1000000) + ostime.tv_usec;
#endif
}

void timer_end()
{
#if defined(_WIN32)
    microsec_end = clock();
#else
    gettimeofday(&ostime, NULL);
    microsec_end = ((unsigned long long)ostime.tv_sec * 1000000) + ostime.tv_usec;
#endif
}

void timer_show()
{
#if defined(_WIN32)
    printf("~GB: Elapsed time: %lds", (microsec_end - microsec_start)/1000);
    printf(" (%ldms)\n", (microsec_end - microsec_start));
#else
    printf("~GB: Elapsed time: %lds", (microsec_end - microsec_start)/1000000);
    printf(" (%ldms)\n", (microsec_end - microsec_start)/1000);
#endif
}


int main(int argc, char *argv[])
	{
	unsigned char Game_Name[13];
	unsigned short Global_Checksum=0;

	unsigned char *BufferROM;
	unsigned char *BufferSAVE;

	unsigned char Rom_Type=0;
	unsigned char Rom_Size=0;
	unsigned long game_size=0;
	unsigned char Ram_Size=0;
	unsigned long save_size=0;

	FILE *myfile;
	char rom_name[64];
    unsigned long rom_size=0;
    unsigned char nromBank=0;
	unsigned char CGB=0;
	unsigned char SGB=0;
	unsigned char nramBank=0;
	char dump_name[64];

	unsigned char Arduino_Buffer[32*1024];
	unsigned long i=0;
	unsigned long k=0;
	unsigned char r=0;
	unsigned long l=0;

	unsigned char manufacturer_id=0;
    unsigned char device_id=0;
    unsigned short flash_id=0;


     // Vérifier le nombre d'arguments

    if (argc != 4) {
        printf("Usage: %s <port> <mode> <type>\n", argv[0]);
        printf("  <port>: COM1, COM2, COM3, ...\n");
        printf("  <mode>: -read, -write, -backup, -restore, -erase, -identify\n");
        return 1;
    }

	// Vérifier le premier argument (port série)
    if (strncmp(argv[1], "COM", 3) != 0 || strlen(argv[1]) < 4) {
        printf("Le premier argument doit être un port série valide (ex: COM1, COM2).\n");
        return 1;
    }

  // Vérifier le deuxième argument (mode)
    if (strcmp(argv[2], "-read") != 0 && strcmp(argv[2], "-write") != 0 &&
        strcmp(argv[2], "-backup") != 0 && strcmp(argv[2], "-restore") != 0 &&
		strcmp(argv[2], "-erase") != 0 && strcmp(argv[2], "-identify") != 0 &&
		strcmp(argv[2], "-wipe") != 0 )  
	{
        printf("Le deuxième argument doit être '-read', '-write', '-identify' , '-backup' ou '-restore'.\n");
        return 1;
    }


  // Afficher les informations
    printf("Port série : %s\n", argv[1]);

    if (strcmp(argv[2], "-read") == 0) {
        printf("Mode       : Read ROM\n");
    } else if (strcmp(argv[2], "-write") == 0) {
        printf("Mode       : Write ROM\n");
    } else if (strcmp(argv[2], "-backup") == 0) {
        printf("Mode       : Read Backup RAM\n");
    } else if (strcmp(argv[2], "-restore") == 0) {
        printf("Mode       : Write Backup RAM\n");
    } else if (strcmp(argv[2], "-erase") == 0) {
        printf("Mode       : Erase Backup RAM\n");
	} else if (strcmp(argv[2], "-identify") == 0) {
        printf("Mode       : Flash Memory Identification\n");
    } else if (strcmp(argv[2], "-wipe") == 0) {
		printf("Mode       : Flash Memory Erase\n");
	}

/*
	struct sp_port **port_list;
	printf("Getting port list.\n");

	enum sp_return result = sp_list_ports(&port_list);
	if (result != SP_OK)
		{
		printf("sp_list_ports() failed!\n");
		return -1;
		}

	for (i = 0; port_list[i] != NULL; i++)
		{
		struct sp_port *port = port_list[i];
		char *port_name = sp_get_port_name(port);
		printf("Found port #%i: %s\n", i, port_name);
		}
	printf("Found %d ports.\n", i);
*/
	struct sp_port *port;
	sp_get_port_by_name(argv[1], &port);
	printf("Port name: %s\n", sp_get_port_name(port));
	printf("Description: %s\n", sp_get_port_description(port));

	// The port must be open to access its configuration. 
	printf("Opening port.\n");
	sp_open(port, SP_MODE_READ_WRITE);	
		
	printf("Setting port to 38400 8N1, no flow control.\n");
	sp_set_baudrate(port, 38400);
	sp_set_bits(port, 8);
	sp_set_parity(port, SP_PARITY_NONE);
	sp_set_stopbits(port, 1);
	sp_set_flowcontrol(port, SP_FLOWCONTROL_NONE);	

	// Get the ports to send and receive on. 
	struct sp_port *tx_port = port;
	struct sp_port *rx_port = port;

	// Allocate a buffer to send and receive data. 
	unsigned char *Serial_Buffer_IN = malloc(128+1);
	unsigned char *Serial_Buffer_OUT = malloc(128+1);

	// We'll allow a 4 second timeout for send and receive. 
	unsigned int timeout = 4000;

	char *data = "~";
	int size = strlen(data);

	//Clear Serial Buffer
	printf("Cleaning Serial Buffer.. Please Wait...\n");
	sp_blocking_write(tx_port, data, size, timeout);
	sp_blocking_read(rx_port, Serial_Buffer_IN, 128, timeout);

    for (i = 0; i < 128; i++)
		{
        Serial_Buffer_IN[i] = 0x00;
        Serial_Buffer_OUT[i] = 0x00;
		}
    i=0;

	printf("Sending '%s' (%d bytes) on port %s.\n", data, size, sp_get_port_name(tx_port));
	sp_blocking_write(tx_port, data, size, timeout);
	// Check whether we sent all of the data. 
	/*if (result == size)
		printf("Sent %d bytes successfully.\n", size);
	else
		printf("Timed out, %d/%d bytes sent.\n", result, size);*/

	// Try to receive the data on the other port. 
	//printf("Receiving %d bytes on port %s.\n", 128, sp_get_port_name(rx_port));
	//result=0;
     sp_blocking_read(rx_port, Serial_Buffer_IN, 128, timeout);
	
	//for(int c=0;c<128;c++) printf("Received '%c'.\n", buf[c]);



	printf("GB Dumper Ready \n");
	printf("Hardware Firmware version %d",Serial_Buffer_IN[2]);
	printf(".%d\n",Serial_Buffer_IN[1]);
	printf("Software Firmware version %d",MAX_VERSION);
	printf(".%d\n",MIN_VERSION);
	

	printf("\nReading ROM Header...\n\n");
	int j=0;
	for (i = 0; i < 128; i++)
		{
		printf("%02X ",Serial_Buffer_IN[i]);
		j++;
		if (j==16)
			{
			printf("\n");
			j=0;
			}
		}

	//Global Checksum
	Global_Checksum = (Serial_Buffer_IN[35]<<8) | Serial_Buffer_IN[36];

	// Game Name
	for (i = 0; i < 13; i++) Game_Name[i] = 0x20;
	for (i = 0; i < 13; i++) Game_Name[i] = Serial_Buffer_IN[i+9];
	printf("\nGame Name : %s \n",Game_Name);

	//Cartridge type
	Rom_Type = Serial_Buffer_IN[28];
	// Special Cartridge Type ( Ex : MBC30 )
	if ( Global_Checksum == 0x9A40) Rom_Type = 0x14; // Pokemon Crystal JAP
	printf("Cartridge Type : %02X ",Rom_Type);

	switch(Rom_Type)
		{
		case 0x00:
			printf("(ROM ONLY)");
			break;
		case 0x01:
			printf("(MBC1)");
			break;
		case 0x02:
			printf("(MBC1+RAM)");
			break;
		case 0x03:
			printf("(MBC1+RAM+BATTERY)");
			break;
		case 0x05:
			printf("(MBC2)");
			break;
		case 0x06:
			printf("(MBC2+BATTERY)");
			break;
		case 0x08:
			printf("(ROM+RAM)");
			break;
		case 0x09:
			printf("(ROM+RAM+BATTERY)");
			break;
		case 0x0B:
			printf("(MMM01)");
			break;
		case 0x0C:
			printf("(MMM01+RAM)");
			break;
		case 0x0D:
			printf("(MMM01+RAM+BATTERY)");
			break;
		case 0x0F:
			printf("(MBC3+TIMER+BATTERY)");
			break;
		case 0x10:
			printf("(MBC3+TIMER+RAM+BATTERY)");
			break;
		case 0x11:
			printf("(MBC3)");
			break;
		case 0x12:
			printf("(MBC3+RAM)");
			break;
		case 0x13:
			printf("(MBC3+RAM+BATTERY)");
			break;
		case 0x14:
			printf("(MBC30+TIMER+RAM+BATTERY)");
			break;
		case 0x19:
			printf("(MBC5)");
			break;
		case 0x1A:
			printf("(MBC5+RAM)");
			break;
		case 0x1B:
			printf("(MBC5+RAM+BATTERY)");
			break;
		case 0x1C:
			printf("(MBC5+RUMBLE)");
			break;
		case 0x1D:
			printf("(MBC5+RUMBLE+RAM)");
			break;
		case 0x1E:
			printf("(MBC5+RUMBLE+RAM+BATTERY)");
			break;
		case 0x20:
			printf("(MBC6)");
			break;
		case 0x22:
			printf("(MBC7+SENSOR+RUMBLE+RAM+BATTERY)");
			break;
		case 0xFC:
			printf("(POCKET CAMERA)");
			break;
		case 0xFD:
			printf("(BANDAI TAMA5)");
			break;
		case 0xFE:
			printf("(HuC3)");
			break;
		case 0xFF:
			printf("(HuC1+RAM+BATTERY)");
			break;
		default:
			printf("(Unknown)");
			break;
		}	
	
	//Game Size
	Rom_Size = Serial_Buffer_IN[29];
	printf("\nGame Size : ");
	switch(Rom_Size)
		{
		case 0x00:
			printf("32 Ko");
			game_size=32*1024;
			break;
		case 0x01:
			printf("64 Ko");
			game_size=64*1024;
			break;
		case 0x02:
			printf("128 Ko");
			game_size=128*1024;
			break;
		case 0x03:
			printf("256 Ko ");
			game_size=256*1024;
			break;
		case 0x04:
			printf("512 Ko");
			game_size=512*1024;
			break;
		case 0x05:
			printf("1024 Ko");
			game_size=1024*1024;
			break;
		case 0x06:
			printf("2048 Ko");
			game_size=2048*1024;
			break;
		case 0x07:
			printf("4096 Ko");
			game_size=4096*1024;
			break;
		}
	
	//Backup RAM Size
	Ram_Size = Serial_Buffer_IN[30];
	printf("\nBackup RAM Size : ");
	switch(Ram_Size)
		{
		case 0x00:
			printf("N/A");
			break;
		case 0x01:
			printf("2 Ko");
			save_size=2*1024;
			break;
		case 0x02:
			printf("8 Ko");
			save_size=8*1024;
			break;
		case 0x03:
			printf("32 Ko ");
			save_size=32*1024;
			break;
		case 0x05:
			printf("64 Ko");
			save_size=64*1024;
			break;
		case 0x04:
			printf("128 Ko");
			save_size=128*1024;
			break;
		}

	// CGB and SGB


	CGB = Serial_Buffer_IN[24];
	if ( CGB  == 0xC0)
		printf("\nGame only works on GameBoy Color");
	else
		printf("\nGameBoy / GameBoy Color compatible game");

	SGB = Serial_Buffer_IN[27];
	if ( SGB  == 0x03)
		printf("\nGame have Super GameBoy enhancements\n");
	else
		printf("\nNo Super GameBoy enhancements\n");



	

//******************* */
//READ ROM command
//*********************/

		
 if ((strcmp(argv[2], "-read") == 0 )) 
{
    printf("\nDUMP ROM Command ! \n");
        
	BufferROM = (unsigned char*)malloc(game_size);
	for (k = 0; k < game_size; k++) BufferROM[k] = 0xFF;
	k=0;

	printf("Starting ROM dump...\n");   
    sp_flush(rx_port,0);

	unsigned char first_packet=0;
	FILE *myfile;
	
	for (i = 0; i < 128; i++)
		{
		Serial_Buffer_IN[i] = 0x00;
		Serial_Buffer_OUT[i] = 0x00;
		}
	i=0;
	timer_start();
	while ( k < (game_size) )
		{
    
		if ( first_packet == 0)
			{
			Serial_Buffer_OUT[0]=0x45;
			sp_blocking_write(tx_port, Serial_Buffer_OUT, 128, 200);
            sp_blocking_read(rx_port, Serial_Buffer_IN, 128, 200);
            for (i = 0; i < 128; i++)
				{
                BufferROM[k] = Serial_Buffer_IN[i];
                k++;
                }
            i=0;
            first_packet=1;
            }
        else
            {
			char data1[1];	data1[0]=0xAA;
            sp_blocking_write(tx_port, data1, 1, 200);
            sp_blocking_read(rx_port, Serial_Buffer_IN, 128, 200);
            i=0;
            for (i = 0; i < 128; i++)
				{
                BufferROM[k] = Serial_Buffer_IN[i];
                k++;
                }
            i=0;
            }
        printf("\rROM dump in progress: %ld%%",(100*k)/(game_size/1024)/1024);
        fflush(stdout);
        }

	if ( CGB  == 0xC0) myfile = fopen("dump_gbc.gbc","wb");
    else myfile = fopen("dump_gb.gb","wb");

    printf("\nFile Saved !\n");
	timer_end();
    timer_show();
	
	fwrite(BufferROM, 1,game_size, myfile);
    fclose(myfile);

	free(Serial_Buffer_IN);
	free(Serial_Buffer_OUT);	
	sp_close(port);
	sp_free_port(port);
}

//******************* */
//Backup RAM command
//*********************/

		
else if (strcmp(argv[2], "-backup") == 0) 
{
        printf("\nBackup RAM Command... \n");
		BufferSAVE = (unsigned char*)malloc(save_size);
		unsigned char first_packet=0;
	    FILE *myfile;
	

		sp_flush(rx_port,0);
	
	for (i = 0; i < 128; i++)
		{
		Serial_Buffer_IN[i] = 0x00;
		Serial_Buffer_OUT[i] = 0x00;
		}
	     i=0;

        for (k = 0; k < save_size; k++)
        {
            BufferSAVE[k] = 0xFF;
        }
        k=0;
        i=0;

		printf("Starting Backup RAM dump...\n");

		while ( k < (save_size) )
		{
    
		if ( first_packet == 0)
			{
			Serial_Buffer_OUT[0]=0x47;
			sp_blocking_write(tx_port, Serial_Buffer_OUT, 128, 200);
            first_packet=1;
            }
        else
            {
			char data1[1];	data1[0]=0xAA;
            sp_blocking_write(tx_port, data1, 1, 200);
            sp_blocking_read(rx_port, Serial_Buffer_IN, 128, 200);
            i=0;
            for (i = 0; i < 128; i++)
				{
                BufferSAVE[k] = Serial_Buffer_IN[i];
                k++;
                }
            i=0;
            }
        printf("\rBackup RAM dump in progress: %ld%%",(100*k)/(save_size/1024)/1024);
        fflush(stdout);
        }

	if ( CGB  == 0xC0) myfile = fopen("dump_gbc.sav","wb");
    else myfile = fopen("dump_gb.sav","wb");

    printf("\nFile Saved !\n");
	
	fwrite(BufferSAVE, 1,save_size, myfile);
    fclose(myfile);

	free(Serial_Buffer_IN);
	free(Serial_Buffer_OUT);	
	sp_close(port);
	sp_free_port(port);

}

//******************* */
//ERASE command
//*********************/

else if (strcmp(argv[2], "-erase") == 0) 
{
        printf("\nErase Backup RAM Command... \n");
		sp_flush(rx_port,0);
        Serial_Buffer_OUT[0]=0x49;
		sp_blocking_write(tx_port, Serial_Buffer_OUT, 128, 200);
        sp_blocking_read(rx_port, Serial_Buffer_IN, 128, 200);
		printf("\nBackup RAM Sucessfully Erased ...\n");
		free(Serial_Buffer_IN);
	    free(Serial_Buffer_OUT);	
	    sp_close(port);
	    sp_free_port(port);

}


//************************* */
//Restore Backup RAM command
//***************************/


else if (strcmp(argv[2], "-restore") == 0) 
{
        printf("\nRestore Backup RAM Command... \n");
		sp_flush(rx_port,0);
        
		// Open and buffer input save

		BufferSAVE = (unsigned char*)malloc(save_size);
	    FILE *myfile;

        printf(" Save file: ");
        scanf("%60s", dump_name);
        myfile = fopen(dump_name,"rb");
        fseek(myfile,0,SEEK_END);
        save_size = ftell(myfile);
        BufferSAVE = (unsigned char*)malloc(save_size);
        rewind(myfile);
        fread(BufferSAVE, 1, save_size, myfile);
        fclose(myfile);

		// Calculate number of ram bank

        printf(" Save file size is %ld",save_size/1024);
        printf(" Ko \n");
        nramBank=(save_size/1024)/8;
        printf(" Number of RAM Bank : %d \n",nramBank);

		// Buffer Bank

        for (k = 0; k < 32*1024; k++)
        {
            Arduino_Buffer[k] = BufferSAVE[j+k];
        }
        j=j+8*1024;
        k=0;

        // Cleaning Buffer OUT & IN

        for (k = 0; k < 128; k++)
        {
            Serial_Buffer_OUT[k]=0xFF;
            Serial_Buffer_IN[k]=0xFF;
        }
        k=0;
        i=0;
        r=1;
        while ( r < nramBank+1)
        {

			printf(" Writting Bank %d/%d... \n",r,nramBank);

            for (i = 0; i < 128; i++)
            {

				Serial_Buffer_OUT[0]=0x48; // Command number
                Serial_Buffer_OUT[4]=r-1; // Bank number
                Serial_Buffer_OUT[5]=i; // Frame number

				for (k = 0; k < 64; k++)
                {
                    Serial_Buffer_OUT[64+k] = Arduino_Buffer[k+l];
                }

				k=0;

				// Write Bank

				//printf(" Write Buffer \n");
                Serial_Buffer_OUT[0]=0x48;
				sp_blocking_write(tx_port, Serial_Buffer_OUT, 128, 200);
			//	printf("Bank Writted ! \n");

				// Wait Transmission completed command

				Serial_Buffer_IN[6] =0x00;
				while ( Serial_Buffer_IN[6] != 0xDD )
                {
                    sp_blocking_read(rx_port,Serial_Buffer_IN, 128, 200);
                }

				//printf("Buffer Writted ! \n");
				 printf("\rBackup RAM write in progress: %ld%%",(100*l)/(save_size/1024)/1024);
        fflush(stdout);

				l=l+64;
			}
            r=r+1;
            i=0;
        }


        printf("\nSRAM Sucessfully Writted ...\n");

		free(Serial_Buffer_IN);
	    free(Serial_Buffer_OUT);	
	    sp_close(port);
	    sp_free_port(port);

}


//************************* */
//Detect Flash Memory
//***************************/

else if (strcmp(argv[2], "-identify") == 0) 
{
        printf("\nDetect Flash Memory Command... \n");
		printf("Try to Detect AMD compatible Flash...\n");
		sp_flush(rx_port,0);
        Serial_Buffer_OUT[0]=0x4B;
		sp_blocking_write(tx_port, Serial_Buffer_OUT, 128, 200);
        sp_blocking_read(rx_port, Serial_Buffer_IN, 128, 200);

		printf("\nReading USB Buffer IN ...\n\n");
	int j=0;
	for (i = 0; i < 128; i++)
		{
		printf("%02X ",Serial_Buffer_IN[i]);
		j++;
		if (j==16)
			{
			printf("\n");
			j=0;
			}
		}

		manufacturer_id = Serial_Buffer_IN[0];
        device_id = Serial_Buffer_IN[1];
        flash_id = (manufacturer_id<<8) | device_id;
		printf("\n");
        switch(flash_id)
        {
        case 0x01AD :
            printf("Memory : AM29F016 \n");
            printf("Capacity : 16Mb \n");
            break;
		case 0x0141 :
            printf("Memory : AM29F032 \n");
            printf("Capacity : 32Mb \n");
            break;
        case 0x017E :
            printf("Memory : S29GL064 \n");
            printf("Capacity : 64Mb \n");
            break;
        default :
            printf("No AMD or compatible Flash detected \n");
            printf("Manufacturer ID : %02X  \n",manufacturer_id);
            printf("Device ID : %02X  \n",device_id);
            break;
        }

}

//************************* */
//Erase Flash Memory
//***************************/

else if (strcmp(argv[2], "-wipe") == 0) 
{
        printf("\nErase Flash Memory Command... \n");
		sp_flush(rx_port,0);
        Serial_Buffer_OUT[0]=0x4D;
		sp_blocking_write(tx_port, Serial_Buffer_OUT, 128, 200);
        sp_blocking_read(rx_port, Serial_Buffer_IN, 128, 90000);
		printf("Flash Memory Erased Sucessfully !...\n");

}


//************************* */
//Write Flash Memory
//***************************/


else if (strcmp(argv[2], "-write") == 0) 
{
        printf("\nWrite Flash Memory Command... \n");
		sp_flush(rx_port,0);
   
		// Open and buffer input ROM File

        printf(" ROM file: ");
        scanf("%60s", rom_name);
        myfile = fopen(rom_name,"rb");
        fseek(myfile,0,SEEK_END);
        rom_size = ftell(myfile);
        BufferROM = (unsigned char*)malloc(rom_size);
        rewind(myfile);
        fread(BufferROM, 1, rom_size, myfile);
        fclose(myfile);

        // Calculate number of rom bank

        printf(" ROM file size is %ld",rom_size/1024);
        printf(" Ko \n");
        nromBank=(rom_size/1024)/16;
       // nromBank=128;
        printf(" Number of ROM Bank : %d \n",nromBank);

        printf("Write Flash Memory...\n");
		timer_start();
        j=0;

		 // Cleaning Buffer OUT & IN

        for (k = 0; k < 128; k++)
        {
            Serial_Buffer_OUT[k]=0xFF;
            Serial_Buffer_IN[k]=0xFF;
        }
        k=0;

     //   nromBank=1;
        k=0;
        i=0;
        r=1;
		unsigned char FirstPaquet=0;

	while ( r < nromBank+1)  
    {


			  Serial_Buffer_OUT[0]=0x5A; // Command number for lock write mode
			  Serial_Buffer_OUT[4]=r-1;

			  sp_blocking_write(tx_port, Serial_Buffer_OUT, 128, 200);
				// Wait Transmission completed command

				Serial_Buffer_IN[6] =0x00;
				while ( Serial_Buffer_IN[6] != 0xDD )
                {
                    sp_blocking_read(rx_port,Serial_Buffer_IN, 128, 200);
                }

				FirstPaquet = 1;

			printf(" Writting Bank %d/%d... \n",r,nromBank);
			for (i = 0; i < 128; i++)  
            {

               /* Serial_Buffer_OUT[0]=0x4E; // Command number
                Serial_Buffer_OUT[4]=r-1; // Bank number
                Serial_Buffer_OUT[5]=i; // Frame number*/

				// Buffer Bank in serial paquet

				for (k = 0; k < 128; k++) // 64
                {
                    Serial_Buffer_OUT[k] = BufferROM[k+l];
                }

				// Write Bank
				k=0;
				//Serial_Buffer_OUT[0]=0x4E;
				sp_blocking_write(tx_port, Serial_Buffer_OUT, 128, 200);
				// Wait Transmission completed command

				Serial_Buffer_IN[6] =0x00;
				while ( Serial_Buffer_IN[6] != 0xDD )
                {
                    sp_blocking_read(rx_port,Serial_Buffer_IN, 128, 200);
                }

				 printf("\rROM write in progress: %ld%%",(100*l)/(rom_size/1024)/1024);
                 fflush(stdout);

				l=l+128;
			}      
            r=r+1;
            i=0;      
	  }
        printf("\nFlash Memory Sucessfully Writted ...\n");
		timer_end();
        timer_show();

		free(Serial_Buffer_IN);
	    free(Serial_Buffer_OUT);	
	    sp_close(port);
	    sp_free_port(port);

}

	
	// Free the array created by sp_list_ports(). 

	// sp_free_port_list(port_list);
	


	/* Note that this will also free all the sp_port structures
	 * it points to. If you want to keep one of them (e.g. to
	 * use that port in the rest of your program), take a copy
	 * of it first using sp_copy_port(). */

    
	return 0;
	}
