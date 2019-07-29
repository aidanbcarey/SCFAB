// nRF_05.c  12/30/16  Modified from NG Hello-world below:
// Add functin to trade one byte acc. to SPI protocol.
//This will be the basis of all other functions.
//implemented here in SPI_exchange, which accepts a pointer for the return variable.
//and a function that reads status and returns a byte.
// and now a function that reads a register value and returns its contents.
//  I was not able to have these second functions accept a pointer.  
//Why not?  Maybe need to use volatile unsigned char?  Nope.  It works to use a dummy variable internal to the function. Also have to //Transfer the value from this variable to the pointed-to variable before leaving town.
//This is nRF_06.c
//In xxx_07.c, add a function to write to a register.  This will be a prototype for buffer writing.
//the 07 version does not work because detailsin timing at the start of the second byte are note correct.  08 will try to address this.
//in 08 was not able to fix write function.  Wrote a new one that looks more like Neil's intention.  Doesn't work yet.
// in 09, all issues of passing pointers are better, but still does not give the right answer when writing and then reading.
//hello.nRF24L01P.44.c
//
// nRF24L01+ 9600 baud FTDI SPI bridge (incomplete)
//    no fuse
//
// Neil Gershenfeld
//    11/29/16
//
// (c) Massachusetts Institute of Technology 2016
// This work may be reproduced, modified, distributed,
// performed, and displayed for any purpose. Copyright is
// retained and must be preserved. The work is provided
// as is; no warranty is provided, and users accept all 
// liability.
//
// includes
//
#include <avr/io.h>
#include <util/delay.h>
//
// macros
//
#define output(directions,pin) (directions |= pin) // set port direction for output
#define set(port,pin) (port |= pin) // set port pin
#define clear(port,pin) (port &= (~pin)) // clear port pin
#define pin_test(pins,pin) (pins & pin) // test for port pin
#define bit_test(byte,bit) (byte & (1 << bit)) // test for bit set
#define bit_delay_time 102 // bit delay in us for 9600 with overhead at 8 MHz
#define bit_delay() _delay_us(bit_delay_time) // RS232 bit delay
#define half_bit_delay() _delay_us(bit_delay_time/2) // RS232 half bit delay
#define char_delay() _delay_ms(10) // char delay
#define SPI_delay() _delay_us(1) // SPI delay
//
// define serial
//
#define serial_port PORTA
#define serial_direction DDRA
#define serial_pins PINA
#define serial_pin_in (1 << PA2)
#define serial_pin_out (1 << PA3)
//
// define pins
//
#define CE_port PORTB
#define CE_direction DDRB
#define CE_pin (1 << PB2)
#define CS_port PORTA
#define CS_direction DDRA
#define CS_pin (1 << PA7)
#define SCK_port PORTB
#define SCK_direction DDRB
#define SCK_pin (1 << PB0)
#define MOSI_port PORTB
#define MOSI_direction DDRB
#define MOSI_pin (1 << PB1)
#define MISO_pins PINA
#define MISO_direction DDRA
#define MISO_pin (1 << PA0)
#define IRQ_pins PINA
#define IRQ_direction DDRA
#define IRQ_pin (1 << PA1)
//
// define commands
//
#define R_REGISTER 000A
#define W_REGISTER 001A
#define R_RX_PAYLOAD 0110001
#define W_TX_PAYLOAD 10100000
#define FLUSH_TX 11100001
#define FLUSH_RX 11100010
#define REUSE_RX_PL 11100011
#define R_RX_PL_WID 01100000
#define W_ACK_PAYLOAD 10101
#define W_TX_PAYLOAD_NOACK 10110000
#define NOP 0b11111111
//
// define registers
//
#define CONFIG 0x00
#define EN_AA 0x01
#define EN_RXADDR 0x02
#define SETUP_AW 0x03
#define SETUP_RETR 0x04
#define RF_CH 0x05
#define RF_SETUP 0x06
#define STATUS 0x07
#define OBSERVE_TX 0x08
#define RPD 0x09
#define RX_ADDR_P0 0x0A
#define RX_ADDR_P1 0x0B
#define RX_ADDR_P2 0x0C
#define RX_ADDR_P3 0x0D
#define RX_ADDR_P4 0x0E
#define RX_ADDR_P5 0x0F
#define TX_ADDR 0x10
#define RX_PW_P0 0x11
#define RX_PW_P1 0x12
#define RX_PW_P2 0x13
#define RX_PW_P3 0x14
#define RX_PW_P4 0x15
#define RX_PW_P5 0x16
#define FIFO_STATUS 0x17
#define DYNPD 0x1C
//
// get_char
//    read character into rxbyte on pins pin
//    assumes line driver (inverts bits)
//
//
void get_char(volatile unsigned char *pins, unsigned char pin, char *rxbyte) {
   *rxbyte = 0;
   while (pin_test(*pins,pin))
      //
      // wait for start bit
      //
      ;
   //
   // delay to middle of first data bit
   //
   half_bit_delay();
   bit_delay();
   //
   // unrolled loop to read data bits
   //
   if pin_test(*pins,pin)
      *rxbyte |= (1 << 0);
   else
      *rxbyte |= (0 << 0);
   bit_delay();
   if pin_test(*pins,pin)
      *rxbyte |= (1 << 1);
   else
      *rxbyte |= (0 << 1);
   bit_delay();
   if pin_test(*pins,pin)
      *rxbyte |= (1 << 2);
   else
      *rxbyte |= (0 << 2);
   bit_delay();
   if pin_test(*pins,pin)
      *rxbyte |= (1 << 3);
   else
      *rxbyte |= (0 << 3);
   bit_delay();
   if pin_test(*pins,pin)
      *rxbyte |= (1 << 4);
   else
      *rxbyte |= (0 << 4);
   bit_delay();
   if pin_test(*pins,pin)
      *rxbyte |= (1 << 5);
   else
      *rxbyte |= (0 << 5);
   bit_delay();
   if pin_test(*pins,pin)
      *rxbyte |= (1 << 6);
   else
      *rxbyte |= (0 << 6);
   bit_delay();
   if pin_test(*pins,pin)
      *rxbyte |= (1 << 7);
   else
      *rxbyte |= (0 << 7);
   //
   // wait for stop bit
   //
   bit_delay();
   half_bit_delay();
   }
//
// put_char
//    send character in txchar on port pin
//    assumes line driver (inverts bits)
//
void put_char(volatile unsigned char *port, unsigned char pin, char txchar) {
   //
   // start bit
   //
   clear(*port,pin);
   bit_delay();
   //
   // unrolled loop to write data bits
   //
   if bit_test(txchar,0)
      set(*port,pin);
   else
      clear(*port,pin);
   bit_delay();
   if bit_test(txchar,1)
      set(*port,pin);
   else
      clear(*port,pin);
   bit_delay();
   if bit_test(txchar,2)
      set(*port,pin);
   else
      clear(*port,pin);
   bit_delay();
   if bit_test(txchar,3)
      set(*port,pin);
   else
      clear(*port,pin);
   bit_delay();
   if bit_test(txchar,4)
      set(*port,pin);
   else
      clear(*port,pin);
   bit_delay();
   if bit_test(txchar,5)
      set(*port,pin);
   else
      clear(*port,pin);
   bit_delay();
   if bit_test(txchar,6)
      set(*port,pin);
   else
      clear(*port,pin);
   bit_delay();
   if bit_test(txchar,7)
      set(*port,pin);
   else
      clear(*port,pin);
   bit_delay();
   //
   // stop bit
   //
   set(*port,pin);
   bit_delay();
   //
   // char delay
   //
   bit_delay();
   }

	  //
	  //SPI exchange command
	  //
	  //

	  
   void SPI_exchange(unsigned char give, unsigned char *take ) {
   unsigned char bit;
   //
   // init
   //
   *take = 0;
  
   //
   // bit loop
   //
   
   
   for (bit = 0; bit < 8; ++bit) {
	 
	  clear(SCK_port,SCK_pin);
	  clear(CS_port,CS_pin);
	  SPI_delay();
	  if bit_test(give,(7-bit))
	  set(MOSI_port,MOSI_pin);
	  else
		clear(MOSI_port,MOSI_pin);
      set(SCK_port,SCK_pin);
      if pin_test(MISO_pins,MISO_pin)
         *take |= (1 << (7-bit));
	  SPI_delay();
      }
   SPI_delay();
   }

   //
   // SPI_read_status
   //    read status register
   //
void Read_status(unsigned char *ret) {
	   unsigned char var = *ret;
	   SPI_exchange(NOP, &var);
	   set(CS_port,CS_pin);
	   *ret = var;
      }
   //
   //
   //
   //
unsigned char Read_register(unsigned char reg) {
	   unsigned char ret;		
	   SPI_exchange(reg, &ret);
	   SPI_exchange(NOP, &ret);
	   set(CS_port,CS_pin);
	     	   return ret;
	        }

void Write_to_register(unsigned char reg, unsigned char value){
 	SPI_exchange( reg, &value);
	SPI_delay();
 	SPI_exchange(NOP, &value);
	set(CS_port,CS_pin);
	}



void SPI_write(unsigned char command,unsigned char bytes,unsigned char *write_buffer) {
   
    unsigned char bit;
	unsigned char i;
  
    clear(CS_port,CS_pin);  //can we do this outside the function?//
    // bit loop
       for (bit = 0; bit < 8; ++bit) {
	 
	 	  clear(SCK_port,SCK_pin);
 	  
	 	  SPI_delay();
	 	  if bit_test(command,(7-bit))
	 	  set(MOSI_port,MOSI_pin);
	 	  else
	 		clear(MOSI_port,MOSI_pin);
	       set(SCK_port,SCK_pin);
	 	  SPI_delay();
	     }
   
	for (i =0; i<bytes; ++i){
	    
		for (bit = 0; bit < 8; ++bit) {
	 
	 	  clear(SCK_port,SCK_pin);
 	  
	 	  SPI_delay();
	 	  if bit_test(write_buffer[i],(7-bit))
	 	  set(MOSI_port,MOSI_pin);
	 	  else
	 		clear(MOSI_port,MOSI_pin);
	      set(SCK_port,SCK_pin);
	 	  SPI_delay();
	     }
	}
    SPI_delay();
   }
//
// main
//
int main(void) {
   unsigned char result;
   //unsigned char wreg = 0b00100000;  //for writing to register 00!
   unsigned char writedata[2];
   
   // set clock divider to /1
   //
   CLKPR = (1 << CLKPCE);
   CLKPR = (0 << CLKPS3) | (0 << CLKPS2) | (0 << CLKPS1) | (0 << CLKPS0);
   //
   // initialize output pins
   //
   set(serial_port,serial_pin_out);
   output(serial_direction,serial_pin_out);
   output(CS_direction,CS_pin);
   set(CS_port,CS_pin);
   clear(CE_port,CE_pin);
   output(CE_direction,CE_pin);
   clear(SCK_port,SCK_pin);
   output(SCK_direction,SCK_pin);
   clear(MOSI_port,MOSI_pin);
   output(MOSI_direction,MOSI_pin);
   //
   // main loop
   //
  // while (1) {
   _delay_ms(500);//
	   //  Call function to read status.  This function accepts a pointer and modifies the variable internally.
	   //
	   Read_status(&result);
       put_char(&serial_port,serial_pin_out, result);
       char_delay();
//
// 	   Call function to read a register.  This function returns a value.
//
	   
//
 	   // Write_to_register(0x01, 0x0C);
 // 	   char_delay();
	   //_delay_ms(100);
	   
	   result = Read_register(0x00);  //EN_AA is 3F
 	   put_char(&serial_port,serial_pin_out, result	);
 	   char_delay();
	   
	   //Write_to_register(0b0010000, 0x0C);
	   //char_delay();
		//   _delay_ms(100);
	   writedata[0] = 0x0C;
	   SPI_write(0b00100000,1,writedata);
	   char_delay();
//
 	   result = Read_register(0x00);  //EN_AA is 3F
 	   put_char(&serial_port,serial_pin_out, result	);
 	   char_delay();
	   
	   
	   //}
   return(1);
  }
