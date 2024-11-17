//Linux Command : gcc main.c -I./linux-deps/include -L./linux-deps/lib -o GBDuino -lserialport
//colordiff -y <(xxd ./dump_gbc.gbc) <(xxd /home/usagipc/Jeux/GameBoy_GameBoyColor/Micro_Machines_1_and_2_Twin_Turbo.gbc) > results.txt
//colordiff -y <(xxd ./dump_gb.gb) <(xxd /home/usagipc/Jeux/GameBoy_GameBoyColor/Kirby_s_Dream_Land.gb) > results.txt

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#elif _POSIX_C_SOURCE >= 199309L
#include <time.h>   // for nanosleep
#else
#include <unistd.h> // for usleep
#endif

void sleep_ms(int milliseconds){ // cross-platform sleep function
#ifdef WIN32
	Sleep(milliseconds);
#elif _POSIX_C_SOURCE >= 199309L
	struct timespec ts;
	ts.tv_sec = milliseconds / 1000;
	ts.tv_nsec = (milliseconds % 1000) * 1000000;
	nanosleep(&ts, NULL);
#else
	if (milliseconds >= 1000)
	  sleep(milliseconds / 1000);
	usleep((milliseconds % 1000) * 1000);
#endif
}

#define MAX_VERSION 	1
#define MIN_VERSION 	0

#include <libserialport.h>

/* Helper function for error handling. */
int check(enum sp_return result)
{
		/* For this example we'll just exit on any error by calling abort(). */
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
}

int main()
	{
	unsigned char Game_Name[15];
	unsigned short Global_Checksum=0;

	unsigned char *BufferROM;

	unsigned char Rom_Type=0;
	unsigned char Rom_Size=0;
	unsigned long game_size=0;
	unsigned char Ram_Size=0;
	unsigned long save_size=0;
	unsigned char CGB=0;
	unsigned char SGB=0;
	
	unsigned long i=0;
	unsigned long k=0;

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

	struct sp_port *port;
	check(sp_get_port_by_name("/dev/ttyUSB0", &port));
	printf("Port name: %s\n", sp_get_port_name(port));
	printf("Description: %s\n", sp_get_port_description(port));

	/* The port must be open to access its configuration. */
	printf("Opening port.\n");
	check(sp_open(port, SP_MODE_READ_WRITE));	
		
	printf("Setting port to 115200 8N1, no flow control.\n");
	check(sp_set_baudrate(port, 115200));
	check(sp_set_bits(port, 8));
	check(sp_set_parity(port, SP_PARITY_NONE));
	check(sp_set_stopbits(port, 1));
	check(sp_set_flowcontrol(port, SP_FLOWCONTROL_NONE));	

	/* Get the ports to send and receive on. */
	struct sp_port *tx_port = port;
	struct sp_port *rx_port = port;

	/* Allocate a buffer to send and receive data. */
	unsigned char *Serial_Buffer_IN = malloc(128+1);
	char *Serial_Buffer_OUT = malloc(128+1);

	/* We'll allow a 4 second timeout for send and receive. */
	unsigned int timeout = 4000;

	//Clear Serial Buffer
	printf("Cleaning Serial Buffer.. Please Wait...\n");
	sp_blocking_read(rx_port, "~", 128, timeout);

    for (i = 0; i < 128; i++)
		{
        Serial_Buffer_IN[i] = 0x00;
        Serial_Buffer_OUT[i] = 0x00;
		}
    i=0;

	char *data = "~";
	int size = strlen(data);

	printf("Sending '%s' (%d bytes) on port %s.\n", data, size, sp_get_port_name(tx_port));
	result = check(sp_blocking_write(tx_port, data, size, timeout));
	/* Check whether we sent all of the data. */
	if (result == size)
		printf("Sent %d bytes successfully.\n", size);
	else
		printf("Timed out, %d/%d bytes sent.\n", result, size);

	/* Try to receive the data on the other port. */
	printf("Receiving %d bytes on port %s.\n", 128, sp_get_port_name(rx_port));
	result=0;
	result=check(sp_blocking_read(rx_port, Serial_Buffer_IN, 128, timeout));
	
	//for(int c=0;c<128;c++) printf("Received '%c'.\n", buf[c]);

	printf("GB Dumper Ready \n");
	printf("Hardware Firmware version %d",Serial_Buffer_IN[2]);
	printf(".%d\n",Serial_Buffer_IN[1]);
	printf("Software Firmware version %d",MAX_VERSION);
	printf(".%d\n",MIN_VERSION);

	printf("\nReading ROM Header...\n");
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
	for (i = 0; i < 11; i++) Game_Name[i] = Serial_Buffer_IN[i+9];
	printf("Game Name : %s \n",Game_Name);

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

	//READ ROM
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
	
	while ( k < (game_size) )
		{
    
		if ( first_packet == 0)
			{
			Serial_Buffer_OUT[0]=0x45;
			sp_blocking_write(tx_port, Serial_Buffer_OUT, 128, 20);
            sp_blocking_read(rx_port, Serial_Buffer_IN, 128, 100);
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
			char data1[1];
			data1[0]=0xAA;
            sp_blocking_write(tx_port, data1, 1, 20);
            sp_blocking_read(rx_port, Serial_Buffer_IN, 128, 100);
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
	
	fwrite(BufferROM, 1,game_size, myfile);
    fclose(myfile);

	free(Serial_Buffer_IN);
	free(Serial_Buffer_OUT);	
	check(sp_close(port));
	sp_free_port(port);
	
	/* Free the array created by sp_list_ports(). */
	sp_free_port_list(port_list);
	/* Note that this will also free all the sp_port structures
	 * it points to. If you want to keep one of them (e.g. to
	 * use that port in the rest of your program), take a copy
	 * of it first using sp_copy_port(). */
	return 0;
	}
