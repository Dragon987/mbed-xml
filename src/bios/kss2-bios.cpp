#include "kss2-bios.h"

#include "common.h"

#define FILL_VALUE(parent, childName, dest, minVal, maxVal)                                   \
    {                                                                                         \
        int a = atoi(dxml_child(parent, childName)                                            \
                         ->txt);                                                              \
        if (!(a >= minVal && a <= maxVal))                                                    \
            printf("Invalid value for child %s of %s. It should've been between %d and %d\n", \
                   childName, parent->name, minVal, maxVal);                                  \
        else                                                                                  \
            dest = a;                                                                         \
    }

#define FILL_VALUE_2(parent, childNameNoStr, dest, minVal, maxVal)                            \
    {                                                                                         \
        int a = atoi(dxml_child(parent, #childNameNoStr)                                      \
                         ->txt);                                                              \
        if (!(a >= minVal && a <= maxVal))                                                    \
            printf("Invalid value for child %s of %s. It should've been between %d and %d\n", \
                   #childNameNoStr,                                                           \
                   parent->name, minVal, maxVal);                                             \
        else                                                                                  \
            dest.childNameNoStr = a;                                                          \
    }

namespace kss2_bios
{

auto load(const char *filename,
          SBios_Dinamic &bios_dyn,
          SRestart &restart,
          S_various_flegs &vf) -> int
{

    dxml_t file = dxml_open(filename);

    dxml_t kssBios = dxml_child(file, "KSS_BIOS");

    auto &sbd = bios_dyn.proc;

    FILL_VALUE(kssBios, "mod", sbd.mod, 0, 3);
    FILL_VALUE(kssBios, "cord_loc", sbd.koord_lok, 0, 1);
    FILL_VALUE(kssBios, "fors_table", sbd.fors_tab, 0, 1);
    FILL_VALUE(kssBios, "vh_fl", sbd.v_trep, 0, 1);
    FILL_VALUE(kssBios, "pd_fl", sbd.p_trep, 0, 1);
    FILL_VALUE(kssBios, "adaptive", sbd.detektor, 0, 1);
    FILL_VALUE(kssBios, "stages", sbd.najava, 0, 1);

    dxml_t knfMode = dxml_child(file, "KNF_MODE");

    // Now knf_mode is being loaded.
    /* It is very important that elements of c and xml 
     structures have same names */
    auto &km = bios_dyn.knfl.km;
    FILL_VALUE_2(knfMode, pg1, km, 0, 3);
    FILL_VALUE_2(knfMode, pg2, km, 0, 3);
    FILL_VALUE_2(knfMode, pg3, km, 0, 3);
    FILL_VALUE_2(knfMode, pg4, km, 0, 3);
    FILL_VALUE_2(knfMode, vpg1, km, 0, 1);
    FILL_VALUE_2(knfMode, vpg2, km, 0, 1);
    FILL_VALUE_2(knfMode, vpg3, km, 0, 1);
    FILL_VALUE_2(knfMode, vpg4, km, 0, 1);
    FILL_VALUE_2(knfMode, glm, km, 0, 3);
    FILL_VALUE_2(knfMode, glt, km, 0, 1);
    FILL_VALUE_2(knfMode, fl, km, 0, 1);

    dxml_t variosFlags = dxml_child(file, "VARIOUS_FLAGS");

    // for (dxml_t c = variosFlags->child; c; c = c->sibling)
    //     printf("%s\n", c->name);

    FILL_VALUE(variosFlags, "CD_demoONOFF", vf.CD_demoONOFF_flag, 0, 3);
    FILL_VALUE(variosFlags, "setUPDW", vf.setUPDW_flag, 0, 3);
    FILL_VALUE(variosFlags, "RTCold", vf.RTCold_flag, 0, 3);
    FILL_VALUE(variosFlags, "Logger_ONOFF", vf.Logger_ONOFF, 0, 3);
    FILL_VALUE(variosFlags, "reserved1", vf.reserved1, 0, 3);
    FILL_VALUE(variosFlags, "reserved4", vf.reserved4, 0, 3);
    FILL_VALUE(variosFlags, "reserved2", vf.reserved2, 0, 3);
    FILL_VALUE(variosFlags, "reserved3", vf.reserved3, 0, 3);

    dxml_t restartd = dxml_child(file, "RESART");

    //Next two lines should be checked
    FILL_VALUE(restartd, "nrof_restarts", restart.no, 0, 3);

    FILL_VALUE(restartd, "wait_for_restart", restart.time, 0, 63);

    dxml_free(file);
    return 0;
}

auto num_to_str(const int num, const size_t size) -> char *
{
    auto buffer = new char[size + 1];
    memset(buffer, 0, size + 1);
    snprintf(buffer, size + 1, "%d", num);
    return buffer;
}

#define DXML_SET_TXT(a, b)  \
    {                       \
        auto d = a;         \
        auto e = b;         \
        dxml_set_txt(d, e); \
    }

auto save(const char *filename,
          const SBios_Dinamic &bios_dyn,
          const SRestart &restart,
          const S_various_flegs &vf) -> int
{
    auto root = dxml_new("KSS_BIOS_DATA");
    create_children(root, "KSS_BIOS", "KNF_MODE", "VARIOUS_FLAGS", "RESART");

    auto kss_bios = dxml_child(root, "KSS_BIOS");
    auto knf_mode = dxml_child(root, "KNF_MODE");
    auto var_flags = dxml_child(root, "VARIOUS_FLAGS");
    auto restart_tag = dxml_child(root, "RESTART");

    // filling up kss_bios
    create_children(kss_bios, "mod", "cord_loc", "fors_table", "vh_fl", "pd_fl", "adaptive", "stages");
    const auto &sbd = bios_dyn.proc;
    DXML_SET_TXT(dxml_child(kss_bios, "mod"), num_to_str(sbd.mod, 1));
    DXML_SET_TXT(dxml_child(kss_bios, "cord_loc"), num_to_str(sbd.koord_lok, 1));
    DXML_SET_TXT(dxml_child(kss_bios, "fors_table"), num_to_str(sbd.fors_tab, 1));
    DXML_SET_TXT(dxml_child(kss_bios, "vh_fl"), num_to_str(sbd.v_trep, 1));
    DXML_SET_TXT(dxml_child(kss_bios, "pd_fl"), num_to_str(sbd.p_trep, 1));
    DXML_SET_TXT(dxml_child(kss_bios, "adaptive"), num_to_str(sbd.detektor, 1));
    DXML_SET_TXT(dxml_child(kss_bios, "stages"), num_to_str(sbd.najava, 1));

    // filling up knf_mode
    create_children(knf_mode, "pg1", "pg2", "pg3", "pg4",
                    "vpg1", "vpg2", "vpg3", "vpg4",
                    "glm", "glt", "fl");
    const auto &knf = bios_dyn.knfl.km;
    DXML_SET_TXT(dxml_child(knf_mode, "pg1"), num_to_str(knf.pg1, 1));
    DXML_SET_TXT(dxml_child(knf_mode, "pg2"), num_to_str(knf.pg2, 1));
    DXML_SET_TXT(dxml_child(knf_mode, "pg3"), num_to_str(knf.pg3, 1));
    DXML_SET_TXT(dxml_child(knf_mode, "pg4"), num_to_str(knf.pg4, 1));
    DXML_SET_TXT(dxml_child(knf_mode, "vpg1"), num_to_str(knf.vpg1, 1));
    DXML_SET_TXT(dxml_child(knf_mode, "vpg2"), num_to_str(knf.vpg2, 1));
    DXML_SET_TXT(dxml_child(knf_mode, "vpg3"), num_to_str(knf.vpg3, 1));
    DXML_SET_TXT(dxml_child(knf_mode, "vpg4"), num_to_str(knf.vpg4, 1));
    DXML_SET_TXT(dxml_child(knf_mode, "glm"), num_to_str(knf.glm, 1));
    DXML_SET_TXT(dxml_child(knf_mode, "glt"), num_to_str(knf.glt, 1));
    DXML_SET_TXT(dxml_child(knf_mode, "fl"), num_to_str(knf.fl, 1));

    // filling up var_flags
    create_children(var_flags, "CD_demoONOFF", "setUPDW",
                    "RTCold", "Logger_ONOFF",
                    "reserved1", "reserved2",
                    "reserved3", "reserved4");
    DXML_SET_TXT(dxml_child(var_flags, "CD_demoONOFF"), num_to_str(vf.CD_demoONOFF_flag, 1));
    DXML_SET_TXT(dxml_child(var_flags, "setUPDW"), num_to_str(vf.setUPDW_flag, 1));
    DXML_SET_TXT(dxml_child(var_flags, "RTCold"), num_to_str(vf.RTCold_flag, 1));
    DXML_SET_TXT(dxml_child(var_flags, "Logger_ONOFF"), num_to_str(vf.Logger_ONOFF, 1));
    DXML_SET_TXT(dxml_child(var_flags, "reserved1"), num_to_str(vf.reserved1, 1));
    DXML_SET_TXT(dxml_child(var_flags, "reserved2"), num_to_str(vf.reserved2, 1));
    DXML_SET_TXT(dxml_child(var_flags, "reserved3"), num_to_str(vf.reserved3, 1));
    DXML_SET_TXT(dxml_child(var_flags, "reserved4"), num_to_str(vf.reserved4, 1));

    // filling up restart
    create_children(restart_tag, "nrof_restarts", "wait_for_restart");
    DXML_SET_TXT(dxml_child(restart_tag, "nrof_restarts"), num_to_str(restart.no, 1));
    DXML_SET_TXT(dxml_child(restart_tag, "wait_for_restart"), num_to_str(restart.time, 2));

    auto buffer = dxml_toxml(root);
    auto fp = fopen(filename, "w");
    fwrite(buffer, sizeof(char), strlen(buffer), fp);
    fclose(fp);
    delete[] buffer;

    return 0;
}
} // namespace kss2_bios