#include "infoobjects.hpp"

#include "104enums.hpp"

namespace IEC104
{
    /* ID 1 == M_SP_NA_1 */
    static StaticRegistration<static_cast<int>(Type::M_SP_NA_1), DataSinglePoint, 
                              RegisteredBy::INTERNAL> gRegisterType1;
    
    /* ID 3 == M_DP_NA_1 */
    static StaticRegistration<static_cast<int>(Type::M_DP_NA_1), DataDoublePoint, 
                              RegisteredBy::INTERNAL> gRegisterType3;
    
    /* ID 11 == M_ME_NB_1 */
    static StaticRegistration<static_cast<int>(Type::M_ME_NB_1), DataMeasuredScaled,
                              RegisteredBy::INTERNAL> gRegisterType11;

    /* ID 13 == M_ME_NC_1 */
    static StaticRegistration<static_cast<int>(Type::M_ME_NC_1), DataMeasuredFloat, 
                              RegisteredBy::INTERNAL> gRegisterType13;
}