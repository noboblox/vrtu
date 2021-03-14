#ifndef IEC104_104ENUMS_HPP_
#define IEC104_104ENUMS_HPP_

#include "core/namedenum.hpp"

namespace IEC104
{
    enum class DoublePoint
    {
        INTERMEDIATE = 0, // 0b00
        OFF          = 1, // 0b01
        ON           = 2, // 0b10
        FAULTY       = 3, // 0b11
    };
    using DoublePointEnum = NamedEnum<DoublePoint>;

    enum class ReasonCode
    {
        PERIODIC                      = 1,
        BACKGROUND_SCAN               = 2,
        SPONTANEOUS                   = 3,
        INITIALIZED                   = 4,
        REQUEST                       = 5,
                                     
        ACTIVATION                    = 6,
        CONFIRM_ACTIVATION            = 7,
        CANCEL_ACTIVATION             = 8,
        CONFIRM_CANCELLATION          = 9,
        FINISHED_ACTIVATION           = 10,
                                     
        RESPONSE_TO_REMOTE_CONTOL     = 11,
        RESPONSE_TO_LOCAL_CONTROL     = 12,
                                     
        FILE_TRANSFER                 = 13,
                                     
        RESERVED_CODE_14              = 14,
        RESERVED_CODE_15              = 15,
        RESERVED_CODE_16              = 16,
        RESERVED_CODE_17              = 17,
        RESERVED_CODE_18              = 18,
        RESERVED_CODE_19              = 19,
                                     
        GENERAL_INTERROGATION         = 20,
        GROUP_1_INTERROGATION         = 21,
        GROUP_2_INTERROGATION         = 22,
        GROUP_3_INTERROGATION         = 23,
        GROUP_4_INTERROGATION         = 24,
        GROUP_5_INTERROGATION         = 25,
        GROUP_6_INTERROGATION         = 26,
        GROUP_7_INTERROGATION         = 27,
        GROUP_8_INTERROGATION         = 28,
        GROUP_9_INTERROGATION         = 29,
        GROUP_10_INTERROGATION        = 30,
        GROUP_11_INTERROGATION        = 31,
        GROUP_12_INTERROGATION        = 32,
        GROUP_13_INTERROGATION        = 33,
        GROUP_14_INTERROGATION        = 34,
        GROUP_15_INTERROGATION        = 35,
        GROUP_16_INTERROGATION        = 36,

        COUNTER_INTERROGATION         = 37,
        COUNTER_GROUP_1_INTERROGATION = 38,
        COUNTER_GROUP_2_INTERROGATION = 39,
        COUNTER_GROUP_3_INTERROGATION = 40,
        COUNTER_GROUP_4_INTERROGATION = 41,

        RESERVED_CODE_42              = 42,
        RESERVED_CODE_43              = 43,

        UNKNOWN_TYPE_ID               = 44,
        UNKNOWN_REASON                = 45,
        UNKNOWN_COMMON_ADDRESS        = 46,
        UNKNOWN_INFO_ADDRESS          = 47,

        CUSTOM_CODE_48                = 48,
        CUSTOM_CODE_49                = 49,
        CUSTOM_CODE_50                = 50,
        CUSTOM_CODE_51                = 51,
        CUSTOM_CODE_52                = 52,
        CUSTOM_CODE_53                = 53,
        CUSTOM_CODE_54                = 54,
        CUSTOM_CODE_55                = 55,
        CUSTOM_CODE_56                = 56,
        CUSTOM_CODE_57                = 57,
        CUSTOM_CODE_58                = 58,
        CUSTOM_CODE_59                = 59,
        CUSTOM_CODE_60                = 60,
        CUSTOM_CODE_61                = 61,
        CUSTOM_CODE_62                = 62,
        CUSTOM_CODE_63                = 63,
    } ;
    using ReasonCodeEnum = NamedEnum<ReasonCode>;

    enum class Type
    {
        M_SP_NA_1 = 1,
        M_DP_NA_1 = 3,
        M_ME_NB_1 = 11,
        M_ME_NC_1 = 13,
    };
    using TypeEnum = NamedEnum<Type>;
}

#endif
