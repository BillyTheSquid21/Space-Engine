#pragma once
#ifndef XML_WRAPPER_HPP
#define XML_WRAPPER_HPP

#include <memory>
#include <rapidxml/rapidxml.hpp>
#include <string>

struct XML_Doc_Wrapper
{
	std::shared_ptr<std::string> data;
	std::shared_ptr<rapidxml::xml_document<>> doc;
};

#endif