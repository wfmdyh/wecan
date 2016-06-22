#include "stdafx.h"
#include "OPCTag.h"
#include "WTOPCsvrAPI.h"

OPCTag::OPCTag()
{
	VariantInit(&m_varValue);
	m_wQuality = OPC_QUALITY_BAD;
	m_bIsWritable = TRUE;
	m_hTag = NULL;
}


OPCTag::~OPCTag()
{
	if (m_hTag != NULL)
	{
		RemoveTag(m_hTag);
		m_hTag = NULL;
	}
	VariantInit(&m_varValue);
	m_wQuality = OPC_QUALITY_BAD;
}
