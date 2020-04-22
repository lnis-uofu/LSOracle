
ifeq ($(ENABLE_LSO),1)
OBJS += passes/oracle/oracle.o
OBJS += passes/oracle/lso_blif_write.o
ifneq ($(LSOEXTERNAL),)
passes/oracle/oracle.o: CXXFLAGS += -DLSOEXTERNAL='"$(LSOEXTERNAL)"'
passes/oracle/lso_blif_write.o: CXXFLAGS += -DLSOEXTERNAL='"$(LSOEXTERNAL)"'
endif
endif