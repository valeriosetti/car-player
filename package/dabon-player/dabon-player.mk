################################################################################
#
# DABon player
#
################################################################################

DABON_PLAYER_VERSION = 1.0
DABON_PLAYER_SITE = $(BR2_EXTERNAL_CAR_PLAYER_PATH)/package/dabon-player/src
DABON_PLAYER_SITE_METHOD = local

BR2_STATIC_LIBS=0
DABON_PLAYER_CONF_OPTS += -DBUILD_FOR_ODROIDC2=ON
DABON_PLAYER_CONF_OPTS += -DSKIP_WELCOME_SCREEN=OFF

DABON_PLAYER_DEPENDENCIES += gstreamer1
#DABON_PLAYER_DEPENDENCIES += gobject-2.0
DABON_PLAYER_DEPENDENCIES += libglib2

# Overrider CMake's install() command because otherwise LVGL installs static
# libraries and headers on the target but they are useless there...
define DABON_PLAYER_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(DABON_PLAYER_BUILDDIR)/dabon-player $(TARGET_DIR)/usr/bin
	$(INSTALL) -d $(TARGET_DIR)/usr/share/dabon-player/images/
	$(INSTALL) -D -m 0644 $(DABON_PLAYER_BUILDDIR)/player/images/* $(TARGET_DIR)/usr/share/dabon-player/images/
endef

$(eval $(cmake-package))
