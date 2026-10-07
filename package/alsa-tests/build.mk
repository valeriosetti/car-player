################################################################################
#
# ALSA tests
#
################################################################################

ALSA_TESTS_VERSION = 1.0
ALSA_TESTS_SITE = $(BR2_EXTERNAL_CAR_PLAYER_PATH)/package/alsa-tests/src
ALSA_TESTS_SITE_METHOD = local

ALSA_TESTS_DEPENDENCIES += alsa-lib

$(eval $(cmake-package))
