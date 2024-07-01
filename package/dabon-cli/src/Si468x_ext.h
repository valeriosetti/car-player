#ifndef _SI468X_EXT_H_
#define _SI468X_EXT_H_

#include "stdint.h"
#include "utils.h"
#include "Si468x.h"

#define MAX_DAB_SERVICES_PER_ENSAMBLE    32
#define MAX_DAB_COMPONENTS_PER_SERVICE    15
struct dab_service_list {
    struct Si468x_DAB_digital_service_list_header header;
    uint8_t* raw_service_list;
    struct Si468x_DAB_digital_service* service_list[MAX_DAB_SERVICES_PER_ENSAMBLE+1];
    struct Si468x_DAB_digital_service_component* components_list[MAX_DAB_SERVICES_PER_ENSAMBLE+1][MAX_DAB_COMPONENTS_PER_SERVICE+1];
};

int Si468x_start_image(uint8_t* btldr_image, uint32_t btldr_size, uint8_t* app_image, uint32_t app_size);
int Si468x_get_dab_service_list(struct dab_service_list* output);

#endif //_SI468X_EXT_H_
