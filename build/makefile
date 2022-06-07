#   $@  表示目标文件；
#   $^  表示所有的依赖文件；
#   $<  表示第一个依赖文件；

#   =   延时赋值，该变量只有在调用的时候，才会被赋值；
#   :=   直接赋值，与延时赋值相反，使用直接赋值的话，变量的值定义时就已经确定了；
#   ?=  若变量的值为空，则进行赋值，通常用于设置默认值;
#   +=  追加赋值，可以往变量后面增加新的内容。


#选择编译器
#gcc
#arm-linux-gnueabihf-gcc
#arm-buildroot-linux-uclibcgnueabi-gcc
CC := gcc

#目标文件
EXEC := demo

#依赖文件
OBJS += ../src/*.c

#头文件路径
CFLAGS += -I ../inc/
CFLAGS += -I ../src/

#库路径
#LIB += -L ../lib

#库
LIB += -lpthread
LIB += -lm
#LIB += -lssl
#LIB += -lcrypto

$(EXEC) : $(OBJS)
	$(CC) -o $(EXEC) $^ $(CFLAGS) $(LIB)
  
.PHONY:clean
clean:
	rm -rf $(EXEC)