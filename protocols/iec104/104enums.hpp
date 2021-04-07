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
        // Process information - monitoring direction
        M_SP_NA_1 = 1,
        M_SP_TA_1 = 2,
        M_DP_NA_1 = 3,
        M_DP_TA_1 = 4,
        M_ST_NA_1 = 5,
        M_ST_TA_1 = 6,
        M_BO_NA_1 = 7,
        M_BO_TA_1 = 8,
        M_ME_NA_1 = 9,
        M_ME_TA_1 = 10,
        M_ME_NB_1 = 11,
        M_ME_TB_1 = 12,
        M_ME_NC_1 = 13,
        M_ME_TC_1 = 14,
        M_IT_NA_1 = 15,
        M_IT_TA_1 = 16,
        M_EP_TA_1 = 17,
        M_EP_TB_1 = 18,
        M_EP_TC_1 = 19,
        M_PS_NA_1 = 20,
        M_ME_ND_1 = 21,

        M_SP_TB_1 = 30,
        M_DP_TB_1 = 31, // e.g. switching device position with timestamp
        M_ST_TB_1 = 32,
        M_BO_TB_1 = 33,
        M_ME_TD_1 = 34,
        M_ME_TE_1 = 35,
        M_ME_TF_1 = 36,
        M_IT_TB_1 = 37,
        M_EP_TD_1 = 38,
        M_EP_TE_1 = 39,
        M_EP_TF_1 = 40,

        // Process information - control direction
        C_SC_NA_1 = 45,
        C_DC_NA_1 = 46, // e.g. switching device command
        C_RC_NA_1 = 47,
        C_SE_NA_1 = 48,
        C_SE_NB_1 = 49,
        C_SE_NC_1 = 50,
        C_BO_NA_1 = 51,

        // System information - monitoring direction
        M_EI_NA_1 = 70, // Station initialization finished

        // System information - control direction
        C_IC_NA_1 = 100, // general interrogation
        C_CI_NA_1 = 101,
        C_RQ_NA_1 = 102,
        C_CS_NA_1 = 103,
        C_TS_NA_1 = 104,
        C_RP_NA_1 = 105,
        C_CD_NA_1 = 106,

        // Parameters - control direction
        P_ME_NA_1 = 110,
        P_ME_NB_1 = 111,
        P_ME_NC_1 = 112,
        P_AC_NA_1 = 113,

        // File transfer
        F_FR_NA_1 = 120,
        F_SR_NA_1 = 121,
        F_SC_NA_1 = 122,
        F_LS_NA_1 = 123,
        F_AF_NA_1 = 124,
        F_SG_NA_1 = 125,
        F_DR_TA_1 = 126
    };
    using TypeEnum = NamedEnum<Type>;
}

#endif
