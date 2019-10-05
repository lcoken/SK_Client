#include defines.mk

GCC_PATH=C:\Program Files (x86)\Mingw\mingw64\bin

VERS := (($(MAJOR) * 255 + $(MINOR)) * 255 + $(CHG_H)) * 255 + $(CHG_L)
# quite compile.
Q = @

CC = "$(GCC_PATH)\gcc"
MV = mv
#CC = mingw32-make.exe

OUTPUT_EXE = skhl
OUTPUT_DIR = output

# CFLAGS
CFLAGS := -g -Wall -DSKHL_DEBUG -DVERS -DSK_WINDOWS

# 库文件存放地址
LIB := -L ./lib

# 链接库
LIB +=

# 头文件所在地址
INCLUDE :=-I ./inc/

# 通过find命令，找到所有的.c文件
CSources := $(shell find src/ -name "*.c")

Objs := $(CSources:.c=.o)

CLEAN_FILE = $(wildcard output/)

$(OUTPUT_EXE):$(Objs)
	$(Q) $(CC) $^ -o $@ $(CFLAGS) $(LIB)
	$(Q) test -d $(OUTPUT_DIR) || mkdir -p $(OUTPUT_DIR)
	$(Q) $(MV) src/*.o $(OUTPUT_DIR)
	$(Q) echo "Make $(OUTPUT_EXE) done!"

%.o : %.c
	$(CC) -c $(INCLUDE) $(CFLAGS) $< -o $@

.PHONY : clean

test:
	@echo version : $(VERS)

clean:
	rm -rf $(CLEAN_FILE)
	rm *.exe
	$(Q) echo "Make clean done!"
