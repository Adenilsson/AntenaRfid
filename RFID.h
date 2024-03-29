
/*
 * RFID.c
 *
 * Created: 31/08/2017 06:01:26
 *  Author: Gduo Solu��es
 */ 

#define STATUS_ERR_TIMEOUT  10
#define STATUS_ERR_BAD_DATA 20
#define STATUS_OK           30
#define SCIO  2
#define TXCT  5
#define MCW   0b0000000
#define B_header 0
#define T_rfid   14

uint8_t data_r[T_rfid];
uint64_t expo[6] = {1,256,65536,16777216,4294967296,1000000000000};
uint8_t f_4ms=0;
uint8_t f_500us =0;
uint8_t f_scio =0;
uint8_t count_4ms = 40;
uint8_t count_500us = 5;

void enable_extSCIO(void);
void disable_extSCIO(void);
uint16_t crc_kermit( const unsigned char *input_str, size_t num_bytes );

unsigned long count_RFID =0;
unsigned long count_ERROR =0;


	
//Inicializa o RFID configurando as portas
void RFID_init(){
	pinMode(TXCT,OUTPUT);
	delay(500);
	pinMode(SCIO,INPUT);
  digitalWrite(TXCT,1);
}

void bit_Write(uint8_t bit_val){
	if(bit_val==0)
		digitalWrite(TXCT,1);
	else
		digitalWrite(TXCT,0);
	delayMicroseconds(128);
}

void request_RFID(){
	digitalWrite(TXCT,01);
	delayMicroseconds(50);	//50us
	digitalWrite(TXCT,1);
	delayMicroseconds(2000);
	digitalWrite(TXCT,0);
  delayMicroseconds(2202);
	digitalWrite(TXCT,1);
 
	uint8_t data= MCW;
	for(uint8_t i=0;i<8;i++){
		bit_Write((data&1)&&(1));
		data=data>>1;
	}
	digitalWrite(TXCT,1);
	delay(50);
	digitalWrite(TXCT,0);
}

void bit_read(uint8_t *data){
	if(digitalRead(SCIO) ==0)
		*data = ((*data>>1)|0x80);
	else
		*data = ((*data>>1)&~(0x80));
}

uint8_t read_RFID(uint64_t *RFID){
	//Aguarda at� que o primeiro bit de dado seja recebido. Caso at� 4ms nao receba, retorna erro de TIMEOUT
	uint16_t count = 0;
	while((digitalRead(SCIO) ==0) && (++count<50000));
	if(count>=50000){
		return STATUS_ERR_TIMEOUT;
	}
	delayMicroseconds(92); //96us
	bit_read(&data_r[0]);
	for( int i=0; i<7;i++){
		delayMicroseconds(92); //64 us
		bit_read(&data_r[0]);
	}
	delayMicroseconds(92); //96us
	for(int k=1;k<14;k++){
		//Aguarda at� que o primeiro bit de dado seja recebido. Caso at� 4ms nao receba, retorna erro de TIMEOUT
		count =0;
		while((digitalRead(SCIO) ==0) && (++count<10000));
		if(count>=10000)
			return STATUS_ERR_TIMEOUT;
		delayMicroseconds(92); //96us
		bit_read(&data_r[k]);
		for( int i=0; i<7;i++){
			delayMicroseconds(60); //64
			bit_read(&data_r[k]);
		}
		delayMicroseconds(100); //96us
	}
	*RFID=0;
//	RFID = data_r[1];
	//Leitura dos 5 bytes (40 bits) sendo 38 bits de ID (elimina-se os 2 ultimos bits do byte 5)
	for(int i=0;i<5;i++){
    //SerialUSB.print(data_r[i+1],HEX);
    //SerialUSB.print(" ");
		if(i==4)
			*RFID += ((unsigned long)data_r[i+1]&0x3F)*expo[i];
		else
			*RFID += (unsigned long)data_r[i+1]*expo[i];
	}
    //SerialUSB.println("");
   // SerialUSB.print("RFID: ");
    //SerialUSB.println((uint32_t)(*RFID));

	//Forma-se o c�digo do pa�s ou o c�digo particular de 10bits
	uint16_t CC_RFID =0;
	CC_RFID = data_r[6];
	CC_RFID <<= 1;
	if(data_r[5]&&0x80){
		CC_RFID |= 0x01;
	}
	CC_RFID <<= 1;
	if(data_r[7]&&0x01){
		CC_RFID |= 0x01;
	}
	*RFID += (unsigned long)CC_RFID*expo[5];	
	uint16_t checksum_l = 0;
	for(int i=1;i<9;i++){
		checksum_l += data_r[i];
	}
	uint16_t checksum_r = data_r[9]*256+data_r[10];
	
	uint16_t *data_crc =(uint16_t*)&data_r[1];
	uint16_t crc_result = crc_kermit((uint8_t*)data_crc,8);
	
	char aux3[6];

	if(checksum_r != crc_result || checksum_r ==0){
		count_ERROR++;
		return STATUS_ERR_BAD_DATA;
	}
	count_RFID++;
	
	return STATUS_OK;		
}


uint8_t DATA[8]={0xda,0X48,0Xb9,0X04,0X80,0XF5,0X00,0X80};
uint8_t DATA2[8]={0xA4,0X4A,0XB9,0X04,0X80,0XF5,0X00,0X80};
static uint16_t		crc_tab[256]={
0x0,    0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0xf78};

/*
 * uint16_t crc_kermit( const unsigned char *input_str, size_t num_bytes );
 *
 * The function crc_kermit() calculates the 16 bits Kermit CRC in one pass for
 * a byte string of which the beginning has been passed to the function. The
 * number of bytes to check is also a parameter.
 */

uint16_t crc_kermit( const unsigned char *input_str, size_t num_bytes ) {

	uint16_t crc;
	uint16_t tmp;
	uint16_t short_c;
	uint16_t low_byte;
	uint16_t high_byte;
	const unsigned char *ptr;
	size_t a;
	crc = 0;
	ptr = input_str;

	if ( ptr != NULL ) 
		for (a=0; a<num_bytes; a++) {

			short_c = 0x00ff & (uint16_t) *ptr;
			tmp     =  crc       ^ short_c;
			crc     = (crc >> 8) ^ crc_tab[ tmp & 0xff ];
			ptr++;
		}

	low_byte  = (crc & 0xff00) >> 8;
	high_byte = (crc & 0x00ff) << 8;
	crc       = low_byte | high_byte;

	return crc;

}  /* crc_kermit */
