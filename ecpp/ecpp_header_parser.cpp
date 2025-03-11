#include "ecpp_header_parser.hpp"
#include "ecpp_property_parser.hpp"
#include <iostream>

using namespace std;

EcppHeaderParser::EcppHeaderParser(string_view name, const string& source) : source(source)
{
  this->name = name;
  for (unsigned int i = 0 ; i < source.length() ; ++i)
  { if (source[i] == '\n') line_count++; }
  while (cursor < source.length())
  {
    switch (context)
    {
    case Lookup:
      advance_lookup();
      break ;
    case Preprocessor:
      advance_preprocessor();
      break ;
    case Typedef:
    case Namespace:
    case Extern:
      advance_typedef_or_namespace();
      break ;
    case Comment:
    case MultilineComment:
      advance_comment();
      break ;
    case Property:
      advance_property();
      break ;
    }
  }
}

void EcppHeaderParser::advance_lookup()
{
  pattern_begin = cursor;
  if (source[cursor] == '#')
  {
    context = Preprocessor;
    cursor++;
  }
  else if (source.substr(cursor, 8) == "typedef ")
  {
    context = Typedef;
    cursor += 8;
  }
  else if (source.substr(cursor, 6) == "using ")
  {
    context = Namespace;
    cursor += 6;
  }
  else if (source.substr(cursor, 7) == "extern ")
  {
    context = Extern;
    cursor += 7;
  }
  else if (source[cursor] == '/' && source[cursor + 1] == '/')
  {
    context = Comment;
    cursor += 2;
  }
  else if (source[cursor] == '/' && source[cursor + 1] == '*')
  {
    context = MultilineComment;
    cursor += 2;
  }
  else if (source[cursor] != ' ' && source[cursor] != '\t' && source[cursor] != '\n' && source[cursor] != '\r')
  {
    context = Property;
    cursor++;
  }
  else
    cursor++;
}

void EcppHeaderParser::advance_comment()
{
  if (context == Comment && source[cursor] == '\n')
  {
    context = Lookup;
    cursor++;
  }
  else if (context == MultilineComment && source[cursor] == '*' && source[cursor + 1] == '/')
  {
    context = Lookup;
    cursor += 2;
  }
  else
    cursor++;
}

void EcppHeaderParser::advance_preprocessor()
{
  if (source[cursor] == '\n')
  {
    preprocessor.push_back(source.substr(pattern_begin, cursor - pattern_begin));
    context = Lookup;
    cursor++;
  }
  else if (source[cursor] == '\\' && source[cursor + 1] == '\n')
    cursor += 2;
  else
    cursor++;
}

void EcppHeaderParser::advance_typedef_or_namespace()
{
  if (source[cursor] == ';')
  {
    preprocessor.push_back(source.substr(pattern_begin, cursor - pattern_begin + 1));
    context = Lookup;
  }
  cursor++;
}

void EcppHeaderParser::advance_property()
{
  EcppProperty property = EcppPropertyParser(source, cursor).property;

  if (property.name.length() > 0)
    properties.push_back(property);
  context = Lookup;
}
