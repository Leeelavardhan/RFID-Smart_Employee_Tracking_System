void InitializeUART1(void);
unsigned char Receive_char_UART1(void);
void Receive_string_UART1(char *string, unsigned int length);
void Transmit_char_UART1(char character);
void Transmit_string_UART1(char *string);
char *ExtractRFID(char *str);

