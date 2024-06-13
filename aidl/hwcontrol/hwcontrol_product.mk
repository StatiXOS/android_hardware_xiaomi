#
# HWControl
#

PRODUCT_PACKAGES += \
    XiaomiPad5HwCtrl

# Sepolicy for HwControl Hal
SYSTEM_EXT_PRIVATE_SEPOLICY_DIRS += vendor/hardware/xiaomi/sepolicy/hwcontrol/private
SYSTEM_EXT_PUBLIC_SEPOLICY_DIRS += vendor/hardware/xiaomi/sepolicy/hwcontrol/public
BOARD_VENDOR_SEPOLICY_DIRS += vendor/hardware/xiaomi/sepolicy/hwcontrol/vendor
