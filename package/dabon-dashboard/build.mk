################################################################################
#
# DABon dashboard
#
################################################################################

DABON_DASHBOARD_VERSION = 1.0
DABON_DASHBOARD_SITE = $(BR2_EXTERNAL_CAR_PLAYER_PATH)/package/dabon-dashboard/src
DABON_DASHBOARD_SITE_METHOD = local

DABON_DASHBOARD_DEPENDENCIES += gstreamer1
DABON_DASHBOARD_DEPENDENCIES += libglib2
DABON_DASHBOARD_DEPENDENCIES += lvgl
DABON_DASHBOARD_DEPENDENCIES += wireplumber
DABON_DASHBOARD_DEPENDENCIES += host-pkgconf

$(eval $(cmake-package))
