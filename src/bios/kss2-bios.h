#pragma once

namespace kss2_bios
{
using uchar = unsigned char;
using i8 = char;

struct S_konflmode
{
    uchar pg1 : 2; /*  modovi podgrupa:  B_MOD_TEST 0   B_MOD_ZT  1	B_MOD_SC  2	 B_MOD_A  3 */
    uchar pg2 : 2;
    uchar pg3 : 2;
    uchar pg4 : 2;

    uchar vpg1 : 1;
    uchar vpg2 : 1;
    uchar vpg3 : 1;
    uchar vpg4 : 1;

    uchar glm : 2; /*   glavni mod:  B_MOD_TEST 0   B_MOD_ZT  1	B_MOD_SC  2	 B_MOD_A  3	*/
    uchar glt : 1;
    uchar fl : 1;
};

union Ukonflmod {
    struct S_konflmode km;
    uchar bytes[sizeof(struct S_konflmode)];
};

struct Sbd
{
    uchar mod : 2; /*  B_MOD_TEST 0   B_MOD_ZT  1	B_MOD_SC  2	 B_MOD_A  3	*/
    uchar koord_lok : 1;
    uchar fors_tab : 1;
    uchar v_trep : 1;
    uchar p_trep : 1;
    uchar detektor : 1;
    uchar najava : 1;
};

struct SBios_Dinamic
{
    struct Sbd proc;
    union Ukonflmod knfl;
};

struct SRestart
{
    uchar no : 2;
    uchar time : 6;
    uchar temp_no;
    uchar refresh;
};

struct S_various_flegs
{
    i8 CD_demoONOFF_flag : 2; /* demo CD */
    i8 setUPDW_flag : 2;      /* odlazak u TEST tokom parametrizacije  */
    i8 RTCold_flag : 2;       /* RTC koordinacija po starom */
    i8 Logger_ONOFF : 2;      /* Logger_ONOFF */
    i8 reserved1 : 2;         /* rezerva 1 */
    i8 reserved2 : 2;         /* rezerva 2 */
    i8 reserved3 : 2;         /* rezerva 3 */
    i8 reserved4 : 2;         /* rezerva 4 */
};

auto load(const char *filename,
          SBios_Dinamic &bios_dyn,
          SRestart &restart,
          S_various_flegs &vf) -> int;

auto save(const char *filename,
          const SBios_Dinamic &bios_dyn,
          const SRestart &restart,
          const S_various_flegs &vf) -> int;

} // namespace kss2_bios