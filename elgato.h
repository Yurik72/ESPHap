#ifndef __ELGATO_H__
#define __ELGATO_H__

#define ELGATO_EPOCH_OFFSET    978307200

#define HOMEKIT_SERVICE_ELGATO_HISTORY  "E863F007-079E-48FF-8F27-9C2605A29F52" 

#define HOMEKIT_CHARACTERISTIC_ELGATO_SET_TIME                    "E863F121-079E-48FF-8F27-9C2605A29F52"  // 121  => S2W2
#define HOMEKIT_CHARACTERISTIC_ELGATO_HISTORY_REQUEST             "E863F11C-079E-48FF-8F27-9C2605A29F52"  // 11C  => S2W1
#define HOMEKIT_CHARACTERISTIC_ELGATO_HISTORY_STATUS              "E863F116-079E-48FF-8F27-9C2605A29F52"  // 116  => S2R1
#define HOMEKIT_CHARACTERISTIC_ELGATO_HISTORY_ENTRIES             "E863F117-079E-48FF-8F27-9C2605A29F52"  // 117  => S2R2


#define HOMEKIT_DECLARE_CHARACTERISTIC_ELGATO_SET_TIME(_value, ...) \
    .type = HOMEKIT_CHARACTERISTIC_ELGATO_SET_TIME, \
    .description = "SetTime", \
    .format = homekit_format_data, \
    .permissions = homekit_permissions_paired_read \
                 | homekit_permissions_notify, \
    .value = HOMEKIT_DATA_(NULL,128), \
    ##__VA_ARGS__


#define HOMEKIT_DECLARE_CHARACTERISTIC_ELGATO_HISTORY_REQUEST(_value, ...) \
    .type = HOMEKIT_CHARACTERISTIC_ELGATO_HISTORY_REQUEST, \
    .description = "History", \
    .format = homekit_format_data, \
    .permissions = homekit_permissions_paired_read \
                 | homekit_permissions_notify, \
    .value = HOMEKIT_DATA_(NULL,128), \
    ##__VA_ARGS__

#define HOMEKIT_DECLARE_CHARACTERISTIC_ELGATO_HISTORY_STATUS(_value, ...) \
    .type = HOMEKIT_CHARACTERISTIC_ELGATO_HISTORY_STATUS, \
    .description = "History status", \
    .format = homekit_format_data, \
    .permissions = homekit_permissions_paired_read \
                 | homekit_permissions_notify, \
    .value = HOMEKIT_DATA_(NULL,128), \
    ##__VA_ARGS__

#define HOMEKIT_DECLARE_CHARACTERISTIC_ELGATO_HISTORY_ENTRIES(_value, ...) \
    .type = HOMEKIT_CHARACTERISTIC_ELGATO_HISTORY_ENTRIES, \
    .description = "History entries", \
    .format = homekit_format_data, \
    .permissions = homekit_permissions_paired_read \
                 | homekit_permissions_notify, \
    .value = HOMEKIT_DATA_(NULL,128), \
    ##__VA_ARGS__
#endif
