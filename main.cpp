// Copyright �2015 Black Sphere Studios

#include "bss-util/cJSON.h"
#include "bss-util/cUBJSON.h"
#include <iostream>
#include <fstream>
#include <io.h>

using namespace bss_util;

void convert(const UBJSONValue& from, JSONValue& to)
{
  switch(from.tag())
  {
  case UBJSONValue::Type<cStr>::value: to = from.get<cStr>(); break;
  case UBJSONValue::Type<bool>::value: to = from.get<bool>(); break;
  case UBJSONValue::Type<uint8_t>::value: to = static_cast<int64_t>(from.get<uint8_t>()); break;
  case UBJSONValue::Type<char>::value: to = static_cast<int64_t>(from.get<char>()); break;
  case UBJSONValue::Type<int16_t>::value: to = static_cast<int64_t>(from.get<int16_t>()); break;
  case UBJSONValue::Type<int32_t>::value: to = static_cast<int64_t>(from.get<int32_t>()); break;
  case UBJSONValue::Type<int64_t>::value: to = static_cast<int64_t>(from.get<int64_t>()); break;
  case UBJSONValue::Type<float>::value: to = static_cast<double>(from.get<float>()); break;
  case UBJSONValue::Type<double>::value: to = from.get<double>(); break;
  case UBJSONValue::Type<UBJSONValue::UBJSONArray>::value:
  {
    auto& v = from.get<UBJSONValue::UBJSONArray>();
    to = JSONValue::JSONArray();
    for(size_t i = 0; i < v.Length(); ++i)
    {
      to.get<JSONValue::JSONArray>().SetLength(i + 1);
      convert(v[i], to.get<JSONValue::JSONArray>()[i]);
    }
    break;
  }
  case UBJSONValue::Type<UBJSONValue::UBJSONObject>::value:
  {
    auto& v = from.get<UBJSONValue::UBJSONObject>();
    to = JSONValue::JSONObject();
    for(size_t i = 0; i < v.Length(); ++i)
    {
      to.get<JSONValue::JSONObject>().SetLength(i + 1);
      to.get<JSONValue::JSONObject>()[i].first = v[i].first;
      convert(v[i].second, to.get<JSONValue::JSONObject>()[i].second);
    }
    break;
  }
  }
}

void convert(const JSONValue& from, UBJSONValue& to)
{
  switch(from.tag())
  {
  case JSONValue::Type<cStr>::value:
    if(from.get<cStr>().length() > 6 && !strncmp(from.get<cStr>(), "$file:", 6))
    {
      std::fstream fs(from.get<cStr>().substr(6), std::ios::in | std::ios::binary);
      if(fs)
      {
        to = UBJSONValue::UBJSONBinary();
        auto& v = to.get<UBJSONValue::UBJSONBinary>();
        fs.seekg(0, std::ios_base::end);
        v.SetLength(fs.tellg());
        fs.seekg(0);
        fs.read((char*)(unsigned char*)v, v.Length());
        fs.close();
        break;
      }
    }
    to = from.get<cStr>();
    break;
  case JSONValue::Type<bool>::value: to = from.get<bool>(); break;
  case JSONValue::Type<int64_t>::value: to = from.get<int64_t>(); break;
  case JSONValue::Type<double>::value: to = from.get<double>(); break;
  case JSONValue::Type<JSONValue::JSONArray>::value:
  {
    auto& v = from.get<JSONValue::JSONArray>();
    to = UBJSONValue::UBJSONArray();
    for(size_t i = 0; i < v.Length(); ++i)
    {
      to.get<UBJSONValue::UBJSONArray>().SetLength(i + 1);
      convert(v[i], to.get<UBJSONValue::UBJSONArray>()[i]);
    }
    break;
  }
  case JSONValue::Type<JSONValue::JSONObject>::value:
  {
    auto& v = from.get<JSONValue::JSONObject>();
    to = UBJSONValue::UBJSONObject();
    for(size_t i = 0; i < v.Length(); ++i)
    {
      to.get<UBJSONValue::UBJSONObject>().SetLength(i + 1);
      to.get<UBJSONValue::UBJSONObject>()[i].first = v[i].first;
      convert(v[i].second, to.get<UBJSONValue::UBJSONObject>()[i].second);
    }
    break;
  }
  }

}

int main(int argc, char** argv)
{
  std::string s;
  std::string sout;

  if(!_isatty(_fileno(stdin)))
    while(std::cin >> s);

  if(s.empty())
  {
    if(argc < 2 || !argv[1])
    {
      std::cout << "No argument supplied." << std::endl;
      return -1;
    }
    s = argv[1];
  }

  if(argc > 2)
    sout = argv[2];

  std::fstream fs(s, std::ios::in | std::ios::binary);
  if(!fs)
  {
    std::cout << "File does not exist: " << s << std::endl;
    return -2;
  }

  char isubj = fs.get() == '{' &&
    (fs.peek() == UBJSONTuple::TYPE_INT8 ||
      fs.peek() == UBJSONTuple::TYPE_UINT8 ||
      fs.peek() == UBJSONTuple::TYPE_INT16 ||
      fs.peek() == UBJSONTuple::TYPE_INT32 ||
      fs.peek() == UBJSONTuple::TYPE_INT64 ||
      fs.peek() == UBJSONTuple::TYPE_CHAR ||
      fs.peek() == UBJSONTuple::TYPE_COUNT ||
      fs.peek() == UBJSONTuple::TYPE_TYPE);

  fs.seekg(0);

  if(sout.empty())
  {
    sout = s;
    const char* dot = strrchr(sout.c_str(), '.');
    if(dot)
      sout.resize(dot - sout.c_str());
    sout += isubj ?".json": ".ubj";
  }

  std::fstream fout(sout, std::ios::out | std::ios::trunc | std::ios::binary);
  if(!fs)
  {
    std::cout << "Could not open output file: " << sout << std::endl;
    return -3;
  }

  if(isubj)
  {
    UBJSONValue file;
    ParseUBJSON<UBJSONValue>(file, fs);
    fs.close();
    JSONValue out;
    convert(file, out);
    WriteJSON<JSONValue>(out, fout);
  }
  else
  {
    JSONValue file;
    ParseJSON<JSONValue>(file, fs);
    fs.close();
    UBJSONValue out;
    convert(file, out);
    WriteUBJSON<UBJSONValue>(out, fout);
  }

  fout.close();
  return 0;
}