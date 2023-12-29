#include "ecpp_header_parser.hpp"
#include "ecpp_body.hpp"
#include <crails/utils/string.hpp>
#include <sstream>

using namespace std;

static bool property_has_initializer(EcppProperty property) { return property.shared || property.default_value.has_value(); }

static string ecpp_result(const EcppHeader& header, const string& body, const EcppOptions& options)
{
  stringstream result;
  string header_fullname = options.function_prefix.length()
    ? string(options.function_prefix.data()) + '_' + string(header.name)
    : string(header.name);

  result
    << "#include <sstream>" << endl
    << "#include \"" << options.crails_include << "render_target.hpp\"" << endl
    << "#include \"" << options.crails_include << "shared_vars.hpp\"" << endl
    << "#include \"" << options.parent_header << '"' << endl;
   for (auto preprocessor_line : header.preprocessor)
    result << preprocessor_line << endl;
  result << endl;

  result
    << "class " << header_fullname << " : public " << options.parent_class << endl
    << '{' << endl
    << "public:" << endl;

  // Constructor and property initializers
  result
    << "  " << header_fullname << "(const Crails::Renderer& renderer, Crails::RenderTarget& target, Crails::SharedVars& vars) :" << endl
    << "    " << options.parent_class << "(renderer, target, vars)";
  for (auto property : header.properties)
  {
    if (property_has_initializer(property))
    {
      result << ", " << endl << "    ";
      if (property.shared)
      {
        string casting_type = property.type;

        result << property.name << '(';
        if (property.reference)
        {
          casting_type[casting_type.length() - 1] = '*';
          result << "reinterpret_cast<" << property.type << ">(*";
        }
        if (property.default_value.has_value())
          result << "Crails::cast<" << casting_type << ">(vars, \"" << property.name << "\", " << property.default_value.value() << ')';
        else
          result << "Crails::cast<" << casting_type << ">(vars, \"" << property.name << "\")";
        if (property.reference)
          result << ')';
        result << ')';
      }
      else if (property.default_value.has_value())
        result << property.name << '(' << property.default_value.value() << ')';
    }
  }
  result << endl << "  {}" << endl << endl;

  // Render
  result
    << "  void render()" << endl
    << "  {" << endl
    << body << endl
    << "    std::string _out_buffer = " << options.out_property_name << ".str();" << endl
    << "    _out_buffer = this->apply_post_render_filters(_out_buffer);" << endl
    << "    this->target.set_body(_out_buffer);" << endl
    << "  }" << endl;

  // Properties
  if (!options.inherited_stream)
  {
    result << "private:" << endl
      << "  std::stringstream " << options.out_property_name << ';' << endl;
  }
  for (auto property : header.properties)
    result << "  " << property.type << ' ' << property.name << ';' << endl;
  result << "};" << endl << endl;

  // Exported function
  result
    << "void " << options.function_prefix << '_' << Crails::underscore(header.name.data())
    << "(const Crails::Renderer& renderer, Crails::RenderTarget& target, Crails::SharedVars& vars)" << endl
    << '{' << endl
    << "  " << header_fullname << "(renderer, target, vars).render();" << endl
    << '}';
  return result.str();
}

static pair<string, string> ecpp_split_template_header_and_body(const string& source)
{
  regex zone_delimiter("^//\\s+END\\s+LINKING\\s*$", regex_constants::multiline | regex_constants::icase);
  auto  match = sregex_iterator(source.begin(), source.end(), zone_delimiter);

  if (match != sregex_iterator())
  {
    return {
      source.substr(0, match->position(0)), // header
      source.substr(match->position(0) + match->length() + 1) // body
    };
  }
  return {"", source};
}

string ecpp_generate(const string& source, const EcppOptions& options)
{
  auto       parts = ecpp_split_template_header_and_body(source);
  string     body;
  EcppHeader header = EcppHeaderParser(options.output_name, parts.first);

  if (parts.first.length() > 0)
    header.line_count++;
  body = options.body_mode == "markup"
    ? EcppMarkupBody(parts.second, header.line_count, options.out_property_name).str()
    : parts.second;
  return ecpp_result(header, body, options);
}
