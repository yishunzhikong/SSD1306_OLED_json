CC = gcc
LD = gcc

SOURCES = SSD1306_OLED_Library/SSD1306_OLED.c Main/Main.c Main/dataapi.c I2C_Library/I2C.c cJSON/cJSON.c
OBJS := $(SOURCES:.c=.o)
CPPFLAGS := -I SSD1306_OLED_Library -I I2C_Library -I cJSON
CFLAGS := -g

ssd: $(OBJS)
	$(CC) $^ -o $@

clean:
	rm -rf ssd $(OBJS)

