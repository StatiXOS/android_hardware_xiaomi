#
# Copyright (C) 2024 StatiXOS
# SPDX-License-Identifier: Apache-2.0
#

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.consumerir.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.consumerir.xml

PRODUCT_PACKAGES += \
    android.hardware.ir-service.xiaomi

BOARD_VENDOR_SEPOLICY_DIRS += vendor/hardware/xiaomi/sepolicy/consumerir
