################################################################################
#
# devmem2
#
################################################################################

DEVMEM2_SITE_METHOD = git
DEVMEM2_SITE = https://github.com/valeriosetti/devmem2
DEVMEM2_VERSION = 74c6e41f7c9748345964731015d0759844c3b097
DEVMEM2_LICENSE = GPL-2.0+

BR_NO_CHECK_HASH_FOR += $(DEVMEM2_SOURCE)

define DEVMEM2_BUILD_CMDS
	$(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) devmem2
endef

define DEVMEM2_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/devmem2 $(TARGET_DIR)/sbin/devmem2
endef

$(eval $(generic-package))
