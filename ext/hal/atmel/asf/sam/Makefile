# Makefile - Atmel SAM SDK
#
# Copyright (c) 2016 Piotr Mienkowski
# SPDX-License-Identifier: Apache-2.0
#

ifeq ($(CONFIG_SOC_SERIES),"samg")
ZEPHYRINCLUDE += -I$(srctree)/ext/hal/atmel/asf/sam/include/$(CONFIG_SOC_SERIES)/$(CONFIG_SOC)
else
ZEPHYRINCLUDE += -I$(srctree)/ext/hal/atmel/asf/sam/include/$(CONFIG_SOC_SERIES)
endif

