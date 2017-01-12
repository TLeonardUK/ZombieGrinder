// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_LINUX

/*
#include "Engine/Platform/Linux/Linux_DwarfUtils.h"

#include <sys/cdefs.h>
#include <sys/param.h>
#include <dwarf.h>
#include <err.h>
#include <fcntl.h>
#include <gelf.h>
#include <getopt.h>
#include <libdwarf.h>
#include <libelftc.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <cxxabi.h>

extern char* g_executable_file_path;

// Mangled to fuck from:
// http://sourceforge.net/apps/trac/elftoolchain/browser/trunk/addr2line/addr2line.c

void dwarf_searchfunc(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Addr addr, const char **rlt_func)
{
	Dwarf_Die ret_die, spec_die;
	Dwarf_Error de;
	Dwarf_Half tag;
	Dwarf_Unsigned lopc, hipc;
	Dwarf_Off ref;
	Dwarf_Attribute sub_at, spec_at;
	char *func0;
	int ret;

	if (*rlt_func != NULL)
		return;

	if (dwarf_tag(die, &tag, &de)) {
		goto cont_search;
	}
	if (tag == DW_TAG_subprogram) {
		if (dwarf_attrval_unsigned(die, DW_AT_low_pc, &lopc, &de) ||
		    dwarf_attrval_unsigned(die, DW_AT_high_pc, &hipc, &de))
			goto cont_search;
		if (addr < lopc || addr >= hipc)
			goto cont_search;

		// Found it! 

		*rlt_func = unknown;
		ret = dwarf_attr(die, DW_AT_name, &sub_at, &de);
		if (ret == DW_DLV_ERROR)
			return;
		if (ret == DW_DLV_OK) {
			if (dwarf_formstring(sub_at, &func0, &de))
				*rlt_func = unknown;
			else
				*rlt_func = func0;
			return;
		}

		//
		// If DW_AT_name is not present, but DW_AT_specification is
		// present, then probably the actual name is in the DIE
		// referenced by DW_AT_specification.
		//
		if (dwarf_attr(die, DW_AT_specification, &spec_at, &de))
			return;
		if (dwarf_global_formref(spec_at, &ref, &de))
			return;
		if (dwarf_offdie(dbg, ref, &spec_die, &de))
			return;
		if (dwarf_attrval_string(spec_die, DW_AT_name, rlt_func, &de))
			*rlt_func = unknown;

		return;
	}

cont_search:

	// Search children. 
	ret = dwarf_child(die, &ret_die, &de);
	if (ret == DW_DLV_ERROR)
		return;
	else if (ret == DW_DLV_OK)
		dwarf_searchfunc(dbg, ret_die, addr, rlt_func);

	// Search sibling. 
	ret = dwarf_siblingof(dbg, die, &ret_die, &de);
	if (ret == DW_DLV_ERROR)
		return;
	else if (ret == DW_DLV_OK)
		dwarf_searchfunc(dbg, ret_die, addr, rlt_func);
}

void dwarf_translate(Dwarf_Debug dbg, int addr_original, DwarfInfo* info)
{
	Dwarf_Die die;
	Dwarf_Line *lbuf;
	Dwarf_Error de;
	Dwarf_Half tag;
	Dwarf_Unsigned lopc, hipc, addr, lineno, plineno;
	Dwarf_Signed lcount;
	Dwarf_Addr lineaddr, plineaddr;
	const char *funcname;
	char *file, *file0, *pfile;
	int i, ret;

	addr = (Dwarf_Unsigned)addr_original;
	lineno = 0;
	file = "<unknown>";

	while ((ret = dwarf_next_cu_header(dbg, NULL, NULL, NULL, NULL, NULL,
	    &de)) ==  DW_DLV_OK) {
		die = NULL;
		while (dwarf_siblingof(dbg, die, &die, &de) == DW_DLV_OK) {
			if (dwarf_tag(die, &tag, &de) != DW_DLV_OK) {
				goto out;
			}
			// XXX: What about DW_TAG_partial_unit? 
			if (tag == DW_TAG_compile_unit)
				break;
		}
		if (die == NULL) {
			goto out;
		}
		if (!dwarf_attrval_unsigned(die, DW_AT_low_pc, &lopc, &de) &&
		    !dwarf_attrval_unsigned(die, DW_AT_high_pc, &hipc, &de)) {
			//
			// Check if the address falls into the PC range of
			// this CU.
			//
			if (addr < lopc || addr >= hipc)
				continue;
		}

		if (dwarf_srclines(die, &lbuf, &lcount, &de) != DW_DLV_OK) {
			goto out;
		}

		plineaddr = ~0ULL;
		plineno = 0;
		pfile = "<unknown>";
		for (i = 0; i < lcount; i++) {
			if (dwarf_lineaddr(lbuf[i], &lineaddr, &de)) {
				goto out;
			}
			if (dwarf_lineno(lbuf[i], &lineno, &de)) {
				goto out;
			}
			if (dwarf_linesrc(lbuf[i], &file0, &de)) {
			} else
				file = file0;
			if (addr == lineaddr)
				goto out;
			else if (addr < lineaddr && addr > plineaddr) {
				lineno = plineno;
				file = pfile;
				goto out;
			}
			plineaddr = lineaddr;
			plineno = lineno;
			pfile = file;
		}
	}

out:
	funcname = NULL;
	if (ret == DW_DLV_OK)
		dwarf_searchfunc(dbg, die, addr, &funcname);
	
	strcpy(info->File, file);
	info->Line = lineno;

	if (funcname == NULL)
		strcpy(info->Function, "<unknown>");
	if (demangle)
	{
		int demangle_status = 0;
		size_t size = 0;
		char* demangled = abi::_cxa_demangle(funcname NULL, &size, &demangle_status);

		if (demangled != NULL && demangle_status == 0)
		{			
			strncpy(info->Function, demangled, size);
		}
		else
		{
			strcpy(info->Function, funcname);
		}
	}
	else
		strcpy(info->Function, funcname);
}

bool dwarf_addr2line(int addr, DwarfInfo* info)
{
	if ((fd = open(g_executable_file_path, O_RDONLY)) < 0)
	{
		DBG_LOG("dwarf_addr2line: Failed to open executable file.");
		return false;
	}
	
	if (dwarf_init(fd, DW_DLC_READ, NULL, NULL, &dbg, &de))
	{
		DBG_LOG("dwarf_addr2line: dwarf_init failed.");
		return false;
	}
	
	if (dwarf_get_elf(dbg, &e, &de) != DW_DLV_OK)
	{
		DBG_LOG("dwarf_addr2line: dwarf_get_elf failed.");
		return false;
	}
	
	dwarf_translate(dbg, addr, info);

	dwarf_finish(dbg, &de);

	return true;
}

*/
#endif