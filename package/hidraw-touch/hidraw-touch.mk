################################################################################
#
# hidraw-touch
#
################################################################################

HIDRAW_TOUCH_VERSION = 1.0
HIDRAW_TOUCH_SITE = ./package/hidraw-touch/src
HIDRAW_TOUCH_SITE_METHOD = local


define HIDRAW_TOUCH_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D)
endef

define HIDRAW_TOUCH_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/hidraw-touch $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))
