
void InitializeUART0(void);
unsigned char Receive_char_UART0(void);
void Receive_string_UART0(char *string, unsigned int length);
void Transmit_char_UART0(char character);
void Transmit_string_UART0(char *string);
void UART0_ISR(void) __irq;   // prototype

