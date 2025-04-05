
extern void LCD_WriteCommand(uint8_t cmd);

extern void LCD_WriteData(unsigned char data);

extern void Init_QC12864B(void);

extern void Clear_QC12864B(void);

extern void Print_QC12864B(uint8_t line, unsigned char *string);
