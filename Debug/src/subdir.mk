################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Common.cpp \
../src/Compression.cpp \
../src/Dictionary.cpp \
../src/HashBucket16.cpp \
../src/HashSegmented.cpp \
../src/Query.cpp \
../src/TT.cpp \
../src/TripleTable.cpp 

CPP_DEPS += \
./src/Common.d \
./src/Compression.d \
./src/Dictionary.d \
./src/HashBucket16.d \
./src/HashSegmented.d \
./src/Query.d \
./src/TT.d \
./src/TripleTable.d 

OBJS += \
./src/Common.o \
./src/Compression.o \
./src/Dictionary.o \
./src/HashBucket16.o \
./src/HashSegmented.o \
./src/Query.o \
./src/TT.o \
./src/TripleTable.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I/usr/include -O0 -g3 -Wall -c -fmessage-length=0 -std=gnu++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/Common.d ./src/Common.o ./src/Compression.d ./src/Compression.o ./src/Dictionary.d ./src/Dictionary.o ./src/HashBucket16.d ./src/HashBucket16.o ./src/HashSegmented.d ./src/HashSegmented.o ./src/Query.d ./src/Query.o ./src/TT.d ./src/TT.o ./src/TripleTable.d ./src/TripleTable.o

.PHONY: clean-src

