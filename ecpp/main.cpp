#include <crails/utils/string.hpp>
#include <crails/read_file.hpp>
#include <crails/renderer.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>
#include "ecpp_header_parser.hpp"
#include "ecpp_body.hpp"

using namespace std;
using namespace boost;

std::string ecpp_generate(const std::string& source, const EcppOptions& options);

void throw_template_error(std::string_view error_desc, const std::string& source, unsigned int cursor, unsigned int header_lines)
{
  std::stringstream message;
  unsigned int line = 1 + header_lines;
  unsigned int col = 0;

  for (unsigned int i = 0 ; i < cursor ; ++i, ++col)
  { if (source[i] == '\n') { line++; col = 0; } }
  message << "line " << line << ':' << col << " > " << error_desc;
  throw std::runtime_error(message.str());
}

static string path_to_classname(const std::string& path, const std::string& prefix)
{
  const string base = prefix.length() ? prefix + '_' + path : path;
  string result;
  bool uppercase = true;

  result = "Ecpp";
  for (unsigned short i = 0 ; i < base.size() ; ++i)
  {
    if (base[i] >= 'a' && base[i] <= 'z' && uppercase)
    {
      result += base[i] - 'a' + 'A';
      uppercase = false;
    }
    else if ((base[i] >= 'a' && base[i] <= 'z') || (base[i] >= 'A' && base[i] <= 'Z') || (base[i] >= '0' && base[i] <= '9'))
      result += base[i];
    else if (!uppercase)
      uppercase = true;
  }
  return result;
}

static std::ofstream* output_stream;

static EcppOptions options_from_command_line(program_options::variables_map options)
{
  EcppOptions result;

  if (options.count("crails-include"))
  {
    result.crails_include = options["crails-include"].as<string>();
    result.parent_header = options["crails-include"].as<string>() + "template.hpp";
  }
  if (options.count("template-header"))
    result.parent_header = options["template-header"].as<string>();
  if (options.count("template-type"))
    result.parent_class = options["template-type"].as<string>();
  if (options.count("out_property_name"))
    result.out_property_name = options["out-property-name"].as<string>();
  if (options.count("render-mode"))
    result.body_mode = options["render-mode"].as<string>();
  if (options.count("function-prefix"))
    result.function_prefix = options["function-prefix"].as<string>();
  if (options.count("stream-property"))
  {
    result.out_property_name = options["stream-property"].as<string>();
    result.inherited_stream = true;
  }
  if (options.count("name"))
    result.output_name = options["name"].as<string>();
  else
    result.output_name = path_to_classname(options["input"].as<string>(), result.function_prefix.data());
  if (options.count("output"))
  {
    output_stream = new std::ofstream(options["output"].as<string>());
    if (output_stream->is_open())
      result.output = output_stream;
    else
      throw std::runtime_error("Failed to open `" + options["output"].as<string>() + '`');
  }
  else
    result.output = &std::cout;
  return result;
}

int main(int argc, char** argv)
{
  program_options::variables_map options;
  program_options::options_description desc("Options");
  string input;

  desc.add_options()
    ("help,h",             "product help message")
    ("name,n",             program_options::value<std::string>(), "class name for your template")
    ("input,i",            program_options::value<std::string>(), "ecpp source to process")
    ("output,o",           program_options::value<std::string>(), "output file")
    ("crails-include,c",   program_options::value<std::string>(), "include folder to crails (defaults to `crails`)")
    ("template-type,t",    program_options::value<std::string>(), "name of the template class (default to Crails::Template)")
    ("template-header,z",  program_options::value<std::string>(), "path to the header defining the parent class")
    ("render-mode,m",      program_options::value<std::string>(), "raw or markup (defaults to `markup`)")
    ("function-prefix,p",  program_options::value<std::string>(), "use a prefix for the generated template functions (defaults to `render`)")
    ("stream-property",    program_options::value<std::string>(), "use a stream property provided by the template-type");
  program_options::store(program_options::parse_command_line(argc, argv, desc), options);
  program_options::notify(options);
  if (options.count("help"))
    std::cout << "usage: " << argv[0] << " -i [filename] [options]" << std::endl << desc;
  else if (options.count("input") == 0)
    return -1;
  else if (Crails::read_file(options["input"].as<string>(), input))
  {
    try
    {
       EcppOptions ecpp_options = options_from_command_line(options);

       *ecpp_options.output << ecpp_generate(input, ecpp_options);
    }
    catch (const std::runtime_error& e)
    {
      std::cerr << options["input"].as<string>() << ": " << e.what() << std::endl;
      return -1;
    }
  }
  if (output_stream)
  {
    output_stream->close();
    delete output_stream;
  }
  return 0;
}
