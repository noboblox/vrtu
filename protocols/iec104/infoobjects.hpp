#ifndef IEC104_INFOOBJECTS_HPP_
#define IEC104_INFOOBJECTS_HPP_

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "protocols/iec104/104enums.hpp"
#include "protocols/iec104/infoaddress.hpp"
#include "protocols/iec104/quality.hpp"

namespace IEC104
{
	class BaseInfoObject;

    using SharedInfoObject = std::shared_ptr<BaseInfoObject>;
    class InfoObjectFactory
    {
    public:
        static SharedInfoObject Create(uint8_t aType);
        static void RegisterInfoObject(int aType, int aPriority, int aInfoElementSize, 
                                       const std::function<SharedInfoObject(void)>& arCreationFunction);
        static int GetSize(uint8_t aType) noexcept;
        static bool HasType(uint8_t aType) noexcept;

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

        static std::map<LookupKey, int> msRegistered;
        static std::vector<std::function<SharedInfoObject(void)>> msFunctions;
        static std::vector<int> msSizes;
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
     * @tparam SIZE Decoding size of the InfoObject excluding the IOA size
     */

    enum class RegisteredBy
    {
        INTERNAL = 0,
        EXTERNAL = 1
    };

    template <typename INFOOBJECT, int TYPE_ID, uint16_t SIZE, RegisteredBy aPriority>
    class StaticRegistration
    {
    public:
        explicit StaticRegistration() noexcept
        {
			InfoObjectFactory::RegisterInfoObject(TYPE_ID, static_cast<int> (aPriority), SIZE,
				[]() -> SharedInfoObject
				{
					return SharedInfoObject(new INFOOBJECT);
				});
        }
    };

    class BaseInfoObject
    {
    public:
        int GetTypeId() const;
        IEC104::InfoAddress GetAddress() const;

        virtual void ReadFrom(ByteStream& arInput, int aAddressSize);
        virtual void WriteTo(ByteStream& arOutput) const;

        template <typename INFOOBJECT>
        INFOOBJECT& As()
        {
            if (INFOOBJECT::TYPE_ID != GetTypeId())
                throw std::invalid_argument("Cannot convert to target type");
            return static_cast<INFOOBJECT&>(*this);
        }
        
        template <typename INFOOBJECT>
        const INFOOBJECT& As() const
        {
            if (INFOOBJECT::TYPE_ID != GetTypeId())
                throw std::invalid_argument("Cannot convert to target type");
            return static_cast<const INFOOBJECT&>(*this);
        }

        // Standalone info object. Can be added to an ASDU via append
        BaseInfoObject(int aTypeId);

    private:
        int mType;
        InfoAddress mAddress;
    };


    // Type 1: M_SP_NA_1 ////////////////////////////////////////////////////////////
    class DataSinglePoint : public BaseInfoObject
    {
    public:
        static constexpr int TYPE_ID   = Type::M_SP_NA_1;
        static constexpr int DATA_SIZE = 1;

        DataSinglePoint() : BaseInfoObject(TYPE_ID) {}
        void ReadFrom(ByteStream& arInput, int aAddressSize) override;
        void WriteTo(ByteStream& arOutput) const override;

        bool val = false;
        Quality q = Quality();
    };

    // Type 3: M_DP_NA_1 ////////////////////////////////////////////////////////////
    class DataDoublePoint : public BaseInfoObject
    {
    public:
        static constexpr int TYPE_ID   = Type::M_DP_NA_1;
        static constexpr int DATA_SIZE = 1;

        DataDoublePoint() : BaseInfoObject(TYPE_ID) {}
        void ReadFrom(ByteStream& arInput, int aAddressSize) override;
        void WriteTo(ByteStream& arOutput) const override;

        DoublePointEnum val = DoublePoint::OFF;
        Quality q = Quality();
    };

    // Type 11: M_ME_NB_1 ////////////////////////////////////////////////////////////
    class DataMeasuredScaled : public BaseInfoObject
    {
    public:
        static constexpr int TYPE_ID = Type::M_ME_NB_1;
        static constexpr int DATA_SIZE = 3;

        DataMeasuredScaled() : BaseInfoObject(TYPE_ID) {}
        void ReadFrom(ByteStream& arInput, int aAddressSize) override;
        void WriteTo(ByteStream& arOutput) const override;

        int val = 0;
        // TODO ValueQuality
    };

    // Type 13: M_ME_NC_1 ////////////////////////////////////////////////////////////
    class DataMeasuredFloat : public BaseInfoObject
    {
    public:
        static constexpr int TYPE_ID = Type::M_ME_NC_1;
        static constexpr int DATA_SIZE = 5;

        DataMeasuredFloat() : BaseInfoObject(TYPE_ID) {}
        void ReadFrom(ByteStream& arInput, int aAddressSize) override;
        void WriteTo(ByteStream& arOutput) const override;

        float val = 0.0;
        // TODO ValueQuality
    };

    // Type 100: C_IC_NA_1 ////////////////////////////////////////////////////////////
    class DataInterrogationCommand : public BaseInfoObject
    {
    public:
        static constexpr int TYPE_ID = Type::C_IC_NA_1;
        static constexpr int DATA_SIZE = 1;

        DataInterrogationCommand() : BaseInfoObject(TYPE_ID) {}
        void ReadFrom(ByteStream& arInput, int aAddressSize) override;
        void WriteTo(ByteStream& arOutput) const override;

        InterrogationQualifierEnum val = InterrogationQualifier::UNUSED;
        // TODO Check if members are correct
    };
}
#endif
