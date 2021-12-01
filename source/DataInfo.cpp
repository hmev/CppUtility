#include "DataInfo.h"

namespace utility
{
    DataInfo::DataInfo(const std::string_view& _status): status(_status){}

    DataInfo::DataInfo(const std::string_view& _status, const InfoSetType& _info)
        : status(_status), info(_info){}

    void DataInfo::Append(const FieldType& key, const ValueType& value)
    {
        info.insert({key, value});
    }

    void DataInfo::Append(const InfoSetType& _another)
    {
        for (auto pair : _another) info.insert(pair);
    }

    void DataInfo::Output(std::ostream& os, OutputFormat format) const
    {
        switch (format)
        {
        case(Compact):
            OutputCompact(os);
            break;
        case(Equation):
            OutputEquation(os);
            break;
        case(JSON):
            OutputJson(os);
            break;
        }
    }

    void DataInfo::OutputCompact(std::ostream& os) const
    {
        os << "Header";
        for (auto pair : info)
            os << pair.first << " ";
        os << std::endl;
        for (auto pair : info) 
            os << pair.second << " ";
        os << std::endl;
    }

    void DataInfo::OutputEquation(std::ostream& os) const
    {
        for (auto pair: info)
            os << pair.first << " = " << std::any_cast<std::string>(pair.second) << ", ";
    }
}