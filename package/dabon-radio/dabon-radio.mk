################################################################################
#
# DABon radio
#
################################################################################

DABON_RADIO_VERSION = 1.0
DABON_RADIO_SITE = $(BR2_EXTERNAL_CAR_PLAYER_PATH)/package/dabon-radio/src
DABON_RADIO_SITE_METHOD = local

define COPY_LIBSI468x_SRC
	echo "Copying libsi468x sources to the build directory"
	mkdir -p $(@D)/libsi468x
	cp -r $(BR2_EXTERNAL_CAR_PLAYER_PATH)/package/libsi468x/* $(@D)/libsi468x/

	echo "Copying LVGL sources to the build directory"
	mkdir -p $(@D)/lvgl
	cp -r $(BR2_EXTERNAL_CAR_PLAYER_PATH)/package/lvgl/* $(@D)/lvgl/
endef
DABON_RADIO_PRE_CONFIGURE_HOOKS += COPY_LIBSI468x_SRC

DABON_RADIO_DEPENDENCIES += gstreamer1
DABON_RADIO_DEPENDENCIES += libglib2
DABON_RADIO_DEPENDENCIES += lvgl
DABON_RADIO_DEPENDENCIES += host-pkgconf

$(eval $(cmake-package))
