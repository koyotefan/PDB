include $(PCF_DEV_HOME)/makehead

BUILD = libPDB.a

SRCS 	= $(wildcard *.cpp)
HEADERS = $(filter-out, $(wildcard *.hpp))
OBJS 	= $(SRCS:.cpp=.o)

#CFLAGS += -D__T_DEBUG

all :: $(BUILD)

$(BUILD) : $(OBJS)
	$(AR) $(AR_OPTION) $(BUILD) $(OBJS)
	$(CP) $(BUILD) $(PCF_LIB_DIR)/$(BUILD) 
	$(CP) *.hpp $(PCF_INC_DIR) 

install : $(BUILD)
	$(CP) $(BUILD) $(PCF_LIB_DIR)/$(BUILD) 
	$(CP) *.hpp $(PCF_INC_DIR) 

clean:
	$(RM) $(OBJS) $(BUILD) core.* *.d
