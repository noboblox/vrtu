#include "protocols/iec104/infoobjects.hpp"

#include "protocols/iec104/104enums.hpp"

namespace IEC104
{
    /*
     * This file registers all IEC104 information objects known to the IEC104 code, at program initialization
     * Custom objects may be registered by client code in the clients own source files.
     * 
     * They will be registered as long as the following requirements are met:
     *   - The client-code needs to define own classes, which inherit from IEC104::BaseInfoObject
     *   - The classes are registered with "RegisteredBy::EXTERNAL" inside a SOURCE file (never a header)
     *     (to override potentially existing registrations provided here)
     *   - Are linked to the final executable
     */

    /*
     * ATTENTION: Always make sure that the registered size matches the info-object size from IEC-60870-5-104
     * >>> EXCLUDING the IOA size. <<<
     * 
     * The IOA size may be configured to something else than "3",
     * which means it needs to be added at runtime.
     * 
     * The decoding functions ignore the size registered here. 
     * They just read the number of bytes they expect.
     * 
     * The size here is used, to allow static bound-checks BEFORE decoding InfoObjects.
     * If you have a mismatch between the registered size and the actual decoded size,
     * you may get false-positive errors, or less intuitive real errors
     */


    /* ID 1 == M_SP_NA_1 */
    static StaticRegistration<DataSinglePoint, DataSinglePoint::TYPE_ID, DataSinglePoint::DATA_SIZE,
                              RegisteredBy::INTERNAL> gRegisterType1;
    
    /* ID 3 == M_DP_NA_1 */
    static StaticRegistration<DataDoublePoint, DataDoublePoint::TYPE_ID, DataDoublePoint::DATA_SIZE,
                              RegisteredBy::INTERNAL> gRegisterType3;
    
    /* ID 11 == M_ME_NB_1 */
    static StaticRegistration<DataMeasuredScaled, DataMeasuredScaled::TYPE_ID, DataMeasuredScaled::DATA_SIZE,
                              RegisteredBy::INTERNAL> gRegisterType11;

    /* ID 13 == M_ME_NC_1 */
    static StaticRegistration<DataMeasuredFloat, DataMeasuredFloat::TYPE_ID, DataMeasuredFloat::DATA_SIZE,
                              RegisteredBy::INTERNAL> gRegisterType13;
}