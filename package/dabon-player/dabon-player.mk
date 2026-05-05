################################################################################
#
# DABon player
#
################################################################################

DABON_PLAYER_VERSION = 1.0
DABON_PLAYER_SITE = $(BR2_EXTERNAL_CAR_PLAYER_PATH)/package/dabon-player/src
DABON_PLAYER_SITE_METHOD = local

DABON_PLAYER_DEPENDENCIES += gstreamer1
DABON_PLAYER_DEPENDENCIES += libglib2
DABON_PLAYER_DEPENDENCIES += lvgl
DABON_PLAYER_DEPENDENCIES += host-pkgconf

$(eval $(cmake-package))
