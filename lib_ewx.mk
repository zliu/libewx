# Copyright (c) 2003-2005, Cavium Networks. All rights reserved.
#
# This Software is the property of Cavium Networks.  The Software and all
# accompanying documentation are copyrighted.  The Software made available
# here constitutes the proprietary information of Cavium Networks.  You
# agree to take reasonable steps to prevent the disclosure, unauthorized use
# or unauthorized distribution of the Software.  You shall use this Software
# solely with Cavium hardware.
#
# Except as expressly permitted in a separate Software License Agreement
# between You and Cavium Networks, you shall not modify, decompile,
# disassemble, extract, or otherwise reverse engineer this Software.  You
# shall not make any copy of the Software or its accompanying documentation,
# except for copying incident to the ordinary and intended use of the
# Software and the Underlying Program and except for the making of a single
# archival copy.
#
# This Software, including technical data, may be subject to U.S.  export
# control laws, including the U.S.  Export Administration Act and its
# associated regulations, and may be subject to export or import regulations
# in other countries.  You warrant that You will comply strictly in all
# respects with all such regulations and acknowledge that you have the
# responsibility to obtain licenses to export, re-export or import the
# Software.
#
# TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
# AND WITH ALL FAULTS AND CAVIUM MAKES NO PROMISES, REPRESENTATIONS OR
# WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
# TO THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY
# REPRESENTATION OR DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT
# DEFECTS, AND CAVIUM SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES
# OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR
# PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET
# POSSESSION OR CORRESPONDENCE TO DESCRIPTION.  THE ENTIRE RISK ARISING OUT
# OF USE OR PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
#
# File version info: $Id: Makefile 22923 2007-03-06 02:32:39Z kreese $

#VPATH = .:..

#  standard common Makefile fragment

sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

LIBRARY := $(OBJ_DIR)/libewx.a

LIB_COMMON_OBJS  :=  \
	$(OBJ_DIR)/ewx_board_init.o	\
	$(OBJ_DIR)/ewx_shell.o	\
	$(OBJ_DIR)/ewx_uart.o	\
	$(OBJ_DIR)/ewx_slb.o	\
	$(OBJ_DIR)/ewx_led.o	\
	$(OBJ_DIR)/ewx_spi.o	\
	$(OBJ_DIR)/ewx_mem.o	\
	$(OBJ_DIR)/ewx_hash_table.o		\
	$(OBJ_DIR)/ewx_helper.o		\
	$(OBJ_DIR)/ewx_log.o	\
	$(OBJ_DIR)/ewx_thread.o	\
	$(OBJ_DIR)/ewx_debug.o \
	$(OBJ_DIR)/ewx_board_check.o \
	$(OBJ_DIR)/ewx_forward.o \
	$(OBJ_DIR)/ewx_helper_util.o

OBJS_$(d) := $(LIB_COMMON_OBJS)

DEPS_$(d) := $(LIB_COMMON_OBJS:.o=.d)

LIBS_LIST := $(LIBS_LIST) $(LIBRARY)

CLEAN_LIST := $(CLEAN_LIST)
CLEAN_LIST += $(OBJS_$(d))
CLEAN_LIST += $(DEPS_$(d))
CLEAN_LIST += $(LIBRARY)


-include $(DEPS_$(d))

$(LIBRARY): $(OBJS_$(d))
	$(AR) -cr $@ $^

$(OBJ_DIR)/%.o: $(d)/%.c
	$(COMPILE)

d := $(dirstack_$(sp))
sp := $(basename $(sp))
