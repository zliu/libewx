######################*license start*###################################
# OCTEON SDK
# 
# Copyright (c) 2003-2005 Cavium Networks. All rights reserved.
# 
# This file, which is part of the OCTEON SDK from Cavium Networks,
# contains proprietary and confidential information of Cavium Networks and
# its suppliers.
# 
# Any licensed reproduction, distribution, modification, or other use of
# this file or the confidential information or patented inventions
# embodied in this file is subject to your license agreement with Cavium
# Networks. Unless you and Cavium Networks have agreed otherwise in
# writing, the applicable license terms can be found at:
# licenses/cavium-license-type2.txt
# 
# All other use and disclosure is prohibited.
# 
# Contact Cavium Networks at info@caviumnetworks.com for more information.
#####################*license end*#####################################/


#
# File version info: $Id: Makefile 35609 2008-07-08 19:44:08Z pkapoor $


ifndef OCTEON_ROOT
OCTEON_ROOT = ../..
endif

MODEL=${OCTEON_MODEL}
TOP:=${PWD}

OBJ_DIR:=$(TOP)/obj

PHONY: libewx doc
#  standard common Makefile fragment
all: $(OBJ_DIR) libewx 

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

include $(OCTEON_ROOT)/common.mk

dir := $(TOP)
include $(dir)/lib_ewx.mk

CFLAGS_GLOBAL +=
CFLAGS_LOCAL = -g -O2 -W -Wall  -Wno-unused-parameter  

libewx: ${OBJ_DIR}/libewx.a
	@mkdir -p lib_ewx
	@cp *.h $^ lib_ewx
	@tar -czvf libewx.tgz lib_ewx 2>& 1 > /dev/null
	@rm -rf lib_ewx

doc:
	doxygen docs.conf

clean:
	rm -f $(CLEAN_LIST) libewx.tgz

dist_clean:
	rm -rf ${OBJ_DIR}
