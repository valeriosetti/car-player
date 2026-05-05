################################################################################
#
# Launcher
#
################################################################################

LAUNCHER_VERSION = 1.0
LAUNCHER_SITE = $(BR2_EXTERNAL_CAR_PLAYER_PATH)/package/launcher/src
LAUNCHER_SITE_METHOD = local

LAUNCHER_CONF_OPTS += -DCMAKE_BUILD_TYPE=Debug

LAUNCHER_DEPENDENCIES += lvgl
LAUNCHER_DEPENDENCIES += host-pkgconf

$(eval $(cmake-package))
