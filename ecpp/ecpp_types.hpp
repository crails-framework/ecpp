#pragma once
#include <string>
#include <string_view>
#include <optional>
#include <vector>
#include <ostream>

struct EcppOptions
{
  std::ostream*    output            = nullptr;
  std::string      output_name;
  std::string_view crails_include    = "crails/";
  std::string      parent_header     = "crails/template.hpp";
  std::string_view parent_class      = "Crails::Template";
  bool             inherited_stream  = false;
  std::string_view out_property_name = "ecpp_stream";
  std::string_view body_mode         = "markup";
  std::string_view function_prefix   = "render";
};

struct EcppProperty
{
  bool                       shared = false;
  bool                       pointer = false;
  bool                       reference = false;
  std::string                type;
  std::string                name;
  std::optional<std::string> default_value;
};

struct EcppHeader
{
  std::string_view          name;
  std::vector<std::string>  preprocessor;
  std::vector<EcppProperty> properties;
  unsigned int              line_count = 0;
};
