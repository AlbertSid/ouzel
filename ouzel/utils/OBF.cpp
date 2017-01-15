// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "OBF.h"
#include "Utils.h"

namespace ouzel
{
    namespace obf
    {
        // reading
        static uint32_t readInt8(const std::vector<uint8_t>& buffer, uint32_t offset, uint8_t& result)
        {
            if (buffer.size() - offset < sizeof(result))
            {
                return 0;
            }

            result = *reinterpret_cast<const uint8_t*>(buffer.data() + offset);

            return sizeof(result);
        }

        static uint32_t readInt16(const std::vector<uint8_t>& buffer, uint32_t offset, uint16_t& result)
        {
            if (buffer.size() - offset < sizeof(result))
            {
                return 0;
            }

            result = decodeUInt16Big(buffer.data() + offset);

            return sizeof(result);
        }

        static uint32_t readInt32(const std::vector<uint8_t>& buffer, uint32_t offset, uint32_t& result)
        {
            if (buffer.size() - offset < sizeof(result))
            {
                return 0;
            }

            result = decodeUInt32Big(buffer.data() + offset);

            return sizeof(result);
        }

        static uint32_t readInt64(const std::vector<uint8_t>& buffer, uint32_t offset, uint64_t& result)
        {
            if (buffer.size() - offset < sizeof(result))
            {
                return 0;
            }

            result = decodeUInt64Big(buffer.data() + offset);

            return sizeof(result);
        }

        static uint32_t readFloat(const std::vector<uint8_t>& buffer, uint32_t offset, float& result)
        {
            if (buffer.size() - offset < sizeof(float))
            {
                return 0;
            }

            result = *reinterpret_cast<const float*>(buffer.data() + offset);

            return sizeof(result);
        }

        static uint32_t readDouble(const std::vector<uint8_t>& buffer, uint32_t offset, double& result)
        {
            if (buffer.size() - offset < sizeof(double))
            {
                return 0;
            }

            result = *reinterpret_cast<const double*>(buffer.data() + offset);

            return sizeof(result);
        }

        static uint32_t readString(const std::vector<uint8_t>& buffer, uint32_t offset, std::string& result)
        {
            uint32_t originalOffset = offset;

            if (buffer.size() - offset < sizeof(uint16_t))
            {
                return 0;
            }

            uint16_t length = decodeUInt16Big(buffer.data() + offset);

            offset += sizeof(length);

            if (buffer.size() - offset < length)
            {
                return 0;
            }

            result.assign(reinterpret_cast<const char*>(buffer.data() + offset), length);
            offset += length;

            return offset - originalOffset;
        }

        static uint32_t readLongString(const std::vector<uint8_t>& buffer, uint32_t offset, std::string& result)
        {
            uint32_t originalOffset = offset;

            if (buffer.size() - offset < sizeof(uint32_t))
            {
                return 0;
            }

            uint32_t length = decodeUInt32Big(buffer.data() + offset);

            offset += sizeof(length);

            if (buffer.size() - offset < length)
            {
                return 0;
            }

            result.assign(reinterpret_cast<const char*>(buffer.data() + offset), length);
            offset += length;

            return offset - originalOffset;
        }

        static uint32_t readByteArray(const std::vector<uint8_t>& buffer, uint32_t offset, std::vector<uint8_t>& result)
        {
            uint32_t originalOffset = offset;

            if (buffer.size() - offset < sizeof(uint32_t))
            {
                return 0;
            }

            uint32_t length = decodeUInt32Big(buffer.data() + offset);

            offset += sizeof(length);

            if (buffer.size() - offset < length)
            {
                return 0;
            }

            result.assign(reinterpret_cast<const uint8_t*>(buffer.data() + offset),
                          reinterpret_cast<const uint8_t*>(buffer.data() + offset) + length);
            offset += length;

            return offset - originalOffset;
        }

        static uint32_t readObject(const std::vector<uint8_t>& buffer, uint32_t offset, std::map<uint32_t, Value>& result)
        {
            uint32_t originalOffset = offset;

            if (buffer.size() - offset < sizeof(uint32_t))
            {
                return 0;
            }

            uint32_t count = decodeUInt32Big(buffer.data() + offset);

            offset += sizeof(count);

            for (uint32_t i = 0; i < count; ++i)
            {
                if (buffer.size() - offset < sizeof(uint32_t))
                {
                    return 0;
                }

                uint32_t key = decodeUInt32Big(buffer.data() + offset);

                offset += sizeof(key);

                Value node;

                uint32_t ret = node.decode(buffer, offset);

                if (ret == 0)
                {
                    return 0;
                }
                offset += ret;

                result[static_cast<uint32_t>(key)] = node;
            }

            return offset - originalOffset;
        }

        static uint32_t readArray(const std::vector<uint8_t>& buffer, uint32_t offset, std::vector<Value>& result)
        {
            uint32_t originalOffset = offset;

            if (buffer.size() - offset < sizeof(uint32_t))
            {
                return 0;
            }

            uint32_t count = decodeUInt32Big(buffer.data() + offset);

            offset += sizeof(count);

            for (uint32_t i = 0; i < count; ++i)
            {
                Value node;
                uint32_t ret = node.decode(buffer, offset);

                if (ret == 0)
                {
                    return 0;
                }

                offset += ret;

                result.push_back(node);
            }

            return offset - originalOffset;
        }

        static uint32_t readDictionary(const std::vector<uint8_t>& buffer, uint32_t offset, std::map<std::string, Value>& result)
        {
            uint32_t originalOffset = offset;

            if (buffer.size() - offset < sizeof(uint32_t))
            {
                return 0;
            }

            uint32_t count = decodeUInt32Big(buffer.data() + offset);

            offset += sizeof(count);

            for (uint32_t i = 0; i < count; ++i)
            {
                if (buffer.size() - offset < sizeof(uint16_t))
                {
                    return 0;
                }

                uint16_t length = decodeUInt16Big(buffer.data() + offset);

                offset += sizeof(length);

                if (buffer.size() - offset < length)
                {
                    return 0;
                }

                std::string key(reinterpret_cast<const char*>(buffer.data() + offset), length);
                offset += length;

                Value node;

                uint32_t ret = node.decode(buffer, offset);

                if (ret == 0)
                {
                    return 0;
                }
                offset += ret;

                result[key] = node;
            }

            return offset - originalOffset;
        }

        // writing
        static uint32_t writeInt8(std::vector<uint8_t>& buffer, uint8_t value)
        {
            buffer.push_back(value);

            return sizeof(value);
        }

        static uint32_t writeInt16(std::vector<uint8_t>& buffer, uint16_t value)
        {
            uint8_t data[sizeof(value)];

            encodeUInt16Big(data, value);

            buffer.insert(buffer.end(), std::begin(data), std::end(data));

            return sizeof(value);
        }

        static uint32_t writeInt32(std::vector<uint8_t>& buffer, uint32_t value)
        {
            uint8_t data[sizeof(value)];

            encodeUInt32Big(data, value);

            buffer.insert(buffer.end(), std::begin(data), std::end(data));

            return sizeof(value);
        }

        static uint32_t writeInt64(std::vector<uint8_t>& buffer, uint64_t value)
        {
            uint8_t data[sizeof(value)];

            encodeUInt64Big(data, value);

            buffer.insert(buffer.end(), std::begin(data), std::end(data));

            return sizeof(value);
        }

        static uint32_t writeFloat(std::vector<uint8_t>& buffer, float value)
        {
            buffer.insert(buffer.end(),
                          reinterpret_cast<const uint8_t*>(&value),
                          reinterpret_cast<const uint8_t*>(&value) + sizeof(value));

            return sizeof(float);
        }

        static uint32_t writeDouble(std::vector<uint8_t>& buffer, double value)
        {
            buffer.insert(buffer.end(),
                          reinterpret_cast<const uint8_t*>(&value),
                          reinterpret_cast<const uint8_t*>(&value) + sizeof(value));

            return sizeof(double);
        }

        static uint32_t writeString(std::vector<uint8_t>& buffer, const std::string& value)
        {
            uint8_t lengthData[sizeof(uint16_t)];

            encodeUInt16Big(lengthData, static_cast<uint16_t>(value.length()));

            buffer.insert(buffer.end(), std::begin(lengthData), std::end(lengthData));

            uint32_t size = sizeof(lengthData);

            buffer.insert(buffer.end(),
                          reinterpret_cast<const uint8_t*>(value.data()),
                          reinterpret_cast<const uint8_t*>(value.data()) + value.length());
            size += static_cast<uint32_t>(value.length());

            return size;
        }

        static uint32_t writeLongString(std::vector<uint8_t>& buffer, const std::string& value)
        {
            uint8_t lengthData[sizeof(uint32_t)];

            encodeUInt32Big(lengthData, static_cast<uint32_t>(value.length()));

            buffer.insert(buffer.end(), std::begin(lengthData), std::end(lengthData));

            uint32_t size = sizeof(lengthData);

            buffer.insert(buffer.end(),
                          reinterpret_cast<const uint8_t*>(value.data()),
                          reinterpret_cast<const uint8_t*>(value.data()) + value.length());
            size += static_cast<uint32_t>(value.length());

            return size;
        }

        static uint32_t writeByteArray(std::vector<uint8_t>& buffer, const std::vector<uint8_t>& value)
        {
            uint8_t lengthData[sizeof(uint32_t)];

            encodeUInt32Big(lengthData, static_cast<uint32_t>(value.size()));

            buffer.insert(buffer.end(), std::begin(lengthData), std::end(lengthData));

            uint32_t size = sizeof(lengthData);

            buffer.insert(buffer.end(), value.begin(), value.end());
            size += static_cast<uint32_t>(value.size());

            return size;
        }

        static uint32_t writeObject(std::vector<uint8_t>& buffer, const std::map<uint32_t, Value>& value)
        {
            uint8_t lengthData[sizeof(uint32_t)];

            encodeUInt32Big(lengthData, static_cast<uint32_t>(value.size()));

            buffer.insert(buffer.end(), std::begin(lengthData), std::end(lengthData));

            uint32_t size = sizeof(lengthData);

            for (const auto& i : value)
            {
                uint8_t keyData[sizeof(uint32_t)];

                encodeUInt32Big(keyData, i.first);

                buffer.insert(buffer.end(), std::begin(keyData), std::end(keyData));

                size += sizeof(keyData);

                size += i.second.encode(buffer);
            }

            return size;
        }

        static uint32_t writeArray(std::vector<uint8_t>& buffer, const std::vector<Value>& value)
        {
            uint8_t lengthData[sizeof(uint32_t)];

            encodeUInt32Big(lengthData, static_cast<uint32_t>(value.size()));

            buffer.insert(buffer.end(), std::begin(lengthData), std::end(lengthData));

            uint32_t size = sizeof(lengthData);

            for (const auto& i : value)
            {
                size += i.encode(buffer);
            }

            return size;
        }

        static uint32_t writeDictionary(std::vector<uint8_t>& buffer, const std::map<std::string, Value>& value)
        {
            uint8_t lengthData[sizeof(uint32_t)];

            encodeUInt32Big(lengthData, static_cast<uint32_t>(value.size()));

            buffer.insert(buffer.end(), std::begin(lengthData), std::end(lengthData));

            uint32_t size = sizeof(lengthData);

            for (const auto& i : value)
            {
                uint8_t lengthData[sizeof(uint16_t)];

                encodeUInt16Big(lengthData, static_cast<uint16_t>(i.first.length()));

                buffer.insert(buffer.end(), std::begin(lengthData), std::end(lengthData));

                size += sizeof(lengthData);

                buffer.insert(buffer.end(),
                              reinterpret_cast<const uint8_t*>(i.first.data()),
                              reinterpret_cast<const uint8_t*>(i.first.data()) + i.first.length());
                size += static_cast<uint32_t>(i.first.length());

                size += i.second.encode(buffer);
            }

            return size;
        }

        uint32_t Value::decode(const std::vector<uint8_t>& buffer, uint32_t offset)
        {
            uint32_t originalOffset = offset;

            if (buffer.size() - offset < 1)
            {
                return 0;
            }

            type = *reinterpret_cast<const Type*>(buffer.data() + offset);
            offset += 1;

            uint32_t ret = 0;

            switch (type)
            {
                case Type::NONE:
                {
                    break;
                }
                case Type::INT8:
                {
                    uint8_t int8Value;
                    if ((ret = readInt8(buffer, offset, int8Value)) == 0)
                    {
                        return 0;
                    }

                    intValue = int8Value;
                    break;
                }
                case Type::INT16:
                {
                    uint16_t int16Value;
                    if ((ret = readInt16(buffer, offset, int16Value)) == 0)
                    {
                        return 0;
                    }

                    intValue = int16Value;
                    break;
                }
                case Type::INT32:
                {
                    uint32_t int32Value;
                    if ((ret = readInt32(buffer, offset, int32Value)) == 0)
                    {
                        return 0;
                    }

                    intValue = int32Value;
                    break;
                }
                case Type::INT64:
                {
                    if ((ret = readInt64(buffer, offset, intValue)) == 0)
                    {
                        return 0;
                    }
                    break;
                }
                case Type::FLOAT:
                {
                    float floatValue;
                    if ((ret = readFloat(buffer, offset, floatValue)) == 0)
                    {
                        return 0;
                    }

                    doubleValue = floatValue;
                    break;
                }
                case Type::DOUBLE:
                {
                    if ((ret = readDouble(buffer, offset, doubleValue)) == 0)
                    {
                        return 0;
                    }
                    break;
                }
                case Type::STRING:
                {
                    if ((ret = readString(buffer, offset, stringValue)) == 0)
                    {
                        return 0;
                    }
                    break;
                }
                case Type::LONG_STRING:
                {
                    if ((ret = readLongString(buffer, offset, stringValue)) == 0)
                    {
                        return 0;
                    }
                    break;
                }
                case Type::BYTE_ARRAY:
                {
                    if ((ret = readByteArray(buffer, offset, byteArrayValue)) == 0)
                    {
                        return 0;
                    }
                    break;
                }
                case Type::OBJECT:
                {
                    if ((ret = readObject(buffer, offset, objectValue)) == 0)
                    {
                        return 0;
                    }
                    break;
                }
                case Type::ARRAY:
                {
                    if ((ret = readArray(buffer, offset, arrayValue)) == 0)
                    {
                        return 0;
                    }
                    break;
                }
                case Type::DICTIONARY:
                {
                    if ((ret = readDictionary(buffer, offset, dictionaryValue)) == 0)
                    {
                        return 0;
                    }
                    break;
                }
                default: return 0;
            }

            offset += ret;

            return offset - originalOffset;
        }

        uint32_t Value::encode(std::vector<uint8_t>& buffer) const
        {
            uint32_t size = 0;

            uint32_t ret = 0;

            buffer.push_back(static_cast<uint8_t>(type));
            size += 1;

            switch (type)
            {
                case Type::NONE: break;
                case Type::INT8: ret = writeInt8(buffer, static_cast<uint8_t>(intValue)); break;
                case Type::INT16: ret = writeInt16(buffer, static_cast<uint16_t>(intValue)); break;
                case Type::INT32: ret = writeInt32(buffer, static_cast<uint32_t>(intValue)); break;
                case Type::INT64: ret = writeInt64(buffer, intValue); break;
                case Type::FLOAT: ret = writeFloat(buffer, static_cast<float>(doubleValue)); break;
                case Type::DOUBLE: ret = writeDouble(buffer, doubleValue); break;
                case Type::STRING: ret = writeString(buffer, stringValue); break;
                case Type::LONG_STRING: ret = writeLongString(buffer, stringValue); break;
                case Type::BYTE_ARRAY: ret = writeByteArray(buffer, byteArrayValue); break;
                case Type::OBJECT: ret = writeObject(buffer, objectValue); break;
                case Type::ARRAY: ret = writeArray(buffer, arrayValue); break;
                case Type::DICTIONARY: ret = writeDictionary(buffer, dictionaryValue); break;
                default: return 0;
            }

            size += ret;

            return size;
        }
    } // namespace obf
} // namespace ouzel
