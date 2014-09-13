#pragma once

// namespace
namespace ge {

////////////////////////////////////////////////////////////////////////////////
///	Material is a resource representing surface properties.
///
////////////////////////////////////////////////////////////////////////////////


class IMaterial 
{
public:
	virtual void release() = 0;

};


} // namespace