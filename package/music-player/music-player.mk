################################################################################
#
# music-player
#
################################################################################

MUSIC_PLAYER_VERSION = 1.0
MUSIC_PLAYER_SITE = ./package/music-player/src
MUSIC_PLAYER_SITE_METHOD = local

MUSIC_PLAYER_DEPENDENCIES = qt5base qt5multimedia gstreamer1 gst1-plugins-good gst1-plugins-base

define MUSIC_PLAYER_CONFIGURE_CMDS
	$(HOST_DIR)/usr/bin/qmake $(@D)/music-player.pro -o $(@D)/Makefile
endef

define MUSIC_PLAYER_BUILD_CMDS
	$(TARGET_CONFIGURE_OPTS) $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D)
endef

define MUSIC_PLAYER_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/music-player $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))
