#pragma once
#include <string>
#include <string_view>
#include <optional>
#include <vector>
#include <ostream>

struct EcppOptions
{
  std::ostream* output            = nullptr;
  std::string   output_name;
  std::string   crails_include    = "crails/";
  std::string   parent_header     = "crails/template.hpp";
  std::string   parent_class      = "Crails::Template";
  bool          inherited_stream  = false;
  std::string   out_property_name = "ecpp_stream";
  std::string   body_mode         = "markup";
  std::string   function_prefix   = "render";
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
