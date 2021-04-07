#ifndef NAMEDENUM_HPP_
#define NAMEDENUM_HPP_

#include <stdexcept>
#include <string>
#include <vector>

/**
 * @brief Class for managing enums with string labels for each value
 * 
 * To use this class three steps are necessary:
 *  - 1. Define a new enum
 *  - 2. Typedef a new type which specializes this template-class with the new enum
 *  - 3. Initialize the static member "msDefinition" of this specialized template-class once within a source file
 */
template <typename Enum>
class NamedEnum
{
    // TODO: msDefinition has no checks for duplicates
    // TODO: Default construction means value 0. Provide a configurable default value
    /*
     * TODO: Value based ranged checks! Currently invalid values are possible and arent checked.
     * This is undesirable as they are usually read from the ByteStream, 
     * which can contain garbage values from network
     */

public:
    class Definition
    {
    public:
        Definition(Enum aValue, const char* apLabel)
            : mValue(aValue), mLabel(apLabel)
        {
        }

        Enum GetValue() const noexcept { return mValue; }
        const std::string& GetLabel() const noexcept { return mLabel; }

    private:
        Enum mValue;
        std::string mLabel;
    };

    using DefinitionContainer = const std::vector<Definition>;

    NamedEnum()
        : mValue()
    {
    }

    NamedEnum(Enum aValue)
        : mValue(aValue)
    {
    }

    NamedEnum(const std::string& arLabel)
        : mValue(FindValue(arLabel))
    {
    }

    operator Enum() const noexcept { return mValue; }

    Enum GetValue() const
    {
        return mValue;
    }

    const std::string& GetLabel(bool aAllowUndefined = false) const
    {
        for (const auto& r_def : msDefinition)
        {
            if (r_def.GetValue() == mValue)
                return r_def.GetLabel();
        }
        
        if (aAllowUndefined)
        {
            static const std::string LABEL_UNDEFINED("unknown");
            return LABEL_UNDEFINED;
        }

        throw std::invalid_argument("Value does not have a label");
    }

private:
    void FindValue(const std::string& arLabel) const
    {
        for (const auto& r_def : msDefinition)
        {
            if (r_def.GetLabel() == arLabel)
                return r_def.GetValue();
        }
        throw std::invalid_argument("Label is not part of the enum");
    }

    Enum mValue;
    static DefinitionContainer msDefinition;
};

#endif