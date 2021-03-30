#ifndef IEC104_INFOOBJECTS_HPP_
#define IEC104_INFOOBJECTS_HPP_

#include <functional>
#include <map>
#include <memory>

#include "core/data.hpp"
#include "protocols/iec104/104enums.hpp"
#include "protocols/iec104/data104.hpp"
#include "protocols/iec104/infoaddress.hpp"

namespace IEC104
{
	class BaseInfoObject;

    using SharedInfoObject = std::shared_ptr<BaseInfoObject>;
    class InfoObjectFactory
    {
    public:
        static SharedInfoObject Create(uint8_t aType);
        static void RegisterInfoObject(int aType, int aPriority, const std::function<SharedInfoObject(void)>& arCreationFunction);

    private:
        class LookupKey
        {
        public:
            enum {ANY_PRIORITY = 0};

            LookupKey(int aType, int aPriority)
                : mType(aType), mPriority(aPriority) {}

            // Compare type id only
            bool operator==(const LookupKey& arOther) const noexcept { return mType == arOther.mType; }
            bool operator<(const LookupKey& arOther) const noexcept { return mType < arOther.mType; }
            int GetPriority() const noexcept { return mPriority; }

        private:
            int mType, mPriority;
        };


        InfoObjectFactory(); // No instance
        using CreateFunctions = std::map<LookupKey, std::function<SharedInfoObject(void)>>;
        static CreateFunctions msFunctions;
    };
    
    /**
     * @brief Compile-time registration of IEC104 info objects
     *
     * Example: Registration for SinglePoint (Type ID == 1):
     *
     * source.cpp:
     * @code
     *   #include "infoobjects.hpp"
     *
     *   static StaticRegistration<1, SinglePoint> gRegisterType1;
     * @endcode
     *
     * @tparam TYPE_ID IEC 60870-5-101/-104 type id
     * @tparam INFOOBJECT Class to be created for type id
     */

    enum class RegisteredBy
    {
        INTERNAL = 0,
        EXTERNAL = 1
    };

    template <int TYPE_ID, typename INFOOBJECT, RegisteredBy aPriority>
    class StaticRegistration
    {
    public:
        explicit StaticRegistration() noexcept
        {
			InfoObjectFactory::RegisterInfoObject(TYPE_ID, static_cast<int> (aPriority),
				[]() -> SharedInfoObject
				{
					return SharedInfoObject(new INFOOBJECT);
				});
        }
    };

    class BaseInfoObject : public DataStruct
    {
    public:
        // Standalone info object. Can be added to an ASDU via append
        BaseInfoObject(const std::string& arName, int aTypeId);

        int GetTypeId() const;
        IEC104::InfoAddress GetAddress() const;

        virtual void ReadFrom(ByteStream& arInput, int aAddressSize);
        virtual void WriteTo(ByteStream& arOutput) const;

        /*
         * Modification: Type is written only, if the object is a standalone object (without an ASDU)
         */
        void WriteJson(JSON::ValueHandle& arValue) const override;

    protected:
        void RequireNull(int aChecked);

    private:
        DataEnum<TypeEnum> mType;
        DataInfoAddress mAddress;
    };

    // Type 1: M_SP_NA_1 ////////////////////////////////////////////////////////////
    class DataSinglePoint : public BaseInfoObject
    {
    public:
        DataSinglePoint();
        void ReadFrom(ByteStream& arInput, int aAddressSize) override;
        void WriteTo(ByteStream& arOutput) const override;

    private:
        DataBool mValue;
        DataQuality mQuality;
    };

    // Type 3: M_DP_NA_1 ////////////////////////////////////////////////////////////
    class DataDoublePoint : public BaseInfoObject
    {
    public:
        DataDoublePoint();
        void ReadFrom(ByteStream& arInput, int aAddressSize) override;
        void WriteTo(ByteStream& arOutput) const override;

    private:
        DataEnum<DoublePointEnum> mValue;
        DataQuality mQuality;
    };

    // Type 11: M_ME_NB_1 ////////////////////////////////////////////////////////////
    class DataMeasuredScaled : public BaseInfoObject
    {
    public:
        DataMeasuredScaled();
        void ReadFrom(ByteStream& arInput, int aAddressSize) override;
        void WriteTo(ByteStream& arOutput) const override;

    private:
        DataInt mValue;
        DataQuality mQuality;
    };

    // Type 13: M_ME_NC_1 ////////////////////////////////////////////////////////////
    class DataMeasuredFloat : public BaseInfoObject
    {
    public:
        DataMeasuredFloat();
        void ReadFrom(ByteStream& arInput, int aAddressSize) override;
        void WriteTo(ByteStream& arOutput) const override;

    private:
        DataFloat mValue;
        DataQuality mQuality;
    };
}
#endif
