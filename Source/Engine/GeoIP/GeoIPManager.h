// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_GEOIP_MANAGER_
#define _ENGINE_GEOIP_MANAGER_

#include "Generic/Patterns/Singleton.h"

#include "Engine/Engine/FrameTime.h"

#include "Generic/Types/HashTable.h"

#include "LibGeoIP/GeoIP.h"

#include <string>
#include <vector>

struct GeoIPResult
{
	MEMORY_ALLOCATOR(GeoIPResult, "Engine");

public:
	int			CountryID;
	const char* CountryName;
	const char*	CountryShortName;
};	

class GeoIPManager : public Singleton<GeoIPManager>
{
	MEMORY_ALLOCATOR(GeoIPManager, "Engine");

private:
	GeoIP*						m_database;
	HashTable<GeoIPResult, int>	m_cache;

public:
	~GeoIPManager();
	GeoIPManager();

	// Loads a geoip database from the given data buffer.
	bool Load_Database(const char* file_name);

	// Looks up information about a specific address.
	GeoIPResult Lookup_IP(int address);

	// Looks up the geoip-result based on a short-country-code.
	GeoIPResult Lookup_Short_Country(const char* short_country_code);

};

#endif

