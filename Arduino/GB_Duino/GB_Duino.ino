/*  GB_Duino
 *  GameBoy Shield for Arduino Mega 2560
 *  X-death 12/2024
 */

/*To Do :
Fix Mysic Quest Backup RAM
Fix DQ Monsters JAP
*/
/*To Do :
Fix Mysic Quest Backup RAM
Fix DQ Monsters JAP
*/

// GB Dumper PINOUT
/*
 const int GB_DATA_D0 = 37;  // PC0
 const int GB_DATA_D1 = 36;
 const int GB_DATA_D2 = 35;
 const int GB_DATA_D3 = 34;
 const int GB_DATA_D4 = 33;
 const int GB_DATA_D5 = 32;
 const int GB_DATA_D6 = 31;
 const int GB_DATA_D7 = 30; // PC7

 const int GB_ADDR_A0 = 0;   // ADC0
 const int GB_ADDR_A1 = 1;
 const int GB_ADDR_A2 = 2;
 const int GB_ADDR_A3 = 3;
 const int GB_ADDR_A4 = 4;
 const int GB_ADDR_A5 = 5;
 const int GB_ADDR_A6 = 6;
 const int GB_ADDR_A7 = 7;  // ADC7
 const int GB_ADDR_A8 = 8;
 const int GB_ADDR_A9 = 9;
 const int GB_ADDR_A10 = 10;
 const int GB_ADDR_A11 = 11;
 const int GB_ADDR_A12 = 12;
 const int GB_ADDR_A13 = 13;
 const int GB_ADDR_A14 = 14;
 const int GB_ADDR_A15 = 15;  // ADC15
*/

// Specific GB Dumper Macro

#define SET_WR_LOW    PORTH &= ~(1 << 5); // Wr '0'
#define SET_WR_HI     PORTH |= (1 << 5);  // Wr '1'
#define SET_RD_LOW    PORTH &= ~(1 << 3); // Rd '0' 
#define SET_RD_HI     PORTH |= (1 << 3);  // Rd '1'
#define SET_CS_LOW    PORTH &= ~(1 << 6); // Cs '0' 
#define SET_CS_HI     PORTH |= (1 << 6);  // Cs '1'
#define DX_AS_INPUT   DDRC = 0x00;
#define DX_AS_OUTPUT  DDRC = 0xFF;

// Specific GB Dumper Var

unsigned char CurrentByte=0;
unsigned short packet_crc=0;
unsigned char i=0;
unsigned char Packet_ready=0;
unsigned char buffer_temp[64];
unsigned char Serial_init=0;
unsigned char Serial_count=0;
unsigned char Serial_received=0;
unsigned char rx_byte = 0;
unsigned char lock=0;
unsigned char Init_Mapper = 0;
unsigned long Addr_Counter=0;

// Specific GB Flasher Var

// Version

#define MAX_VERSION   1
#define MIN_VERSION   2
#define PACKETSIZE    72   // Size of packet to Send


unsigned char packet[PACKETSIZE];     // Var to create buffer
unsigned char logo_correct;
unsigned char first_packet=0;
unsigned char start_packet=0;
unsigned char operation=0;
unsigned short crc16;
unsigned char mapper =0;
unsigned char currBank=0;
char game_name[17];
unsigned char RTC_DATA[12];
unsigned char init_bank=0;
unsigned char Manufacturer_ID=0;
unsigned char Device_ID=0;
unsigned char octet=0;


// New var

volatile unsigned char data_avail = 0; // flag to be set by interrupt routine
volatile unsigned char bytes_received = 0;
unsigned char waiting_for_ack = 0;
volatile unsigned char response_code = 0;
unsigned char control_char = 0;
unsigned char packet_type=0;
unsigned char op_type =0;
unsigned char mapper_init=0;

unsigned char tx_packet[128];          // but it's just easier to keep them separate
unsigned char header_received = 0 ;
unsigned char header_title[16];
unsigned char mbc_type=0;
unsigned char mbc_num=0;
unsigned char rom_size=0;
unsigned char ram_size=0;
unsigned int num_rom_banks=0;
unsigned char num_ram_banks =0;
unsigned char cgb=0;
unsigned char sgb=0;
unsigned char header_checksum=0;
unsigned char header_checksum_valid=0;
unsigned short global_checksum=0;
unsigned char logo_is_valid=0;
unsigned char packet_temp[64];
unsigned char num_frames;
unsigned long k=0;
int incomingByte = 0; // for incoming serial data
const int BUFFER_SIZE = 128;
unsigned char Arduino_Buffer[BUFFER_SIZE];
unsigned long l=0;
unsigned long m=0;
unsigned char fix=0;
unsigned char Cur_Bank=0;
unsigned long retry=0;


#define MAX_VERSION   1
#define MIN_VERSION   0

#define VERSION_MAJOR 0x00
#define VERSION_MINOR 0x10
#define FRAMESIZE  64

/* packet types */

#define ACK  0xAA
#define NAK  0xF0
#define DATA 0x55
#define END  0x0F

#define CONFIG 0x00
#define NORMAL_DATA 0x01
#define LAST_DATA 0x02
#define ERASE 0x03
#define STATUS 0x04

#define RESERVED 0x00
#define NREAD_ID 0x00
#define READ_ID 0x01

/* operations */
#define RROM 0x00
#define RRAM 0x01
#define WROM 0x02
#define WRAM 0x03
#define EFLA 0x00
#define ERAM 0x01

#define ENABLE  1
#define DISABLE 0

#define ROM 0
#define RAM 1

/* Mapper Config supported other value return Unknown packet[28] */

#define ROMONLY 0
#define MBC_NONE 0
#define MBC1 1
#define MBC2 2
#define MBC3 3
#define MBC4 4
#define MBC5 5
#define RUMBLE 6
#define POCKET_CAMERA 10
#define MBC_UNKNOWN 99

unsigned char Nintendo_Logo[] =   // nintendo logo
{
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};


/*
 *  00  ROM Only (32Ko)
 *  01  ROM + MBC1
 *  02  ROM+MBC1+RAM
 *  03  ROM+MBC1+RAM+BATT
 *  05  ROM+MBC2
 *  06  ROM+MBC2+BATTERY
 *  08  ROM+RAM
 *  09  ROM+RAM+BATTERY
 *  10  ROM+MBC3+TIMER+RAM+BAT
 *  11  ROM+MBC3
 *  12  ROM+MBC3+RAM
 *  13  ROM+MBC3+RAM+BATT
 *  19  ROM + MBC5
 *  1A  ROM+MBC5+RAM
 *  1B  ROM+MBC5+RAM+BATT
 *  1C  ROM+MBC5+RUMBLE
 *  1D  ROM+MBC5+RUMBLE+SRAM
 *  1E  ROM+MBC5+RUMBLE+SRAM+BATT
 *  1F  Pocket Camera
 *  FD  Bandai TAMA5
 *  FE  Hudson HuC-3
 */

/* Supported ROM Size packet[29] */

/*
 *  00  32 Ko
 *  01  64 Ko
 *  02  128 Ko
 *  03  256 Ko
 *  04  512 Ko
 *  05  1024 Ko
 *  06  2048 Ko
 /*
  * desc_t rom_sizes[] = {
  {0x00, "32KB"}, {0x01, "64KB"}, {0x02, "128KB"}, {0x03, "256KB"},
  {0x04, "512KB"}, {0x05, "1MB"}, {0x06, "2MB"}, {0x52, "1.1MB"},
  {0x53, "1.2MB"}, {0x54, "1.5MB"}
};

 /* Supported RAM Size packet[30] */

/*
 *  00  0 Ko
 *  01  2 Ko
 *  02  8 Ko
 *  03  32 Ko
 *  04  128 Ko
 *
desc_t ram_sizes[] = {
  {0x00, "0KB"}, {0x01, "2KB"}, {0x02, "8KB"}, {0x03, "32KB"}, {0x04, "128KB"}
};

  */

// Specific Flasher Function



unsigned char verify_logo()
{
    for (unsigned char i = 0; i < sizeof(Nintendo_Logo); i++)
        if(readByte_GB(0x0104+i) != Nintendo_Logo[i])
            return 0;
    return 1;
}

// Specific GB Dumper Function

void SetDataInput(void)
{
    DDRC = 0x00;
}

void SetDataOutput(void)
{
    DDRC = 0xFF;
}

void DirectWrite8(unsigned char value)
{
    PORTC = value;
}

unsigned char DirectRead8(void)
{
    unsigned char value=0;
    value = PORTC;
    return value;
}

void SetAddress(unsigned short Addr)
{
    // SetAddress
    PORTF = Addr & 0xFF;
    PORTK = (Addr >> 8) & 0xFF;
}

void SetWr(unsigned char value)
{
    if (value == 0)
    {
        PORTH &= ~(1 << 5);
    }
    else
    {
        PORTH |= (1 << 5);
    }
}

void SetRd(unsigned char value)
{
    if (value == 0)
    {
        PORTH &= ~(1 << 3);
    }
    else
    {
        PORTH |= (1 << 3);
    }
}

void SetCs(unsigned char value)
{
    if (value == 0)
    {
        PORTH &= ~(1 << 6);
    }
    else
    {
        PORTH |=  (1 << 6);
    }
}

void enable_cart(void)
{
   SetCs(0);
   SetRd(0);
}

void disable_cart(void)
{
     SetCs(1);
     SetRd(1);
}

void wait(unsigned char n)
{
  for(unsigned char a=0;a<n;n++)
  {
    __asm__("nop");
  }
}

/******************************************
  Low level functions
*****************************************/
byte readByte_GB(word myAddress)
{
    // Set address
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;
    // Switch data pins to input
    DDRC = 0x00;
    // Enable pullups
    PORTC = 0xFF;

    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

    // Switch RD(PH6) to LOW
    PORTH &= ~(1 << 3);

    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

    // Read
    byte tempByte;
    tempByte = 0;
    tempByte = PINC;

    // Switch RD(PH6) to HIGH
    PORTH |= (1 << 3);

    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

    return tempByte;
}

byte readByteSRAM_GB(word myAddress)
{
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;
    // Switch data pins to input
    DDRC = 0x00;
    // Enable pullups
    PORTC = 0xFF;

    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    // Pull CS(PH3) CLK(PH1)(for FRAM MOD) LOW
    PORTH &= ~(1 << 6);
    // Pull RD(PH6) LOW
    PORTH &= ~(1 << 3);

    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    // Read
    byte tempByte = PINC;

    // Pull RD(PH6) HIGH
    PORTH |= (1 << 3);
    // Pull CS(PH3) HIGH
    PORTH |= (1 << 6);

    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    return tempByte;
}
/*
void writeByte_GB(int myAddress, byte myData) 
{
  // Set address
  PORTF = myAddress & 0xFF;         //ADR LSB - adr max = FFFF
  PORTK = (myAddress >> 8) & 0xFF;  //ADR MSB
  // Set data
  PORTC = myData;
  DX_AS_OUTPUT
  wait(4); // A VERIFIER SI vraiment utile... 
  SET_WR_LOW
  wait(4);   // obligatoire //
  SET_WR_HI
  wait(4); // A VERIFIER SI vraiment utile... 
  DX_AS_INPUT
  PORTC = 0xFF; // Enable pullups
}
*/


void writeByte_GB(int myAddress, byte myData)
{
    // Set address
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;
    // Set data
    PORTC = myData;
    // Switch data pins to output
    DDRC = 0xFF;

    // Wait till output is stable
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

    // Pull WR(PH5) low
    PORTH &= ~(1 << 5);

    // Leave WR low for at least 60ns
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

    // Pull WR(PH5) HIGH
    PORTH |= (1 << 5);
    // Leave WR high for at least 50ns
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");


    // Switch data pins to input
    DDRC = 0x00;
    // Enable pullups
    PORTC = 0xFF;
}


void writeByte_GB_SRAM(int myAddress, byte myData)
{
    // Set address
    PORTF = myAddress & 0xFF;
    PORTK = (myAddress >> 8) & 0xFF;
    // Set data
    PORTC = myData;
    // Switch data pins to output
    DDRC = 0xFF;

    // Wait till output is stable
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

    // Pull WR(PH5) low
    PORTH &= ~(1 << 5);

    // Pull CE low
    PORTH &= ~(1 << 6);

    // Leave WR low for at least 60ns
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

    // Pull WR(PH5) HIGH
    PORTH |= (1 << 5);

    // Pull CE High
    PORTH |=  (1 << 6);

    // Leave WR high for at least 50ns
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");


    // Switch data pins to input
    DDRC = 0x00;
    // Enable pullups
    PORTC = 0xFF;
}

/******************************************
  CFI Flash function
*****************************************/

void startCFIMode(void)

{
    writeByte_GB(0xAAAA, 0xf0); //x8  mode reset command
    delay(100);
    writeByte_GB(0xAAAA, 0xf0); //Double reset to get out of possible Autoselect + CFI mode
    delay(100);
    writeByte_GB(0xAA, 0x98);  //x8 CFI Query command
}

/* Identify the different flash chips.
   Sets the global variables flashBanks, flashX16Mode and flashSwitchLastBits
*/
void identifyCFI_GB() 
{

   // Reset flash
  // Switch data pins to output
  DDRC = 0xFF;
  PORTH &= ~(1 << 5); // Wr '0'
  writeByte_GB(0x6000, 0); // Set ROM Mode
  writeByte_GB(0x2000, 0); // Set Bank to 0
  writeByte_GB(0x3000, 0);

  startCFIMode();

      // Switch data pins to input
    DDRC = 0x00;

    char cfiQRYx8[7];
    sprintf(cfiQRYx8, "%02X%02X%02X", readByte_GB(0x20), readByte_GB(0x22), readByte_GB(0x24));
  if (strcmp(cfiQRYx8, "515259") == 0) 
  { // QRY in x8 mode
    Serial.write("CFI x8 Mode Started Sucessfully !\n");
  } 

  unsigned char flash_cfi_size=0;
  flash_cfi_size = readByte_GB(0x4E);
  if ( flash_cfi_size == 0x17)
  {
    Serial.write("Flash Size 64Mb \n");
  }

  // Reset flash
  writeByte_GB(0xAAA, 0xf0); //x8  mode reset command
  
 /* unsigned char Query1=readByte_GB(0x80);
  unsigned char Query2=readByte_GB(0x82);
  unsigned char Query3=readByte_GB(0x84);

  Serial.println(Query1,HEX);
  Serial.println(Query2,HEX);
  Serial.println(Query3,HEX);*/
  
  
  

    // Reset flash
  writeByte_GB(0xAAA, 0xf0); //x8  mode reset command
  delay(100);

}


/******************************************
  Flasher functions
*****************************************/

// New Function

unsigned char identify_mbc(unsigned char type)

{

    switch (type)
    {
    case 0x00:
        return MBC_NONE;
        break;
    case 0x01:
        return MBC1;
        break;
    case 0x02:
        return MBC1;
        break;
    case 0x03:
        return MBC1;
        break;
    case 0x05:
        return MBC2;
        break;
    case 0x06:
        return MBC2;
        break;
    case 0x08:
        return MBC_UNKNOWN;
        break;
    case 0x09:
        return MBC_UNKNOWN;
        break;
    case 0x0B:
        return MBC_UNKNOWN;
        break;
    case 0x0C:
        return MBC_UNKNOWN;
        break;
    case 0x0D:
        return MBC_UNKNOWN;
        break;
    case 0x0F:
        return MBC3;
        break;
    case 0x10:
        return MBC3;
        break;
    case 0x11:
        return MBC3;
        break;
    case 0x12:
        return MBC3;
        break;
    case 0x13:
        return MBC3;
        break;
    case 0x15:
        return MBC4;
        break;
    case 0x16:
        return MBC4;
        break;
    case 0x17:
        return MBC4;
        break;
    case 0x19:
        return MBC5;
        break;
    case 0x1A:
        return MBC5;
        break;
    case 0x1B:
        return MBC5;
        break;
    case 0x1C:
        return MBC5;
        break;
    case 0x1D:
        return MBC5;
        break;
    case 0x1E:
        return MBC5;
        break;
    case 0xFC:
        return POCKET_CAMERA;
        break;
    case 0xFD:
        return MBC_UNKNOWN;
        break;
    case 0xFE:
        return MBC_UNKNOWN;
        break;
    case 0xFF:
        return MBC_UNKNOWN;
        break;
    }

    return MBC_UNKNOWN;

}

void rd_header()
{

    for (unsigned char i = 0; i < 16; i++)
        header_title[i] = readByte_GB(i + 0x0134);
    cgb = header_title[15] & 0x80;
    sgb = readByte_GB(0x0146);
    mbc_type = readByte_GB(0x0147); // Identify cartridge MBC
    mbc_num = identify_mbc(mbc_type); // Return correct mapper info in GB Cart format
    rom_size = readByte_GB(0x0148);

  /*  mbc_type = 0x00;
    rom_size = 0x00;
    mbc_num = MBC_NONE;*/

    if (rom_size == 0x00)
    {
        num_rom_banks = 2;   // 32 Kb
    }
    if (rom_size == 0x01)
    {
        num_rom_banks = 4;   // 64 Kb
    }
    if (rom_size == 0x02)
    {
        num_rom_banks = 8;   // 128 Kb
    }
    if (rom_size == 0x03)
    {
        num_rom_banks = 16;   // 256 Kb
    }
    if (rom_size == 0x04)
    {
        num_rom_banks = 32;   // 512 Kb
    }
    if (rom_size == 0x05)
    {
        num_rom_banks = 64;   // 1024 Kb
    }
    if (rom_size == 0x06)
    {
        num_rom_banks = 128;   // 2048 Kb
    }
    if (rom_size == 0x07)
    {
        num_rom_banks = 256;   // 4096 Kb LCDZ Fix
    }
    //num_rom_banks = 16;

    /*

    Specifies the ROM Size of the cartridge. Typically calculated as "32KB shl N".

    00h -  32KByte (no ROM banking)
    01h -  64KByte (4 banks)
    02h - 128KByte (8 banks)
    03h - 256KByte (16 banks)
    04h - 512KByte (32 banks)
    05h -   1MByte (64 banks)  - only 63 banks used by MBC1
    06h -   2MByte (128 banks) - only 125 banks used by MBC1
    07h -   4MByte (256 banks)
    08h -   8MByte (512 banks)
    52h - 1.1MByte (72 banks)
    53h - 1.2MByte (80 banks)
    54h - 1.5MByte (96 banks)

    */

    /*
     Available RAM sizes are: 2KByte (at A000-A7FF), 8KByte (at A000-BFFF), and 32KByte (in form of four 8K banks at A000-BFFF).
    */
    ram_size = readByte_GB(0x0149);
    if (ram_size == 0x00)
    {
        num_ram_banks = 0;   // No Backup RAM
    }
    if (ram_size == 0x01)
    {
        num_ram_banks = 1;   // 2 Ko
    }
    if (ram_size == 0x02)
    {
        num_ram_banks = 2;   // 8 Ko
    }
    if (ram_size == 0x03)
    {
        num_ram_banks = 4;   // 32 Ko
    }
    if (ram_size == 0x04)
    {
        num_ram_banks = 8;   // 64 Ko
    }


    header_checksum = readByte_GB(0x014D);
    // verify header checksum
    short check = 0;
    for(u16 addr = 0x0134; addr <= 0x014C; addr++)
        check = check - readByte_GB(addr) - 1;
    header_checksum_valid = ((check & 0xFF) == header_checksum);
    global_checksum = 256 * readByte_GB(0x014E) + readByte_GB(0x014F);
    header_received = 1;


// Special Mapper init

    if (mbc_num == MBC1 && num_ram_banks == 2) // MBC1 with 8k RAM
    {
        writeByte_GB(0x0000,0x0A);
        writeByte_GB(0x6000,0x00);
        num_frames = 64;
    }

    if (mbc_num == MBC1 && num_ram_banks == 4) // MBC1 with 32k RAM
    {
        writeByte_GB(0x0000,0x0A);
        writeByte_GB(0x6000,0x01);
        num_frames = 128;
    }

    /*To Do for MBC2*/


}

void send_header_info()
{
    Serial.write(DATA); // data
    Serial.write(0x00); // ?
    Serial.write(MAX_VERSION); // hw version
    Serial.write(MIN_VERSION); // hw version
    Serial.write(0x00); // mfr id
    Serial.write(0x00); // chip id
    Serial.write(0x00); // bbl flag
    Serial.write(0x00); // ?
    Serial.write(verify_logo()); // Logo Correct
    Serial.write(header_title, sizeof(header_title));
    Serial.write(0x00);
    Serial.write(0x00);
    Serial.write(sgb); // sgb flag
    Serial.write(mbc_type); // mbc type
    Serial.write(rom_size); // rom size
    Serial.write(ram_size); // ram size
    Serial.write(0x00); // ?
    Serial.write(0x00); // ?
    Serial.write(header_checksum_valid); // ?
    Serial.write(header_checksum); // ?
    Serial.write((unsigned char)((global_checksum & 0xFF00) >> 8)); // global checksum (upper)
    Serial.write((unsigned char)(global_checksum & 0x00FF)); // global checksum (lower)
    for (unsigned char i = 37; i < 128; i++)
        Serial.write(0x00); // fill the rest of the packet
}

void select_rom_bank(u16 bank) // TODO implement other MBCs
{

    if (mbc_num == MBC1 && !(bank%0x20))
        return; // bank 0 is at 0x0000-0x3FFF; 20h, 40h, 60h not supported
    if (mbc_num == MBC3 && !bank)
        return;  // bank 0 is at 0x0000-0x3FFF

    writeByte_GB(0x2000,bank&0x00FF);

    if (mbc_num == MBC2 && bank < 0x1F) // aka bank 31
    {
        writeByte_GB(0x2100,bank);
        writeByte_GB(0x0000, 0x0A); // Enable RAM
    }

    if (mbc_num == MBC2 && bank > 0x1F) // aka bank 31
    {
        writeByte_GB(0x2100,bank);
    }

    if (mbc_num == MBC1 && bank < 0x1F) // aka bank 31
    {
        writeByte_GB(0x2100,bank);
    }


    if (mbc_num == MBC1 && bank > 0x1F) // aka bank 31
    {

        //writeByte_GB(0x6000, 0); // Set ROM Mode
        writeByte_GB(0x2000, bank & 0x1F);
        writeByte_GB(0x4000, bank >> 5);
        /*writeByte_GB(0x6000,0x00);
        writeByte_GB(0x4000,(bank >> 5) & 0x0003);*/
    }

    else if (mbc_num == MBC5 && bank > 0x00FF) // aka bank 255
    {
        writeByte_GB(0x3000,1);
    }

}

void select_ram_bank(unsigned char bank) // TODO implement other MBCs
{
    if (mbc_num == MBC1)
    {
        if (bank)
            writeByte_GB(0x6000, 1);
        else
            return; // default mode is rom banking, with max 8k ram (single bank)
    }

    if (mbc_num == MBC5)
    {
        if ( mapper_init == 0 )
        {
            writeByte_GB(0x0000, 0x0A);    // Enable RAM
            mapper_init=1;
        }
    }
    writeByte_GB(0x4000,bank);
}



void send_rom_packet(unsigned short bank, unsigned char frame) // 256 frames per 16k bank
{

    if (!bank)
    {
        for (unsigned char i = 0; i < 128; i++)
            tx_packet[i] = readByte_GB(i+(128*frame));
    }
    else
    {
        select_rom_bank(bank);
        for (unsigned char i = 0; i < 128; i++)
            tx_packet[i] = readByte_GB(0x4000+(128*frame)+i);
    }

    for (unsigned char i = 0; i < 128; i++)
        Serial.write(tx_packet[i]);

}

void send_ram_packet(unsigned char bank, unsigned char frame) // 128 frames per 8k bank
{
    tx_packet[0] = DATA;
    if (rom_size == 1 && frame == 31) // only have 2k to send
        tx_packet[1] = LAST_DATA;
    else if (bank == num_ram_banks-1 && frame == 127)
        tx_packet[1] = LAST_DATA;
    else
        tx_packet[1] = NORMAL_DATA;
    tx_packet[2] = 0x00;
    tx_packet[3] = frame;
    tx_packet[4] = 0x00;
    tx_packet[5] = bank;
// select_ram_bank(bank);

    for (unsigned char i = 0; i < 64 ; i++)
        tx_packet[6+i] = readByteSRAM_GB(0xA000+(FRAMESIZE*frame)+i);
// for (unsigned char i = 0; i < FRAMESIZE; i++) tx_packet[6+i] = 0xAA;


    /* u16 crc = generate_crc16(tx_packet);

     tx_packet[PACKETSIZE-2] = (unsigned char)((crc & 0xFF00) >> 8);
     tx_packet[PACKETSIZE-1] = (unsigned char)(crc & 0x00FF);*/

    //for (unsigned char i = 0; i < PACKETSIZE; i++) Serial.write(tx_packet[i]);
}

void fill_ram(unsigned char byte)
{
    set_ram_state(ENABLE);
//  u08 num_frames = 8192 / FRAMESIZE; // frames per page of ram
    //if (header.rom_size == 1) num_frames /= 4; // only 2k ram
    for(unsigned char bank = 0; bank < num_ram_banks; bank++)
    {
        select_ram_bank(bank);
        for(unsigned char frame = 0; frame < num_frames; frame++)
            //for(unsigned char i = 0; i < FRAMESIZE; i++)
            for (unsigned char i = 0; i < FRAMESIZE; i++)
                writeByte_GB(0xA000+i+(FRAMESIZE*frame),byte);
    }
    set_ram_state(DISABLE);
}

// NOTE: This will also enable the RTC registers on MBC3

void set_ram_state(unsigned char state)
{
    if (state == 1)
    {
        writeByte_GB(0x0000,0x0A);   // enable
    }
    else
        writeByte_GB(0x0000,0x00);     // disable
}

/*

// NOTE: This will also enable the RTC registers on MBC3.
void set_ram_state(u08 state)
{
  u16 addr = 0x0000; // can be anything from 0x0000 to 0x1FFF. MBC doesn't care.
  L_ADDR_PORT = (u08)addr;
  U_ADDR_PORT = (u08)(addr >> 8);
  DATA_DDR = 0xFF; // output
  if (state) DATA_PORT = 0x0A; // enable
  else (DATA_PORT) = 0x00;     // disable
  cbi(CTRL_PORT, WR);
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");
  sbi(CTRL_PORT, WR);
}
*/

//////////// Flash Memory Function ////////////

void writeFlash8(int address, int byte)
{
    SetAddress(address);
    SetDataOutput();
    DirectWrite8(byte);
    SetCs(0);
    SetWr(0);
    delay(32);
    SetCs(1);
    SetWr(1);
    SetDataInput();
}

void reset_command()
{
     SetDataOutput();
    writeFlash8(0x5555,0xAA);
    writeFlash8(0x2AAA,0x55);
    writeFlash8(0x5555,0xF0);
    delay(32);
    /*SetDataOutput();
    writeByte_GB(0xAAA,0xF0);
    delay(32);*/
}

void EraseFlash()
{

    unsigned char i=0;
    writeByte_GB(0xAAA,0xAA);
    writeByte_GB(0x555,0x55);
    writeByte_GB(0xAAA,0x80);

    for (i=0; i<64; i++)
    {
        delay(1700000);
    }

}

void infosId()
{

    /*SetDataOutput();
    writeByte_GB(0xAAA,0xF0);
    delay(100);
    writeByte_GB(0xAAA,0xF0);
    delay(16);
    SetDataOutput();
    writeByte_GB(0xAAA,0xAA);
    writeByte_GB(0x555,0x55);
    writeByte_GB(0xAAA,0x90);
        // Switch data pins to input
    DDRC = 0x00;
    Manufacturer_ID = readByte_GB(0);
        // Switch data pins to output
    DDRC = 0xFF;
    writeByte_GB(0xAAA,0xF0);
    delay(16);
    writeByte_GB(0xAAA,0xAA);
    writeByte_GB(0x555,0x55);
    writeByte_GB(0xAAA,0x90);
            // Switch data pins to input
    DDRC = 0x00;
    Device_ID = readByte_GB(2);

    delay(16);*/


  /////// INFO ID for x8 only device //////////

    // Reset flash
    SetDataOutput();
    writeFlash8(0x5555,0xF0);
    delay(100);

    delay(16);
    SetDataOutput();
    writeFlash8(0x5555,0xAA);
    writeFlash8(0x2AAA,0x55);
    writeFlash8(0x5555,0x90);
    SetAddress(0);
    SetCs(0);
    delay(16);

// Switch data pins to input
    DDRC = 0x00;
    // Enable pullups
    PORTC = 0xFF;

    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

    // Switch RD(PH6) to LOW
    PORTH &= ~(1 << 3);

    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

    // Read
    Manufacturer_ID = PINC;

    // Switch RD(PH6) to HIGH
    PORTH |= (1 << 3);

    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

    Device_ID = readByte_GB(1);
    SetCs(1);
   

}



//////////////////////////////////////////////

void setup()
{

    // Init Serial

    // start serial port at 38400 bps:

    Serial.begin(38400);

    // Cleanup Serial Input buffer

    for (l = 0; l < 128; l++)
    {
        Arduino_Buffer[l]=0xFF;
    }
    l=0;



    // Set Data Pins to Output
    SetDataOutput();
    // Set Address Pins to Output
    //A0-A7
    DDRF = 0xFF;
    //A8-A15
    DDRK = 0xFF;

// Set Control Pins to Output RST(PH0) CLK(PH1) CS(PH3) WR(PH5) RD(PH6)
    DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 5) | (1 << 6);
    // Output a high signal on all pins, pins are active low therefore everything is disabled now
    PORTH |= (1 << 3) | (1 << 5) | (1 << 6);
    // Output a low signal on CLK(PH1) to disable writing GB Camera RAM
    // Output a low signal on RST(PH0) to initialize MMC correctly
    PORTH &= ~((1 << 0) | (1 << 1));

    // Set Data Pins (D0-D7) to Input
    DDRC = 0x00;
    // Enable Internal Pullups
// PORTC = 0xFF;

    // RST(PH0) to H
     PORTH |= (1 << 0);

      // Output a low signal on RST(PH0) to initialize MMC correctly
      PORTH &= ~((1 << 0));

          // RST(PH0) to H
     PORTH |= (1 << 0);

       delay(400);

       // CFI TEST
      // identifyCFI_GB();
      // while(1);

/*
          // RST(PH0) to H
     PORTH |= (1 << 0);

     // Clean RTC Data buffer

   for (unsigned char i = 0; i < 12; i++)
    {
        RTC_DATA[i]=0xFF;
    }
    i=0;    

    // Dummy read some byte to increase compatibility with Homebrew clone MBC

    for (unsigned char i = 0; i < 128; i++)
    {
        readByte_GB(i);
    }
    i=0;
    l=0;

    // reset_command();
    //  EraseFlash();

    // Test set audio pin to output
    
   // PORTH |= (1 << 4); // Audio '1'
    //PORTH &= ~(1 << 4); // Audio '0'


    // RTC Test

  /*  Serial.write("Pokemon GOLD RTC Test : \n\n");

      // Enable RTC Register
         writeByte_GB(0x0000, 0x0A);
         delay(4);

while(1)
{
         // Latch current time to RTC Register

         writeByte_GB(0x6000, 0);
         delay(4);
         writeByte_GB(0x6000, 1);
        writeByte_GB(0x4000, 0x08);
        RTC_DATA[0]= readByteSRAM_GB(0xA000);
        Serial.write("Value of Second RTC Register :  ");
        Serial.println(RTC_DATA[0],HEX);
        writeByte_GB(0x4000, 0x09);
        RTC_DATA[1]= readByteSRAM_GB(0xA000);
        Serial.write("Value of minute RTC Register :  ");
        Serial.println(RTC_DATA[1],HEX);
        writeByte_GB(0x4000, 0x0A);
        RTC_DATA[2]= readByteSRAM_GB(0xA000);
        Serial.write("Value of hour RTC Register :  ");
        Serial.println(RTC_DATA[2],HEX);
        Serial.write("\n");

        delay(500);
}

         

    while(1);*/



}

void loop()
{


    /* if (Serial.available() > 0 )
     {
         rx_byte = Serial.read();
         Serial_received = 1;
     }*/

    while ( Serial_received == 0)
    {
        // read the incoming byte:
        int rxlen = Serial.available(); // number of bytes available in Serial buffer
        if (rxlen > 0)
        {
            int rlen; // number of bytes to read
            if (rxlen > 128) // check if the data exceeds the buffer size
                rlen = 128;    // if yes, read BUFFER_SIZE bytes. The remaining will be read in the next time
            else
                rlen = rxlen;
            // read the incoming bytes:
            rlen = Serial.readBytes(Arduino_Buffer, 128);
            Serial_received = 1;
            rlen=0;
            rxlen=0;
        }
        //if (rlen == 128) {Serial_received = 1;rlen=0; }
    }

    if ( Arduino_Buffer[0] == 0x7E && Serial_received == 1) // Send Header

    {
        rd_header();
        send_header_info();
        /*for (u16 frame = 0; frame <= 127; frame++)
          {
            Serial.write(0xAA);
          }*/
        Serial_received = 0;
    }

    if ( Arduino_Buffer[0] == 0x45 && Serial_received == 1) // Dump ROM

    {
        for (u16 bank = 0; bank < num_rom_banks; bank++)
        {
            if (mbc_num == MBC1 && bank > 0 && !(bank%0x20))
            {
                continue;
            }
            for (u16 frame = 0; frame <= 127; frame++) // 16k size rom bank (16*1024)/128 = 128
            {
                send_rom_packet(bank, (unsigned char)frame);
                waiting_for_ack = 1;
                while ( Serial.read() != ACK)
                    asm("nop"); // Wait
                response_code = 0x00;
                waiting_for_ack = 0;
            }

        }
        Serial_received = 0;
        rx_byte =0;
    }

    if ( Arduino_Buffer[0] == 0x47 && Serial_received == 1) // Dump RAM

    {
        set_ram_state(ENABLE);
        SetCs(0);
        SetWr(1);
        num_ram_banks=4;


        //if (header.ram_size == 1) num_frames = 32; // accomodate 2k ram
        //  else num_frames = 128; // all other sizes have 8k banks
        for (unsigned char bank = 0; bank < num_ram_banks; bank++)
        {
            select_ram_bank(bank);
            for (unsigned char frame = 0; frame < 64; frame++) // (8*1024)/128=64
            {
                //send_ram_packet(bank, frame);
                //for (unsigned char i = 0; i < 64; i++) tx_packet[6+i] = readByteSRAM_GB(0xA000+(64*frame)+i);
                for (unsigned char i = 0; i < 128 ; i++)
                    tx_packet[i] = readByteSRAM_GB(0xA000+(128*frame)+i);
                for (unsigned char i = 0; i < 128; i++)
                    Serial.write(tx_packet[i]);
                while ( Serial.read() != ACK)
                    asm("nop"); // Wait
            }
        }
        set_ram_state(DISABLE);


        Serial_received = 0;
        rx_byte =0;
    }



    if ( Arduino_Buffer[0] == 0x48 && Serial_received == 1) // WRITE RAM

    {

        if ( Init_Mapper == 0)
        {

            SetAddress(0x00);
            SetAddress(0x6000);
            SetDataOutput();
            PORTC = 0x00;
            PORTH &= ~(1 << 5); // Wr '0'
            delay(32);
            PORTH |= (1 << 5); // Wr '1'

            SetAddress(0x0);
            SetDataOutput();
            DirectWrite8(0x0A);
            PORTH &= ~(1 << 5); // Wr '0'
            delay(32);
            PORTH |= (1 << 5); // Wr '1'

            Init_Mapper =1;


        }

        // Prepare Write Mode

        SetRd(1);
        SetCs(1);
        SetAddress(0x8000); // Set A15 to "1"

        k=0;

        // Prepare Buffer

        select_ram_bank(Arduino_Buffer[4]); // assign bank
        SetCs(0);
        delay(5);
        l=Arduino_Buffer[5];
        l=l*64;
        // Write Buffer


        for (k = 0; k < 64; k++)
        {
            writeByte_GB_SRAM(0xA000+l+k,Arduino_Buffer[k+64]);
            //writeByte_GB_SRAM(0xA000+k+l,Arduino_Buffer[k]);


        }

        //writeByte_GB_SRAM(0xA000+l+k-1,Arduino_Buffer[6]); // Stupid Fix another thanks to serial bullshit
        //if ( fix == 0){writeByte_GB(0xA000+8192,Arduino_Buffer[4]);fix=1;}
        k=0;
        delay(5);


        // Send Transfert Completed command

        // Serial.write(0xDD);


        for (unsigned char i = 0; i < 16; i++)
        {
            // Send Escape command
            Serial.write(0xDD);
        }



        //set_ram_state(DISABLE);
        SetCs(1);

        Serial_received = 0;
        rx_byte =0;

    }

    if ( Arduino_Buffer[0] == 0x49 && Serial_received == 1) // ERASE RAM

    {
        set_ram_state(ENABLE);
        for (unsigned char i = 0; i < 4; i++)
        {
            select_ram_bank(i);
            SetCs(0);

            for (k = 0; k < 8192*2; k++)
            {
                writeByte_GB(0xA000+k,0xFF);
            }
            k=0;
        }
        for (unsigned char i = 0; i < 128; i++)
        {
            Serial.write(0xAA);
        }

        set_ram_state(DISABLE);
        SetCs(1);
        Serial_received = 0;
        rx_byte =0;


    }

     if ( Arduino_Buffer[0] == 0x4E && Serial_received == 1) // Write GB Flash

    {

        if ( Init_Mapper == 0)
        {

            SetAddress(0x00);
            SetAddress(0x4000);
            SetDataOutput();
            PORTC = 0x00;
            PORTH &= ~(1 << 5); // Wr '0'
            delay(32);
            PORTH |= (1 << 5); // Wr '1'
            Init_Mapper =1;

                    PORTH &= ~(1 << 5); // Wr '0'
        SetDataOutput();
        writeByte_GB(0xAAA,0xF0);
        delay(100);
        mbc_num = MBC5;
        select_rom_bank(0);
        writeByte_GB(0x4000, 0x0);
        delay(32);
        }

          // Prepare Write Mode
        k=0;

        // Prepare Buffer

      //  select_rom_bank(0);
       // select_rom_bank(Arduino_Buffer[4]); // assign bank
        //l=Arduino_Buffer[5]*64;


       if ( Addr_Counter == 16384)
        {
            mbc_num = MBC5;
            Cur_Bank=Cur_Bank+1;
            writeByte_GB(0x2100,Cur_Bank);
            writeByte_GB(0x3000, 0x0);
            Addr_Counter=0;
        }
        
        SetCs(0);

                  for (k = 0; k < 64; k++)
        {
                writeByte_GB(0xAAA,0xAA);
    writeByte_GB(0x555,0x55);
    writeByte_GB(0xAAA,0xA0);
            if ( Cur_Bank == 0){writeByte_GB(Addr_Counter +k, Arduino_Buffer[k+64]);}
            else {writeByte_GB(0x4000+Addr_Counter +k, Arduino_Buffer[k+64]);}     

               //  delay(3);   

        DX_AS_INPUT;

           // delay(3);   

        // Set OE/RD(PH6) LOW
            PORTH &= ~(1 << 3);

            delay(2);

             /*while (((PINC & 0x80) != (Arduino_Buffer[k+64] & 0x80)) || retry < 30) 
             {
                 __asm__("nop\n\t");
                retry++;
            }*/

             // Switch OE/RD(PH6) to HIGH
            PORTH |= (1 << 3);
            
           // writeByte_GB(0x4000+l+k,Arduino_Buffer[k+64]);
        }
        retry=0;

   
              Addr_Counter=Addr_Counter+64;
              k=0;

       
        
        // Send Transfert Completed command

        for (unsigned char i = 0; i < 64; i++)
        {
            // Send Escape command
            Serial.write(0xDD);
        }

           Serial_received = 0;
           rx_byte =0;
    }
    


    if ( Arduino_Buffer[0] == 0x4D && Serial_received == 1) // Erase GB Flash

    {

        PORTH &= ~(1 << 5); // Wr '0'
        SetDataOutput();
        writeByte_GB(0xAAA,0xF0);
        delay(100);

    writeByte_GB(0xAAA,0xAA);
    writeByte_GB(0x555,0x55);
    writeByte_GB(0xAAA,0x80);
        writeByte_GB(0xAAA,0xAA);
    writeByte_GB(0x555,0x55);
    writeByte_GB(0xAAA,0x10);

        i=0;
        delay(1700*6);

        SetDataOutput();
writeByte_GB(0xAAA,0xF0);
        delay(100);


        i=0;
        SetWr(1);

        for (unsigned char i = 0; i < 128; i++)
        {
            Serial.write(0xAA);
        }
        Serial_received = 0;
        rx_byte =0;

    }

    if ( Arduino_Buffer[0] == 0x4B && Serial_received == 1) // Detect GB Flash

    {

        PORTH &= ~(1 << 5); // Wr '0'
        reset_command();
        infosId();
        Arduino_Buffer[0] = Manufacturer_ID;
        Arduino_Buffer[1] = Device_ID;
        Arduino_Buffer[6] = 0xAA;

        for (unsigned char i = 0; i < 128; i++)
        {
            Serial.write(Arduino_Buffer[i]);
        }


        Serial_received = 0;
        rx_byte =0;



    }

    
    if ( Arduino_Buffer[0] == 0x4F && Serial_received == 1) // Extras

    {

       if ( Arduino_Buffer[1] == 0x01 && Serial_received == 1) // TEST MBC5 RUMBLE
       {

              writeByte_GB(0x4000, 8); // RUMBLE Enable
              delay(800);
              writeByte_GB(0x4000, 0); // RUMBLE Disable
                            writeByte_GB(0x4000, 8); // RUMBLE Enable
              delay(800);
              writeByte_GB(0x4000, 0); // RUMBLE Disable
                            writeByte_GB(0x4000, 8); // RUMBLE Enable
              delay(800);
              writeByte_GB(0x4000, 0); // RUMBLE Disable

            for (unsigned char i = 0; i < 64; i++)
            {
                // Send Escape command
                Serial.write(0xDD);
             }
        Serial_received = 0;
        rx_byte =0;
       }

       if ( Arduino_Buffer[1] == 0x02 && Serial_received == 1) // DUMP RTC
       {

         // Enable RTC Register
         writeByte_GB(0x0000, 0x0A);
         delay(4);

        //writeByte_GB(0xA000+1,0x02);
        //delay(4);

         // Latch current time to RTC Register

         writeByte_GB(0x6000, 0);
         delay(4);
         writeByte_GB(0x6000, 1);
         delay(4);
         delay(4);
         writeByte_GB(0x6000, 0);
         delay(4);
         writeByte_GB(0x6000, 1);
         delay(4);

        // Read RTC Register

        writeByte_GB(0x4000, 0x08);
        RTC_DATA[0]= readByteSRAM_GB(0xA000);
        writeByte_GB(0x4000, 0x09);
        RTC_DATA[1]= readByteSRAM_GB(0xA000);
        writeByte_GB(0x4000, 0x0A);
        RTC_DATA[2]= readByteSRAM_GB(0xA000);
        writeByte_GB(0x4000, 0x0B);
        RTC_DATA[3]= readByteSRAM_GB(0xA000);
        writeByte_GB(0x4000, 0x0C);
        RTC_DATA[4]= readByteSRAM_GB(0xA000);
        writeByte_GB(0x4000, 0x0D);
        RTC_DATA[5]= readByteSRAM_GB(0xA000);
        writeByte_GB(0x4000, 0x07);
        RTC_DATA[6]= readByteSRAM_GB(0xA000);   
        delay(4);

        i=0;


         for (unsigned char i = 0; i < 12; i++)
        {
           Arduino_Buffer[i]=RTC_DATA[i];
        }
        i=0;

        Arduino_Buffer[13] = 0xAA;

         for (unsigned char i = 0; i < 128; i++)
        {
            Serial.write(Arduino_Buffer[i]);
        }
        Serial_received = 0;
        rx_byte =0;
       }
       
          delay(4);
        // Disable RTC Register
         writeByte_GB(0x0000, 0x00);
         delay(4);

      
    }


    if ( Arduino_Buffer[0] == 0x4C && Serial_received == 1) // Debug Test

    {

      
    }




}














