#pragma once

#include <unordered_map>
#include <string_view>
#include <any>
#include <ostream>
#include <variant>

namespace utility
{
    class DataInfo
    {
    public:
        using FieldType = std::string_view;
        using ValueType = std::variant<unsigned int, int, long, long long, float, double, long double, bool, std::string, std::string_view>;
        using InfoSetType = std::unordered_map<FieldType, ValueType>;

    public:
        std::string_view    status;
        InfoSetType         info;
        int                 precision;

        DataInfo(const std::string_view& status);
        DataInfo(const std::string_view& status, const InfoSetType& info);

        void Append(const FieldType& key, const ValueType& value);
        void Append(const InfoSetType& _another);

        enum OutputFormat 
        {
            Compact,
            Equation,
            JSON,
        };

        void SetPrecision(int p);
        void Flush(std::ostream& os);
        void Output(std::ostream& os, OutputFormat format) const;

    private:
        void OutputCompact(std::ostream& os) const;
        void OutputEquation(std::ostream& os) const;
        void OutputJson(std::ostream& os) const;
    };
}