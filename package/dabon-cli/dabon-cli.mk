################################################################################
#
# hello
#
################################################################################

DABON_CLI_VERSION = 1.0
DABON_CLI_SITE = $(BR2_EXTERNAL_CAR_PLAYER_PATH)/package/dabon-cli/src
DABON_CLI_SITE_METHOD = local

DABON_CLI_CONF_OPTS += -DCMAKE_BUILD_TYPE=Debug

define COPY_LIBSI468x_SRC
	echo "Copying libsi468x sources to the build directory"
	mkdir -p $(@D)/libsi468x
	cp -r $(BR2_EXTERNAL_CAR_PLAYER_PATH)/package/libsi468x/* $(@D)/libsi468x/
endef
DABON_CLI_PRE_CONFIGURE_HOOKS += COPY_LIBSI468x_SRC

DABON_CLI_DEPENDENCIES += libgpiod

$(eval $(cmake-package))
