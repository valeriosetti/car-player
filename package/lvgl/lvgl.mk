################################################################################
#
# lvgl
#
################################################################################

LVGL_SITE_METHOD = git
LVGL_SITE = https://github.com/lvgl/lvgl
LVGL_VERSION = v8.3.10
LVGL_INSTALL_STAGING = YES
LVGL_LICENSE = MIT

#BR_NO_CHECK_HASH_FOR += $(LVGL_SOURCE)

#define LVGL_BUILD_CMDS
#	$(TARGET_CONFIGURE_OPTS) $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D)
#endef

#$(eval $(generic-package))
$(eval $(cmake-package))
