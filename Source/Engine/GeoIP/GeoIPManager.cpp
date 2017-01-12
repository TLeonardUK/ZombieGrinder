// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/GeoIP/GeoIPManager.h"

GeoIPManager::GeoIPManager()
	: m_database(NULL)
{
}

GeoIPManager::~GeoIPManager()
{
	if (m_database != NULL)
	{
		GeoIP_delete(m_database);
		m_database = NULL;
	}
}

bool GeoIPManager::Load_Database(const char* file_name)
{	
	if (m_database != NULL)
	{
		GeoIP_delete(m_database);
		m_database = NULL;
	}

	DBG_LOG("Loading GeoIP database from: %s", file_name);

	m_database = GeoIP_open(file_name, GEOIP_STANDARD);
	if (m_database == NULL)
	{
		return false;
	}

	return true;
}

GeoIPResult GeoIPManager::Lookup_IP(int address)
{
	GeoIPResult result;

	// Already in cache?
	if (m_cache.Get(address, result))
	{
		return result;
	}

	// Grab it!
	result.CountryID		= GeoIP_id_by_ipnum(m_database, address);
	result.CountryShortName	= GeoIP_country_code[result.CountryID];
	result.CountryName		= GeoIP_utf8_country_name[result.CountryID];

	// Into cache you go!
	m_cache.Set(address, result);

	return result;
}

GeoIPResult GeoIPManager::Lookup_Short_Country(const char* short_country_code)
{
	int country_code = 0;

	for (int i = 0; i < 255; i++)
	{
		if (stricmp(GeoIP_country_code[i], short_country_code) == 0)
		{
			country_code = i;
			break;
		}
	}

	GeoIPResult result;
	result.CountryID		= country_code;
	result.CountryShortName	= GeoIP_country_code[result.CountryID];
	result.CountryName		= GeoIP_utf8_country_name[result.CountryID];

	return result;
}
