################################################################################
#
# hello
#
################################################################################

DABON_CLI_VERSION = 1.0
DABON_CLI_SITE = $(BR2_EXTERNAL_CAR_PLAYER_PATH)/package/dabon-cli/src
DABON_CLI_SITE_METHOD = local

define DABON_CLI_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D)
endef

define DABON_CLI_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/dabon-cli $(TARGET_DIR)/usr/bin
	$(INSTALL) -m 0666 -D $(@D)/external_firmwares/*.bin -t $(TARGET_DIR)/external_firmwares/
endef

$(eval $(generic-package))
