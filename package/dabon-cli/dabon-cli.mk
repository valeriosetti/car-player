################################################################################
#
# hello
#
################################################################################

DABON_CLI_VERSION = 1.0
DABON_CLI_SITE = $(BR2_EXTERNAL_CAR_PLAYER_PATH)/package/dabon-cli/src
DABON_CLI_SITE_METHOD = local

DABON_CLI_DEPENDENCIES += libgpiod

$(eval $(cmake-package))
