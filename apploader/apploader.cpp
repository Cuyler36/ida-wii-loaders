/*
*  IDA Nintendo GameCube Apploader Loader Module
*  (C) Copyright 2018 Jeremy Olsen
*
*/

#include "apploader.h"
#include "apploader_track.h"


/*-----------------------------------------------------------------
*
*   Check if input file can be an apploader file. The supposed header
*   is checked for sanity. If so return and fill in the formatname
*   otherwise return 0
*
*/

int idaapi accept_file(qstring *fileFormatName, qstring *processor, linput_t *li, const char *filename)
{
    apploader_track test_valid(li);

    // Check if valid
    if (!test_valid.is_good())
        return 0;

    // file has passed all sanity checks and might be a rel
    fileFormatName->sprnt("Nintendo GameCube Apploader");
    processor->sprnt("PPC");

    return(ACCEPT_FIRST | 0xD07);
}



/*-----------------------------------------------------------------
*
*   File was recognised as an apploader and user has selected it.
*   Now load it into the database
*
*/

void idaapi load_file(linput_t *fp, ushort neflag, const char * /*fileformatname*/)
{
    msg("---------------------------------------\n");
    msg("Nintendo Apploader Loader Plugin 0.1\n");
    msg("---------------------------------------\n");

    // We need PowerPC support to do anything with rels
    set_processor_type("ppc:PAIRED", setproc_level_t::SETPROC_LOADER);

    // Set lis+addi resolution to aggressive
    int lisres = 1;
    ph.set_idp_options("PPC_LISOFF", IDPOPT_BIT, &lisres);

    set_compiler_id(COMP_GNU);

    apploader_track apploader = apploader_track(fp);
    inf.start_ea = 0x81200000;

    // map selector 1 to 0
    set_selector(1, 0);

    // Create the sections
    auto header = apploader.header;
    auto name = std::string(NAME_CODE) + std::string("_boot");
    if (!add_segm(1, inf.start_ea, inf.start_ea + header.size, name.c_str(), CLASS_CODE)) {
        err_msg("Failed to create entry segment!");
        return;
    }

    if (!file2base(fp, sizeof(apploader_header), inf.start_ea, inf.start_ea + header.size, FILEREG_PATCHABLE)) {
        err_msg("Failed to load entry segment!");
        return;
    }

    name = std::string(NAME_CODE) + std::string("_trailer");
    if (!add_segm(1, inf.start_ea + header.size, inf.start_ea + header.size + header.trailerSize, name.c_str(), CLASS_CODE)) {
        err_msg("Failed to create data segment!");
        return;
    }

    if (!file2base(fp, sizeof(apploader_header) + header.size, inf.start_ea + header.size,
        inf.start_ea + header.size + header.trailerSize, FILEREG_PATCHABLE)) {
        err_msg("Failed to load data segment!");
        return;
    }

    // Set entrypoint function name
    set_name(header.entryPoint, "entrypoint");
    add_func(header.entryPoint);
}

/*-----------------------------------------------------------------
*
*   Loader Module Descriptor Blocks
*
*/

extern "C" loader_t LDSC = {
  IDP_INTERFACE_VERSION,
  0, /* no loader flags */
  accept_file,
  load_file,
  NULL,
};
